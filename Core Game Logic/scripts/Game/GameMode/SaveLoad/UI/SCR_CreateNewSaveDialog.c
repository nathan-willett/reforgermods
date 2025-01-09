/*!
Confirming dialog will create a new save
This dialog should require only name - that is define in list configuration
*/
class SCR_CreateNewSaveDialog: SCR_EditorSaveDialog
{	
	protected SCR_ConfigurableDialogUi m_ConfirmPrompt;
	
	//---------------------------------------------------------------------------------------------
	protected override void Init(Widget root, SCR_ConfigurableDialogUiPreset preset, MenuBase proxyMenu)
	{
		m_bEnableDependencies = true;
		super.Init(root, preset, proxyMenu);
	}
	
	//---------------------------------------------------------------------------------------------
	protected override void OnConfirm()
	{			
		m_sSaveName = m_ConfigList.FindEntry("name").ValueAsString();
			
		// Check validity 
		Widget invalidEntry = m_ConfigList.GetInvalidEntry();
		if (invalidEntry)
			return;
		
		// Save game
		if (GetGame().GetSaveManager().FileExists(ESaveType.USER, m_sSaveName))
		{
			//--- Confirm prompt
			m_ConfirmPrompt = SCR_ConfigurableDialogUi.CreateFromPreset(SCR_CommonDialogs.DIALOGS_CONFIG, "session_save_override");
			m_ConfirmPrompt.SetTitle(m_sSaveName);
			m_ConfirmPrompt.m_OnConfirm.Insert(OnConfirmPrompt);
		}
		else
		{
			//--- Creating a new file - save directly
			OnConfirmPrompt();
		}
		
		m_OnConfirm.Invoke(this);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnConfirmPrompt()
	{	
		// Save 
		WorldSaveManifest newManifest = ManifestFromConfigList();
		
		// Enable all dependencies by default
		GameProject.GetLoadedAddons(newManifest.m_aDependencyIds);
		
		GetGame().GetSaveManager().Save(ESaveType.USER, m_sSaveName, newManifest);
		
		Close();
	}
	
	//---------------------------------------------------------------------------------------------
	void SCR_CreateNewSaveDialog()
	{
		SCR_ConfigurableDialogUi.CreateFromPreset(SCR_SaveWorkshopManagerUI.DIALOGS_CONFIG, "create_new", this);
	}
}