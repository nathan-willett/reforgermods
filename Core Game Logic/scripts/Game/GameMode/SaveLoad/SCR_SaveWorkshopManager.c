/*!
Scripted api for handling workshop and backend interactions
*/

class SCR_SaveWorkshopManager
{
	// Fallback value used for develepomnet in  test world while not starting via main menu
	protected const string FALLBACK_SCENARIO_NAME = "GM_TestWorld";
	protected const string FALLBACK_SCENARIO_ID = "{D46718CC67B45055}Missions/GM_TestWorld.conf";
	
	protected const int ID_LENGTH = 16;
	protected const int THUMBNAIL_WIDTH = 800;
	protected const string SAVE_EXTENSION = ".save";
	protected const string SAVE_EXTENSION_MODDED = ".save_";
	protected const string SAVE_FILE_EXTENSION = ".json";
	protected const string SESSION_SAVE_NAME = "TestSaveName";
	
	protected const string SAVE_FORMAT = "%1-%2-%3";
	
	protected static ref SCR_SaveWorkshopManager s_Instance;
	
	protected string m_sCurrentSave;
	protected WorldSaveItem m_CurrentSaveItem;
	protected ref SCR_EditedSaveManifest m_EditedManifest;
	
	protected ref array<MissionWorkshopItem> m_aMissions = {};
	protected ref array<string> m_aScenarioSources = {};
	
	protected ref ScriptInvokerString m_OnCurrentSaveChanged;
	
	protected ref SCR_BackendCallback m_UploadCallback = new SCR_BackendCallback(); // Uploading save to workshop/server
	protected ref SCR_BackendCallback m_LoadSavesPageCallback = new SCR_BackendCallback(); // Loading of save page from workshop
	protected ref SCR_BackendCallback m_ChangeCallback; // Changing status of uploaded save - e.g. is publicly visible
	protected ref SCR_BackendCallback m_DeletePublishedCallback = new SCR_BackendCallback(); // Delete save from workshop 
	
	//----------------------------------------------------------------------------------------
	// Public
	//----------------------------------------------------------------------------------------

	//----------------------------------------------------------------------------------------
	static SCR_SaveWorkshopManager GetInstance()
	{
		if (!s_Instance)
			s_Instance = new SCR_SaveWorkshopManager();
		
		return s_Instance;
	} 
	
	//----------------------------------------------------------------------------------------
	//! Setup workshop data to get offline items and scenarios
	void Init()
	{
		WorkshopApi workshop = GetGame().GetBackendApi().GetWorkshop();
		workshop.ScanOfflineItems();
		
		// Fill missions
		workshop.ReadDefaultScenarios(m_aScenarioSources);
		
		if (m_aMissions.IsEmpty())
			workshop.GetPageScenarios(m_aMissions, 0, SCR_WorkshopUiCommon.PAGE_SCENARIOS);
	}
	
	//----------------------------------------------------------------------------------------
	/*! 
	Upload save to workshop. Use GetUploadCallback() to listen to upload response.
	\param[in] save			Save item that will be uploaded.
	\param[in] manifest		Save meta data that will be used in save description.
	\param[in] public		Will set if save will be visible and downloadable by non owners/contributors.
	*/
	void UploadSave(notnull WorldSaveItem save, notnull WorldSaveManifest manifest, bool public)
	{
		manifest.m_bUnlisted = !public;
		save.UploadWorldSave(manifest, m_UploadCallback);
	}
	
	//----------------------------------------------------------------------------------------
	/*!
	Find local World save item by save file name 
	\param[in] string 		Save file name to find 
	\return				Return true if save was found and deleted successfully
	*/
	bool DeleteOfflineSaveByName(string fileName)
	{
		WorldSaveItem save = FindSaveItemBySaveFileName(fileName);
		if (!save)
		{
			Print(string.Format("Could not delete local save as '%1' could not be found", fileName), LogLevel.WARNING);
			return false;
		}
		
		if (m_CurrentSaveItem == save)
			SetCurrentSave("", null);
		
		save.DeleteLocally();

		return true;
	}
	
