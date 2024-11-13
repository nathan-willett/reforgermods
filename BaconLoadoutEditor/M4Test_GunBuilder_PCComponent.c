enum Bacon_GunBuilderUI_ResponseType {
	ADD_INVENORY_ITEM,
	SWAP_ATTACHMENT,
	LOADOUT
}

enum Bacon_GunBuilderUI_ActionType {
	ADD_ITEM,
	REMOVE_ITEM,
	REPLACE_ITEM,
	GET_LOADOUTS,
	SAVE_LOADOUT,
	CLEAR_LOADOUT,
	APPLY_LOADOUT,
	GET_ADMIN_LOADOUTS,
	SAVE_LOADOUT_ADMIN,
	APPLY_LOADOUT_ADMIN,
	SET_AI_LOADOUT_ADMIN,
	CLEAR_LOADOUT_ADMIN
}

// requests
class Bacon_GunBuilderUI_Network_Request {
	Bacon_GunBuilderUI_ActionType actionType;
	
	string Repr() {
		return string.Format("action: %1", SCR_Enum.GetEnumName(Bacon_GunBuilderUI_ActionType, actionType));
	}
}
class Bacon_GunBuilderUI_Network_StorageRequest: Bacon_GunBuilderUI_Network_Request {
	RplId storageRplId;
	int storageSlotId = -1;
	ResourceName prefab = "";
	
	override string Repr() {
		return string.Format("action: %1, rplId: %2, slotId: %3, prefab: %4", SCR_Enum.GetEnumName(Bacon_GunBuilderUI_ActionType, actionType), storageRplId, storageSlotId, prefab);
	}
}
class Bacon_GunBuilderUI_Network_LoadoutRequest: Bacon_GunBuilderUI_Network_Request {
	int loadoutSlotId = -1;
	RplId arsenalComponentRplId;
}

// responses
class Bacon_GunBuilderUI_Network_Response {
	bool success;
	string message;
	ref Bacon_GunBuilderUI_Network_Request request;
}

class Bacon_GunBuilderUI_PlayerControllerComponentClass: ScriptComponentClass {};

class Bacon_GunBuilderUI_PlayerControllerComponent: ScriptComponent {
	ref ScriptInvoker m_OnSwapRequestResponseRplId = new ScriptInvoker();
	ref ScriptInvoker m_OnResponse_Storage = new ScriptInvoker();
	ref ScriptInvoker m_OnResponse_Loadout = new ScriptInvoker();
	
	Bacon_GunBuilder_LoadoutStorageComponent m_LoadoutStorageComponent;
	SCR_MapMarkerEntrySquadLeader m_MapMarkerEntrySquadLeader;
	
	PlayerManager m_playerManager;
	SCR_BaseGameMode m_gameMode;
	PlayerController m_PC;
	
	static Bacon_GunBuilderUI_PlayerControllerComponent LocalInstance;
	static Bacon_GunBuilderUI_PlayerControllerComponent ServerInstance;

	static ref array<ref Bacon_GunBuilder_PlayerLoadout> AdminLoadoutMetadata = {};
	
	override void OnPostInit(IEntity owner) {
		SetEventMask(owner, EntityEvent.INIT);
		m_PC = PlayerController.Cast(owner);
	};
	
	override void EOnInit(IEntity owner) {
		if (m_PC.GetPlayerId() == SCR_PlayerController.GetLocalPlayerId()) {
			LocalInstance = this;
		}
		
		m_playerManager = GetGame().GetPlayerManager();
		
		if (!Replication.IsServer()) {
			GetGame().GetCallqueue().CallLater(AskForLoadouts, 100, false);
			return;
		}
		
		if (SCR_PlayerController.GetLocalPlayerId() == 0)
			ServerInstance = this;
		
		m_gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (!m_gameMode) {
			Print("Bacon_GunBuilderUI_PlayerControllerComponent.EOnInit | Failed to obtain game mode information", LogLevel.WARNING);
			return;
		}

		m_LoadoutStorageComponent = Bacon_GunBuilder_LoadoutStorageComponent.Cast(GetGame().GetGameMode().FindComponent(Bacon_GunBuilder_LoadoutStorageComponent));
		UpdateServerLoadouts();
		
		SCR_MapMarkerManagerComponent markerManager = SCR_MapMarkerManagerComponent.GetInstance();
		if (!markerManager) {
			Print("Bacon_GunBuilderUI_PlayerControllerComponent.EOnInit | SCR_MapMarkerManagerComponent is null", LogLevel.WARNING);
			return;
		}
			
		SCR_MapMarkerConfig markerConfig = markerManager.GetMarkerConfig();
		if (!markerConfig) {
			Print("Bacon_GunBuilderUI_PlayerControllerComponent.EOnInit | SCR_MapMarkerConfig is null", LogLevel.WARNING);
			return;
		}
			
		m_MapMarkerEntrySquadLeader = SCR_MapMarkerEntrySquadLeader.Cast(markerConfig.GetMarkerEntryConfigByType(SCR_EMapMarkerType.SQUAD_LEADER));
	};

	void UpdateServerLoadouts() {
		AdminLoadoutMetadata.Clear();
		m_LoadoutStorageComponent.GetPlayerLoadoutMetadata(0, "", "admin", AdminLoadoutMetadata, true);

		// Rpc(RpcDo_UpdateLoadouts, loadoutsJson);
	}
	
