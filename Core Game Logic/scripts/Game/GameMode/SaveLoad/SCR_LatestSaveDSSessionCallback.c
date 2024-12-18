[BaseContainerProps()]
class SCR_LatestSaveDSSessionCallback: DSSessionCallback
{
	[Attribute(".LatestSave")]
	protected string m_sFileName;
	
	protected bool m_bIsLoaded;
	protected bool m_bWriteToFile;
	
	protected ref SCR_LatestSaveStruct m_Struct = new SCR_LatestSaveStruct();
	
	
	//----------------------------------------------------------------------------------------
	/*!
	Set the latest save file name for given mission.
	\param missionFileName Mission save file name
	\param saveFileName Latest save file name
	*/
	void SetFileName(string missionFileName, string saveFileName)
	{
		m_Struct.SetFileName(missionFileName, saveFileName);
		WriteToFile();
		
		Print(string.Format("SCR_LatestSaveDSSessionCallback: Latest save for %1 is now %2", missionFileName, saveFileName), LogLevel.VERBOSE);
	}
	
	//----------------------------------------------------------------------------------------
	/*!
	Remove the latest save file name for given mission.
	\param missionFileName Mission save file name
	*/
	void RemoveFileName(string missionFileName)
	{
		m_Struct.RemoveFileName(missionFileName);
		WriteToFile();
	}
	
	//----------------------------------------------------------------------------------------
	/*!
	Find the latest save file name for given mission.
	\param missionFileName Mission save file name
	\param[out] outSaveFileName String to be filled with the latest save file name
	\return True if the latest save file name was found
	*/
	bool FindFileName(string missionFileName, out string outSaveFileName)
	{
		return m_Struct.FindFileName(missionFileName, outSaveFileName);
	}
	/*!
	Print out all latest saves kept in the memory.
	*/
	
	//----------------------------------------------------------------------------------------
	void Log()
	{
		m_Struct.Log();
	}
	
	//----------------------------------------------------------------------------------------
	/*!
	Store latest saves kept in the memory to external file.
	*/
	void WriteToFile()
	{
		SessionStorage storage = GetGame().GetBackendApi().GetStorage();
		storage.AssignFileIDCallback(m_sFileName, this);
		if (GetGame().GetSaveManager().CanSaveToCloud())
		{
			Print(string.Format("SCR_LatestSaveDSSessionCallback: RequestSave: %1", m_sFileName), LogLevel.VERBOSE);
			storage.RequestSave(m_sFileName);
		}
		else
		{
			PrintFormat(string.Format("SCR_LatestSaveDSSessionCallback: LocalSave: %1", m_sFileName), LogLevel.VERBOSE);
			storage.LocalSave(m_sFileName);
		}
		m_bWriteToFile = true;
	}
	
	//----------------------------------------------------------------------------------------
	/*!
	Read latest saves from external file.
	*/
	void ReadFromFile()
	{
		//--- Latest saves were already loaded, don't do it again and use what's already in the memory.
		if (m_bIsLoaded)
		{
			//--- File is already loaded, proceed straight to init 
			GetGame().GetSaveManager().LoadOnInit();
			return;
		}
		
		SessionStorage storage = GetGame().GetBackendApi().GetStorage();
		if (!storage.CheckFileID(m_sFileName))
		{
			//--- File does not exist, proceed straight to init 
			Print(string.Format("SCR_LatestSaveDSSessionCallback: Unable to load %1, it does not exist!", m_sFileName), LogLevel.VERBOSE);
			GetGame().GetSaveManager().LoadOnInit();
			return;
		}
		
		m_bIsLoaded = true;

		//--- Load latest saves from save file
		storage.AssignFileIDCallback(m_sFileName, this);
		if (GetGame().GetSaveManager().CanSaveToCloud())
		{
			Print(string.Format("SCR_LatestSaveDSSessionCallback: RequestLoad: %1", m_sFileName), LogLevel.VERBOSE);
			storage.RequestLoad(m_sFileName);
		}
		else
		{
			Print(string.Format("SCR_LatestSaveDSSessionCallback: LocalLoad: %1", m_sFileName), LogLevel.VERBOSE);
			storage.LocalLoad(m_sFileName);
		}
	}
	
	//----------------------------------------------------------------------------------------
	override void OnSaving(string fileName)
	{
		GetGame().GetBackendApi().GetStorage().ProcessSave(m_Struct, fileName);
	}
	
	//----------------------------------------------------------------------------------------
	override void OnLoaded(string fileName)
	{
		GetGame().GetBackendApi().GetStorage().ProcessLoad(m_Struct, fileName);
		Print(string.Format("SCR_LatestSaveDSSessionCallback: Loading file '%1' succeeded!", fileName), LogLevel.VERBOSE);
		
		GetGame().GetSaveManager().LoadOnInit();
	}
	
	//----------------------------------------------------------------------------------------
	override void OnLoadFailed(string fileName)
	{
		Print(string.Format("SCR_LatestSaveDSSessionCallback: Loading file '%1' failed!", fileName), LogLevel.WARNING);
	}
	
	//----------------------------------------------------------------------------------------
	override void OnSaveSuccess(string fileName)
	{
		m_bWriteToFile = false;
		GetGame().GetSaveManager().GetOnLatestSave().Invoke(fileName);
	}
	
	//----------------------------------------------------------------------------------------
	bool IsLoaded()
	{
		return m_bIsLoaded;
	}
}