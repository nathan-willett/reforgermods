

// --------------------------------------------------------------------------------------------------------------------
// ------           This script is part of REAPER CORE | an Arma Reforger SP/COOP Modification                   ------
// ------   You are not allowed to use this script or parts of it in your mod or pass it off as your own work.   ------
// ------                                                                                                        ------
// ------                         Written by REAPER 2024 - www.reaper-as.de                                      ------
// --------------------------------------------------------------------------------------------------------------------


modded class SCR_ArsenalManagerComponent : SCR_BaseGameModeComponent
{	
	[Attribute("1", UIWidgets.CheckBox, desc: "Auto save players loadout", category: "REAPER Loadout Management")]
	protected bool m_EnableAutoSave;

	[Attribute("0", UIWidgets.CheckBox, desc: "Save players Loadout on server", category: "REAPER Loadout Management")]
	protected bool m_SaveLoadoutDataOnServer;

	[Attribute("default", UIWidgets.EditBox, desc: "Prefix for this Mission to save Loadouts", category: "REAPER Loadout Management")]
	protected string m_SaveFilePrefix;
					
	[Attribute("1", UIWidgets.CheckBox, desc: "Notice player if Loadout is saved", category: "REAPER Loadout Management")]
	protected bool m_NoticePlayerOnSave;
	
	[Attribute(UIWidgets.Auto, desc: "Do not save the following Prefabs", params: "et", category: "REAPER Loadout Management")]  //Restricted Items will not saved
	protected ref array<ResourceName> m_RestrictedPrefabs;	
		
	protected SCR_PopUpNotification m_popup;
	protected string m_rootPath = "$profile:/REAPER/Loadouts";
	protected string m_defaultIdent = "0000-DEV";
	
	protected bool m_localPlayerHaveLoadout = false;
	
	//-------------------------------------------------------------------------------------
	//------------ OnPlayerRegistered ( called after Player connected  --------------------
	//-------------------------------------------------------------------------------------
	
	override protected void OnPlayerRegistered(int playerId)
	{
		super.OnPlayerRegistered(playerId);
		
		if(!Replication.IsServer()) return;
		
		GetGame().GetCallqueue().CallLater(REAPER_InitLoadoutData, 500, false, playerId); // Calllater to receive PlayerIdent	
	}
	
	//-------------------------------------------------------------------------------------
	
	bool REAPER_GetLocalPlayerHaveLoadout() { return m_localPlayerHaveLoadout; }
		
	//----------------------------------------------------------------------
	//------------ REAPER_InitLoadoutData (Load files)  --------------------
	//----------------------------------------------------------------------
	
	protected void REAPER_InitLoadoutData(int playerId)
	{	
		if(!m_SaveLoadoutDataOnServer) return; 
			
		Print("REAPER: SCR_ArsenalManagerComponent - REAPER_InitLoadoutData");
		
		//Check player IdentID, is Empty in Devmode!
		string playerUID = GetGame().GetBackendApi().GetPlayerIdentityId(playerId);

		if(playerUID.IsEmpty()) {
			if(GetGame().IsDev()) {
				playerUID = m_defaultIdent
			} else {
				Print("SCR_ArsenalManagerComponent - REAPER_InitLoadoutData - No PlayerIdent given!", LogLevel.WARNING);
			}
		}
			
		if(m_aPlayerLoadouts.Contains(playerUID)) m_aPlayerLoadouts.Remove(playerUID);
		
		//Check Loadoutfiles on Server 
		if(m_SaveFilePrefix.IsEmpty()) m_SaveFilePrefix = "default";
		
		string arsenalFile = string.Format("%1/S_%2_%3", m_rootPath, playerUID, m_SaveFilePrefix);
		string playerDataFile = string.Format("%1/C_%2_%3", m_rootPath, playerUID, m_SaveFilePrefix);
		
		if(!FileIO.FileExists(arsenalFile) || !FileIO.FileExists(playerDataFile)) return;
		
		//Get ArsenalLoadout and PlayerLoadoutData from Files
		SCR_ArsenalPlayerLoadout playerLoadout = REAPER_GetArsenalLoadoutFromFile(playerUID, arsenalFile);
		SCR_PlayerLoadoutData loadoutData = REAPER_GetPlayerLoadoutFromFile(playerUID, playerDataFile);
		
		if(!playerLoadout || !loadoutData) return;
		
		m_aPlayerLoadouts.Set(playerUID, playerLoadout); // Add Arsenal loadout on server
		
		playerLoadout.loadoutData = loadoutData;
		
		DoSendPlayerLoadout(playerId, loadoutData);
		Rpc(DoSendPlayerLoadout, playerId, loadoutData);	
		
		REAPER_SetPlayerHasLoadout(playerId, true, true, false);
		Rpc(REAPER_SetPlayerHasLoadout, playerId, true, true, false);
		
		Print("REAPER: SCR_ArsenalManagerComponent - REAPER_InitLoadoutData - Data loaded for PlayerID: " + playerId);
	}
	
	//---------------------------------------------------------------------------------
	//------------------ GetPlayerArsenalLoadout fix for DEV --------------------------
	//---------------------------------------------------------------------------------
	
	override bool GetPlayerArsenalLoadout(string playerUID, out SCR_ArsenalPlayerLoadout playerLoadout)
	{
		if(playerUID.IsEmpty()) {
			if(GetGame().IsDev()) {
				playerUID = m_defaultIdent
			} else {
				Print("REAPER: SCR_ArsenalManagerComponent - GetPlayerArsenalLoadout - No PlayerIdent given!", LogLevel.WARNING);
			}
		}			
		return m_aPlayerLoadouts.Find(playerUID, playerLoadout) && playerLoadout.loadout != string.Empty;
	}	
	
	//----------------------------------------------------------------------------------------
	//-------------------- Load ArsenalLoadout from File (if exists)  ------------------------
	//----------------------------------------------------------------------------------------
	
	SCR_ArsenalPlayerLoadout REAPER_GetArsenalLoadoutFromFile(string playerIdent, string filename) {
					
		SCR_JsonLoadContext context = new SCR_JsonLoadContext();		
		SCR_ArsenalPlayerLoadout ArsenalLoadout = new SCR_ArsenalPlayerLoadout();
		
		if(!context.LoadFromFile(filename)) {
			Print("REAPER_GetArsenalLoadoutFromFile - Can not load File!", LogLevel.WARNING);
			return null;
		}
		
		if(!context.ReadValue("", ArsenalLoadout)) {
			Print("REAPER_GetArsenalLoadoutFromFile - Failed to load data from file!", LogLevel.WARNING);
			return null;
		}		
		return ArsenalLoadout;
	}
	
	//----------------------------------------------------------------------------------------
	//-------------------- Load PlayerLoadout from File (if exists)  ------------------------
	//----------------------------------------------------------------------------------------
	
	SCR_PlayerLoadoutData REAPER_GetPlayerLoadoutFromFile(string playerIdent, string filename) {
						
		SCR_JsonLoadContext context = new SCR_JsonLoadContext();		
		SCR_PlayerLoadoutData loadoutData = new SCR_PlayerLoadoutData();
		
		if (!context.LoadFromFile(filename)) {
			Print("REAPER_GetPlayerLoadoutFromFile - Can not load File!", LogLevel.WARNING);
			return null;
		}
		
		if (!context.ReadValue("", loadoutData)) {
			Print("REAPER_GetPlayerLoadoutFromFile - Failed to load data from file!", LogLevel.WARNING);
			return null;
		}		
		return loadoutData;
	}
	
	//---------------------------------------------------------------------------------------------
	//----------------------------- Save LoadoutData to Files -------------------------------------
	//---------------------------------------------------------------------------------------------
	
	protected void REAPER_SaveLoadoutToFile(string playerIdent, SCR_ArsenalPlayerLoadout playerLoadout, SCR_PlayerLoadoutData loadoutData)
	{		
		string arsenalFile = string.Format("%1/S_%2_%3", m_rootPath, playerIdent, m_SaveFilePrefix);
		string playerDataFile = string.Format("%1/C_%2_%3", m_rootPath, playerIdent, m_SaveFilePrefix);
		
		if(!FileIO.MakeDirectory(m_rootPath)) {
			Print("REAPER: SCR_ArsenalManagerComponent - REAPER_SaveLoadoutToFile - Can not create Folder!", LogLevel.ERROR);
			return;
		}
		
		SCR_JsonSaveContext contextA = new SCR_JsonSaveContext;
		contextA.WriteValue("", playerLoadout);
		contextA.SaveToFile(arsenalFile);
		
		SCR_JsonSaveContext contextB = new SCR_JsonSaveContext;
		contextB.WriteValue("", loadoutData);
		contextB.SaveToFile(playerDataFile);
	}
	
	//------------------------------------------------------------------------------------------------
	//----------------------------- Authority SavePlayer Loadout -------------------------------------
	//------------------------------------------------------------------------------------------------
	
	void REAPER_SavePlayerLoadout_S(int playerId, GameEntity characterEntity)
	{		
		if (!GetGameMode().IsMaster()) return;
		
		if (playerId <= 0) return;
		
		string playerUID = GetGame().GetBackendApi().GetPlayerIdentityId(playerId);
		if(playerUID.IsEmpty()) {
			if(GetGame().IsDev()) {
				Print("REAPER: SCR_ArsenalManagerComponent - REAPER_SavePlayerLoadout_S - Using DEV Ident!", LogLevel.WARNING);
				playerUID = m_defaultIdent;
			} else {
				Print("REAPER: SCR_ArsenalManagerComponent - REAPER_SavePlayerLoadout_S - No PlayerIdent given!", LogLevel.WARNING);
				return;
			}
		}		
		
		if(!characterEntity) {
			DoSetPlayerLoadout(playerId, string.Empty, characterEntity, SCR_EArsenalSupplyCostType.DEFAULT);
			return;
		}
		
		SCR_PlayerController clientPlayerController = SCR_PlayerController.Cast(GetGame().GetPlayerManager().GetPlayerController(playerId));
		if (!clientPlayerController || clientPlayerController.IsPossessing()) return;				
		
		string factionKey = SCR_PlayerArsenalLoadout.ARSENALLOADOUT_FACTIONKEY_NONE;
		
		FactionAffiliationComponent factionAffiliation = FactionAffiliationComponent.Cast(characterEntity.FindComponent(FactionAffiliationComponent));
		if(factionAffiliation) factionKey = factionAffiliation.GetAffiliatedFaction().GetFactionKey();
				
		SCR_JsonSaveContext context = new SCR_JsonSaveContext();
		if(!context.WriteValue(SCR_PlayerArsenalLoadout.ARSENALLOADOUT_FACTION_KEY, factionKey) || !context.WriteValue(SCR_PlayerArsenalLoadout.ARSENALLOADOUT_KEY, characterEntity)) return;
							
		SCR_ArsenalPlayerLoadout playerLoadout = m_aPlayerLoadouts.Get(playerUID);
		if(!playerLoadout) playerLoadout = new SCR_ArsenalPlayerLoadout();
		
		string loadoutString = context.ExportToString();	
		playerLoadout.loadout = loadoutString;
		
		m_aPlayerLoadouts.Set(playerUID, playerLoadout);
		
		SCR_PlayerLoadoutData loadoutData = GetPlayerLoadoutData(characterEntity);
		
		playerLoadout.loadoutData = loadoutData;
		
		DoSendPlayerLoadout(playerId, loadoutData);
		Rpc(DoSendPlayerLoadout, playerId, loadoutData);
		
		bool loadoutValid = !loadoutString.IsEmpty();
		bool loadoutChanged = true;
			
		REAPER_SetPlayerHasLoadout(playerId, loadoutValid, loadoutChanged, m_NoticePlayerOnSave);
		Rpc(REAPER_SetPlayerHasLoadout, playerId, loadoutValid, loadoutChanged, m_NoticePlayerOnSave);
		
		//Save Data to local files
		if(m_SaveLoadoutDataOnServer) REAPER_SaveLoadoutToFile(playerUID, playerLoadout, loadoutData);
		
		if(m_NoticePlayerOnSave) m_NoticePlayerOnSave = false; //only notice one time
	}
	
	//------------------------------------------------------------------------------------------------
	//---------------------------- Set Player Has Loadout (modded) -----------------------------------
	//------------------------------------------------------------------------------------------------
	
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void REAPER_SetPlayerHasLoadout(int playerId, bool loadoutValid, bool loadoutChanged, bool notifyPlayer)
	{	
		if (playerId == SCR_PlayerController.GetLocalPlayerId()) {
			
			if (m_bLocalPlayerLoadoutAvailable != loadoutValid || loadoutChanged) {
				
				if(notifyPlayer) {
					m_popup = SCR_PopUpNotification.GetInstance();		
					if(m_popup) m_popup.PopupMsg("REAPER AutoSave...", 5, "Your current loadout has been saved!");			
				}
				
				m_localPlayerHaveLoadout = true; 
				
				Print("REAPER_SetPlayerHasLoadout - Loadout Saved!");
				
			} else { 
				Print("REAPER_SetPlayerHasLoadout - Error: Loadout not Saved!");
			}
			
			m_bLocalPlayerLoadoutAvailable = loadoutValid;
		}
		m_OnPlayerLoadoutUpdated.Invoke(playerId, loadoutValid);
	}
	
	//------------------------------------------------------------------------------------------------
	
	override protected SCR_PlayerLoadoutData GetPlayerLoadoutData(GameEntity characterEntity)
	{
		SCR_PlayerLoadoutData loadoutData();
		
		//Save current CharacterPrefab
		loadoutData.characterPrefab = characterEntity.GetPrefabData().GetPrefab().GetResourceName();
		
		EquipedLoadoutStorageComponent loadoutStorage = EquipedLoadoutStorageComponent.Cast(characterEntity.FindComponent(EquipedLoadoutStorageComponent));
		if (loadoutStorage) {
			int slotsCount = loadoutStorage.GetSlotsCount();
			for (int i = 0; i < slotsCount; ++i)
			{
				InventoryStorageSlot slot = loadoutStorage.GetSlot(i);
				if(!slot) continue;
				
				IEntity attachedEntity = slot.GetAttachedEntity();
				if (!attachedEntity) continue;
				
				ResourceName prefabName;
				BaseContainer prefab = attachedEntity.GetPrefabData().GetPrefab();
				while (prefabName.IsEmpty() && prefab)
				{
					prefabName = prefab.GetResourceName();
					prefab = prefab.GetAncestor();
				}
				
				if (prefabName.IsEmpty()) continue;
				
				SCR_ClothingLoadoutData clothingData();
				clothingData.SlotIdx = i;
				clothingData.ClothingPrefab = prefabName;
				
				loadoutData.Clothings.Insert(clothingData);
			}
		}
		
		EquipedWeaponStorageComponent weaponStorage = EquipedWeaponStorageComponent.Cast(characterEntity.FindComponent(EquipedWeaponStorageComponent));
		if (weaponStorage) {
			int slotsCount = weaponStorage.GetSlotsCount();
			for (int i = 0; i < slotsCount; ++i)
			{
				InventoryStorageSlot slot = weaponStorage.GetSlot(i);
				if(!slot) continue;
				
				IEntity attachedEntity = slot.GetAttachedEntity();
				if(!attachedEntity) continue;
				
				ResourceName prefabName;
				BaseContainer prefab = attachedEntity.GetPrefabData().GetPrefab();
				while (prefabName.IsEmpty() && prefab)
				{
					prefabName = prefab.GetResourceName();
					prefab = prefab.GetAncestor();
				}
				
				if (prefabName.IsEmpty()) continue;
				
				SCR_WeaponLoadoutData weaponData();
				weaponData.SlotIdx = i;
				weaponData.WeaponPrefab = prefabName;
				
				loadoutData.Weapons.Insert(weaponData);
			}
			
			// REAPER: Get Primary weapon attachments 
			InventoryStorageSlot wSlot = weaponStorage.GetSlot(0);
			if(wSlot && wSlot.GetAttachedEntity()) {
				
				BaseWeaponComponent pwComp = BaseWeaponComponent.Cast(wSlot.GetAttachedEntity().FindComponent(BaseWeaponComponent));
				if(pwComp) {
	
					array<GenericComponent> attachments = {};
					pwComp.FindComponents(AttachmentSlotComponent, attachments);
		
					int attachmentSlots = attachments.Count();				
					for (int i = 0; i < attachmentSlots; ++i) {
						
						AttachmentSlotComponent aSlot = AttachmentSlotComponent.Cast(attachments[i]);				
						IEntity attachedEntity = aSlot.GetAttachedEntity();
						ResourceName attachmentResource = string.Empty;
						
						if(attachedEntity) attachmentResource = attachedEntity.GetPrefabData().GetPrefab().GetResourceName();
						
						// ------- OLD METHOD --------
						//if(!attachedEntity) continue;
						
						//ResourceName prefabName;
						//BaseContainer prefab = attachedEntity.GetPrefabData().GetPrefab();
						//while (prefabName.IsEmpty() && prefab)
						//{
						//	prefabName = prefab.GetResourceName();
						//	prefab = prefab.GetAncestor();
						//}					
						//if(prefabName.IsEmpty()) continue;
						
						SCR_WeaponAttachmentLoadoutData attachmentData();
						attachmentData.SlotIdx = i;
						attachmentData.AttachmentPrefab = attachmentResource;			
						loadoutData.Attachments.Insert(attachmentData);
					}
					
					//Fix for suppressors in Reforger 1.2.1
					BaseMuzzleComponent muzzleComp = BaseMuzzleComponent.Cast(pwComp.FindComponent(BaseMuzzleComponent));
					if(muzzleComp) {
											
						array<GenericComponent> attachmentMuzzles = {};
						muzzleComp.FindComponents(AttachmentSlotComponent, attachmentMuzzles);
											
						int muzzleSlots = attachmentMuzzles.Count();
						for (int i = 0; i < muzzleSlots; ++i) {
							
							AttachmentSlotComponent muzzleSlot = AttachmentSlotComponent.Cast(attachmentMuzzles[i]);					
							IEntity attachedMuzzleEnt = muzzleSlot.GetAttachedEntity();
							ResourceName muzzleResource = string.Empty;
							
							if(attachedMuzzleEnt) muzzleResource = attachedMuzzleEnt.GetPrefabData().GetPrefab().GetResourceName();
							
							// ------- OLD METHOD --------
							//if(!attachedMuzzleEnt) continue;
							
							//ResourceName muzzleResource;
							//BaseContainer prefab = attachedMuzzleEnt.GetPrefabData().GetPrefab();
							//while (muzzleResource.IsEmpty() && prefab)
							//{
							//	muzzleResource = prefab.GetResourceName();
							//	prefab = prefab.GetAncestor();
							//}		
							//if(muzzleResource.IsEmpty()) continue;
							
							SCR_WeaponAttachmentMuzzleLoadoutData attachmentData();
							attachmentData.SlotIdx = i;
							attachmentData.AttachmentMuzzlePrefab = muzzleResource;		
							loadoutData.AttachmentMuzzles.Insert(attachmentData);				
						}						
					}					
				}			
			}

		}
		
		return loadoutData;
	}

	//----------------------------------------------------------------------------------
	
	bool REAPER_GetIsAutoSaveEnabled() {return m_EnableAutoSave; }
	bool REAPER_GetIsNoticePlayerEnabled() {return m_NoticePlayerOnSave; }
}






