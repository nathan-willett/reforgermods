


// --------------------------------------------------------------------------------------------------------------------
// ------           This script is part of REAPER CORE | an Arma Reforger SP/COOP Modification                   ------
// ------   You are not allowed to use this script or parts of it in your mod or pass it off as your own work.   ------
// ------                                                                                                        ------
// ------                         Written by REAPER 2024 - www.reaper-as.de                                      ------
// --------------------------------------------------------------------------------------------------------------------


// This file modifies the SCR_LoadoutRequestUIComponent class to set the character's identity name when a loadout is requested.
// 
// The function RequestPlayerLoadout calls REAPER_SetCharIdent(), which retrieves the player's character identity (name, alias, 
// surname) and updates the UI with the formatted name.
// 
// This ensures that loadouts display the correct character identity in the UI.


// Mod to use Loadout-Name from Character Ident 

modded class SCR_LoadoutRequestUIComponent : SCR_DeployRequestUIBaseComponent
{
	//override protected void RequestPlayerLoadout(SCR_LoadoutButton loadoutBtn)  // Old Version!
	override protected void RequestPlayerLoadout(SCR_BasePlayerLoadout loadout)
	{
		super.RequestPlayerLoadout(loadout);
		
		REAPER_SetCharIdent();
	}
	
	//-----------------------------------------------------------------------------------------------------------------
	
	override void RefreshLoadoutPreview()
	{
		super.RefreshLoadoutPreview();
		
		REAPER_SetCharIdent();
	}
	
	//-----------------------------------------------------------------------------------------------------------------
	
	override protected void SetLoadoutPreview(SCR_BasePlayerLoadout loadout)
	{
		super.SetLoadoutPreview(loadout);
		
		REAPER_SetCharIdent();
	}
	
	//-----------------------------------------------------------------------------------------------------------------
	
	protected void REAPER_SetCharIdent()
	{
		if(m_PreviewedEntity) {
			
			SCR_CharacterIdentityComponent identComp = SCR_CharacterIdentityComponent.Cast(m_PreviewedEntity.FindComponent(SCR_CharacterIdentityComponent));
			
			if(identComp) {		
				string name, alias, surname, formated;
				
				identComp.GetFormattedFullName(formated, name, alias, surname);

				string charName = string.Format("%1 %2 (%3)",surname, name, alias);
				
				if(m_wLoadoutName) m_wLoadoutName.SetText(charName);
				if(m_wExpandButtonName) m_wExpandButtonName.SetText(charName);	
			}
		}	
	}
	
}