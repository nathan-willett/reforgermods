void ScriptInvoker_SaveManagerCoreMethod(ESaveType type, string fileName);
typedef func ScriptInvoker_SaveManagerCoreMethod;
typedef ScriptInvokerBase<ScriptInvoker_SaveManagerCoreMethod> ScriptInvoker_SaveManagerCore;

/*!
Manager of external session save files.
*/
[BaseContainerProps(configRoot: true)]
class SCR_SaveManagerCore: SCR_GameCoreBase
{
	protected const string ITEM_SAVE_POSTFIX = ".save_";
	
	protected const string GAME_SESSION_STORAGE_FILE_NAME_TO_LOAD = "SCR_SaveFileManager_FileNameToLoad";
	protected const string GAME_SESSION_STORAGE_USED_CLI = "SCR_SaveFileManager_UsedCLI";
	protected const string CLI_PARAM = "loadSessionSave";
	
	[Attribute()]
	protected ref array<ref SCR_DSSessionCallback> m_aCallbacks;
	
	[Attribute()]
	protected ref SCR_LatestSaveDSSessionCallback m_LatestSaveCallback;
	
	protected ref SCR_DSSessionCallbackSessionStorage m_SessionStorageCallback;
	
	protected string m_sMissionSaveFileName;
	protected ref SCR_MissionHeader m_WorkbenchMissionHeader;
	
	protected ref ScriptInvoker_SaveManagerCore m_OnSaved = new ScriptInvoker_SaveManagerCore();
	protected ref ScriptInvoker_SaveManagerCore m_OnSaveFailed = new ScriptInvoker_SaveManagerCore();
	protected ref ScriptInvoker_SaveManagerCore m_OnLoaded = new ScriptInvoker_SaveManagerCore();
	protected ref ScriptInvoker_SaveManagerCore m_OnDeleted = new ScriptInvoker_SaveManagerCore();
	protected ref ScriptInvokerString m_OnLatestSave = new ScriptInvokerString();
	
	protected bool m_bLoadedOnInit;
	protected bool m_bDebugDelete;
	protected ref SCR_ServerSaveRequestCallback m_UploadCallback;
	
	//////////////////////////////////////////////////////////////////////////////////////////
	///@name Actions
	///@{
	//////////////////////////////////////////////////////////////////////////////////////////
	
	/*!
	Create a save of given type.
	\param type Save type
	\param customName Custom addition to file name (optional; applicable only to some save types)
	\return True if save request was initiated
	*/
	bool Save(ESaveType type, string customName = string.Empty, WorldSaveManifest manifest = null, WorldSaveItem usedSave = null)
	{
		SCR_DSSessionCallback callback = FindCallback(type);
		if (!callback)
		{
			Print(string.Format("SCR_SaveManagerCore: Cannot save, no rules found for save type %1! Check configuration of SCR_SaveLoadComponent on game mode.", typename.EnumToString(ESaveType, type)), LogLevel.WARNING);
			return false;
		}
		
		string fileName = callback.GetFileName(m_sMissionSaveFileName, customName);
		
		return callback.SaveSession(fileName, manifest, usedSave);
	}
	
	//----------------------------------------------------------------------------------------
	/*!
	Create the current save of given type.
	\param type Save type
	\return True if save request was initiated
	*/
	bool OverrideCurrentSave(ESaveType type)
	{
		SCR_DSSessionCallback callback = FindCallback(type);
		if (!callback)
		{
			Print(string.Format("SCR_SaveManagerCore: Cannot override current save, no rules found for save type %1! Check configuration of SCR_SaveLoadComponent on game mode.", typename.EnumToString(ESaveType, type)), LogLevel.WARNING);
			return false;
		}
		
		string customName = callback.GetCurrentCustomName();
		string fileName = callback.GetFileName(m_sMissionSaveFileName, customName);
		
		return customName && callback.SaveSession(fileName);
	}
	
	//----------------------------------------------------------------------------------------
	/*!
	Load the given save file.
	This will "insert" it straight to running session, which can lead to issues (especially when loading the save file multiple times).
	Consider restarting the world first.
	\param Save file name
	\return True if load request was initiated
	*/
	bool Load(string fileName)
	{
		SCR_DSSessionCallback callback = FindCallback(fileName);
		if (!callback)
		{
			Print(string.Format("SCR_SaveManagerCore: Cannot load save file '%1', no rules found for it!", fileName), LogLevel.WARNING);
			return false;
		}
		
		return callback.LoadSession(fileName);
	}
	
	//----------------------------------------------------------------------------------------
	/*!
	Restart the current world and load the latest save.
	\return True if load request was initiated
	*/
	bool RestartAndLoad()
	{
		string latestSaveFileName;
		return FindLatestSave(m_sMissionSaveFileName, latestSaveFileName) && RestartAndLoad(latestSaveFileName);
	}
	