//----------------------------------------------------------------
//-------------- Modded SCR_PlayerArsenalLoadout -----------------
//--------- check restriced items while loadout spawned ----------
//----------------------------------------------------------------

[BaseContainerProps(configRoot: true), BaseContainerCustomTitleField("m_sLoadoutName")]
modded class SCR_PlayerArsenalLoadout : SCR_FactionPlayerLoadout
{	
	override void OnLoadoutSpawned(GenericEntity pOwner, int playerId)
	{
		GameEntity playerEntity = GameEntity.Cast(pOwner);
		
		// ------------------------------- REAPER ---------------------------------
		// Delete all Weapons from the new spawned PlayerEntity! 
		// Otherwise we get Problems with the Spawned Gloves from the Loadout!
		// ------------------------------------------------------------------------
		SCR_InventoryStorageManagerComponent invMgrComp = SCR_InventoryStorageManagerComponent.Cast(playerEntity.FindComponent(SCR_InventoryStorageManagerComponent));
		ChimeraCharacter char = ChimeraCharacter.Cast(playerEntity);
		
		if(invMgrComp && char) {
			array<IEntity> cWeapons = {};
			char.GetWeaponManager().GetWeaponsList(cWeapons);
			foreach(IEntity weap : cWeapons) {
				invMgrComp.TryDeleteItem(weap);
				if(weap) SCR_EntityHelper.DeleteEntityAndChildren(weap);
			}
		}
			
		// Lets spawn the Loadout by super method	
		super.OnLoadoutSpawned(pOwner, playerId);
		
		// Check restricted items from REAPER_ItemLoadoutManager
		SCR_DataCollectorComponent dataCollector = GetGame().GetDataCollector();
		array<IEntity> cItems = {};
		invMgrComp.GetItems(cItems);
		foreach(IEntity item : cItems) {
			
			REAPER_ItemLoadoutManager loadoutMgr = REAPER_ItemLoadoutManager.Cast(item.FindComponent(REAPER_ItemLoadoutManager));
			if(loadoutMgr) {
				
				if(loadoutMgr.REAPER_GetDeleteOnSpawn()) {
					
					SCR_EntityHelper.DeleteEntityAndChildren(item);
					if(item) delete item;
					
				} else if(loadoutMgr.REAPER_GetDeleteOnFirstSpawn()) {
					
					if(dataCollector) {
						int sCount = dataCollector.REAPER_GetSpawnCount(playerId);
						if(sCount < 2) { // SpawnCount = 1 on the first 
							SCR_EntityHelper.DeleteEntityAndChildren(item); 
							if(item) delete item;
						} 
					}	
				}
			}
		}		
	}
	


}	






