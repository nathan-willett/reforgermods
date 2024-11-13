class Bacon_GunBuilder_PlayerLoadout {
	string metadata_clothes;
	string metadata_weapons;
	string prefab;
	string data;
	int slotId;
}

// identity -> factionkey -> slot
class Bacon_GunBuilder_PlayerFactionLoadoutStorage {
	static int MAX_LOADOUTS_PER_PLAYER = 5;
	static int MAX_ADMIN_LOADOUTS = 10;
	// key: factionKey
	ref map<string, ref map<int, ref Bacon_GunBuilder_PlayerLoadout>> playerLoadouts = new map<string, ref map<int, ref Bacon_GunBuilder_PlayerLoadout>>;
	
	static bool SerializeCharacter(IEntity characterEntity, out string serialized) {
		GameEntity ent = GameEntity.Cast(characterEntity);
		if (!ent) {
			Print(string.Format("Bacon_GunBuilder_PlayerFactionLoadoutStorage::SerializeCharacter | Character entity is not a Game Entity: %1", characterEntity), LogLevel.ERROR);
			return false;
		}
		
		SCR_JsonSaveContext saveContext = new SCR_JsonSaveContext();
		if (!saveContext.WriteValue("", ent)) {
			Print(string.Format("Bacon_GunBuilder_PlayerFactionLoadoutStorage::SerializeCharacter | Failed to serialize Game Entity: %1", ent), LogLevel.ERROR);
			return false;
		}
		
		serialized = saveContext.ExportToString();
		return true;
	}
	
	static void FillPlayerLoadoutWeaponMetadata(BaseInventoryStorageComponent storage, out string outWeapons) {
		int slotsCount = storage.GetSlotsCount();
		
		InventoryStorageSlot slot;
		BaseWeaponComponent weapon;
		string weaponSlotName;
		string extractedWeaponName;
	
		array<string> words = {};
		
		for (int i = 0; i < slotsCount; i++) {
			slot = storage.GetSlot(i);
			
			if (!slot.GetAttachedEntity())
				continue;
			
			weapon = BaseWeaponComponent.Cast(slot.GetParentContainer());
			if (!weapon)
				continue;
			
			if (!Bacon_GunBuilderUI_Helpers.GetWeaponSlotTypeString(weapon, weaponSlotName))
				continue;
			
			if (weaponSlotName == "primary" || weaponSlotName == "secondary") {
				if (!Bacon_GunBuilderUI_Helpers.GetItemNameFromEntityValidate(slot.GetAttachedEntity(), extractedWeaponName))
					continue;
				
				// outWeapons = string.Format("%1%2\n", outWeapons, extractedWeaponName);
				words.Insert(extractedWeaponName);
			}
		}
		
		outWeapons = SCR_StringHelper.Join("\n", words, false);
	}
	
	static void FillPlayerLoadoutClothesMetadata(BaseInventoryStorageComponent storage, out string outClothes) {
		int slotsCount = storage.GetSlotsCount();
		
		InventoryStorageSlot slot;
		typename areaType;
		string extractedName;
		
		array<string> words = {};
	
		for (int i = 0; i < slotsCount; i++) {
			slot = storage.GetSlot(i);
			
			if (!slot.GetAttachedEntity())
				continue;
			
			if (!Bacon_GunBuilderUI_Helpers.GetLoadoutAreaType(slot, areaType))
				continue;
					
			if (areaType == LoadoutHeadCoverArea || areaType == LoadoutJacketArea || areaType == LoadoutVestArea || areaType == LoadoutPantsArea) {
				if (!Bacon_GunBuilderUI_Helpers.GetItemNameFromEntityValidate(slot.GetAttachedEntity(), extractedName))
					continue;

				// outClothes = string.Format("%1%2\n", outClothes, extractedName);
				words.Insert(extractedName);
			}
		}
		
		outClothes = SCR_StringHelper.Join("\n", words, false);
	}
	