	//----------------------------------------------------------------------------------------
	/*! 
	Delete published save from workshop. Use GetDeletePublishedCallback() to listen to delete response.
	\param[in] save 		Which save should be deleted - save must be online on workshop.
	*/
	void DeletePublishedSave(notnull WorldSaveItem save)
	{	
		save.DeleteOnline(m_DeletePublishedCallback);
	}
	
	//----------------------------------------------------------------------------------------
	/*!
	Get mission workshop item of current scenario. 
	\warning 		Scenario must be loaded from main menu. GetCurrentScenarioId() and GetCurrentScenarioName() can be used while loading scenario directly in wb.
	\return 		Return MissionWorkshopItem of current loaded scenario header.
	*/
	static MissionWorkshopItem GetCurrentScenario()
	{
		MissionHeader header = GetGame().GetMissionHeader();
		if (!header)
			return null;
		
		string path = header.GetHeaderResourcePath();
		
		WorkshopApi workshop = GetGame().GetBackendApi().GetWorkshop();
		MissionWorkshopItem mission = workshop.GetInGameScenario(path);
		
		if (!mission)
			mission = workshop.GetCurrentMission();
		
		return mission;
	}
	
	//----------------------------------------------------------------------------------------
	//! Get resource path of current scenario or fallback to path of testing world
	static ResourceName GetCurrentScenarioId()
	{
		MissionHeader header = GetGame().GetMissionHeader();
		
		#ifdef WORKBENCH
		if (!header)
			return FALLBACK_SCENARIO_ID;
		#endif
		
		return header.GetHeaderResourceName();
	}
	
	//----------------------------------------------------------------------------------------
	static MissionWorkshopItem GetScenarioMissionWorkshopItem(MissionHeader missionHeader)
	{
		string path = missionHeader.GetHeaderResourceName();
		return  GetGame().GetBackendApi().GetWorkshop().GetInGameScenario(path);
	}
	
	//----------------------------------------------------------------------------------------
	static string GetScenarioNameFile(MissionWorkshopItem missionItem)
	{
		if (!missionItem)
			return string.Empty;
		
		string name = missionItem.Name();
		
		if (name.Length() > 0 && name[0] == "#")
			name.Replace("#AR-", "");
		
		return name;
	}
	
	//----------------------------------------------------------------------------------------
	//! Get name of current scenario or fallback to testing world name for save file name
	static string GetCurrentScenarioNameFile()
	{
		// Get ws item from mission
		MissionWorkshopItem missionItem = GetCurrentScenario();
		
		#ifdef WORKBENCH
		if (!missionItem)
			return FALLBACK_SCENARIO_NAME;
		#endif
		
		return GetScenarioNameFile(missionItem);
	}
	
	//----------------------------------------------------------------------------------------
	//! Get translated name of current scenario or fallback to testing world name
	static string GetCurrentScenarioNameTranslated()
	{
		// Get ws item from mission
		MissionWorkshopItem missionItem = GetCurrentScenario();
		
		#ifdef WORKBENCH
		if (!missionItem)
			return FALLBACK_SCENARIO_NAME;
		#endif
		
		if (!missionItem)
			return string.Empty;
		
		string name = missionItem.Name();
		
		if (name.Length() > 0 && name[0] == "#")
			name = WidgetManager.Translate(name);
		
		return name;
	}
	
	//----------------------------------------------------------------------------------------
	//! Get thumbnail of current scenario or use fallback back image
	ResourceName CurrentScenarioImage()
	{
		MissionWorkshopItem missionItem = GetCurrentScenario();
		
		#ifdef WORKBENCH
		if (!missionItem)
			return "E:/aRMA4/A4Data/UI/Textures/MissionLoadingScreens/Reforger_ConflictArt_UI.png";
		#endif
		
		return missionItem.Thumbnail().GetLocalScale(THUMBNAIL_WIDTH).Path();
	}
	