	void AskForLoadouts() {
		Rpc(RpcAsk_LoadoutsPlease);
	}
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RpcAsk_LoadoutsPlease() {
		SCR_JsonSaveContext saveContext = new SCR_JsonSaveContext();
		saveContext.WriteValue("", AdminLoadoutMetadata);
		
		string loadoutsJson = saveContext.ExportToString();
		
		Rpc(RpcDo_UpdateLoadoutsOwner, loadoutsJson);
	}
	
	void BroadcastLoadoutChange() {
		SCR_JsonSaveContext saveContext = new SCR_JsonSaveContext();
		saveContext.WriteValue("", AdminLoadoutMetadata);
		
		string loadoutsJson = saveContext.ExportToString();
		Rpc(RpcDo_UpdateLoadoutsBroadcast, loadoutsJson);
	}

	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	void RpcDo_UpdateLoadoutsOwner(string json) {
		SCR_JsonLoadContext loadContext = new SCR_JsonLoadContext();
		loadContext.ImportFromString(json);
		loadContext.ReadValue("", AdminLoadoutMetadata);
	}

	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RpcDo_UpdateLoadoutsBroadcast(string json) {
		SCR_JsonLoadContext loadContext = new SCR_JsonLoadContext();
		loadContext.ImportFromString(json);
		loadContext.ReadValue("", AdminLoadoutMetadata);
	}
	
	SCR_InventoryStorageManagerComponent GetPlayerInventoryManager(int playerId) {
		IEntity character = m_playerManager.GetPlayerControlledEntity(playerId);
		if (!character)
			return null;
		
		return SCR_InventoryStorageManagerComponent.Cast(character.FindComponent(SCR_InventoryStorageManagerComponent));
	}
	
	BaseInventoryStorageComponent GetPlayerStorage(Bacon_GunBuilderUI_StorageType storageType, int playerId) {
		IEntity character = m_playerManager.GetPlayerControlledEntity(playerId);
		
		if (storageType == Bacon_GunBuilderUI_StorageType.CHARACTER_LOADOUT)
			return BaseInventoryStorageComponent.Cast(character.FindComponent(SCR_CharacterInventoryStorageComponent));
		
		if (storageType == Bacon_GunBuilderUI_StorageType.CHARACTER_WEAPON)
			return BaseInventoryStorageComponent.Cast(character.FindComponent(EquipedWeaponStorageComponent));
		
		return null;
	}
	
	BaseInventoryStorageComponent GetPlayerWeaponStorage(int playerId, int weaponSlotId) {
		IEntity character = m_playerManager.GetPlayerControlledEntity(playerId);

		BaseInventoryStorageComponent weaponStorage = BaseInventoryStorageComponent.Cast(character.FindComponent(EquipedWeaponStorageComponent));
		return BaseInventoryStorageComponent.Cast(weaponStorage.GetSlot(weaponSlotId).GetAttachedEntity().FindComponent(BaseInventoryStorageComponent));

	}

	void RequestAction(Bacon_GunBuilderUI_Network_StorageRequest request) {
		// request.Pack();	
		
		SCR_JsonSaveContext saveContext = new SCR_JsonSaveContext();
		saveContext.WriteValue("", request);

		string requestString = saveContext.ExportToString();
		
		Print(string.Format("Bacon_GunBuilderUI_PlayerControllerComponent.RequestAction | Sending request: %1", requestString), LogLevel.DEBUG);
		Rpc(RpcAsk_RequestAction, SCR_PlayerController.GetLocalPlayerId(), requestString);
	}

	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RpcAsk_RequestAction(int playerId, string requestJson) {
		Bacon_GunBuilderUI_Network_StorageRequest request = new Bacon_GunBuilderUI_Network_StorageRequest();
		SCR_JsonLoadContext loadContext = new SCR_JsonLoadContext();
		loadContext.ImportFromString(requestJson);
		loadContext.ReadValue("", request);
		// request.ExpandFromRAW(requestJson);
		
		Print(string.Format("Bacon_GunBuilderUI_PlayerControllerComponent.RpcAsk_RequestAction | Processing request from player %1: %2", playerId, requestJson), LogLevel.DEBUG);
		
		Managed entity = Replication.FindItem(request.storageRplId);
		if (!entity) {
			SendActionResponse(request, false, "Invalid entity provided");
			return;
		}
		
		BaseInventoryStorageComponent editedStorage = BaseInventoryStorageComponent.Cast(entity);
		if (!editedStorage) {
			SendActionResponse(request, false, "Provided entity is no a storage component");
			return;
		}
		
		SCR_InventoryStorageManagerComponent storageManager = GetPlayerInventoryManager(playerId);
		if (!storageManager) {
			SendActionResponse(request, false, "Character storage manager not found");
			return;
		}

		switch (request.actionType) {
			case Bacon_GunBuilderUI_ActionType.ADD_ITEM: {
				Action_AddItemToStorage(request, editedStorage, storageManager);
				break;
			}
			case Bacon_GunBuilderUI_ActionType.REMOVE_ITEM: {
				Action_RemoveItemFromStorage(request, editedStorage, storageManager);
				break;
			}
			case Bacon_GunBuilderUI_ActionType.REPLACE_ITEM: {
				Action_ReplaceItemInSlotWithPrefab(request, editedStorage, storageManager);
				break;
			}
		}
	}
			
