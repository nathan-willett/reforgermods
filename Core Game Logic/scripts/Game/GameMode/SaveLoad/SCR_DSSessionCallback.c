/*!
Callback for easy handling of world saving and loading.
Controlled from SCR_SaveLoadComponent.
*/
[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(ESaveType, "m_eType")]
class SCR_DSSessionCallback: DSSessionCallback
{
	[Attribute(ESaveType.USER.ToString(), UIWidgets.ComboBox, "Save file type.", enums: ParamEnumArray.FromEnum(ESaveType))]
	protected ESaveType m_eType;
	
	[Attribute(desc: "Unique extension added in front of .json extension.\nUsed for identifying save types without opening files.\n\nMust contain only letters!")]
	protected string m_sExtension;
	
	[Attribute("-", desc: "Character added between mission name and custom name.\nWhen empty, custom name will not be used.")]
	protected string m_sCustomNameDelimiter;
	
	[Attribute(desc: "When enabled, save file will never be saved in cloud.")]
	protected bool m_bAlwaysLocal;
	
	[Attribute(desc: "When enabled, save of this type will also become the latest save for given mission.")]
	protected bool m_bRegisterLatestSave;
	
	[Attribute()]
	protected ref SCR_UIInfo m_Info;
	
	protected SCR_MissionStruct m_Struct;
	protected bool m_bLoadPreview;
	protected string m_sCurrentFileName;
	
	//////////////////////////////////////////////////////////////////////////////////////////
	// Public
	//////////////////////////////////////////////////////////////////////////////////////////
	
	/*!
	\return Save type of this callback
	*/
	ESaveType GetSaveType()
	{
		return m_eType;
	}
	
	//----------------------------------------------------------------------------------------
	/*!
	\return UI info representing this save type
	*/
	SCR_UIInfo GetInfo()
	{
		return m_Info;
	}
	
	//----------------------------------------------------------------------------------------
	string GetExtension()
	{
		return m_sExtension;
	}
	
	//----------------------------------------------------------------------------------------
	/*!
	Save current session to a file.
	\param fileName Mission save file name
	\param customName Custom addition to file name (optional; applicable only to some save types)
	\return True if saving operation was requested
	*/
	bool SaveSession(string fileName, WorldSaveManifest manifest = null, WorldSaveItem usedSave = null)
	{
		if (!IsCompatible(fileName))
			return false;
		
		SessionStorage storage = GetGame().GetBackendApi().GetStorage();
		storage.AssignFileIDCallback(fileName, this);
		
		// Handle save 
		if (!m_bAlwaysLocal && GetGame().GetSaveManager().CanSaveToCloud())
		{
			Print(string.Format("SCR_DSSessionCallback: RequestSave: %1", fileName), LogLevel.VERBOSE);
			storage.RequestSave(fileName);
		}
		else
		{
			Print(string.Format("SCR_DSSessionCallback: LocalSave: %1", fileName), LogLevel.VERBOSE);
			storage.LocalSave(fileName);
			
			// Create local world save 
			// Store user (manual) save as editing file
			if (m_eType == ESaveType.USER)
			{
				if (manifest)
				{
					manifest.m_aFileNames = {fileName};
					
					if (usedSave)
						usedSave.Save(manifest);
					else
						usedSave = WorldSaveItem.CreateLocalWorldSave(manifest);
					
					SCR_SaveWorkshopManager.GetInstance().SetCurrentSave(fileName, usedSave);
				}
			}
		}
		
		return true;
	}
	
	//----------------------------------------------------------------------------------------
	/*!
	Load session from given save file.
	\param fileName Full save file name
	\return True if loading operation was requested
	*/
	bool LoadSession(string fileName)
	{
		if (!IsCompatible(fileName))
			return false;
		
		m_bLoadPreview = false;
		SessionStorage storage = GetGame().GetBackendApi().GetStorage();
		if (!storage.CheckFileID(fileName))
		{
			Print(string.Format("SCR_DSSessionCallback: Cannot load save file '%1', it does not exist!", fileName), LogLevel.WARNING);
			return false;
		}
		
		storage.AssignFileIDCallback(fileName, this);
		
		if (!m_bAlwaysLocal && GetGame().GetSaveManager().CanSaveToCloud())
		{
			Print(string.Format("SCR_DSSessionCallback: RequestLoad: %1", fileName), LogLevel.VERBOSE);
			storage.RequestLoad(fileName);
		}
		else
		{
			Print(string.Format("SCR_DSSessionCallback: LocalLoad: %1", fileName), LogLevel.VERBOSE);
			storage.LocalLoad(fileName);
		}
		return true;
	}
	