	//----------------------------------------------------------------------------------------
	/*!
	Find ID in save file name for format 'scenario_name.type_id'
	\param[in] fileName 	Full save file name in 
	\return 			of save workshop item, unpublished saves should return empty string
	*/
	static string GetSaveFileID(string fileName)
	{
		int lastUnderscorePos = fileName.LastIndexOf("_");
		int idLen = fileName.Length() - lastUnderscorePos - 1;
		
		// Check id length
		if (idLen != ID_LENGTH)
			return "";
		
		return fileName.Substring(lastUnderscorePos + 1, idLen);
	}
	
	//----------------------------------------------------------------------------------------
	/*!
	Get offline workshop items and filter out only WorldSaveItem type
	\param[out] saves 		WorldSaveItem array which should be filled by result saves.
	*/
	void GetOfflineSaves(out array<WorldSaveItem> saves)
	{
		array<WorkshopItem> offlines = {};
		GetGame().GetBackendApi().GetWorkshop().GetOfflineItems(offlines);
		
		WorldSaveItem save;
		
		// Use only WorldSaveItem
		foreach (WorkshopItem item : offlines)
		{
			save = WorldSaveItem.Cast(item);
			if (save)
				saves.Insert(save);
		}
	}
	
	//----------------------------------------------------------------------------------------
	/*!
	Return world save item that cotains save file with provided name
	\warning			With standard use by player there should be always unique name. But in case of repeating names wrong item could be found.
	\param[in] fileName 	Full save file name.
	\return 			WorldSaveItem which contains save with this name
	*/
	WorldSaveItem FindSaveItemBySaveFileName(string fileName)
	{	
		array<WorldSaveItem> saves = {};
		GetOfflineSaves(saves);
				
		Revision rev;
		array<string> saveFiles = {};
		
		// Setup file name and id
		string id = GetSaveFileID(fileName);
		if (!id.IsEmpty())
			fileName.Replace("_" + id, "");
		
		foreach (WorldSaveItem save : saves)
		{
			// Has revision
			rev = save.GetActiveRevision();
			if (!rev)
				rev = save.GetLocalRevision();
			
			if (!rev)
			{
				Print(string.Format("Failed to find revision for save item: {%1} %2", save.Id(), save.Name()), LogLevel.WARNING);
				continue;
			}
			
			// Has files
			rev.GetFiles(saveFiles);
			if (saveFiles.IsEmpty())
			{
				Print(string.Format("No save files in save item: {%1} %2", save.Id(), save.Name()), LogLevel.WARNING);
				continue;
			}
			
			// Compare save file names
			string workshopSaveFileName = saveFiles[0];
			
			if (workshopSaveFileName.Contains(SAVE_FILE_EXTENSION))
				workshopSaveFileName = FilePath.StripExtension(workshopSaveFileName);
			
			// Remove modded id
			if (workshopSaveFileName == fileName)
			{
				if (id.IsEmpty())
					return save;
				
				if (id == save.Id())
					return save;
			}
		}
		
		return null;
	}
	
	//----------------------------------------------------------------------------------------
	/*!
	Convert GUID format '{id}Missions/Scenario.cof' to 'id'
	\param[in] guid 	GUID in form of resource path - {id}Missions/Scenario.cof
	\return 		GUID in hexadecimal form stripped of resource path - id
	*/
	static string ScenarioGUIDToID(string guid)
	{
		if (guid.IsEmpty())
			return "";
		
		string id = guid;
		int pos = id.IndexOf("}");
		if (pos > -1)
			id = id.Substring(0, pos);
		
		id.Replace("{", "");
		id.Replace("}", "");
		
		return id;
	}
	