	void Action_AddItemToStorage(Bacon_GunBuilderUI_Network_StorageRequest request, BaseInventoryStorageComponent storage, SCR_InventoryStorageManagerComponent storageManager) {
		// for things like the alice vest we need to use different logic
		// this is stupid
		ClothNodeStorageComponent loadoutCloth = ClothNodeStorageComponent.Cast(storage);
		if (!loadoutCloth) {
			Bacon_GunBuilderUI_InvCb_UIResponse cb = new Bacon_GunBuilderUI_InvCb_UIResponse();
			
			cb.messageOk = "Item added";
			cb.messageFailed = "Failed to add prefab into storage";
			cb.component = this;
			cb.request = request;
			
			storageManager.TrySpawnPrefabToStorage(request.prefab, storage, request.storageSlotId, EStoragePurpose.PURPOSE_ANY, cb); 
			return; 
		}
		
		// IEntity itemEntity = Bacon_GunBuilderUI_Helpers.PrepareTemporaryEntity(request.prefab);
		IEntity itemEntity = Bacon_GunBuilderUI_Helpers.PrepareTemporaryEntityAtCoords(request.prefab, storageManager.GetOwner().GetOrigin());
		if (!itemEntity) {
			SendActionResponse(request, false, "Failed to spawn temporary entity"); return; }
//		
//		Resource loaded = Resource.Load(request.prefab);
//		if (!loaded) {
//			SendActionResponse(request, false, "Failed to load temporary entity prefab"); return; }
//
//		IEntity itemEntity = GetGame().SpawnEntityPrefab(loaded, GetGame().GetWorld(), null);
//		if (!itemEntity) {
//			SendActionResponse(request, false, "Failed to spawn temporary entity"); return; }
//		
		BaseInventoryStorageComponent appropriateStorage = storageManager.FindStorageForInsert(itemEntity, storage, EStoragePurpose.PURPOSE_ANY);
		if (!appropriateStorage) {
			SendActionResponse(request, false, "Failed to find suitable storage");
			SCR_EntityHelper.DeleteEntityAndChildren(itemEntity);
			return;
		}
		
		Bacon_GunBuilderUI_InvCb_DeleteTemporaryEntityOnFailure deleteCb = new Bacon_GunBuilderUI_InvCb_DeleteTemporaryEntityOnFailure();
			
		deleteCb.messageOk = "Item added";
		deleteCb.messageFailed = "Failed to add prefab into substorage";
		deleteCb.temporaryEntity = itemEntity;
		deleteCb.component = this;
		deleteCb.request = request;

		storageManager.TryInsertItemInStorage(itemEntity, appropriateStorage, -1, deleteCb);
	}
	
	void Action_RemoveItemFromStorage(Bacon_GunBuilderUI_Network_StorageRequest request, BaseInventoryStorageComponent storage, SCR_InventoryStorageManagerComponent storageManager) {
		InventoryStorageSlot slot = storage.GetSlot(request.storageSlotId);
		if (!slot) {
			SendActionResponse(request, false, "Requested slot does not exist");
			return;
		}
		IEntity attachedEntity = slot.GetAttachedEntity();
  		if (!attachedEntity) {
			SendActionResponse(request, false, "Provided entity is invalid");
			return;
		}
		
		Bacon_GunBuilderUI_InvCb_UIResponse cb = new Bacon_GunBuilderUI_InvCb_UIResponse();
			
		cb.messageOk = "Item removed";
		cb.messageFailed = "Failed to remove item from storage";
		cb.request = request;
		cb.component = this;
			
		storageManager.TryDeleteItem(attachedEntity, cb);
	}
	
	void Action_ReplaceItemInSlotWithPrefab(Bacon_GunBuilderUI_Network_StorageRequest request, BaseInventoryStorageComponent storage, SCR_InventoryStorageManagerComponent storageManager) {
		InventoryStorageSlot slot = storage.GetSlot(request.storageSlotId);
		if (!slot) {
			SendActionResponse(request, false, "Requested slot does not exist");
			return;
		}
		
		IEntity attachedEntity = slot.GetAttachedEntity();
  		if (!attachedEntity) {
			// IEntity itemEntity = Bacon_GunBuilderUI_Helpers.PrepareTemporaryEntity(request.prefab);
			IEntity itemEntity = Bacon_GunBuilderUI_Helpers.PrepareTemporaryEntityAtCoords(request.prefab, storageManager.GetOwner().GetOrigin());
			if (!itemEntity) {
				SendActionResponse(request, false, "Failed to spawn temporary entity"); return; }

			Bacon_GunBuilderUI_InvCb_DeleteTemporaryEntityOnFailure deleteCb = new Bacon_GunBuilderUI_InvCb_DeleteTemporaryEntityOnFailure();
			
			deleteCb.messageOk = "Item added";
			deleteCb.messageFailed = "Failed to add prefab into substorage";
			deleteCb.temporaryEntity = itemEntity;
			deleteCb.component = this;
			deleteCb.request = request;

			storageManager.TryInsertItemInStorage(itemEntity, storage, request.storageSlotId, deleteCb);
			
//			cb.messageOk = "Slot updated";
//			cb.messageFailed = "Failed to spawn prefab into storage";
//			cb.component = this;
//			cb.request = request;
//			
//			storageManager.TrySpawnPrefabToStorage(request.prefab, storage, request.storageSlotId, EStoragePurpose.PURPOSE_ANY, cb);
		} else {
			Bacon_GunBuilder_InvCb_SpawnAfterDelete deleteCb = new Bacon_GunBuilder_InvCb_SpawnAfterDelete();
			deleteCb.request = request;
			deleteCb.component = this;
			deleteCb.storageManager = storageManager;
			deleteCb.slotStorage = storage;
	
			storageManager.TryDeleteItem(attachedEntity, deleteCb);
		}
	}
	
