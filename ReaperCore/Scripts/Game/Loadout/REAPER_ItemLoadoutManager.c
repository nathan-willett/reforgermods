
// --------------------------------------------------------------------------------------------------------------------
// ------           This script is part of REAPER CORE | an Arma Reforger SP/COOP Modification                   ------
// ------   You are not allowed to use this script or parts of it in your mod or pass it off as your own work.   ------
// ------                                                                                                        ------
// ------                         Written by REAPER 2024 - www.reaper-as.de                                      ------
// --------------------------------------------------------------------------------------------------------------------

// This Class can be attached onto an item to handle its loadout behavior 
// This Class can also used for mission related Items that should not be saved in playerLoadout 
// Since 12.11.2024 this Component will now also handle the MobileSpawn!
// Restricted Items will be removed in SCR_PlayerArsenalLoadout (OnLoadoutSpawned)

[EntityEditorProps(category: "GameScripted/Items/", description: "Item Loadout Manager", color: "96 255 0 255")]

class REAPER_ItemLoadoutManagerClass: ScriptComponentClass {
};


class REAPER_ItemLoadoutManager: ScriptComponent 
{
	[Attribute("0", UIWidgets.CheckBox, "Should the owner Entity always deleted after player spawned? (Related if the item is saved in Loadout - Mobile Spawn for example)", category: "REAPER Item Loadout Manager")]
	protected bool m_deleteOnSpawn;
	
	[Attribute("1", UIWidgets.CheckBox, "Should this Entity only be deleted after player spawns for the first time? (Keys and Mission related Items can be handle here)", category: "REAPER Item Loadout Manager")]
	protected bool m_deleteOnlyOnFirstSpawn;
	
	[Attribute("1", UIWidgets.CheckBox, "Should the owner Entity dropped from the Inventory on character death? (will also work on AI)", category: "REAPER Item Loadout Manager")]
	protected bool m_dropOnDeath;	
	
	[Attribute("1", UIWidgets.CheckBox, "Should the owner Entity dropped from the Inventory while player disconnects?", category: "REAPER Item Loadout Manager")]
	protected bool m_dropOnDisconnect;	
	
	[Attribute("1", UIWidgets.CheckBox, "If owner is dropped while in ocean, should it move to a close beach?", category: "REAPER Item Loadout Manager")]
	protected bool m_MoveFromOcean;	
	
	[Attribute("1", UIWidgets.CheckBox, "Should the owner Entity withdraw from GarbageSystem?", category: "REAPER Item Loadout Manager")]
	protected bool m_WithdrawGarbageSystem;	
	
	
	[Attribute("0", UIWidgets.CheckBox, "Play Sound from SCR_SoundDataComponent while Entity is on ground", category: "REAPER Item Loadout Manager (Sound)")]
	protected bool m_PlaySoundWhileOnGround;
	
	[Attribute("onGround", UIWidgets.CheckBox, "Signal name of the sound to trigger in ACP sound file", category: "REAPER Item Loadout Manager (Sound)")]
	protected string m_SoundSignalName;
	
	
	[Attribute("0", UIWidgets.CheckBox, "Should we show a icon on the map while the item is dropped from inventory?", category: "REAPER Item Loadout Manager (Marking)")]
	protected bool m_ShowIconWhileOnGround;	
	
	[Attribute("", UIWidgets.ResourceNamePicker, "Icon (image) to show on the map", params: "edds", category: "REAPER Item Loadout Manager (Marking)")]
	protected ResourceName m_IconToShowOnMap;

	[Attribute("Item", UIWidgets.Auto, "Name/Caption to show below the Icon", category: "REAPER Item Loadout Manager (Marking)")]
	protected string m_IconCaption;	
	
	//----------------------------------------------------------------------------------------------
	
	protected ResourceName m_IconLayout = "{B767487388D09E01}UI/Layouts/REAPER_PlayerMapMarker.layout";	
	
	protected IEntity m_owner; 
	protected SCR_BaseGameMode m_GameMode;
	protected InventoryItemComponent m_itemComp;
	protected RplComponent m_rplComp;
	protected SCR_MapEntity m_MapEntity;
	protected Widget m_RootWidget;
	protected Widget m_MarkerWidget;
	