	//----------------------------------------------------------------------------------------
	/*!
	Restart the current world and load save file of given type.
	\param type Save type
	\param customName Custom addition to file name (optional; applicable only to some save types)
	\return True if load request was initiated
	*/
	bool RestartAndLoad(ESaveType type, string customName = string.Empty)
	{
		SCR_DSSessionCallback callback = FindCallback(type);
		if (!callback)
		{
			Print(string.Format("SCR_SaveManagerCore: Cannot restart and load, no rules found for save type %1! Check configuration of SCR_SaveLoadComponent on game mode.", typename.EnumToString(ESaveType, type)), LogLevel.WARNING);
			return false;
		}
		
		string fileName = callback.GetFileName(m_sMissionSaveFileName, customName);
		return RestartAndLoad(fileName);
	}
	
	//----------------------------------------------------------------------------------------
	/*!
	Restart the current world and load given save file.
	\param Save file name
	\return True if load request was initiated
	*/
	bool RestartAndLoad(string fileName)
	{
		if (Replication.IsClient())
		{
			UploadToWorkshop(fileName);
			return true;
		}
		else
		{
			//--- Server / SP
			if (!SetFileNameToLoad(fileName))
				return false;
			
			GameStateTransitions.RequestScenarioRestart();
			return true;
		}
	}
	
	//----------------------------------------------------------------------------------------
	/*!
	Delete a save file.
	\param fileName Mission save file name
	\param customName Custom addition to file name (optional; applicable only to some save types)
	\return True if the file was deleted
	*/
	bool Delete(ESaveType type, string customName = string.Empty)
	{
		SCR_DSSessionCallback callback = FindCallback(type);
		if (!callback)
		{
			Print(string.Format("SCR_SaveManagerCore: Cannot delete, no rules found for save type %1! Check configuration of SCR_SaveLoadComponent on game mode.", typename.EnumToString(ESaveType, type)), LogLevel.WARNING);
			return false;
		}
		
		return callback.Delete(m_sMissionSaveFileName, customName);
	}
	
	//----------------------------------------------------------------------------------------
	/*!
	Delete a save file.
	\param fileName Full save file name
	\return True if the file was deleted
	*/
	bool Delete(string fileName)
	{
		SCR_DSSessionCallback callback = FindCallback(fileName);
		if (!callback)
		{
			Print(string.Format("SCR_SaveManagerCore: Cannot delete save file '%1', no rules found for it!", fileName), LogLevel.WARNING);
			return false;
		}
		
		return callback.Delete(fileName);
	}
	
	//----------------------------------------------------------------------------------------
	/*!
	Check if file of given type exists.
	\param type Save type
	\param customName Custom addition to file name (optional; applicable only to some save types)
	\return True if the file exists
	*/
	bool FileExists(ESaveType type, string customName = string.Empty)
	{
		SCR_DSSessionCallback callback = FindCallback(type);
		if (!callback)
		{
			Print(string.Format("SCR_SaveManagerCore: Cannot check if file exists, no rules found for save type %1! Check configuration of SCR_SaveLoadComponent on game mode.", typename.EnumToString(ESaveType, type)), LogLevel.WARNING);
			return false;
		}
		
		return callback.FileExists(m_sMissionSaveFileName, customName);
	}
	
	//----------------------------------------------------------------------------------------
	/*!
	Check if file exists.
	\param fileName Full save file name
	\return True if the file exists
	*/
	bool FileExists(string fileName)
	{
		return GetGame().GetBackendApi().GetStorage().CheckFileID(fileName);
	}
	
	//----------------------------------------------------------------------------------------
	/*!
	\return True if save files can be saved online.
	*/
	bool CanSaveToCloud()
	{
		return RplSession.Mode() == RplMode.Dedicated && GetGame().GetBackendApi().GetStorage().GetOnlineWritePrivilege();
	}
	
	//----------------------------------------------------------------------------------------
	/*!
	Open file name and read its meta header.
	\param Save file name
	\return Meta header
	*/
	SCR_MetaStruct GetMeta(string fileName)
	{
		SCR_DSSessionCallback callback = FindCallback(fileName);
		if (!callback)
		{
			Print(string.Format("SCR_SaveManagerCore: Cannot load meta of save file '%1', no rules found for it!", fileName), LogLevel.WARNING);
			return null;
		}
		
		return callback.GetMeta(fileName);
	}
	
	//////////////////////////////////////////////////////////////////////////////////////////
	///@name Lists
	///@{
	//////////////////////////////////////////////////////////////////////////////////////////
	