	void SendActionResponse(Bacon_GunBuilderUI_Network_Request request, bool success, string message = "") {
		Print(string.Format("Bacon_GunBuilderUI_PlayerControllerComponent.RpcDo_SendActionResponse | Operation: %1, success: %2, message: %3", request.Repr(), success, message), LogLevel.DEBUG);
		
		Bacon_GunBuilderUI_Network_Response response = new Bacon_GunBuilderUI_Network_Response();
		response.request = request;
		response.success = success;
		response.message = message;
		// response.Pack();
		
		SCR_JsonSaveContext saveContext = new SCR_JsonSaveContext();
		saveContext.WriteValue("", response);
		
		string responseString = saveContext.ExportToString();
		
		Print(string.Format("Bacon_GunBuilderUI_PlayerControllerComponent.SendActionResponse | Sending response: %1", responseString), LogLevel.DEBUG);
		
		Rpc(RpcDo_SendActionResponse, responseString);
	}
	
	// ---------- loadout stuff
	// loadout request
	void RequestLoadoutAction(Bacon_GunBuilderUI_Network_LoadoutRequest request) {
		SCR_JsonSaveContext saveContext = new SCR_JsonSaveContext();
		saveContext.WriteValue("", request);

		string requestString = saveContext.ExportToString();
		
		Print(string.Format("Bacon_GunBuilderUI_PlayerControllerComponent.RequestLoadoutAction | Sending request: %1", requestString), LogLevel.DEBUG);
		Rpc(RpcAsk_RequestLoadoutAction, SCR_PlayerController.GetLocalPlayerId(), requestString);
	}
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void RpcAsk_RequestLoadoutAction(int playerId, string requestJson) {
		Bacon_GunBuilderUI_Network_LoadoutRequest request = new Bacon_GunBuilderUI_Network_LoadoutRequest();
		SCR_JsonLoadContext loadContext = new SCR_JsonLoadContext();
		loadContext.ImportFromString(requestJson);
		loadContext.ReadValue("", request);
		
		if (!m_LoadoutStorageComponent) {
			SendActionResponse(request, false, "Loadout manager component missing from game mode");
			return;
		}

		SCR_ArsenalManagerComponent arsenalManager;
		if (!SCR_ArsenalManagerComponent.GetArsenalManager(arsenalManager)) {
			SendActionResponse(request, false, "No Arsenal Manager found in the world");
			return;
		}
		
		Print(string.Format("Bacon_GunBuilderUI_PlayerControllerComponent.RpcAsk_RequestLoadoutAction | Processing request from player %1: %2", playerId, requestJson), LogLevel.DEBUG);
		
//		BackendApi backend = GetGame().GetBackendApi();
//		string identity = backend.GetPlayerIdentityId(playerId);
//		
//		if (identity.IsEmpty()) {
//			if (GetGame().IsDev()) {
//				Print(string.Format("Bacon_GunBuilderUI_PlayerControllerComponent.RpcAsk_RequestLoadoutAction | Setting dev identity for playerid %1", playerId), LogLevel.DEBUG);
//				identity = "DEV_IDENTITY";
//			} else {
//				Print(string.Format("Bacon_GunBuilderUI_PlayerControllerComponent.RpcAsk_RequestLoadoutAction | No identity id for player %1: %2", playerId, requestJson), LogLevel.ERROR);
//				return;
//			}
//		}
		string identity;
		if (!Bacon_GunBuilderUI_Helpers.GetPlayerIdentityId(playerId, identity)) {
			SendActionResponse(request, false, "Invalid player identity id");
			return;
		}
		
		string factionKey;
		if (!Bacon_GunBuilderUI_Helpers.GetPlayerEntityFactionKey(playerId, factionKey)) {
			SendActionResponse(request, false, "Invalid faction");
			return;
		}
		
		Managed entity = Replication.FindItem(request.arsenalComponentRplId);
		if (!entity) {
			SendActionResponse(request, false, "Invalid Arsenal Component provided");
			return;
		}
		
		SCR_ArsenalComponent arsenal = SCR_ArsenalComponent.Cast(entity);
		if (!entity) {
			SendActionResponse(request, false, "RplId is not an Arsenal Component");
			return;
		}

		switch (request.actionType) {
			case Bacon_GunBuilderUI_ActionType.GET_LOADOUTS: {
				Action_GetLoadoutList(request, identity, factionKey, playerId);
				break;
			}
			case Bacon_GunBuilderUI_ActionType.SAVE_LOADOUT: {
				Action_SavePlayerLoadout(request, arsenalManager, arsenal, identity, factionKey, playerId);
				break;
			}
			case Bacon_GunBuilderUI_ActionType.APPLY_LOADOUT: {
				Action_ApplyPlayerLoadout(request, identity, factionKey, playerId);
				break;
			}
			case Bacon_GunBuilderUI_ActionType.CLEAR_LOADOUT: {
				Action_ClearPlayerLoadout(request, identity, factionKey, playerId);
				break;
			}
			case Bacon_GunBuilderUI_ActionType.GET_ADMIN_LOADOUTS: {
				if (!SCR_Global.IsAdmin(m_PC.GetPlayerId())) {
					SendActionResponse(request, false, "Not admin");
					return;
				}
				Action_GetLoadoutList(request, identity, factionKey, playerId, true);
				break;
			}
			case Bacon_GunBuilderUI_ActionType.SAVE_LOADOUT_ADMIN: {
				if (!SCR_Global.IsAdmin(m_PC.GetPlayerId())) {
					SendActionResponse(request, false, "Not admin");
					return;
				}
				Action_SavePlayerLoadout(request, arsenalManager, arsenal, identity, factionKey, playerId, true);
				
				if (!Bacon_GunBuilderUI_PlayerControllerComponent.ServerInstance) {
					PrintFormat("Bacon_GunBuilderUI_PlayerControllerComponent | Server instance of player controller component not found! Cannot send loadouts.", LogLevel.ERROR);
					return;
				}
				
				Bacon_GunBuilderUI_PlayerControllerComponent.ServerInstance.UpdateServerLoadouts();
				Bacon_GunBuilderUI_PlayerControllerComponent.ServerInstance.BroadcastLoadoutChange();
				break;
			}
			case Bacon_GunBuilderUI_ActionType.APPLY_LOADOUT_ADMIN: {
				if (!SCR_Global.IsAdmin(m_PC.GetPlayerId())) {
					SendActionResponse(request, false, "Not admin");
					return;
				}
				Action_ApplyPlayerLoadout(request, identity, factionKey, playerId, true);
				break;
			}
			case Bacon_GunBuilderUI_ActionType.CLEAR_LOADOUT_ADMIN: {
				if (!SCR_Global.IsAdmin(m_PC.GetPlayerId())) {
					SendActionResponse(request, false, "Not admin");
					return;
				}
				Action_ClearPlayerLoadout(request, identity, factionKey, playerId, true);
				break;
			}
		}
	}
	