	static bool FillPlayerLoadoutMetadata(IEntity ent, out string outClothes, out string outWeapons) {
		array<BaseInventoryStorageComponent> storages = {};
		
		int storagesCount = Bacon_GunBuilderUI_Helpers.GetAllEntityStorages(ent, storages);
		if (storagesCount < 1)
			return false;
		
		outWeapons = "";
		outClothes = "";

		string temp;
		typename areaType;

		Bacon_GunBuilderUI_StorageType storageType;
		foreach (BaseInventoryStorageComponent storage : storages) {
			storageType = Bacon_GunBuilderUI_Helpers.GetStorageType(storage);
			
			switch (storageType) {
				case Bacon_GunBuilderUI_StorageType.CHARACTER_WEAPON: {
					FillPlayerLoadoutWeaponMetadata(storage, outWeapons);
					break;
				}
				case Bacon_GunBuilderUI_StorageType.CHARACTER_LOADOUT: {
					FillPlayerLoadoutClothesMetadata(storage, outClothes);
					break;
				}
			}
		}
		
		if (outWeapons.IsEmpty())
			outWeapons = "N/A";
		
		if (outClothes.IsEmpty())
			outClothes = "N/A";
		
		return true;
	}
	
	bool SaveLoadout(IEntity characterEntity, string factionKey, int slotId) {
		Bacon_GunBuilder_PlayerLoadout newPlayerLoadout = new Bacon_GunBuilder_PlayerLoadout();
		
		if (!Bacon_GunBuilderUI_Helpers.GetResourceNameFromEntity(characterEntity, newPlayerLoadout.prefab)) {
			Print(string.Format("Bacon_GunBuilder_PlayerFactionLoadoutStorage | Failed to get resource name from entity: %1", characterEntity), LogLevel.ERROR);
			return false;
		}
		
		newPlayerLoadout.slotId = slotId;
		
		if (!Bacon_GunBuilder_PlayerFactionLoadoutStorage.FillPlayerLoadoutMetadata(characterEntity, newPlayerLoadout.metadata_clothes, newPlayerLoadout.metadata_weapons))
			return false;
		
		if (!Bacon_GunBuilder_PlayerFactionLoadoutStorage.SerializeCharacter(characterEntity, newPlayerLoadout.data))
			return false;

		if (!playerLoadouts.Contains(factionKey))
			playerLoadouts.Set(factionKey, new map<int, ref Bacon_GunBuilder_PlayerLoadout>());
		
		auto playerFactionLoadouts = playerLoadouts.Get(factionKey);

		playerFactionLoadouts.Set(slotId, newPlayerLoadout);
		
		return true;
	}
	
	bool GetLoadout(string factionKey, int slotId, out Bacon_GunBuilder_PlayerLoadout playerLoadout) {
		if (!playerLoadouts.Contains(factionKey))
			return false;
		
		auto playerFactionLoadouts = playerLoadouts.Get(factionKey);
		if (!playerFactionLoadouts.Contains(slotId))
			return false;
		
		playerLoadout = playerFactionLoadouts.Get(slotId);
		return true;
	}
	
	bool ClearLoadoutSlot(string factionKey, int slotId) {
		if (!playerLoadouts.Contains(factionKey))
			return false;
		
		auto playerFactionLoadouts = playerLoadouts.Get(factionKey);
		if (!playerFactionLoadouts.Contains(slotId))
			return false;
		
		Bacon_GunBuilder_PlayerLoadout option = new Bacon_GunBuilder_PlayerLoadout();
		option.slotId = slotId;
		
		playerFactionLoadouts.Set(slotId, option);
		return true;
	}
	
	void InitLoadouts(string factionKey) {
		map<int, ref Bacon_GunBuilder_PlayerLoadout> loadoutData = new map<int, ref Bacon_GunBuilder_PlayerLoadout>();
		
		Bacon_GunBuilder_PlayerLoadout option;
		int max;
		if (factionKey == "admin") {
			max = Bacon_GunBuilder_PlayerFactionLoadoutStorage.MAX_ADMIN_LOADOUTS;
		} else {
			max = Bacon_GunBuilder_PlayerFactionLoadoutStorage.MAX_LOADOUTS_PER_PLAYER;
		}
		
		for (int x = 0; x < max; x++) {
			option = new Bacon_GunBuilder_PlayerLoadout();
			option.slotId = x;
			
			loadoutData.Set(x, option);
		}
		
		playerLoadouts.Set(factionKey, loadoutData);
	}
	