	/*!
	Get save file names of given save type.
	\param[out] outLocalSaves Array to be filled with save file names
	\param type Save type
	\param currentMissionOnly When true, return onlyu save files belonging to currently loaded mission
	\return Number of save file names
	*/
	int GetLocalSaveFiles(out notnull array<string> outLocalSaves, ESaveType saveTypes, bool currentMissionOnly)
	{
		string missionFileName;
		if (currentMissionOnly)
			missionFileName = m_sMissionSaveFileName;
		
		return GetLocalSaveFiles(outLocalSaves, saveTypes, missionFileName);
	}
	/*!
	Get save file names of given save type.
	\param[out] outLocalSaves Array to be filled with save file names
	\param type Save type
	\param missionFileName When not an empty string, return only save files belonging to this mission
	\return Number of save file names
	*/
	int GetLocalSaveFiles(out notnull array<string> outLocalSaves, ESaveType saveTypes, string missionFileName = string.Empty)
	{
		int saveCount = GetGame().GetBackendApi().GetStorage().AvailableSaves(outLocalSaves);
		
		for (int i = saveCount - 1; i >= 0; i--)
		{
			string saveName = outLocalSaves[i];
			
			// Check callback
			SCR_DSSessionCallback callback = FindCallback(outLocalSaves[i]);
			if (!callback)
			{
				outLocalSaves.Remove(i);
				continue;
			}
			
			if (!(saveTypes & callback.GetSaveType()))
			{
				outLocalSaves.Remove(i);
				continue;
			}
			
			// Get save meta data - should contains scenario setting
			SCR_MetaStruct metaStruct = callback.GetMeta(outLocalSaves[i]);
			if (!metaStruct)
			{
				outLocalSaves.Remove(i);
				continue;
			}
			
			// Is save of current scenario 
			MissionWorkshopItem mission = SCR_SaveWorkshopManager.GetCurrentScenario();
			
			if (!mission)
				continue;
			
			string metaHeaderResource = metaStruct.GetHeaderResource();
			string metaMissionId = SCR_SaveWorkshopManager.ScenarioGUIDToID(metaStruct.GetHeaderResource());
			
			string missionFullId = mission.Id();
			string missionId = SCR_SaveWorkshopManager.ScenarioGUIDToID(missionFullId);
			
			if (metaMissionId != missionId)
			{
				outLocalSaves.Remove(i);
				continue;
			}	
			
			// Check save enabled
			string ownerId = SCR_SaveWorkshopManager.GetSaveFileID(saveName);
			if (!ownerId.IsEmpty())
			{
				WorkshopItem owner = GetGame().GetBackendApi().GetWorkshop().FindItem(ownerId);
				if (owner && !owner.IsEnabled())
				{
					outLocalSaves.Remove(i);
					continue;
				}	
			}
			
			// Is save of current scenario mod source - Older saves might not contain that
			string scenarioAddon = metaStruct.GetScenarioAddon();
			string missionOwnerId = mission.GetOwnerId();
			
			if (scenarioAddon != missionOwnerId)
				outLocalSaves.Remove(i);
		}
		
		return outLocalSaves.Count();
	}
	///@}
	
	protected bool IsDownloaded(string fileName)
	{
		string ext;
		FilePath.StripExtension(fileName, ext);
		return ext.Contains("_"); //--- Downloaded files have GUID added at the end, e.g., "MissionName-CustomName.save_5D82C234B9132BBC"
	}
	
	//////////////////////////////////////////////////////////////////////////////////////////
	///@name Save Types
	///@{
	//////////////////////////////////////////////////////////////////////////////////////////
	
	/*!
	Check if saving is allowed at this moment.
	\param type Save type
	\return True if saving is allowed
	*/
	bool CanSave(ESaveType type)
	{
		if (!m_sMissionSaveFileName || !FindCallback(type))
			return false;
		
		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		return !gameMode || gameMode.GetState() == SCR_EGameModeState.GAME;
	}
	
	//----------------------------------------------------------------------------------------
	/*!
	Assign JSON struct to specific save type.
	Multiple save types can reuse the same struct.
	\param type Save type
	\param struct JSON mission struct to be assigned.
	*/
	void SetStruct(ESaveType type, SCR_MissionStruct struct)
	{
		foreach (SCR_DSSessionCallback callback: m_aCallbacks)
		{
			if (callback.GetSaveType() == type)
			{
				callback.SetStruct(struct);
				return;
			}
		}
	}
	
	//----------------------------------------------------------------------------------------
	/*!
	Print out JSON struct of given save type.
	\param type Save type
	*/
	void Log(ESaveType type)
	{
		SCR_DSSessionCallback callback = FindCallback(type);
		if (callback)
			callback.Log();
	}
	
	//----------------------------------------------------------------------------------------
	/*!
	Extract mission file name from save file name.
	\param fileName Save file name
	\return Mission file name
	*/
	string GetMissionFileName(string fileName)
	{
		SCR_DSSessionCallback callback = FindCallback(fileName);
		if (callback)
			return callback.GetMissionFileName(fileName);
		else
			return string.Empty;
	}
	