	void UpdatePlayerMarker(int playerId) {
		if (!m_MapMarkerEntrySquadLeader) {
			Print("Bacon_GunBuilderUI_PlayerControllerComponent.UpdatePlayerMarker | m_MapMarkerEntrySquadLeader is null - cannot update marker", LogLevel.WARNING);
			return;
		}
		
		m_MapMarkerEntrySquadLeader.BaconLoadoutEditor_UpdateMarkerTarget(playerId);
	}
	
	void SetAILoadout(int slotId, IEntity target, SCR_AIGroup group) {
		if (!target || slotId < 0)
			return;
		
		if (slotId > AdminLoadoutMetadata.Count())
			return;

		PrintFormat("Bacon_GunBuilderUI_PlayerControllerComponent.SetAILoadout | Loading data for GM slot %1", slotId, level: LogLevel.DEBUG);
		
		string loadoutData;
		string prefab;
		
		int slotIdInternal = AdminLoadoutMetadata[slotId].slotId;
		
		if (!m_LoadoutStorageComponent.GetPlayerLoadoutData(0, "", slotIdInternal, prefab, loadoutData, true)) {
			PrintFormat("Bacon_GunBuilderUI_PlayerControllerComponent.SetAILoadout | Cannot load loadout data for slot %1", slotIdInternal, level: LogLevel.ERROR);
			return;
		}
		
		EntitySpawnParams params = new EntitySpawnParams();
		params.TransformMode = ETransformMode.WORLD;

		target.GetWorldTransform(params.Transform);
		params.Transform[3] = params.Transform[3] + (vector.Up*0.05);
		
		if (prefab.IsEmpty()) {
			PrintFormat("Bacon_GunBuilderUI_PlayerControllerComponent.SetAILoadout | Invalid character prefab", level: LogLevel.ERROR);
			return;
		}
		
		Resource loaded = Resource.Load(prefab);
		if (!loaded) {
			PrintFormat("Bacon_GunBuilderUI_PlayerControllerComponent.SetAILoadout | Failed to load character prefab", level: LogLevel.ERROR);
			return;
		}
		
		IEntity entity = GameEntity.Cast(GetGame().SpawnEntityPrefab(prefab, false, GetGame().GetWorld(), params));
		if (!entity) {
			PrintFormat("Bacon_GunBuilderUI_PlayerControllerComponent.SetAILoadout | Spawning character entity failed", level: LogLevel.ERROR);
			return;
		}
		
		CharacterControllerComponent controller = CharacterControllerComponent.Cast(entity.FindComponent(CharacterControllerComponent));
		if (!controller) {
			RplComponent.DeleteRplEntity(entity, false);
			PrintFormat("Bacon_GunBuilderUI_PlayerControllerComponent.SetAILoadout | Spawned entity has no Character Controller Component", level: LogLevel.ERROR);
			return;
		}
		
		controller.TryEquipRightHandItem(null, EEquipItemType.EEquipTypeUnarmedDeliberate, true);
		
		GetGame().GetCallqueue().CallLater(SetAILoadout_StepTwo, 100, false, loadoutData, controller, entity, target, group);
	}
	void SetAILoadout_StepTwo(string loadoutData, CharacterControllerComponent controller, IEntity newEntity, IEntity previousEntity, SCR_AIGroup group) {
		// controller.TryEquipRightHandItem(null, EEquipItemType.EEquipTypeUnarmedDeliberate, true);

		if (!newEntity || !previousEntity || !controller) {
			PrintFormat("Bacon_GunBuilderUI_PlayerControllerComponent.SetAILoadout_StepTwo | An important entity disappeared before the loadout could be loaded for AI", level: LogLevel.ERROR);
			PrintFormat("Bacon_GunBuilderUI_PlayerControllerComponent.SetAILoadout_StepTwo | newEntity: %1", newEntity, level: LogLevel.ERROR);
			PrintFormat("Bacon_GunBuilderUI_PlayerControllerComponent.SetAILoadout_StepTwo | previousEntity: %1", previousEntity, level: LogLevel.ERROR);
			PrintFormat("Bacon_GunBuilderUI_PlayerControllerComponent.SetAILoadout_StepTwo | controller: %1", controller, level: LogLevel.ERROR);
			return;
		}

		GameEntity entityGame = GameEntity.Cast(newEntity);
		
		SCR_JsonLoadContext ctx = new SCR_JsonLoadContext();
		ctx.ImportFromString(loadoutData);
		ctx.ReadValue("", entityGame);

		group.AddAgentFromControlledEntity(newEntity);
		AIControlComponent aiControlComponent = AIControlComponent.Cast(newEntity.FindComponent(AIControlComponent));
		aiControlComponent.ActivateAI();
		
		RplComponent.DeleteRplEntity(previousEntity, false);
	}
	
	
	void Action_ApplyPlayerLoadout(Bacon_GunBuilderUI_Network_LoadoutRequest request, string identity, string factionKey, int playerId, bool isAdminLoadout = false) {
		IEntity previousEntity = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId);
		if (!previousEntity)
			return;
		