	//----------------------------------------------------------------------------------------
	/*!
	Delete a save file.
	\param fileName Mission save file name
	\param customName Custom addition to file name (optional; applicable only to some save types)
	\return True if the file was deleted
	*/
	bool Delete(string missionFileName, string customName)
	{
		string fileName = GetFileName(missionFileName, customName);
		return IsCompatible(fileName) && Delete(fileName);
	}
	
	//----------------------------------------------------------------------------------------
	/*!
	Delete a save file.
	\param fileName Full save file name
	\return True if the file was deleted
	*/
	bool Delete(string fileName)
	{
		if (!GetGame().GetBackendApi().GetStorage().CheckFileID(fileName))
			return false;
		
		GetGame().GetSaveManager().GetOnDeleted().Invoke(m_eType, fileName); //--- Call before the file is actually deleted
		GetGame().GetBackendApi().GetStorage().LocalDelete(fileName);
		SCR_SaveWorkshopManager.GetInstance().DeleteOfflineSaveByName(fileName);
		Print(string.Format("SCR_DSSessionCallback: LocalDelete: '%1'", fileName), LogLevel.VERBOSE);
		return true;
	}
	
	//----------------------------------------------------------------------------------------
	/*!
	Check if file of given type exists.
	\param fileName Mission save file name
	\param customName Custom addition to file name (optional; applicable only to some save types)
	\return True if the file exists
	*/
	bool FileExists(string fileName, string customName = string.Empty)
	{
		return GetGame().GetBackendApi().GetStorage().CheckFileID(GetFileName(fileName, customName));
	}
	
	//----------------------------------------------------------------------------------------
	/*!
	Open file name and read its meta header.
	\param Save file name
	\return Meta header
	*/
	SCR_MetaStruct GetMeta(string fileName)
	{
		if (!IsCompatible(fileName))
			return null;
		
		m_Struct.ClearCache();
		
		m_bLoadPreview = true;
		SessionStorage storage = GetGame().GetBackendApi().GetStorage();
		storage.AssignFileIDCallback(fileName, this);
		storage.LocalLoad(fileName);

		return m_Struct.GetMeta();
	}
	
	//----------------------------------------------------------------------------------------
	/*!
	Set JSON struct that defines what will be saved.
	\param struct Save struct
	*/
	void SetStruct(SCR_MissionStruct struct)
	{
		m_Struct = struct;
	}
	
	//----------------------------------------------------------------------------------------
	/*!
	\return JSON struct that defines what will be saved.
	*/
	SCR_MissionStruct GetStruct()
	{
		return m_Struct;
	}
	
	//----------------------------------------------------------------------------------------
	/*!
	Print out JSON struct that is currently kept in the memory.
	*/
	void Log()
	{
		m_Struct.Log();
	}
	
	//----------------------------------------------------------------------------------------
	/*!
	\return True if this callback is configured correctly
	*/
	bool IsConfigured()
	{
		return m_Struct != null;
	}
	
	//----------------------------------------------------------------------------------------
	/*!
	Check if a save file name is compatible with this callback, and can be load using its settings.
	\param fileName Save file name
	\return True if compatible
	*/
	bool IsCompatible(string fileName)
	{
		if (!m_Struct)
			return false;
		
		//--- Cannot just use EndsWith(), because downloaded files have a GUID added at the end, e.g., "MissionName-CustomName.save_5D82C234B9132BBC"
		string ext;
		FilePath.StripExtension(fileName, ext);
		bool compatible = ext.StartsWith(m_sExtension);
		
		// Check for saves from workshop 
		if (!compatible)
			compatible = fileName.Contains(string.Format(".%1_", m_sExtension));

		return compatible;
	}
	
	//----------------------------------------------------------------------------------------
	/*!
	Extract mission file name from save file name.
	\param fileName Save file name
	\return Mission file name
	*/
	string GetMissionFileName(string fileName)
	{
		if (!m_sCustomNameDelimiter)
		{
			return FilePath.StripExtension(fileName);
		}
		
		int delimiterIndex = fileName.IndexOf(m_sCustomNameDelimiter);
		if (delimiterIndex >= 0)
			return fileName.Substring(0, delimiterIndex);
		else
			return string.Empty;
	}
	