	void GetPlayerLoadoutOptions(string factionKey, out array<ref Bacon_GunBuilder_PlayerLoadout> loadoutData) {
		if (!playerLoadouts.Contains(factionKey))
			InitLoadouts(factionKey);
		
		auto playerFactionLoadouts = playerLoadouts.Get(factionKey);
//		if (playerFactionLoadouts.Count() < 1)
//			return;
		
		Bacon_GunBuilder_PlayerLoadout option;
		
		int max;
		if (factionKey == "admin") {
			max = Bacon_GunBuilder_PlayerFactionLoadoutStorage.MAX_ADMIN_LOADOUTS;
		} else {
			max = Bacon_GunBuilder_PlayerFactionLoadoutStorage.MAX_LOADOUTS_PER_PLAYER;
		}
		
		for (int x = 0; x < max; x++) {
			option = new Bacon_GunBuilder_PlayerLoadout();
			
			Bacon_GunBuilder_PlayerLoadout loadout = playerFactionLoadouts.Get(x);

			option.slotId = loadout.slotId;
			option.metadata_clothes = loadout.metadata_clothes;
			option.metadata_weapons = loadout.metadata_weapons;
			
			loadoutData.Insert(option);
		}
	}
}

class Bacon_GunBuilder_LoadoutStorageComponentClass: SCR_BaseGameModeComponentClass {}

class Bacon_GunBuilder_LoadoutStorageComponent: SCR_BaseGameModeComponent {
	// key: identityid
	// storage by player id
	private ref map<int, ref Bacon_GunBuilder_PlayerFactionLoadoutStorage> loadoutStorage = new map<int, ref Bacon_GunBuilder_PlayerFactionLoadoutStorage>();
	
	// static ref map<string, ref Bacon_GunBuilder_PlayerFactionLoadoutStorage> loadoutStorage = new map<string, ref Bacon_GunBuilder_PlayerFactionLoadoutStorage>();
	string loadoutPathRoot = "$profile:/BaconLoadoutEditor_Loadouts";

	override void OnPlayerDisconnected(int playerId, KickCauseCode cause, int timeout) {
		if (loadoutStorage.Contains(playerId)) {
			loadoutStorage.Remove(playerId);
		}
	}
	
	bool SaveCurrentPlayerLoadout(int playerId, string identityId, IEntity characterEntity, string factionKey, int slotId, bool isAdminLoadout = false) {
		if (isAdminLoadout) {
			playerId = -100;
			factionKey = "admin";
		}
		
		if (!loadoutStorage.Contains(playerId))
			loadoutStorage.Set(playerId, new Bacon_GunBuilder_PlayerFactionLoadoutStorage());
		
		auto playerLoadoutStorage = loadoutStorage.Get(playerId);
		
		if (!playerLoadoutStorage.SaveLoadout(characterEntity, factionKey, slotId)) {
			return false;
		}
		
		Print(string.Format("Bacon_GunBuilder_LoadoutStorage::SaveCurrentPlayerLoadout | Saving player loadouts for faction %1 identity %2", factionKey, identityId), LogLevel.DEBUG);
		
		return SavePlayerLoadoutToFile(playerId, identityId, factionKey, isAdminLoadout);
	}
	
	// static bool ClearLoadoutSlot(string identityId, string factionKey, int slotId) {
	bool ClearLoadoutSlot(int playerId, string identityId, string factionKey, int slotId, bool isAdminLoadout = false) {
		if (isAdminLoadout) {
			playerId = -100;
			factionKey = "admin";
		}

		auto playerLoadoutStorage = loadoutStorage.Get(playerId);
		
		if (!playerLoadoutStorage.ClearLoadoutSlot(factionKey, slotId))
			return false;
		
		return SavePlayerLoadoutToFile(playerId, identityId, factionKey, isAdminLoadout);
	}