		string loadoutData;
		string prefab;
		
		if (!m_LoadoutStorageComponent.GetPlayerLoadoutData(playerId, factionKey, request.loadoutSlotId, prefab, loadoutData, isAdminLoadout)) {
			SendActionResponse(request, false, "Failed to load loadout");
			return;
		}
		
		EntitySpawnParams params = new EntitySpawnParams();
		params.TransformMode = ETransformMode.WORLD;

		previousEntity.GetWorldTransform(params.Transform);
		params.Transform[3] = params.Transform[3] + (vector.Up*0.05);
		
		if (prefab.IsEmpty()) {
			SendActionResponse(request, false, "Invalid character prefab");
			return;
		}
		
		Resource loaded = Resource.Load(prefab);
		if (!loaded) {
			SendActionResponse(request, false, "Failed to load character prefab");
			return;
		}
		
		IEntity entity = GameEntity.Cast(GetGame().SpawnEntityPrefab(prefab, false, GetGame().GetWorld(), params));
		if (!entity) {
			SendActionResponse(request, false, "Spawning character entity failed");
			return;
		}
		
		CharacterControllerComponent controller = CharacterControllerComponent.Cast(entity.FindComponent(CharacterControllerComponent));
		if (!controller) {
			RplComponent.DeleteRplEntity(entity, false);
			SendActionResponse(request, false, "Spawned entity has no Character Controller Component");
			return;
		}
		
		controller.TryEquipRightHandItem(null, EEquipItemType.EEquipTypeUnarmedDeliberate, true);
		