	//----------------------------------------------------------------------------------------
	/*!
	Check if player can save dirrectly into the save. Player shouldn't be able to override save downloaded from workshop which are not owned by player.
	\param[in] save 	Save item to override - is local save item
	*/
	static bool CanOverrideSave(notnull WorldSaveItem save)
	{
		// TODO: Use complex logic once use of downloaded owned save is working properly
		bool isLocal = save.GetLocalRevision();
		bool playerIsAuthor = false;
		
		if (!isLocal)
			playerIsAuthor = save.IsAuthor(); // Should be actually is contributor
		
		return isLocal || playerIsAuthor;
	}
	
	//----------------------------------------------------------------------------------------
	/*!
	Remove '_id' from provided save file name. _id is present in name of save file downloaded from workshop
	\parma fileName 	Save file name
	\return 			Save file name without additional '_id' if it's present. E.g. GM-Arland-Save1.save_613AA7D56D5C87D5.json returns GM-Arland-Save1.save.json
	*/
	//! Return save name without _id at end of the name 
	//! This is present only id save downloaded from workshop
	static string StripSaveIdFromSaveFileName(string fileName)
	{
		int pos = fileName.IndexOf(SAVE_EXTENSION_MODDED);
		if (pos == -1)
			return fileName;
		
		return fileName.Substring(0, pos + SAVE_EXTENSION.Length());
	}
	
	//----------------------------------------------------------------------------------------
	// Get/Set
	//----------------------------------------------------------------------------------------
	
	//----------------------------------------------------------------------------------------
	SCR_BackendCallback GetUploadCallback()
	{
		return m_UploadCallback;
	}
	
	//----------------------------------------------------------------------------------------
	SCR_BackendCallback GetLoadSavesPageCallback()
	{
		return m_LoadSavesPageCallback;
	}
	
	//----------------------------------------------------------------------------------------
	SCR_BackendCallback GetDeletePublishedCallback()
	{
		return m_DeletePublishedCallback;
	}
	
	//----------------------------------------------------------------------------------------
	void SetCurrentSave(string fileName, WorldSaveItem saveItem)
	{
		m_sCurrentSave = fileName;
		m_CurrentSaveItem = saveItem;
		
		if (m_OnCurrentSaveChanged)
			m_OnCurrentSaveChanged.Invoke(fileName);
	}
	
	//----------------------------------------------------------------------------------------
	string GetCurrentSave(out WorldSaveItem saveItem)
	{
		saveItem = m_CurrentSaveItem;
		return m_sCurrentSave;
	}
	
	//----------------------------------------------------------------------------------------
	ScriptInvokerString GetOnCurrentSaveChanged()
	{
		if (!m_OnCurrentSaveChanged)
			m_OnCurrentSaveChanged = new ScriptInvokerString();
		
		return m_OnCurrentSaveChanged;
	}
		
	//----------------------------------------------------------------------------------------
	void SetEditedSaveManifest(WorldSaveManifest manifest, string editingValue = "", string value = "")
	{
		m_EditedManifest = new SCR_EditedSaveManifest(manifest);
		if (m_EditedManifest)
		{
			m_EditedManifest.SetEditingValue(editingValue);
			m_EditedManifest.SetValue(value);
		}
	}
	
	//----------------------------------------------------------------------------------------
	void ClearEditedSaveManifest()
	{
		m_EditedManifest = null;
	}
	
	//----------------------------------------------------------------------------------------
	void SetEditedSaveManifestEditingValue(string editingValue)
	{
		if (m_EditedManifest)
			m_EditedManifest.SetEditingValue(editingValue);
	}
	
	//----------------------------------------------------------------------------------------
	SCR_EditedSaveManifest GetEditedSaveManifest()
	{
		return m_EditedManifest;
	}
	
	//----------------------------------------------------------------------------------------
	// Constructor
	//----------------------------------------------------------------------------------------
	
	//----------------------------------------------------------------------------------------
	protected void SCR_SaveWorkshopManager()
	{
		s_Instance = this;
	}
	
	//----------------------------------------------------------------------------------------
	void ~SCR_SaveWorkshopManager()
	{
		s_Instance = null;
		m_EditedManifest = null;
	}
}