	// saves ALL loadouts for this faction key to file for the player
	bool SavePlayerLoadoutToFile(int playerId, string identityId, string factionKey, bool isAdminLoadout = false) {
		string path = loadoutPathRoot;
		FileIO.MakeDirectory(path);
		
		if (isAdminLoadout) {
			playerId = -100;
			identityId = "admin_loadouts";
		} else {
			path = string.Format("%1/%2", path, factionKey);
			FileIO.MakeDirectory(path);
			
			path = string.Format("%1/%2", path, identityId.Substring(0, 2));
			FileIO.MakeDirectory(path);
		}
		
		SCR_JsonSaveContext ctx = new SCR_JsonSaveContext();
		
		if (!ctx.WriteValue("", loadoutStorage.Get(playerId)))
			return false;
		
		return ctx.SaveToFile(string.Format("%1/%2", path, identityId));
	}
	
	bool LoadPlayerLoadoutFromFile(int playerId, string identityId, string factionKey, bool isAdminLoadout = false) {
		string path;

		if (isAdminLoadout) {
			playerId = -100;
			factionKey = "admin";
			path = string.Format("%1/admin_loadouts", loadoutPathRoot);
		} else {
			path = string.Format("%1/%2/%3/%4", loadoutPathRoot, factionKey, identityId.Substring(0, 2), identityId);
		}

		if (!FileIO.FileExists(path))
			return false;
		
		SCR_JsonLoadContext ctx = new SCR_JsonLoadContext();
		
		if (!ctx.LoadFromFile(path))
			return false;
		
		Bacon_GunBuilder_PlayerFactionLoadoutStorage playerLoadoutStorage = new Bacon_GunBuilder_PlayerFactionLoadoutStorage();
		
		if (!ctx.ReadValue("", playerLoadoutStorage))
			return false;
		
		Print(string.Format("Bacon_GunBuilder_LoadoutStorage.LoadPlayerLoadoutFromFile | Loaded loadout from file for faction %1 identity %2", factionKey, identityId), LogLevel.DEBUG);
		
		loadoutStorage.Set(playerId, playerLoadoutStorage);
		return true;
	}
	
	bool GetPlayerLoadoutData(int playerId, string factionKey, int slotId, out string prefab, out string loadoutData, bool isAdminLoadout = false) {
		if (isAdminLoadout) {
			playerId = -100;
			factionKey = "admin";
		}
		
		if (!loadoutStorage.Contains(playerId))
			return false;
		
		auto playerLoadoutStorage = loadoutStorage.Get(playerId);
		
		Bacon_GunBuilder_PlayerLoadout playerLoadout;
		if (!playerLoadoutStorage.GetLoadout(factionKey, slotId, playerLoadout))
			return false;
		
		loadoutData = playerLoadout.data;
		prefab = playerLoadout.prefab;
		
		return true;
	}
	
	void GetPlayerLoadoutMetadata(int playerId, string identityId, string factionKey, out array<ref Bacon_GunBuilder_PlayerLoadout> loadoutData, bool isAdminLoadout = false) {
//		if (!Bacon_GunBuilder_LoadoutStorage.loadoutStorage.Contains(identityId)) {
//			if (!Bacon_GunBuilder_LoadoutStorage.LoadPlayerLoadoutFromFile(identityId, factionKey))
//				Bacon_GunBuilder_LoadoutStorage.loadoutStorage.Set(identityId, new Bacon_GunBuilder_PlayerFactionLoadoutStorage());
//		}
		if (isAdminLoadout) {
			playerId = -100;
			factionKey = "admin";
		}
		
		if (!loadoutStorage.Contains(playerId)) {
			if (!LoadPlayerLoadoutFromFile(playerId, identityId, factionKey, isAdminLoadout))
				loadoutStorage.Set(playerId, new Bacon_GunBuilder_PlayerFactionLoadoutStorage());
		}
		
		auto playerLoadoutStorage = loadoutStorage.Get(playerId);

		playerLoadoutStorage.GetPlayerLoadoutOptions(factionKey, loadoutData);
	}
}