	//----------------------------------------------------------------------------------------
	/*!
	Extract custom name from save file name.
	\param fileName Save file name
	\return Custom name
	*/
	string GetCustomName(string fileName)
	{
		SCR_DSSessionCallback callback = FindCallback(fileName);
		if (callback)
			return callback.GetCustomName(fileName);
		else
			return string.Empty;
	}
	
	//----------------------------------------------------------------------------------------
	/*!
	Get UI info representing save of type defined by the file name.
	\param fileName Save file name
	\return UI info
	*/
	SCR_UIInfo GetSaveTypeInfo(string fileName)
	{
		SCR_DSSessionCallback callback = FindCallback(fileName);
		if (callback)
			return callback.GetInfo();
		else
			return null;
	}
	///@}
	
	//----------------------------------------------------------------------------------------
	protected SCR_DSSessionCallback FindCallback(ESaveType type)
	{
		foreach (SCR_DSSessionCallback callback: m_aCallbacks)
		{
			if (callback.GetSaveType() == type && callback.IsConfigured())
				return callback;
		}
		return null;
	}
	
	//----------------------------------------------------------------------------------------
	protected SCR_DSSessionCallback FindCallback(string fileName)
	{
		foreach (SCR_DSSessionCallback callback: m_aCallbacks)
		{
			if (callback.IsCompatible(fileName))
				return callback;
		}
		return null;
	}

	//----------------------------------------------------------------------------------------
	void UploadToWorkshop(string fileName)
	{
		SCR_DSSessionCallback callback = FindCallback(fileName);
		if (!callback)
			return;
		
		m_UploadCallback = new SCR_ServerSaveRequestCallback(fileName);//, callback.GetStruct());
	}
	
	
	//////////////////////////////////////////////////////////////////////////////////////////
	///@name File Name To Load
	///@{
	//////////////////////////////////////////////////////////////////////////////////////////
	
	/*!
	Set the latest save file of given mission as the save that should be loaded upon mission start.
	\param missionHeader Mission header
	\return True if save file exists and was marked for load
	*/
	bool SetFileNameToLoad(SCR_MissionHeader missionHeader)
	{
		if (!missionHeader)
			return false;
		
		//--- Find latest save for the mission
		string latestSaveFileName;
		MissionWorkshopItem mission = SCR_SaveWorkshopManager.GetScenarioMissionWorkshopItem(missionHeader);	
		string missionId = SCR_SaveWorkshopManager.ScenarioGUIDToID(missionHeader.GetHeaderResourceName());
	
		if (!FindLatestSave(missionId, latestSaveFileName))
			return false;
		
		return SetFileNameToLoad(latestSaveFileName);
	}
	
	//----------------------------------------------------------------------------------------
	/*!
	Set which save file should be loaded upon mission start.
	\param fileName Save file name
	\return True if save file exists and was marked for load
	*/
	bool SetFileNameToLoad(string fileName)
	{
		if (!GetGame().GetBackendApi().GetStorage().CheckFileID(fileName))
			return false;
		
		GameSessionStorage.s_Data.Insert(GAME_SESSION_STORAGE_FILE_NAME_TO_LOAD, fileName);
		Print(string.Format("'%1' set as a save file name to load after world start.", fileName), LogLevel.VERBOSE);
		return true;
	}
	
	//----------------------------------------------------------------------------------------
	/*!
	Remove save file name marked to be loaded after mission start.
	Does not remove the file itself.
	*/
	void ResetFileNameToLoad()
	{
		if (GameSessionStorage.s_Data.Contains(GAME_SESSION_STORAGE_FILE_NAME_TO_LOAD))
		{
			GameSessionStorage.s_Data.Remove(GAME_SESSION_STORAGE_FILE_NAME_TO_LOAD);
			Print("Save file name to load after world start removed.", LogLevel.VERBOSE);
		}
	}
	
	//----------------------------------------------------------------------------------------
	/*!
	Check which save file should be loaded after mission start.
	\param[out] fileNameToLoad Save file name
	\return True if some save file is marked to be loaded after mission start
	*/
	bool FindFileNameToLoad(out string fileNameToLoad)
	{
		return GameSessionStorage.s_Data.Find(GAME_SESSION_STORAGE_FILE_NAME_TO_LOAD, fileNameToLoad);
	}
	///@}
	