		GetGame().GetCallqueue().CallLater(Action_ApplyPlayerLoadout_StepTwo, 200, false, loadoutData, controller, entity, previousEntity, request, playerId);
		// Action_ApplyPlayerLoadout_StepTwo(loadoutData, entity, previousEntity, request, playerId);
	}
	void Action_ApplyPlayerLoadout_StepTwo(string loadoutData, CharacterControllerComponent controller, IEntity newEntity, IEntity previousEntity, Bacon_GunBuilderUI_Network_LoadoutRequest request, int playerId) {
		// controller.TryEquipRightHandItem(null, EEquipItemType.EEquipTypeUnarmedDeliberate, true);
		string identity;
		Bacon_GunBuilderUI_Helpers.GetPlayerIdentityId(playerId, identity);
		PrintFormat("Bacon_GunBuilderUI_PlayerControllerComponent.Action_ApplyPlayerLoadout_StepTwo | Loading loadout for player id: %1, identity: %2", playerId, identity, level: LogLevel.NORMAL);
		
		if (!newEntity || !previousEntity || !controller) {
			PrintFormat("Bacon_GunBuilderUI_PlayerControllerComponent.Action_ApplyPlayerLoadout_StepTwo | An important entity disappeared before the loadout could be loaded for player id: %1, identity: %2", playerId, identity, level: LogLevel.ERROR);
			PrintFormat("Bacon_GunBuilderUI_PlayerControllerComponent.Action_ApplyPlayerLoadout_StepTwo | newEntity: %1", newEntity, level: LogLevel.ERROR);
			PrintFormat("Bacon_GunBuilderUI_PlayerControllerComponent.Action_ApplyPlayerLoadout_StepTwo | previousEntity: %1", previousEntity, level: LogLevel.ERROR);
			PrintFormat("Bacon_GunBuilderUI_PlayerControllerComponent.Action_ApplyPlayerLoadout_StepTwo | controller: %1", controller, level: LogLevel.ERROR);
			return;
		}

		GameEntity entityGame = GameEntity.Cast(newEntity);

		SCR_JsonLoadContext ctx = new SCR_JsonLoadContext();
		ctx.ImportFromString(loadoutData);
		ctx.ReadValue("", entityGame);

		SCR_PlayerController.Cast(GetOwner()).SetInitialMainEntity(newEntity);
		
		SendActionResponse(request, true, "Loadout applied");

		RplComponent.DeleteRplEntity(previousEntity, false);
		
		UpdatePlayerMarker(playerId);
		
		if (!m_gameMode) {
			Print("Bacon_GunBuilderUI_PlayerControllerComponent.Action_ApplyPlayerLoadout_StepTwo | No game mode!", LogLevel.ERROR);
			return;
		}
		
		Print("Bacon_GunBuilderUI_PlayerControllerComponent.Action_ApplyPlayerLoadout_StepTwo | Calling OnPlayerSpawned...", LogLevel.DEBUG);
		m_gameMode.GetOnPlayerSpawned().Invoke(playerId, newEntity);
	}
	void Action_GetLoadoutList(Bacon_GunBuilderUI_Network_LoadoutRequest request, string identity, string factionKey, int playerId, bool isAdminLoadout = false) {
		array<ref Bacon_GunBuilder_PlayerLoadout> loadoutOptions = {};

		// Bacon_GunBuilder_LoadoutStorage.GetPlayerLoadoutMetadata(identity, factionKey, loadoutOptions);
		m_LoadoutStorageComponent.GetPlayerLoadoutMetadata(playerId, identity, factionKey, loadoutOptions, isAdminLoadout);

		SCR_JsonSaveContext ctx = new SCR_JsonSaveContext();
		ctx.WriteValue("loadouts", loadoutOptions);
		
		SendActionResponse(request, true, ctx.ExportToString());
	}
	void Action_SavePlayerLoadout(Bacon_GunBuilderUI_Network_LoadoutRequest request, SCR_ArsenalManagerComponent arsenalManager, SCR_ArsenalComponent arsenal, string identity, string factionKey, int playerId, bool isAdminLoadout = false) {
		IEntity controlledEntity = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId);
		if (!controlledEntity)
			return;

		if (!isAdminLoadout && !arsenalManager.GunBuilderUI_CanSaveLoadout(playerId, GameEntity.Cast(controlledEntity), FactionAffiliationComponent.Cast(controlledEntity.FindComponent(FactionAffiliationComponent)), arsenal, true)) {
			SendActionResponse(request, false, "Cannot save loadout: Rejected by Arsenal Manager");
			return;
		}
		
		if (!m_LoadoutStorageComponent.SaveCurrentPlayerLoadout(playerId, identity, controlledEntity, factionKey, request.loadoutSlotId, isAdminLoadout)) {
			SendActionResponse(request, false, "Loadout saving failed");
			return;
		}
		
		Action_GetLoadoutList(request, identity, factionKey, playerId, isAdminLoadout);
	}
	void Action_ClearPlayerLoadout(Bacon_GunBuilderUI_Network_LoadoutRequest request, string identity, string factionKey, int playerId, bool isAdminLoadout = false) {
		if (!m_LoadoutStorageComponent.ClearLoadoutSlot(playerId, identity, factionKey, request.loadoutSlotId, isAdminLoadout)) {
			SendActionResponse(request, false, "Failed to clear loadout slot");
			return;
		}
		
		Action_GetLoadoutList(request, identity, factionKey, playerId, isAdminLoadout);
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	void RpcDo_SendActionResponse(string responseJson) {
		Print(string.Format("Bacon_GunBuilderUI_PlayerControllerComponent.RpcDo_SendActionResponse | Processing response: %1", responseJson), LogLevel.DEBUG);
		
		// Print(response.request);
		SCR_JsonLoadContext loadContext = new SCR_JsonLoadContext();
		loadContext.ImportFromString(responseJson);
		
		Bacon_GunBuilderUI_Network_Response response = new Bacon_GunBuilderUI_Network_Response();
		loadContext.ReadValue("", response);

		switch (response.request.actionType) {
			case Bacon_GunBuilderUI_ActionType.ADD_ITEM:
			case Bacon_GunBuilderUI_ActionType.REMOVE_ITEM:
			case Bacon_GunBuilderUI_ActionType.REPLACE_ITEM: {
				Bacon_GunBuilderUI_Network_StorageRequest storageRequest;
				loadContext.ReadValue("request", storageRequest);
				
				m_OnResponse_Storage.Invoke(response, storageRequest);
				break;
			}
			case Bacon_GunBuilderUI_ActionType.GET_ADMIN_LOADOUTS:
			case Bacon_GunBuilderUI_ActionType.SAVE_LOADOUT_ADMIN:
			case Bacon_GunBuilderUI_ActionType.APPLY_LOADOUT_ADMIN:
			case Bacon_GunBuilderUI_ActionType.CLEAR_LOADOUT_ADMIN:
			case Bacon_GunBuilderUI_ActionType.GET_LOADOUTS:
			case Bacon_GunBuilderUI_ActionType.SAVE_LOADOUT:
			case Bacon_GunBuilderUI_ActionType.CLEAR_LOADOUT:
			case Bacon_GunBuilderUI_ActionType.APPLY_LOADOUT: {
				Bacon_GunBuilderUI_Network_LoadoutRequest loadoutRequest;
				loadContext.ReadValue("request", loadoutRequest);

				m_OnResponse_Loadout.Invoke(response, loadoutRequest);
				break;
			}
		}
		
		
	};

};