	//----------------------------------------------------------------------------------------
	/*!
	Compose save file name.
	\param fileName Mission save file name
	\param customName Custom addition to file name (optional; applicable only to some save types)
	\return Full save file name
	*/
	string GetCustomName(string fileName)
	{
		if (!m_sCustomNameDelimiter)
			return string.Empty;
		
		int delimiterIndex = fileName.IndexOf(m_sCustomNameDelimiter);
		if (delimiterIndex < 0)
			return string.Empty;
		
		delimiterIndex += m_sCustomNameDelimiter.Length();
		int length = fileName.Length() - delimiterIndex;
		fileName = fileName.Substring(delimiterIndex, length);
		
		string ext;
		return FilePath.StripExtension(fileName, ext);
	}
	
	//----------------------------------------------------------------------------------------
	/*!
	\return Custom name of the previously saved or loaded file.
	*/
	string GetCurrentCustomName()
	{
		return GetCustomName(m_sCurrentFileName);
	}
	
	//----------------------------------------------------------------------------------------
	/*!
	Extract custom name from save file name.
	\param fileName Save file name
	\return Custom name
	*/
	string GetFileName(string missionFileName, string customName)
	{
		if (m_sCustomNameDelimiter)
		{
			customName = SCR_StringHelper.Filter(customName, SCR_StringHelper.LETTERS + SCR_StringHelper.DIGITS + "_");		
			missionFileName += m_sCustomNameDelimiter + customName;
		}
		
		return FilePath.AppendExtension(missionFileName, m_sExtension);
	}
	
	//----------------------------------------------------------------------------------------
	void OnGameStart(string missionFileName)
	{
	}
	
	//----------------------------------------------------------------------------------------
	void OnGameEnd(string missionFileName)
	{
		m_sCurrentFileName = string.Empty;
		m_Struct = null;
	}
	
	//----------------------------------------------------------------------------------------
	protected void OnLatestSave(string fileName)
	{
		//--- Call invoker with a delay, because the storage is still blocked in this frame, and whatever listens may also need the storage
		GetGame().GetSaveManager().GetOnLatestSave().Remove(OnLatestSave);
		GetGame().GetCallqueue().CallLater(InvokeOnSaved);
	}
	
	//----------------------------------------------------------------------------------------
	protected void InvokeOnSaved()
	{
		GetGame().GetSaveManager().GetOnSaved().Invoke(m_eType, m_sCurrentFileName);
		SCR_NotificationsComponent.SendLocal(ENotification.EDITOR_SESSION_SAVE_SUCCESS);
	}
	
	//----------------------------------------------------------------------------------------
	protected void InvokeOnSaveFailed()
	{
		GetGame().GetSaveManager().GetOnSaveFailed().Invoke(m_eType, m_sCurrentFileName);
		SCR_NotificationsComponent.SendLocal(ENotification.EDITOR_SESSION_SAVE_FAIL);
	}
	
	//////////////////////////////////////////////////////////////////////////////////////////
	// Override
	//////////////////////////////////////////////////////////////////////////////////////////
	override void OnSaving(string fileName)
	{
		if (m_Struct.Serialize())
			GetGame().GetBackendApi().GetStorage().ProcessSave(m_Struct, fileName);
	}
	
	//----------------------------------------------------------------------------------------
	override void OnSaveSuccess(string fileName)
	{
		m_sCurrentFileName = fileName;
		
		if (m_bRegisterLatestSave)
		{
			//--- Set the save file as the latest save (with a delay; this callback is still used by storage)
			GetGame().GetSaveManager().GetOnLatestSave().Insert(OnLatestSave);
			GetGame().GetCallqueue().CallLater(GetGame().GetSaveManager().SetCurrentMissionLatestSave, 0, false, fileName);
		}
		
		InvokeOnSaved();
		
		Print(string.Format("SCR_DSSessionCallback: Saving save file of type %1 in '%2' succeeded!", typename.EnumToString(ESaveType, m_eType), fileName), LogLevel.VERBOSE);
	}
	
	//----------------------------------------------------------------------------------------
	override void OnSaveFailed(string fileName)
	{
		InvokeOnSaveFailed();
		
		Print(string.Format("SCR_DSSessionCallback: Saving save file of type %1 in '%2' failed!", typename.EnumToString(ESaveType, m_eType), fileName), LogLevel.WARNING);
	}
	
