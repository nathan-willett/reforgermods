class Bacon_GunBuilderUI_Helpers {
	static ItemPreviewManagerEntity m_previewManager;
	static ResourceName m_sDialogPresets = "{05C3F6DD3E39564A}Configs/BaconLoadoutEditor/GunBuilderUI_Dialogs.conf";
	
	static ref SCR_ConfigurableDialogUi m_dialog;
	
	static bool IsLocalPlayerAdmin() {
		int playerId = SCR_PlayerController.GetLocalPlayerId();
		
		return SCR_Global.IsAdmin(playerId);
	}

	static ItemPreviewManagerEntity GetPreviewManager() {
		if (!m_previewManager)
			m_previewManager = ChimeraWorld.CastFrom(GetGame().GetWorld()).GetItemPreviewManager();
		
		return m_previewManager;
	}
	
	static Bacon_GunBuilderUI_PlayerControllerComponent GetPlayerControllerComponent() {
		PlayerController pc = GetGame().GetPlayerController();
		if (!pc)
			return null;

		return Bacon_GunBuilderUI_PlayerControllerComponent.Cast(pc.FindComponent(Bacon_GunBuilderUI_PlayerControllerComponent));
	}
	
	static IEntity PrepareTemporaryEntityAtCoords(ResourceName prefabResource, vector coords) {
		Resource loaded = Resource.Load(prefabResource);
		if (!loaded)
			return null;

		EntitySpawnParams params = new EntitySpawnParams;
		params.TransformMode = ETransformMode.WORLD;
		params.Transform[3] = coords;
		
		return GetGame().SpawnEntityPrefab(prefabResource, false, GetGame().GetWorld(), params);
		
	}
	
	static IEntity PrepareTemporaryEntity(ResourceName prefabResource) {
		Resource loaded = Resource.Load(prefabResource);
		if (!loaded)
			return null;

		return GetGame().SpawnEntityPrefab(prefabResource, false, GetGame().GetWorld(), null);
	}
	
	static bool GetPlayerIdentityId(int playerId, out string identity) {
		BackendApi backend = GetGame().GetBackendApi();
		identity = backend.GetPlayerIdentityId(playerId);
		
		if (identity.IsEmpty()) {
			if (GetGame().IsDev()) {
				Print(string.Format("Bacon_GunBuilderUI_Helpers::GetPlayerIdentityId | Setting dev identity for playerid %1", playerId), LogLevel.DEBUG);
				identity = "dev_identity";
			} else {
				Print(string.Format("Bacon_GunBuilderUI_Helpers::GetPlayerIdentityId | No identity id for player %1", playerId), LogLevel.ERROR);
				return false;
			}
		}
		
		return true;
	}
	
	static int GetCharacterEquippedWeaponMagazineWells(IEntity characterEntity, out set<string> magWells) {
		ChimeraCharacter character = ChimeraCharacter.Cast(characterEntity);
		if (!character)
			return 0;
		
		CharacterControllerComponent controller = character.GetCharacterController();
		if (!controller)
			return 0;
		
		BaseWeaponManagerComponent weaponManager = controller.GetWeaponManagerComponent();
		if (!weaponManager)
			return 0;
		
		array<IEntity> weapons = {};
		weaponManager.GetWeaponsList(weapons);
		
		int count = 0;
		
		BaseMuzzleComponent muzzle;
		BaseMagazineWell magwell;
		
		foreach (IEntity weapon : weapons) {
			muzzle = BaseMuzzleComponent.Cast(weapon.FindComponent(BaseMuzzleComponent));
			if (!muzzle)
				continue;
			
			magwell = muzzle.GetMagazineWell();
			if (!magwell)
				continue;
			
			magWells.Insert(muzzle.GetMagazineWell().Type().ToString());
			count += 1;
		}
		
		return count;
	}
	
	static bool GetResourceNameFromEntity(IEntity ent, out ResourceName resourceName) {
		if (!ent)
			return false;

		EntityPrefabData entityPrefab = ent.GetPrefabData();
		if (!entityPrefab)
			return false;
			
		resourceName = entityPrefab.GetPrefabName();
		return true;
	}
	
	static bool GetPlayerEntityFactionKey(int playerId, out string factionKey) {
		IEntity controlledEntity = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId);
		if (!controlledEntity)
			return false;
		
		FactionAffiliationComponent factionComponent = FactionAffiliationComponent.Cast(controlledEntity.FindComponent(FactionAffiliationComponent));
		if (!factionComponent)
			return false;
		
		Faction faction = factionComponent.GetAffiliatedFaction();
		if (!faction || faction.GetFactionKey().IsEmpty())
			return false;
		
		factionKey = faction.GetFactionKey();
		return true;
	}
	
	static string GetItemNameFromEntity(IEntity ent) {
		if (!ent)
			return "";

		InventoryItemComponent itemComponent = InventoryItemComponent.Cast(ent.FindComponent(InventoryItemComponent));
		if (!itemComponent)
			return "Invalid: No Inventory Item Component";
		
		UIInfo uiInfo = itemComponent.GetUIInfo();
		if (!uiInfo)
			return "Invalid: No UI Info";
		
		return itemComponent.GetUIInfo().GetName();
	}
	
	// returns false for items set to not visible in vicinity
	// static bool GetItemNameFromEntityValidate_Arsenal(IEntity ent, out string outItemName, bool allowInvisibleArsenals = true) {
	static bool GetItemNameFromEntityValidate_Arsenal(IEntity ent, out Bacon_GunBuilderUI_SlotChoice slotChoice, Bacon_GunBuilderUI_Cache cache = null) {
		if (!ent)
			return false;

		InventoryItemComponent itemComponent = InventoryItemComponent.Cast(ent.FindComponent(InventoryItemComponent));
		if (!itemComponent) {
			Print(string.Format("Entity %1 has no Inventory Item Component!", ent), LogLevel.WARNING);
			return false;
		}

		UIInfo uiInfo = itemComponent.GetUIInfo();
		if (!uiInfo) {
			Print(string.Format("Entity %1 has no UI Info!", ent), LogLevel.WARNING);
			return false;
		}
		
		string slotName = itemComponent.GetUIInfo().GetName();
		if (slotName.IsEmpty())
			slotName = "Item";
		
		slotChoice.slotName = slotName;
		
		SCR_ItemAttributeCollection attributes = SCR_ItemAttributeCollection.Cast(itemComponent.GetAttributes());
		if (!attributes || !attributes.IsVisible()) {
			SCR_ArsenalComponent maybeArsenal = SCR_ArsenalComponent.Cast(ent.FindComponent(SCR_ArsenalComponent));
			if (maybeArsenal) {
				slotChoice.slotType = Bacon_GunBuilderUI_SlotType.ARSENAL_ITEM_ARSENAL;
				
				if (cache)
					cache.UpdateSubArsenalItems(slotChoice.prefab, maybeArsenal);
				
				return true;
			}
			
			return false;
		}
		
		slotChoice.slotType = Bacon_GunBuilderUI_SlotType.ARSENAL_ITEM;
		return true;
	}
	
	static bool GetItemNameFromEntityValidate(IEntity ent, out string outItemName) {
		if (!ent)
			return false;

		InventoryItemComponent itemComponent = InventoryItemComponent.Cast(ent.FindComponent(InventoryItemComponent));
		if (!itemComponent) {
			Print(string.Format("Entity %1 has no Inventory Item Component!", ent), LogLevel.WARNING);
			return false;
		}
		
		UIInfo uiInfo = itemComponent.GetUIInfo();
		if (!uiInfo) {
			Print(string.Format("Entity %1 has no UI Info!", ent), LogLevel.WARNING);
			return false;
		}
		
		outItemName = itemComponent.GetUIInfo().GetName();
		if (outItemName.IsEmpty())
			outItemName = "Item";
		
		return true;
	}
	
	static string GetInventorySlotName(InventoryStorageSlot slot) {	
		Print(string.Format("slot: %1", slot));
		BaseWeaponComponent weaponSlot = BaseWeaponComponent.Cast(slot.GetParentContainer());
		if (weaponSlot) {
			string slotType = weaponSlot.GetWeaponSlotType();
			string slotName = slotType.Substring(0, 1);
			slotName.ToUpper();
			return slotName + slotType.Substring(1, slotType.Length()-1);
		}
		
		return slot.GetSourceName();
	}
	
	static SCR_ConfigurableDialogUi CreateDialog(string dialogType, string dialogContent, string debugInformation = "") {
		m_dialog = SCR_ConfigurableDialogUi.CreateFromPreset(m_sDialogPresets, dialogType);
		m_dialog.SetMessage(dialogContent);
		
		if (!debugInformation.IsEmpty())
			RichTextWidget.Cast(m_dialog.GetRootWidget().FindAnyWidget("DebugMessage")).SetText(debugInformation);
		
		m_dialog.GetRootWidget().SetZOrder(1000);
		
		return m_dialog;
	}
	
	// get all storages that have slots
	static int GetAllEntityStorages(IEntity entity, out array<BaseInventoryStorageComponent> outComponents) {
		array<Managed> allComponents = {};
		
		int numStorages = entity.FindComponents(BaseInventoryStorageComponent, allComponents);
		if (numStorages < 1)
			return numStorages;
		
		int storageSlotCount;
		numStorages = 0;
		foreach (Managed component : allComponents) {
			BaseInventoryStorageComponent storageComponent = BaseInventoryStorageComponent.Cast(component);
			storageSlotCount = storageComponent.GetSlotsCount();
			
			if (storageSlotCount < 1)
				continue;
			
			outComponents.Insert(storageComponent);
			numStorages += 1;
		}
		
		return numStorages;
	}
	
	static int GetAllCharacterItemStorageSlots(IEntity entity, out array<InventoryStorageSlot> outSlots) {
		int numCharacterSlots;
		
		SCR_CharacterInventoryStorageComponent characterStorage = SCR_CharacterInventoryStorageComponent.Cast(entity.FindComponent(SCR_CharacterInventoryStorageComponent));
		if (!characterStorage)
			return 0;
		
		int numSlots = characterStorage.GetSlotsCount();
		if (numSlots < 1)
			return 0;
		
		for (int i = 0; i < numSlots; ++i) {
			InventoryStorageSlot clothingSlot = characterStorage.GetSlot(i);
			IEntity attached = clothingSlot.GetAttachedEntity();
			
			if (!attached)
				continue;
			
			BaseInventoryStorageComponent storageComp = BaseInventoryStorageComponent.Cast(attached.FindComponent(BaseInventoryStorageComponent));
			if (!storageComp)
				continue;
			
			if (!SCR_Enum.HasFlag(storageComp.GetPurpose(), EStoragePurpose.PURPOSE_DEPOSIT))
				continue;
			
			outSlots.Insert(clothingSlot);
			numCharacterSlots += 1;
		}
		
		return numCharacterSlots;
	}
	
	// get storage type for storage component
	static Bacon_GunBuilderUI_StorageType GetStorageType(BaseInventoryStorageComponent comp) {
		Print(string.Format("Bacon_GunBuilderUI_Helpers::GetStorageType | Testing storage: %1", comp), LogLevel.DEBUG);
		
		if (SCR_CharacterInventoryStorageComponent.Cast(comp))
			return Bacon_GunBuilderUI_StorageType.CHARACTER_LOADOUT;
		
		if (SCR_WeaponAttachmentsStorageComponent.Cast(comp))
			return Bacon_GunBuilderUI_StorageType.WEAPON;
		
		if (EquipedWeaponStorageComponent.Cast(comp))
			return Bacon_GunBuilderUI_StorageType.CHARACTER_WEAPON;

		if (SCR_SalineStorageComponent.Cast(comp) || SCR_TourniquetStorageComponent.Cast(comp))
			return Bacon_GunBuilderUI_StorageType.IGNORED;

		if (SCR_EquipmentStorageComponent.Cast(comp))
			return Bacon_GunBuilderUI_StorageType.CHARACTER_EQUIPMENT;
		
		return Bacon_GunBuilderUI_StorageType.DEFAULT;
	}
	
	static bool GetLoadoutAreaType(InventoryStorageSlot slot, out typename areaType) {
		LoadoutSlotInfo loadoutSlot = LoadoutSlotInfo.Cast(slot);
		if (!loadoutSlot)
			return false;
		
		LoadoutAreaType loadoutSlotAreaType = loadoutSlot.GetAreaType();
		if (!loadoutSlotAreaType)
			return false;
		
		areaType = loadoutSlotAreaType.Type();
		return true;
	}
	
	static string GetLoadoutSlotAreaTypeString(InventoryStorageSlot slot) {
		typename loadoutSlotAreaType;
		
		if (!GetLoadoutAreaType(slot, loadoutSlotAreaType))
			return "";
		
		return loadoutSlotAreaType.ToString();
	}
	
	static bool IsArsenalItemTypeValidForLoadoutArea(SCR_ArsenalItem arsenalItem, string loadoutSlotAreaTypeStr) {
		if (arsenalItem.GetItemMode() != SCR_EArsenalItemMode.DEFAULT)
			return false;
		
		return true;
	}
	
	static string GetWeaponTypeStringFromWeaponSlot(BaseWeaponComponent weapon) {
		string slotType = weapon.GetWeaponSlotType();
		slotType.ToLower();
		
		return slotType;
	}
	
	static bool GetArsenalItemTypesAndModesForWeaponSlot(string slotType, out SCR_EArsenalItemMode itemModes, out SCR_EArsenalItemType itemTypes) {
		itemModes = SCR_EArsenalItemMode.WEAPON | SCR_EArsenalItemMode.WEAPON_VARIANTS;
		
		if (slotType == "primary") {
			itemTypes = SCR_EArsenalItemType.RIFLE | SCR_EArsenalItemType.SNIPER_RIFLE | SCR_EArsenalItemType.MACHINE_GUN | SCR_EArsenalItemType.ROCKET_LAUNCHER;
			return true;
		}
		
		if (slotType == "launcher") {
			itemTypes = SCR_EArsenalItemType.ROCKET_LAUNCHER;
			return true;
		}
		
		if (slotType == "secondary") {
			itemTypes = SCR_EArsenalItemType.PISTOL;
			return true;
		}
		
		itemModes = SCR_EArsenalItemMode.DEFAULT;
		
		if (slotType == "grenade") {
			itemTypes = SCR_EArsenalItemType.LETHAL_THROWABLE;
			return true;
		}
		
		if (slotType == "throwable") {
			itemTypes = SCR_EArsenalItemType.NON_LETHAL_THROWABLE | SCR_EArsenalItemType.LETHAL_THROWABLE;
			return true;
		}
		
		return false;
	}
	
	static bool IsItemInSlotEditable(IEntity ent) {
		if (!ent)
			return false;

		BaseInventoryStorageComponent storage = BaseInventoryStorageComponent.Cast(ent.FindComponent(BaseInventoryStorageComponent));
		if (!storage)
		 	return false;
		
		// ignore for storages intended for storing items
		if (SCR_UniversalInventoryStorageComponent.Cast(storage))
			return false;
		
		// ignore if it has no slots
		if (storage.GetSlotsCount() < 1)
			return false;
		
		return true;
	}
	
	// figure out slot type from InventoryStorageSlot, which
	// hopefully speeds up lookups later
	static Bacon_GunBuilderUI_SlotType GetSlotTypeFromSlot(InventoryStorageSlot slot) {
		if (LoadoutSlotInfo.Cast(slot))
			return Bacon_GunBuilderUI_SlotType.CHARACTER_LOADOUT;
		
		if (AttachmentSlotComponent.Cast(slot.GetParentContainer()))
			return Bacon_GunBuilderUI_SlotType.ATTACHMENT;
		
		if (BaseWeaponComponent.Cast(slot.GetParentContainer()))
			return Bacon_GunBuilderUI_SlotType.CHARACTER_WEAPON;

		return Bacon_GunBuilderUI_SlotType.UNKNOWN;
	}
	
	// ---------------------- slot validation
	// try to get slot type and validate if we can use it
	// so we dont end up passing invalid slots later
	static bool ValidateAndFillSlotInfo(InventoryStorageSlot slot, out Bacon_GunBuilderUI_SlotInfo slotInfo) {
		slotInfo.slotName = "Invalid";
		slotInfo.itemName = "Unknown Slot Type";
		
		if (!slot) {
			slotInfo.slotName = "Invalid";
			slotInfo.itemName = "Invalid Slot: null";
			return false;
		}
		
		slotInfo.storageSlotId = slot.GetID();
		
		if (IsValidSlot_LoadoutSlotInfo(LoadoutSlotInfo.Cast(slot), slotInfo))
			return true;
		
		if (IsValidSlot_AttachmentSlot(AttachmentSlotComponent.Cast(slot.GetParentContainer()), slot, slotInfo))
			return true;
		
		if (IsValidSlot_CharacterWeaponSlot(BaseWeaponComponent.Cast(slot.GetParentContainer()), slotInfo))
			return true;
		
		if (IsValidSlot_CharacterEquipment(EquipmentStorageSlot.Cast(slot), slotInfo))
			return true;
		
		if (IsValidSlot_Magazine(BaseMuzzleComponent.Cast(slot.GetParentContainer()), slotInfo))
			return true;
		
		return false;
	}
	
	static bool IsValidSlot_Magazine(BaseMuzzleComponent muzzle, out Bacon_GunBuilderUI_SlotInfo slotInfo) {
		// GetMagazineWell
		if (!muzzle)
			return false;
		
		slotInfo.slotName = "Ammo";
		if (muzzle.IsDisposable()) {
			slotInfo.itemName = "Disposable Weapon";
			return false;
		}
		
		slotInfo.itemName = "Invalid Slot: No Magazine Well";
		slotInfo.slotType = Bacon_GunBuilderUI_SlotType.MAGAZINE;
		
		BaseMagazineWell magWell = muzzle.GetMagazineWell();
		if (!magWell)
			return false;
		
		slotInfo.itemName = "";
		
		return true;
	}
	
	static bool IsValidSlot_CharacterEquipment(EquipmentStorageSlot slot, out Bacon_GunBuilderUI_SlotInfo slotInfo) {
		if (!slot)
			return false;
		
		slotInfo.slotName = slot.GetSourceName();
		slotInfo.slotType = Bacon_GunBuilderUI_SlotType.CHARACTER_EQUIPMENT;
		
		return true;
	}
	
	static bool IsValidSlot_LoadoutSlotInfo(LoadoutSlotInfo slot, out Bacon_GunBuilderUI_SlotInfo slotInfo) {
		if (!slot)
			return false;
				
		LoadoutAreaType loadoutSlotAreaType = slot.GetAreaType();
		if (!loadoutSlotAreaType) {
			slotInfo.itemName = "Invalid: No LoadoutAreaType";
			return false;
		}
		
		slotInfo.slotName = slot.GetSourceName();
		slotInfo.slotType = Bacon_GunBuilderUI_SlotType.CHARACTER_LOADOUT;

		return true;
	}
	
	static bool IsValidSlot_AttachmentSlot(AttachmentSlotComponent slot, InventoryStorageSlot inventorySlot, out Bacon_GunBuilderUI_SlotInfo slotInfo) {
		if (!slot)
			return false;
		
		if (!slot.GetAttachmentSlotType()) {
			slotInfo.itemName = "Invalid: No Attachment Slot Type";
			return false;
		}
		
		slotInfo.slotName = inventorySlot.GetSourceName();
		if (slotInfo.slotName.IsEmpty())
			slotInfo.slotName = "Attachment";
		
		slotInfo.slotType = Bacon_GunBuilderUI_SlotType.ATTACHMENT;
		
		return true;
	}
	
	static bool GetWeaponSlotTypeString(BaseWeaponComponent weapon, out string outName) {
		string weaponType = weapon.GetWeaponSlotType();
		if (weaponType.IsEmpty()) {
			return false;
		}
		
		weaponType.ToLower();
		
		outName = weaponType;
		return true;
	}
	
	static bool IsValidSlot_CharacterWeaponSlot(BaseWeaponComponent weapon, out Bacon_GunBuilderUI_SlotInfo slotInfo) {
		if (!weapon)
			return false;

		string weaponType;
		if (!GetWeaponSlotTypeString(weapon, weaponType)) {
			slotInfo.slotName = "Weapon";
			slotInfo.itemName = "Invalid: No Weapon Type";
			return false;
		}

		switch (weaponType) {
			case "primary": {
				slotInfo.slotName = "Primary Weapon";
				break;
			}
			case "launcher": {
				slotInfo.slotName = "Launcher";
				break;
			}
			case "secondary": {
				slotInfo.slotName = "Secondary Weapon";
				break;
			}
			case "grenade": {
				slotInfo.slotName = "Grenade";
				break;
			}
			case "throwable": {
				slotInfo.slotName = "Throwable";
				break;
			}
			default: {
				slotInfo.slotName = "Weapon";
				slotInfo.itemName = "Invalid: Unknown Type";
				return false;
			}
		}
		
		slotInfo.slotType = Bacon_GunBuilderUI_SlotType.CHARACTER_WEAPON;
		
		return true;
	}
	// ---------------------- end slot validation

	static void PlaySoundForSlotType(Bacon_GunBuilderUI_SlotType slotType) {
		switch (slotType) {
			case Bacon_GunBuilderUI_SlotType.CHARACTER_LOADOUT: {
				AudioSystem.PlaySound("{B07341E25EBDF99F}Sounds/Items/_SharedData/Equip/Samples/Backpacks/Items_Equip_GenericBackpack_02.wav");
				break;
			}
			case Bacon_GunBuilderUI_SlotType.CHARACTER_WEAPON: {
				AudioSystem.PlaySound("{260F0D3FFD828137}Sounds/Weapons/_SharedData/PickUp/Samples/Weapons/Weapons_PickUp_Rifle_Plastic_01.wav");
				break;
			}
			case Bacon_GunBuilderUI_SlotType.CHARACTER_EQUIPMENT: {
				AudioSystem.PlaySound("{47D9797F7C0A4ECB}Sounds/Items/_SharedData/PickUp/Samples/Radio_BackPack/Items_PickUp_RadioBackpack_01.wav");
				break;
			}
			case Bacon_GunBuilderUI_SlotType.ATTACHMENT: {
				AudioSystem.PlaySound("{77825D9B98E1EED0}Sounds/Weapons/_SharedData/PickUp/Samples/Magazines/Weapons_PickUp_Magazine_Handgun_01.wav");
				break;
			}
			case Bacon_GunBuilderUI_SlotType.MAGAZINE: {
				AudioSystem.PlaySound("{41435EA7CB79EE9C}Sounds/Weapons/_SharedData/PickUp/Samples/Magazines/Weapons_PickUp_Magazine_Rifle_01.wav");
				break;
			}
			case Bacon_GunBuilderUI_SlotType.OPTION: {
				AudioSystem.PlaySound("{AC4149A069A91112}Sounds/UI/Samples/Menu/UI_Button_Confirm.wav");
				break;
			}
		}
	}
	
	static void PlaySound(string type) {
		switch (type) {
			case "blocked": {
				AudioSystem.PlaySound("{C97850E4341F0CF9}Sounds/UI/Samples/Menu/UI_Button_Fail.wav");
				break;
			}
		}
	}
}