	//Character related
	protected ChimeraCharacter m_character;
	protected SCR_CharacterControllerComponent m_controller;
	protected SCR_InventoryStorageManagerComponent m_invStore;
	protected SCR_VehicleDamageManagerComponent m_vehDmgMgr;
	protected bool m_characterIsPlayer = false; 
	protected int m_characterPlayerId;
	
	//Water related vars 
	protected bool m_newPosFound = false; 
	protected vector m_newPos; 
	protected string m_entitySearchString = "Beach";
	
	[RplProp(onRplName: "OnRPLUpdated")]
	protected bool m_ItemIsOnGround = false;
	
	[RplProp(onRplName: "OnRPLUpdated")]
	protected bool m_ItemIsInVehicle = false;
	
	//----------------------------------------------------------------------------------------------
			
	override protected void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		
		if (SCR_Global.IsEditMode()) return;
					
		m_owner = owner;
				
		SetEventMask(owner, EntityEvent.INIT);
	}
	
	//------------------------------------------------------------------------------------------------
	
	override protected void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);
		
		m_GameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
			
		m_rplComp = RplComponent.Cast(owner.FindComponent(RplComponent)); 
		if(!m_rplComp) {
			Print("REAPER_ItemLoadoutManager - No RplComponent on owner entity!", LogLevel.WARNING);
		}
		
		if(m_ShowIconWhileOnGround) {
			m_MapEntity = SCR_MapEntity.GetMapInstance();	
			if(m_MapEntity) m_MapEntity.GetOnMapOpen().Insert(OnMapOpen);			
		}
		
		m_itemComp = InventoryItemComponent.Cast(owner.FindComponent(InventoryItemComponent)); 
		if(m_itemComp) m_itemComp.m_OnParentSlotChangedInvoker.Insert(OnParentSlotChanged);
		
		if(m_WithdrawGarbageSystem) REAPER_WithdrawGarbageSystem();		
	}
	
	//------------------------------------------------------------------------------------------------
	
	void OnParentSlotChanged(InventoryStorageSlot oldSlot, InventoryStorageSlot newSlot)
	{		
		if(newSlot) {
			ChimeraCharacter parentChar = ChimeraCharacter.Cast(newSlot.GetOwner());
			// OnCharacter
			if(parentChar) {
				
				m_character = parentChar; 
				m_controller = SCR_CharacterControllerComponent.Cast(parentChar.GetCharacterController());
				m_invStore = SCR_InventoryStorageManagerComponent.Cast(parentChar.FindComponent(SCR_InventoryStorageManagerComponent));
				
				if(m_vehDmgMgr) m_vehDmgMgr.GetOnDamageStateChanged().Remove(OnVehicleDamageStateChanged);
				
				if(EntityUtils.IsPlayer(parentChar)) {
																							
					m_characterIsPlayer = true; 
					m_characterPlayerId = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(parentChar);
					m_GameMode.GetOnPlayerDisconnected().Insert(OnPlayerDisconnect);
												
				} else {
					m_characterIsPlayer = false; 
				}
				
				if(m_rplComp && m_rplComp.IsMaster()) {	
					m_ItemIsOnGround = false;
					m_ItemIsInVehicle = false;
					REAPER_PlaySoundOnGround();
					Replication.BumpMe();
				}
				m_character.GetDamageManager().GetOnDamageStateChanged().Insert(OnDamageStateChanged);
				m_controller.GetOnPlayerDeathWithParam().Insert(OnPlayerDeath);	
				
			// InStorage					
			} else {
				
				// Check if the storage is a vehicle (if so, invoke the DamageMager) 
				Vehicle inVehicle = Vehicle.Cast(newSlot.GetOwner()); 
				if(inVehicle) {
					m_vehDmgMgr = SCR_VehicleDamageManagerComponent.Cast(inVehicle.FindComponent(SCR_VehicleDamageManagerComponent));
					if(m_vehDmgMgr) {
						m_ItemIsInVehicle = true;
						m_vehDmgMgr.GetOnDamageStateChanged().Insert(OnVehicleDamageStateChanged);
					}
				}
				
				if(m_characterIsPlayer && m_GameMode) m_GameMode.GetOnPlayerDisconnected().Remove(OnPlayerDisconnect);
				if(m_controller) m_controller.GetOnPlayerDeathWithParam().Remove(OnPlayerDeath);
				if(m_character) m_character.GetDamageManager().GetOnDamageStateChanged().Remove(OnDamageStateChanged);		
				m_character = null; 
				m_controller = null; 

				if(m_rplComp && m_rplComp.IsMaster()) {	
					m_ItemIsOnGround = false;		
					REAPER_PlaySoundOnGround();
					Replication.BumpMe();
				}		
			}
			
		// OnGround		
		} else {
			if(oldSlot) {
				m_owner.Update();
								
				if(m_characterIsPlayer && m_GameMode) m_GameMode.GetOnPlayerDisconnected().Remove(OnPlayerDisconnect);
				if(m_controller) m_controller.GetOnPlayerDeathWithParam().Remove(OnPlayerDeath);
				if(m_character) m_character.GetDamageManager().GetOnDamageStateChanged().Remove(OnDamageStateChanged);
				if(m_vehDmgMgr) m_vehDmgMgr.GetOnDamageStateChanged().Remove(OnVehicleDamageStateChanged);
					
				m_character = null; 
				m_controller = null; 
												
				if(m_rplComp && m_rplComp.IsMaster()) {			
					m_ItemIsOnGround = true;
					m_ItemIsInVehicle = false;
					REAPER_PlaySoundOnGround();
					Replication.BumpMe();
				}				
			}
		}
	
	}
	
	//------------------------------------------------------------------------------------------------
	
	protected void OnVehicleDamageStateChanged(EDamageState state)
	{
		if(state == EDamageState.DESTROYED) {
			if(m_vehDmgMgr) m_vehDmgMgr.GetOnDamageStateChanged().Remove(OnVehicleDamageStateChanged);
			
			if(m_dropOnDeath) REAPER_DropOwnerToGround();		
		}	
	}	

	//------------------------------------------------------------------------------------------------
	
 	protected void OnDamageStateChanged(EDamageState state)
	{		
		if(state == EDamageState.DESTROYED) {
			if(m_dropOnDeath) REAPER_DropOwnerToGround();
			
			if(m_character) m_character.GetDamageManager().GetOnDamageStateChanged().Remove(OnDamageStateChanged);
			if(m_controller) m_controller.GetOnPlayerDeathWithParam().Remove(OnPlayerDeath);						
		}
	}
	
	//------------------------------------------------------------------------------------------------
	
	protected void OnPlayerDeath(SCR_CharacterControllerComponent charController, IEntity instigatorEntity, notnull Instigator instigator)
	{		
		if(m_dropOnDeath) REAPER_DropOwnerToGround();
			
		if(m_characterIsPlayer && m_GameMode) m_GameMode.GetOnPlayerDisconnected().Remove(OnPlayerDisconnect);
		if(m_controller) m_controller.GetOnPlayerDeathWithParam().Remove(OnPlayerDeath);
		if(m_character) m_character.GetDamageManager().GetOnDamageStateChanged().Remove(OnDamageStateChanged);	
	}

	//------------------------------------------------------------------------------------------------
		
	protected void OnPlayerDisconnect(int playerID)
	{
		PlayerManager pMgr = GetGame().GetPlayerManager(); 
		if(pMgr) {
			IEntity playerEnt = pMgr.GetPlayerControlledEntity(playerID);
			
			if(playerEnt && playerEnt == m_character) {
				
				if(m_dropOnDisconnect) REAPER_DropOwnerToGround();
				
				if(m_characterIsPlayer && m_GameMode) m_GameMode.GetOnPlayerDisconnected().Remove(OnPlayerDisconnect);
				if(m_controller) m_controller.GetOnPlayerDeathWithParam().Remove(OnPlayerDeath);
			}
		}
	}	

	//------------------------------------------------------------------------------------------------	
	
	override event protected void OnDelete(IEntity owner)
	{	
		if(m_characterIsPlayer && m_GameMode) m_GameMode.GetOnPlayerDisconnected().Remove(OnPlayerDisconnect);
		if(m_controller) m_controller.GetOnPlayerDeathWithParam().Remove(OnPlayerDeath);
		if(m_character) m_character.GetDamageManager().GetOnDamageStateChanged().Remove(OnDamageStateChanged);
		if(m_itemComp) m_itemComp.m_OnParentSlotChangedInvoker.Remove(OnParentSlotChanged);
		
		super.OnDelete(owner);	
	}
	
	//------------------------------------------------------------------------------------------------	
	
	bool REAPER_GetItemIsOnGround() { return m_ItemIsOnGround; }
	bool REAPER_GetDeleteOnSpawn() { return m_deleteOnSpawn; }
	bool REAPER_GetDeleteOnFirstSpawn() { return m_deleteOnlyOnFirstSpawn; }
		
	//------------------------------------------------------------------------------------------------	
	
	protected void REAPER_DropOwnerToGround()
	{		
		if(m_invStore) {
				
			InventoryItemComponent InvComp = InventoryItemComponent.Cast(m_owner.FindComponent(InventoryItemComponent));

			if(InvComp) {
				InventoryStorageSlot parentSlot = InvComp.GetParentSlot();
				
				if(parentSlot) { 
					if(m_invStore.TryRemoveItemFromStorage(m_owner, parentSlot.GetStorage())) {
						m_owner.Update(); 
					} else {
						m_invStore.MoveItemToVicinity(m_owner);
						m_owner.Update(); 					
					}
									
					InvComp.ShowOwner();
										
					if(m_WithdrawGarbageSystem) REAPER_WithdrawGarbageSystem();
					
					if(m_MoveFromOcean)	REAPER_MoveOwnerFromOcean_S();
					
					if(m_rplComp && m_rplComp.IsMaster()) {
						m_ItemIsOnGround = true;
						m_ItemIsInVehicle = false;
						REAPER_PlaySoundOnGround();
						Replication.BumpMe();
					}					
				}			
			}
		}	
	}

	//------------------------------------------------------------------------------------------------	
	
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]	
	protected void REAPER_MoveOwnerFromOcean_S() {
	
		if(m_owner.GetRootParent() == m_owner) {
			
			if(m_rplComp && !m_rplComp.IsMaster()) return;
				
			BaseWorld world = GetGame().GetWorld();
			
			vector cTerrainTransform[4];
			m_owner.GetWorldTransform(cTerrainTransform);	
			SCR_TerrainHelper.SnapToTerrain(cTerrainTransform, world);
			
			EWaterSurfaceType surfaceType;
			float lakeArea;
			float waterHeight = SCR_WorldTools.GetWaterSurfaceY(world, cTerrainTransform[3], surfaceType, lakeArea);
					
			//Owner is in Ocean
			if(surfaceType == EWaterSurfaceType.WST_OCEAN) {
				
				//Find surface position 
				m_newPosFound = false;
				m_newPos = m_owner.GetOrigin();
				int currentRange = 100; 
				int maxRetrys = 10; 

				while(!m_newPosFound && maxRetrys > 0) {
					
					world.QueryEntitiesBySphere(cTerrainTransform[3], currentRange, REAPER_GetBeachPositionQuery, null, EQueryEntitiesFlags.STATIC);
					
					maxRetrys --;
					currentRange += 100;
				}
				
				//Move the Backpack out of the ocean
				if(m_newPosFound) {
					
					vector freePos;
					bool terrainIsFree = SCR_WorldTools.FindEmptyTerrainPosition(freePos, m_newPos, 3, 2);
					
					vector transformMat[4];
					if(terrainIsFree) {						
						freePos[2] = freePos[2] + 0.1;
						
						m_owner.SetOrigin(freePos);
						m_owner.Update();		
						Rpc(Rpc_SetOwnerOrigin_BCAST, freePos);
							
					} else {
						m_newPos[2] = m_newPos[2] + 0.1;
						
						m_owner.SetOrigin(m_newPos);
						m_owner.Update();		
						Rpc(Rpc_SetOwnerOrigin_BCAST, m_newPos);
					}				
				}
			}	
		}	
	}
	
	//------------------------------------------------------------------------------------------------	
	
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void Rpc_SetOwnerOrigin_BCAST(vector origin)
	{	
		m_owner.SetOrigin(origin);
		m_owner.Update();	
	}	
	
	//------------------------------------------------------------------------------------------------
	
	protected bool REAPER_GetBeachPositionQuery(IEntity ent)
	{	
		if(ent.GetPrefabData() && ent.GetPrefabData().GetPrefabName().Contains(m_entitySearchString)) {
			
			bool isUnderwater = SCR_WorldTools.IsObjectUnderwater(ent);	
			
			if(!isUnderwater) {
				m_newPos = ent.GetOrigin(); 
				m_newPosFound = true; 
				return false;			
			}	
		}
		// Keep looking
		return true;
	}	
	
	//------------------------------------------------------------------------------------------------	
	
	protected void REAPER_PlaySoundOnGround()
	{
		if(m_PlaySoundWhileOnGround && m_ItemIsOnGround) {
			SignalsManagerComponent signalMgr = SignalsManagerComponent.Cast(m_owner.FindComponent(SignalsManagerComponent));
			if (signalMgr) signalMgr.SetSignalValue(signalMgr.AddOrFindSignal(m_SoundSignalName), 1);		
		} else {
			SignalsManagerComponent signalMgr = SignalsManagerComponent.Cast(m_owner.FindComponent(SignalsManagerComponent));
			if (signalMgr) signalMgr.SetSignalValue(signalMgr.AddOrFindSignal(m_SoundSignalName), 0);			
		}		
	}
	
	//------------------------------------------------------------------------------------------------	
	
	protected void REAPER_WithdrawGarbageSystem() 
	{
		ChimeraWorld world = ChimeraWorld.CastFrom(GetGame().GetWorld());
		if(world) {
			GarbageSystem garbageSys = world.GetGarbageSystem();
			if(garbageSys) {										 
				garbageSys.UpdateBlacklist(m_owner, true);
				garbageSys.Withdraw(m_owner);
			}
		}	
	}
	
	//------------------------------------------------------------------------------------------------
	// Handle MapIcon below
	//------------------------------------------------------------------------------------------------
	
	void OnMapOpen(MapConfiguration config)
	{
		if(!m_ItemIsOnGround && !m_ItemIsInVehicle) return; 
		
		m_RootWidget = config.RootWidgetRef;
		
		m_MarkerWidget = GetGame().GetWorkspace().CreateWidgets(m_IconLayout, m_RootWidget);
		ImageWidget wIcon = ImageWidget.Cast(m_MarkerWidget.FindAnyWidget("Icon"));
		TextWidget wName = TextWidget.Cast(m_MarkerWidget.FindAnyWidget("Name"));
		
		if(wName) wName.SetText(m_IconCaption);	
		if(wIcon) wIcon.LoadImageTexture(0, m_IconToShowOnMap);
				
		REAPER_UpdateMarkerPosition();
		
		m_MapEntity.GetOnMapPan().Insert(OnMapPan);
		m_MapEntity.GetOnMapPanEnd().Insert(OnMapPanEnd);	
		m_MapEntity.GetOnMapClose().Insert(OnMapClose);	
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnMapPan(float x, float y, bool adjustedPan) { REAPER_UpdateMarkerPosition(); }
	//------------------------------------------------------------------------------------------------
	protected void OnMapPanEnd(float x, float y) { REAPER_UpdateMarkerPosition(); }	
	//------------------------------------------------------------------------------------------------
		
	protected void OnMapClose(MapConfiguration config)
	{		
		m_MarkerWidget.RemoveFromHierarchy();
		m_MapEntity.GetOnMapPan().Remove(OnMapPan);
		m_MapEntity.GetOnMapPanEnd().Remove(OnMapPanEnd);
		m_MapEntity.GetOnMapClose().Remove(OnMapClose);
	}
	
	//------------------------------------------------------------------------------------------------
		
	protected void REAPER_UpdateMarkerPosition()
	{
		if(m_MarkerWidget) {
			vector pos = m_owner.GetOrigin();
		
			float x, y;
			m_MapEntity.WorldToScreen(pos[0], pos[2], x, y, true);
		
			x = GetGame().GetWorkspace().DPIUnscale(x);
			y = GetGame().GetWorkspace().DPIUnscale(y);
				
			FrameSlot.SetPos(m_MarkerWidget, x, y);		
		}
	}
	
	//------------------------------------------------------------------------------------------------	
	
	//----------------------------------
	//----------  RPL STUFF ------------
	//----------------------------------
		
	override bool RplSave(ScriptBitWriter writer)
	{
		super.RplSave(writer);
		
		writer.WriteBool(m_ItemIsOnGround);	
		writer.WriteBool(m_ItemIsInVehicle);				
		return true;
	}
	
	//------------------------------------------------------------------------------------------------	
	
	override bool RplLoad(ScriptBitReader reader)
	{
		super.RplLoad(reader);
		
		reader.ReadBool(m_ItemIsOnGround);
		reader.ReadBool(m_ItemIsInVehicle);
		REAPER_PlaySoundOnGround();				
		return true;
	}
	
	//------------------------------------------------------------------------------------------------	
	
	protected void OnRPLUpdated()
	{
		REAPER_PlaySoundOnGround();
	}				
}