	//----------------------------------------------------------------------------------------
	override void OnLoaded(string fileName)
	{
		m_Struct.ClearCache();
		GetGame().GetBackendApi().GetStorage().ProcessLoad(m_Struct, fileName);
		
		if (m_bLoadPreview)
		{
			Print(string.Format("SCR_DSSessionCallback: Previewing save file of type %1 from '%2' succeeded!", typename.EnumToString(ESaveType, m_eType), fileName), LogLevel.VERBOSE);
		}
		else
		{
			m_Struct.Deserialize();
			
			m_sCurrentFileName = fileName;
			
			GetGame().GetSaveManager().GetOnLoaded().Invoke(m_eType, fileName);
			
			Print(string.Format("SCR_DSSessionCallback: Loading save file of type %1 from '%2' succeeded!", typename.EnumToString(ESaveType, m_eType), fileName), LogLevel.VERBOSE);
		}
	}
	
	//----------------------------------------------------------------------------------------
	override void OnLoadFailed(string fileName)
	{
		Print(string.Format("SCR_DSSessionCallback: Loading save file of type %1 from '%2' failed!", typename.EnumToString(ESaveType, m_eType), fileName), LogLevel.WARNING);
	}
	
	//----------------------------------------------------------------------------------------
	override void OnDeleteSuccess( string fileName )
	{
		if (m_sCurrentFileName == fileName)
			m_sCurrentFileName = string.Empty;
		
		Print(string.Format("SCR_DSSessionCallback: Deleting save file of type %1 at '%2' succeeded!", typename.EnumToString(ESaveType, m_eType), fileName), LogLevel.VERBOSE);
	}
	
	//----------------------------------------------------------------------------------------
	override void OnDeleteFailed( string fileName )
	{
		Print(string.Format("SCR_DSSessionCallback: Deleting save file of type %1 at '%2' failed!", typename.EnumToString(ESaveType, m_eType), fileName), LogLevel.WARNING);
	}
	
	//----------------------------------------------------------------------------------------
	void SCR_DSSessionCallback()
	{
		//--- Only letters allowed in the extension - other characters could confuse save type recognition
		m_sExtension = SCR_StringHelper.Filter(m_sExtension, SCR_StringHelper.LETTERS);
	}
};

[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(ESaveType, "m_eType")]
class SCR_NumberedDSSessionCallback: SCR_DSSessionCallback
{
	[Attribute("1", UIWidgets.Slider, "", params: "1 10 1")]
	protected int m_iMaxSaves;
	
	//----------------------------------------------------------------------------------------
	override protected string GetFileName(string missionFileName, string customName)
	{
		if (m_iMaxSaves > 1)
		{
			int saveId;
			
			string latestSaveName;
			if (GetGame().GetSaveManager().FindCurrentMissionLatestSave(latestSaveName))
				saveId = GetCustomName(latestSaveName).ToInt();
			
			saveId = (saveId % m_iMaxSaves) + 1;
			customName = saveId.ToString();
		}
		else
		{
			customName = "1";
		}
		return super.GetFileName(missionFileName, customName);
	}
};

[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(ESaveType, "m_eType")]
class SCR_DisposableDSSessionCallback: SCR_DSSessionCallback
{
	//----------------------------------------------------------------------------------------
	protected void DeleteDelayed(string fileName)
	{
		Delete(fileName);
	}
	protected void DeleteIfNotToLoad(string missionFileName)
	{
		//--- Delete the file only when it's not marked for loading
		string fileNameToLoad;
		if (!GetGame().GetSaveManager().FindFileNameToLoad(fileNameToLoad) || !IsCompatible(fileNameToLoad))
			Delete(missionFileName, string.Empty);
	}
	
	//----------------------------------------------------------------------------------------
	override void OnLoaded(string fileName)
	{
		super.OnLoaded(fileName);
		
		//--- Call with delay, won't delete the file when it's still locked after loading
		GetGame().GetCallqueue().CallLater(DeleteDelayed, 1, false, fileName);
	}
	//----------------------------------------------------------------------------------------
	override void OnGameStart(string missionFileName)
	{
		DeleteIfNotToLoad(missionFileName);
		super.OnGameStart(missionFileName);
	}
	
	//----------------------------------------------------------------------------------------
	override void OnGameEnd(string missionFileName)
	{
		DeleteIfNotToLoad(missionFileName);
		super.OnGameEnd(missionFileName);
	}
};