	//----------------------------------------------------------------------------------------
	/*!
	Find name of save file base on provided id 
	\param id of world save item save is coming from
	\return name of file cointaining id if any is found
	*/
	string FindFileNameById(string id)
	{
		array<string> saves = {};
		GetGame().GetBackendApi().GetStorage().AvailableSaves(saves);
		
		foreach (string save : saves)
		{
			if (save.Length() <= ITEM_SAVE_POSTFIX.Length())
				continue;
			
			int idStart = save.IndexOf(".");
			if (idStart < 0)
				continue;
			
			// Check if file name format is matching to save downloaded from workshop
			string formatSubstring = save.Substring(idStart, save.Length() - idStart);
			if (!formatSubstring.Contains(ITEM_SAVE_POSTFIX))
				continue;
			
			// Separate id
			idStart = idStart + ITEM_SAVE_POSTFIX.Length() - 1;
			string saveId = save.Substring(idStart + 1, save.Length() - idStart - 1);
			
			if (saveId == id)
				return save;
		}
		
		return "";
	}
	
	//////////////////////////////////////////////////////////////////////////////////////////
	///@name Latest Save
	///@{
	//////////////////////////////////////////////////////////////////////////////////////////
	
	/*!
	Set the latest save for the currently running mission.
	\param saveFileName Save file name
	*/
	void SetCurrentMissionLatestSave(string saveFileName)
	{
		SetLatestSave(m_sMissionSaveFileName, saveFileName);
	}
	
	//----------------------------------------------------------------------------------------
	/*!
	Remove the latest save for the currently running mission.
	\param saveFileName Save file name
	*/
	void RemoveCurrentMissionLatestSave()
	{
		RemoveLatestSave(m_sMissionSaveFileName);
	}
	
	//----------------------------------------------------------------------------------------
	/*!
	Find the latest save for the currently running mission.
	\param[out] outSaveFileName String to be filled with the latest save file name
	*/
	bool FindCurrentMissionLatestSave(out string outSaveFileName)
	{
		return FindLatestSave(m_sMissionSaveFileName, outSaveFileName);
	}
	
	//----------------------------------------------------------------------------------------
	/*!
	Set the latest save for the given mission.
	\param missionFileName Mission save file name
	\param saveFileName Save file name
	*/
	void SetLatestSave(string missionFileName, string saveFileName)
	{
		m_LatestSaveCallback.SetFileName(missionFileName, saveFileName);
	}
	
	//----------------------------------------------------------------------------------------
	/*!
	Remove the latest save for the given mission.
	\param missionFileName Mission save file name
	\param saveFileName Save file name
	*/
	void RemoveLatestSave(string missionFileName)
	{
		m_LatestSaveCallback.RemoveFileName(missionFileName);
	}
	
	//----------------------------------------------------------------------------------------
	/*!
	Find the latest save for the given mission.
	\param missionFileName Mission save file name
	\param saveFileName Save file name
	*/
	bool FindLatestSave(string missionFileName, out string outSaveFileName)
	{
		return m_LatestSaveCallback.FindFileName(missionFileName, outSaveFileName);
	}
	
	//----------------------------------------------------------------------------------------
	/*!
	Set the latest save for the given mission.
	\param missionFileName Mission header
	\param saveFileName Save file name
	*/
	bool FindLatestSave(SCR_MissionHeader missionHeader, out string outSaveFileName)
	{
		MissionWorkshopItem mission = SCR_SaveWorkshopManager.GetScenarioMissionWorkshopItem(missionHeader);	
		string missionId = SCR_SaveWorkshopManager.ScenarioGUIDToID(missionHeader.GetHeaderResourceName());
		
		return missionHeader && FindLatestSave(missionId, outSaveFileName);
	}
	
	//----------------------------------------------------------------------------------------
	/*!
	Check if the mission has a latest save.
	\param missionFileName Mission save file name
	*/
	bool HasLatestSave(string missionFileName)
	{
		string saveFileName;
		return FindLatestSave(missionFileName, saveFileName);
	}
	
	//----------------------------------------------------------------------------------------
	/*!
	Check if the mission has a latest save.
	\param missionFileName Mission header
	*/
	bool HasLatestSave(SCR_MissionHeader missionHeader)
	{
		string saveFileName;
		return missionHeader && FindLatestSave(missionHeader, saveFileName);
	}
	///@}
	
	//////////////////////////////////////////////////////////////////////////////////////////
	///@name Invokers
	///@{
	//////////////////////////////////////////////////////////////////////////////////////////
	
	//----------------------------------------------------------------------------------------
	/*!
	\return Invoker called when the game is successfully saved.
	*/
	ScriptInvoker_SaveManagerCore GetOnSaved()
	{
		return m_OnSaved;
	}
	
	//----------------------------------------------------------------------------------------
	/*!
	\return Invoker called when the game is unsuccessfully saved.
	*/
	ScriptInvoker_SaveManagerCore GetOnSaveFailed()
	{
		return m_OnSaveFailed;
	}
	//----------------------------------------------------------------------------------------
	/*!
	\return Invoker called when the game is successfully loaded.
	*/
	ScriptInvoker_SaveManagerCore GetOnLoaded()
	{
		return m_OnLoaded;
	}
	
