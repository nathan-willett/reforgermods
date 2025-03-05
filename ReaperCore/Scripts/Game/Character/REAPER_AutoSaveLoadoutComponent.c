

// --------------------------------------------------------------------------------------------------------------------
// ------           This script is part of REAPER CORE | an Arma Reforger SP/COOP Modification                   ------
// ------   You are not allowed to use this script or parts of it in your mod or pass it off as your own work.   ------
// ------                                                                                                        ------
// ------                         Written by REAPER 2024 - www.reaper-as.de                                      ------
// --------------------------------------------------------------------------------------------------------------------


[EntityEditorProps(category: "GameScripted/Character/", description: "Autosave Loadout", color: "96 255 0 255")]

class REAPER_AutoSaveLoadoutComponentClass: ScriptGameComponentClass {
};


class REAPER_AutoSaveLoadoutComponent: ScriptGameComponent 
{
	protected int m_saveLoadoutDelay = 15; 
	protected int m_playerId;
	
	protected ChimeraCharacter m_character;
	protected SCR_CharacterControllerComponent m_controller;
	protected SCR_ArsenalManagerComponent m_arsenalManager;
	protected SCR_InventoryStorageManagerComponent m_InventoryManager;

	// -------------------------------
	// Init Class
	// --------------------------------	
			
	protected override void OnPostInit(IEntity owner)
	{	
		super.OnPostInit(owner);
		
		if(SCR_Global.IsEditMode()) return;
		
		m_controller = SCR_CharacterControllerComponent.Cast(owner.FindComponent(SCR_CharacterControllerComponent));
		if(!m_controller) {
			Print("ERROR: REAPER_AutoSaveLoadoutComponent - No CharacterController on owner!", LogLevel.WARNING);
			return;
		}
		
		m_controller.m_OnControlledByPlayer.Insert(OnControlledByPlayer);
		m_controller.m_OnPlayerDeath.Insert(OnPlayerDeath);
	}

	//------------------------------------------------------------------------
	
	protected void REAPER_SaveLoadoutDelayed()
	{
		m_playerId = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(m_character);
		
		if(m_playerId <= 0) return;
						
		Rpc(REAPER_RpcAsk_SetPlayerLoadout, m_playerId); // Call it on Authority		
	}
			
	//------------------------------------------------------------------------
	
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void REAPER_RpcAsk_SetPlayerLoadout(int playerId)
	{	
		GameEntity saveChar = GameEntity.Cast( GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId) );
				
		if(saveChar && m_arsenalManager) m_arsenalManager.REAPER_SavePlayerLoadout_S(playerId, saveChar);			
	}
		
	//------------------------------------------------------------------------
	
	override protected void OnDelete(IEntity owner)
   	{	
		super.OnDelete(owner);
	
		RemoveInvokedEvents()
	}
		
	//------------------------------------------------------------------------
	
	protected void RemoveInvokedEvents() 
	{
		if(SCR_Global.IsEditMode()) return;
		
		if(m_controller) {
			m_controller.m_OnControlledByPlayer.Remove(OnControlledByPlayer);
			m_controller.m_OnPlayerDeath.Remove(OnPlayerDeath);
		}
		
		if(m_InventoryManager) {
			m_InventoryManager.m_OnItemAddedInvoker.Remove(OnItemAddedListener);
			m_InventoryManager.m_OnItemRemovedInvoker.Remove(OnItemRemovedListener);	
		}
	}
	
	//------------------------------------------------------------------------
	
	void OnControlledByPlayer(IEntity owner, bool controlled)
	{		
		if (SCR_Global.IsEditMode()) return;
		
		SCR_ArsenalManagerComponent.GetArsenalManager(m_arsenalManager);
		if(!m_arsenalManager) return;
		
		if(!m_arsenalManager.REAPER_GetIsAutoSaveEnabled()) return;	
		
		m_character = ChimeraCharacter.Cast(SCR_PlayerController.GetLocalControlledEntity());
		if(!m_character) return;
		

		
		m_InventoryManager = SCR_InventoryStorageManagerComponent.Cast(owner.FindComponent(SCR_InventoryStorageManagerComponent));
		if(!m_InventoryManager) {
			Print("ERROR: REAPER_AutoSaveLoadoutComponent - No InventoryStorageManager on owner!", LogLevel.WARNING);
			return;
		}
		
		GetGame().GetCallqueue().CallLater(REAPER_InvokeInventoryMgrDelayed, 3000, false); // Call it later (because of Loadout spawn)				
	}
	

	//------------------------------------------------------------------------
	
	protected void REAPER_InvokeInventoryMgrDelayed()
	{
		m_InventoryManager.m_OnItemAddedInvoker.Insert(OnItemAddedListener);
		m_InventoryManager.m_OnItemRemovedInvoker.Insert(OnItemRemovedListener);		
	}
	
	//------------------------------------------------------------------------
	
	protected void OnPlayerDeath()
	{
		m_playerId = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(m_character);
		
		if(m_playerId <= 0) return;
					
		Rpc(REAPER_RpcAsk_SetPlayerLoadout, m_playerId); // Call it on Authority

		RemoveInvokedEvents();
	}
	
	
	//------------------------------------------------------------------------

	protected void OnItemAddedListener( IEntity item, notnull BaseInventoryStorageComponent storage )
	{	
		GetGame().GetCallqueue().Remove(REAPER_SaveLoadoutDelayed);
		GetGame().GetCallqueue().CallLater(REAPER_SaveLoadoutDelayed, m_saveLoadoutDelay * 1000, false);
	}
	
	//------------------------------------------------------------------------	
	
	protected void OnItemRemovedListener( IEntity item, notnull BaseInventoryStorageComponent storage )
	{		
		GetGame().GetCallqueue().Remove(REAPER_SaveLoadoutDelayed);
		GetGame().GetCallqueue().CallLater(REAPER_SaveLoadoutDelayed, m_saveLoadoutDelay * 1000, false);
	}
		
	//------------------------------------------------------------------------		
	
}