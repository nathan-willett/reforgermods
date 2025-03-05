

// --------------------------------------------------------------------------------------------------------------------
// ------           This script is part of REAPER CORE | an Arma Reforger SP/COOP Modification                   ------
// ------   You are not allowed to use this script or parts of it in your mod or pass it off as your own work.   ------
// ------                                                                                                        ------
// ------                         Written by REAPER 2024 - www.reaper-as.de                                      ------
// --------------------------------------------------------------------------------------------------------------------

// This file modifies the SCR_RoleSelectionMenu class, primarily to set the background image of the deploy menu.
// 
// It selects the appropriate background image based on the screen’s aspect ratio.
// 
// While not directly related to loadouts, this file customizes the role selection menu to integrate with the REAPER CORE mod.

modded class SCR_RoleSelectionMenu : SCR_DeployMenuBase
{
	protected ResourceName bgImage169 = "{844AEDDCAE56FBEC}UI/Textures/REAPER_DeployBG_169.edds";
	protected ResourceName bgImage219 = "{36B9EDB99A09DC3C}UI/Textures/REAPER_DeployBG_219.edds";
	
	//------------------------------------------------------------------
	
	override void OnMenuOpen()
	{
		super.OnMenuOpen();
		
		ImageWidget background = ImageWidget.Cast(m_MenuHandler.GetRootWidget().FindWidget("bg_image"));

		if(background) {
			//Check Aspect Ratio
			WorkspaceWidget workspace = GetGame().GetWorkspace();
			float aspectratio = workspace.GetWidth() / workspace.GetHeight();
							
			if(aspectratio > 1.8) {
				background.LoadImageTexture(0, bgImage219);
			} else {
				background.LoadImageTexture(0, bgImage169);
			}		
		}		
		
	}	
}