	//----------------------------------------------------------------------------------------
	/*!
	\return Invoker called when a save file is deleted.
	*/
	ScriptInvoker_SaveManagerCore GetOnDeleted()
	{
		return m_OnDeleted;
	}
	
	//----------------------------------------------------------------------------------------
	/*!
	\return Invoker called when a record about the latest save is stored.
	*/
	ScriptInvokerString GetOnLatestSave()
	{
		return m_OnLatestSave;
	}
	
	//----------------------------------------------------------------------------------------
	SCR_ServerSaveRequestCallback GetUploadCallback()
	{
		return m_UploadCallback;
	}
	
	///@}
	
	//////////////////////////////////////////////////////////////////////////////////////////
	// Init
	//////////////////////////////////////////////////////////////////////////////////////////
	/*!
	\return Load a save file upon mission start.
	*/
	void LoadOnInit()
	{
		//--- Stop if mission save file is not defined (e.g., when mission header is missing) or when on client
		if (m_bLoadedOnInit || !m_sMissionSaveFileName || !Replication.IsServer())
			return;
		
		//--- Mark as initialized, so nothing will happen in case somebody is crazy/brave/naive enough to call LoadOnInit() again
		m_bLoadedOnInit = true;
		
		//--- Find save file to be loaded on start
		string fileNameToLoad;
		if (System.IsConsoleApp())
		{
			//--- DEDICATED SERVER
			
			//--- Check if some file was marked for load, e.g, using client's editor interface. If not, continue with evaluating CLI param.
			if (!FindFileNameToLoad(fileNameToLoad))
			{
				//--- Terminate when the file marked by the CLI param was loaded before (happens e.g., after calling the #restart command)
				if (GameSessionStorage.s_Data.Contains(GAME_SESSION_STORAGE_USED_CLI))
					return;
				
				//--- Terminate when the CLI param is not used
				if (!System.GetCLIParam(CLI_PARAM, fileNameToLoad))
				{
					Print(string.Format("SCR_SaveManagerCore: -%1 CLI param not used, no save file is loaded.", CLI_PARAM), LogLevel.VERBOSE);
					return;
				}
				
				//--- Terminate when the CLI param is marked to load the latest save, but there is none defined
				if (fileNameToLoad.IsEmpty() && !FindLatestSave(m_sMissionSaveFileName, fileNameToLoad))
				{
					Print(string.Format("SCR_SaveManagerCore: -%1 CLI param set to load the latest save, but none was defined for the mission '%2'!", CLI_PARAM, m_sMissionSaveFileName), LogLevel.WARNING);
					return;
				}
				
				GameSessionStorage.s_Data.Insert(GAME_SESSION_STORAGE_USED_CLI, fileNameToLoad);
			}
		}
		else
		{
			//--- STANDARD GAME
			
			//--- Show a warning when the CLI param is used incorrectly (otherwise harmless)
			if (System.IsCLIParam(CLI_PARAM))
				Print(string.Format("SCR_SaveManagerCore: -%1 CLI is intended for dedicated server only!", CLI_PARAM), LogLevel.WARNING);
			
			//--- Terminate if requested save file does not exist
			if (!FindFileNameToLoad(fileNameToLoad))
				return;
		}
		
		//--- Load the file and unmark it, so restarting from pause menu won't load it again
		Load(fileNameToLoad);
		ResetFileNameToLoad();
	}
	
	//----------------------------------------------------------------------------------------
	protected void InitDebugMissionHeader(out SCR_MissionHeader missionHeader)
	{
		//--- Mission header not found, create a debug one (play mode in World Editor never has a mission header, even when one for the world exists)
		if (!missionHeader && SCR_SaveLoadComponent.GetInstance())
		{
			m_WorkbenchMissionHeader = new SCR_MissionHeader();
			m_WorkbenchMissionHeader.m_sSaveFileName = FilePath.StripPath(FilePath.StripExtension(GetGame().GetWorldFile()));
			m_WorkbenchMissionHeader.m_bIsSavingEnabled = true;
			missionHeader = m_WorkbenchMissionHeader;
		}
	}
	
