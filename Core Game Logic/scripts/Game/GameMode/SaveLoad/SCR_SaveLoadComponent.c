[ComponentEditorProps(category: "GameScripted/GameMode/Components", description: "")]
class SCR_SaveLoadComponentClass : SCR_BaseGameModeComponentClass
{
}

//! Game mode-specific settings for session saving.
class SCR_SaveLoadComponent : SCR_BaseGameModeComponent
{
	[Attribute(desc: "Struct object which manages saved data. Must be defined, without it saving won't work.")]
	protected ref SCR_MissionStruct m_Struct;
	
	[Attribute(defvalue: "1", desc: "When enabled, save the state when exiting the world.")]
	protected bool m_SaveOnExit;
	
	[Attribute(defvalue: "0", desc: "0 = disabled. 60 seconds is the lowest accepted value otherwise.")]
	protected int m_iAutosavePeriod;
	
	[Attribute(uiwidget: UIWidgets.ResourceNamePicker, params: "conf class=SCR_MissionHeader", desc: "Mission header used for saving/loading in Workbench (where standard mission header is not loaded)")]
	protected ResourceName m_DebugHeaderResourceName;
	
	protected static const int MINIMUM_AUTOSAVE_PERIOD = 60;
	
	/////////////////////////////////////////////////////////////////////////////
	// Public
	/////////////////////////////////////////////////////////////////////////////

	//------------------------------------------------------------------------------------------------
	//! \return Local instance of this component
	static SCR_SaveLoadComponent GetInstance()
	{
		BaseGameMode gameMode = GetGame().GetGameMode();
		if (gameMode)
			return SCR_SaveLoadComponent.Cast(gameMode.FindComponent(SCR_SaveLoadComponent));
		else
			return null;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return true if the world should be saved on exit.
	bool CanSaveOnExit()
	{
		return m_SaveOnExit;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return Mission header used for debugging in World Editor (where mission headers are otherwise unavailable)
	ResourceName GetDebugHeaderResourceName()
	{
		return m_DebugHeaderResourceName;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Check if the mission struct contains a sub-struct of specific type.
	//! \param[in] structType Type of queried struct
	//! \return true if the sub-struct is present
	bool ContainsStruct(typename structType)
	{
		return m_Struct && m_Struct.ContainsStruct(structType);
	}
	
	/////////////////////////////////////////////////////////////////////////////
	// Protected
	/////////////////////////////////////////////////////////////////////////////

	//------------------------------------------------------------------------------------------------
	protected void Autosave()
	{	
		GetGame().GetSaveManager().Save(ESaveType.AUTO);
	}
	
	/////////////////////////////////////////////////////////////////////////////
	// Overrides
	/////////////////////////////////////////////////////////////////////////////

	//------------------------------------------------------------------------------------------------
	override void OnGameModeEnd(SCR_GameModeEndData data)
	{
		GetGame().GetSaveManager().RemoveCurrentMissionLatestSave();
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		if (owner.GetWorld().IsEditMode())
			return;
		
		//--- Assign structs (must be on client as well, it's used to correctly read meta header from save files)
		SCR_SaveManagerCore saveManager = GetGame().GetSaveManager();
		saveManager.SetStruct(ESaveType.USER, m_Struct);
		saveManager.SetStruct(ESaveType.AUTO, m_Struct);
		saveManager.SetStruct(ESaveType.EDITOR, m_Struct);
		
		//--- Initialise autosave on server
		if (Replication.IsServer())
		{
			if (m_iAutosavePeriod == 0)
			{
				Print("SCR_SaveLoadComponent: Periodical autosave is disabled.", LogLevel.WARNING);
				return;
			}
			
			if (m_iAutosavePeriod > 0 && m_iAutosavePeriod < MINIMUM_AUTOSAVE_PERIOD)
			{
				Print("SCR_SaveLoadComponent: Autosave period set too low (" + m_iAutosavePeriod + "), setting to " + MINIMUM_AUTOSAVE_PERIOD, LogLevel.WARNING);
				m_iAutosavePeriod = MINIMUM_AUTOSAVE_PERIOD;
			}
			
			//--- Use call queue, because frame event is not called when the game is paused
			GetGame().GetCallqueue().CallLater(Autosave, m_iAutosavePeriod * 1000, true);
		}
	}
}