class Bacon_GunBuilderUI_InvCb: ScriptedInventoryOperationCallback {
	Bacon_GunBuilderUI_Network_Request request;
	string messageOk;
	string messageFailed;
	
	Bacon_GunBuilderUI_PlayerControllerComponent component;
//	RplId storageRplId;
//	int slotId;

	Bacon_GunBuilderUI_InvCb nextCb;
	
	override void OnComplete() {
		if (component)
			component.SendActionResponse(request, true, messageOk);
	};
	override void OnFailed() {
		if (component)
			component.SendActionResponse(request, false, messageFailed);
	};	
}

class Bacon_GunBuilderUI_InvCb_UIResponse: Bacon_GunBuilderUI_InvCb {
	static Bacon_GunBuilderUI_InvCb_UIResponse CreateResponseCallback(Bacon_GunBuilderUI_InvCb fromCb) {
		Bacon_GunBuilderUI_InvCb_UIResponse cb = new Bacon_GunBuilderUI_InvCb_UIResponse();

		cb.request = fromCb.request;
		cb.component = fromCb.component;
		
		return cb;
	}
};

class Bacon_GunBuilderUI_InvCb_DeleteTemporaryEntityOnFailure: Bacon_GunBuilderUI_InvCb {
	IEntity temporaryEntity;
	
	override void OnFailed() {
		SCR_EntityHelper.DeleteEntityAndChildren(temporaryEntity);

		super.OnFailed();
	};
}

class Bacon_GunBuilder_InvCb_SpawnAfterDelete: Bacon_GunBuilderUI_InvCb {
	SCR_InventoryStorageManagerComponent storageManager;
	BaseInventoryStorageComponent slotStorage;
	
	override void OnComplete() {
		Bacon_GunBuilderUI_Network_StorageRequest storageRequest = Bacon_GunBuilderUI_Network_StorageRequest.Cast(request);
		
		if (storageRequest.prefab != "empty") {
			// IEntity itemEntity = Bacon_GunBuilderUI_Helpers.PrepareTemporaryEntity(storageRequest.prefab);
			IEntity itemEntity = Bacon_GunBuilderUI_Helpers.PrepareTemporaryEntityAtCoords(storageRequest.prefab, storageManager.GetOwner().GetOrigin());
			if (!itemEntity) {
				messageFailed = string.Format("Failed to create temporary entity from prefab: %1", storageRequest.prefab);
				OnFailed(); return;
			}

			Bacon_GunBuilderUI_InvCb_DeleteTemporaryEntityOnFailure deleteCb = new Bacon_GunBuilderUI_InvCb_DeleteTemporaryEntityOnFailure();
				
			deleteCb.messageOk = "Item added";
			deleteCb.messageFailed = "Failed to add prefab into substorage";
			deleteCb.temporaryEntity = itemEntity;
			deleteCb.component = component;
			deleteCb.request = request;
	
			storageManager.TryInsertItemInStorage(itemEntity, slotStorage, storageRequest.storageSlotId, deleteCb);
			return;

//			Bacon_GunBuilderUI_InvCb_UIResponse responseCb = Bacon_GunBuilderUI_InvCb_UIResponse.CreateResponseCallback(this);
//			responseCb.messageOk = "Slot updated";
//			responseCb.messageFailed = string.Format("Failed to spawn prefab into storage %1 - prefab %2", slotStorage.Type(), storageRequest.prefab);
//			
//			storageManager.TrySpawnPrefabToStorage(storageRequest.prefab, slotStorage, storageRequest.storageSlotId, EStoragePurpose.PURPOSE_ANY, responseCb);
//			return;
		}
		
		messageOk = "Item removed";
		super.OnComplete();
	};
	override void OnFailed() {
		if (messageFailed.IsEmpty()) {
			Bacon_GunBuilderUI_Network_StorageRequest storageRequest = Bacon_GunBuilderUI_Network_StorageRequest.Cast(request);
			messageFailed = string.Format("Failed to delete item %1 in storage %2", storageRequest.storageSlotId, slotStorage.Type());
		}
		
		super.OnFailed();
	};
}