	//////////////////////////////////////////////////////////////////////////////////////////
	// Default functions
	//////////////////////////////////////////////////////////////////////////////////////////
	override void OnUpdate(float timeSlice)
	{
		if (m_sMissionSaveFileName && !System.IsConsoleApp() && GetGame().IsDev())
		{
			if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_SAVING_SAVE))
			{
				DiagMenu.SetValue(SCR_DebugMenuID.DEBUGUI_SAVING_SAVE, false);
				
				ESaveType saveType = 1 << DiagMenu.GetValue(SCR_DebugMenuID.DEBUGUI_SAVING_TYPE);
				
				array<string> customNames = {"Alpha", "Kilo", "Zulu"};
				string customName = customNames.GetRandomElement();
				Save(saveType, customName);
			}
			if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_SAVING_LOG))
			{
				DiagMenu.SetValue(SCR_DebugMenuID.DEBUGUI_SAVING_LOG, false);
				
				ESaveType saveType = 1 << DiagMenu.GetValue(SCR_DebugMenuID.DEBUGUI_SAVING_TYPE);
				Log(saveType);
			}
			if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_SAVING_LOAD_LATEST))
			{
				DiagMenu.SetValue(SCR_DebugMenuID.DEBUGUI_SAVING_LOAD_LATEST, false);
				
				string fileName;
				if (FindLatestSave(m_sMissionSaveFileName, fileName))
					Load(fileName);
			}
			if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_SAVING_RESTART_AND_LOAD_LATEST))
			{
				DiagMenu.SetValue(SCR_DebugMenuID.DEBUGUI_SAVING_RESTART_AND_LOAD_LATEST, false);
				RestartAndLoad();
			}
			if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_SAVING_LOG_LATEST))
			{
				DiagMenu.SetValue(SCR_DebugMenuID.DEBUGUI_SAVING_LOG_LATEST, false);
				
				string latestSaveFileName;
				if (FindLatestSave(m_sMissionSaveFileName, latestSaveFileName))
					PrintFormat("The latest save file name for mission '%1' is '%2'", m_sMissionSaveFileName, latestSaveFileName);
				else
					PrintFormat("There is no latest save file name for mission '%1'", latestSaveFileName);
			}
			if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_SAVING_UPLOAD_LATEST))
			{
				DiagMenu.SetValue(SCR_DebugMenuID.DEBUGUI_SAVING_UPLOAD_LATEST, false);
				
				string fileName;
				if (FindLatestSave(m_sMissionSaveFileName, fileName))
				{	
					UploadToWorkshop(fileName);
				}
				else
				{
					Print("SCR_SaveManagerCore: Cannot upload, latest save not found!", LogLevel.WARNING);
				}
			}
			if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_SAVING_DOWNLOAD))
			{
				DiagMenu.SetValue(SCR_DebugMenuID.DEBUGUI_SAVING_DOWNLOAD, false);
				
				m_bDebugDelete = false;
				//DownloadFromWorkshop();
				Print("Download from workshop is disabled");
			}
			if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_SAVING_DELETE))
			{
				DiagMenu.SetValue(SCR_DebugMenuID.DEBUGUI_SAVING_DELETE, false);
				
				m_bDebugDelete = true;
				//DownloadFromWorkshop();
				Print("Download from workshop is disabled");
			}
		}
	}
	
	//----------------------------------------------------------------------------------------
	override void OnGameStart()
	{
		SCR_SaveWorkshopManager.GetInstance().Init();
		
		//--- Saving is not configured for the current world
		if (!SCR_SaveLoadComponent.GetInstance())
		{
			// Load save immidiatelly - works only when getting back to main menu, not when starting game
			OnGameStorageInitialize();
			
			if (m_LatestSaveCallback.IsLoaded())
				return;
			
			// Wait for init - happens on game start
			m_SessionStorageCallback = new SCR_DSSessionCallbackSessionStorage();
			m_SessionStorageCallback.GetOnInitialize().Insert(OnGameStorageInitialize);
			GetGame().GetBackendApi().SetSessionCallback(m_SessionStorageCallback);
			
			return;
		}
			
		//--- Init mission header (use debug one in special conditions)
		SCR_MissionHeader missionHeader = SCR_MissionHeader.Cast(GetGame().GetMissionHeader());

#ifdef WORKBENCH
		InitDebugMissionHeader(missionHeader);	
#endif
		
#ifdef SAVE_MANAGER_DEBUG_HEADER
		InitDebugMissionHeader(missionHeader);	
#endif
		
		//--- Set mission save file name, but only if saving is enabled - Todo: Look for saving component instead of header
		string id = SCR_SaveWorkshopManager.GetCurrentScenarioId();
		m_sMissionSaveFileName = SCR_SaveWorkshopManager.ScenarioGUIDToID(id);
		string nameFormated;
		
		for (int i = 0, count = m_sMissionSaveFileName.Length(); i < count; i++)
		{
			if (m_sMissionSaveFileName[i] == " ")
				 nameFormated += "_";
			else
				nameFormated += m_sMissionSaveFileName[i];
		}
		
		nameFormated.Replace("__", "_");
		nameFormated.Replace("-", "");
		m_sMissionSaveFileName = nameFormated;
			
		// Setup current save 
		SCR_SaveWorkshopManager saveWorkshopManager = SCR_SaveWorkshopManager.GetInstance();
		saveWorkshopManager.SetCurrentSave("", null);
		
		// TODO: Loading of saves could be based on WorldSaveItem id 
		string fileNameToLoad;
		GameSessionStorage.s_Data.Find(GAME_SESSION_STORAGE_FILE_NAME_TO_LOAD, fileNameToLoad);

		if (!fileNameToLoad.IsEmpty())
		{
			WorldSaveItem itemToLoad = saveWorkshopManager.FindSaveItemBySaveFileName(fileNameToLoad);
			saveWorkshopManager.SetCurrentSave(fileNameToLoad, itemToLoad);
		}
		
		//--- Initialize callbacks
		foreach (SCR_DSSessionCallback callback: m_aCallbacks)
		{
			callback.OnGameStart(m_sMissionSaveFileName);
		}
		
		//--- Initialize save manager and load marked save file
		m_LatestSaveCallback.ReadFromFile();
		
		//--- Diag menu init
		if (GetGame().IsDev() && Replication.IsServer() && !System.IsConsoleApp())
		{
			typename enumType = ESaveType;
			const string categoryName = "Save Manager";
			DiagMenu.RegisterMenu(SCR_DebugMenuID.DEBUGUI_SAVING, categoryName, "Game");
			DiagMenu.RegisterRange(SCR_DebugMenuID.DEBUGUI_SAVING_TYPE, "", "Type", categoryName, string.Format("0,%1,0,1", enumType.GetVariableCount() - 1));
			DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_SAVING_LOG, "", "Log Struct By Type", categoryName);
			DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_SAVING_SAVE, "", "Save By Type", categoryName);
			
			DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_SAVING_LOAD_LATEST, "", "Load Latest Save", categoryName);
			DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_SAVING_RESTART_AND_LOAD_LATEST, "", "Restart and Load Latest Save", categoryName);
			DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_SAVING_LOG_LATEST, "", "Log Latest Save File Name", categoryName);
			DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_SAVING_UPLOAD_LATEST, "", "Upload Latest Save", categoryName);
			
			DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_SAVING_DOWNLOAD, "", "Download Workshop Saves", categoryName);
			DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_SAVING_DELETE, "", "Delete Workshop Saves", categoryName);
		}
	}
	
	//----------------------------------------------------------------------------------------
	protected void OnGameStorageInitialize()
	{
		if (SCR_MainMenuEntity.GetInstance())
			m_LatestSaveCallback.ReadFromFile();
		
		if (m_SessionStorageCallback)
			m_SessionStorageCallback.GetOnInitialize().Remove(OnGameStorageInitialize);
	}
	
	//----------------------------------------------------------------------------------------
	override void OnGameEnd()
	{
		foreach (SCR_DSSessionCallback callback: m_aCallbacks)
		{
			callback.OnGameEnd(m_sMissionSaveFileName);
		}
		
		m_bLoadedOnInit = false;
		m_sMissionSaveFileName = string.Empty;
		m_WorkbenchMissionHeader = null;
		
		//--- Clear invokers are the world end
		m_OnSaved.Clear();
		m_OnLoaded.Clear();
		m_OnDeleted.Clear();
		m_OnSaveFailed.Clear();
		
		if (GetGame().IsDev() && Replication.IsServer() && !System.IsConsoleApp())
		{
			DiagMenu.Unregister(SCR_DebugMenuID.DEBUGUI_SAVING);
			DiagMenu.Unregister(SCR_DebugMenuID.DEBUGUI_SAVING_TYPE);
			DiagMenu.Unregister(SCR_DebugMenuID.DEBUGUI_SAVING_SAVE);
			DiagMenu.Unregister(SCR_DebugMenuID.DEBUGUI_SAVING_LOG);
			DiagMenu.Unregister(SCR_DebugMenuID.DEBUGUI_SAVING_LOAD_LATEST);
			DiagMenu.Unregister(SCR_DebugMenuID.DEBUGUI_SAVING_RESTART_AND_LOAD_LATEST);
			DiagMenu.Unregister(SCR_DebugMenuID.DEBUGUI_SAVING_LOG_LATEST);
			DiagMenu.Unregister(SCR_DebugMenuID.DEBUGUI_SAVING_UPLOAD_LATEST);
			DiagMenu.Unregister(SCR_DebugMenuID.DEBUGUI_SAVING_DOWNLOAD);
			DiagMenu.Unregister(SCR_DebugMenuID.DEBUGUI_SAVING_DELETE);
		}
	}
};

//----------------------------------------------------------------------------------------
class SCR_DSSessionCallbackSessionStorage: DSSessionCallback
{
	protected ref ScriptInvokerVoid m_OnInitialize;
	
	//----------------------------------------------------------------------------------------
	ScriptInvokerVoid GetOnInitialize()
	{
		if (!m_OnInitialize)
			m_OnInitialize = new ScriptInvokerVoid();
		
		return m_OnInitialize;
	}
	
	//----------------------------------------------------------------------------------------
	override void OnInitialize()
	{
		super.OnInitialize();
		m_OnInitialize.Invoke();
	}
};