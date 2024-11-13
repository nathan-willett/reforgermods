class Bacon_GunBuilderOpenAction : ScriptedUserAction
{
	override bool HasLocalEffectOnlyScript() { return true; }
	override bool CanBroadcastScript() { return false; }
	//------------------------------------------------------------------------------------------------
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{	
		MenuManager menuManager = GetGame().GetMenuManager();
		Bacon_GunBuilderUI menu = Bacon_GunBuilderUI.Cast(menuManager.OpenMenu(ChimeraMenuPreset.Bacon_GunBuilderUI));
		
		menu.Init(pOwnerEntity, pUserEntity);
	}
};