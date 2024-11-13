class Bacon_GunBuilderUI_Cache {
	private ref map<RplId, BaseInventoryStorageComponent> m_RplId_Storage = new map<RplId, BaseInventoryStorageComponent>();
	private ref map<BaseInventoryStorageComponent, RplId> m_Storage_RplId = new map<BaseInventoryStorageComponent, RplId>();
	
	private ref map<BaseInventoryStorageComponent, Bacon_GunBuilderUI_StorageType> m_Storage_StorageType = new map<BaseInventoryStorageComponent, Bacon_GunBuilderUI_StorageType>();
	
	// cache ALL ARSENAL ITEMS
	private ref array<SCR_ArsenalItem> m_ArsenalItems = {};
	private ref set<ResourceName> m_ArsenalItemsKnownSubArsenals = new set<ResourceName>();
	private ref set<ResourceName> m_ArsenalItemsKnownSubItems = new set<ResourceName>();
	// cache available prefabs per slot type
	private ref map<string, ref array<ResourceName>> m_SlotOptions = new map<string, ref array<ResourceName>>();
	private ref map<string, ref array<SCR_ArsenalItem>> m_ArsenalItemTypes = new map<string, ref array<SCR_ArsenalItem>>();
	
	bool Init(SCR_ArsenalComponent arsenalComponent) {
		bool success = arsenalComponent.GetFilteredArsenalItems(m_ArsenalItems);
		
		return success;
	};
	
	RplId GetStorageRplId(BaseInventoryStorageComponent comp) {
		if (m_Storage_RplId.Contains(comp))
			return m_Storage_RplId.Get(comp);
		
		RplId rplId = Replication.FindId(comp);
		m_RplId_Storage.Set(rplId, comp);
		m_Storage_RplId.Set(comp, rplId);

		return rplId;
	};
	
	BaseInventoryStorageComponent GetStorageByRplId(RplId rplId) {
		return m_RplId_Storage.Get(rplId);
	};
	
	Bacon_GunBuilderUI_StorageType GetStorageType(BaseInventoryStorageComponent comp) {
		if (m_Storage_StorageType.Contains(comp))
			return m_Storage_StorageType.Get(comp);
		
		Bacon_GunBuilderUI_StorageType storageType = Bacon_GunBuilderUI_Helpers.GetStorageType(comp);
		m_Storage_StorageType.Set(comp, storageType);
		
		return storageType;
	};
	
	// chances are we have this cached
	bool TryGetPrefabsFromCache(string cacheKey, out array<ResourceName> outValidPrefabs, out int outItems) {
		if (!m_SlotOptions.Contains(cacheKey))
			return false;
		
		array<ResourceName> choices = m_SlotOptions.Get(cacheKey);
		foreach (ResourceName choice: choices) {
			outValidPrefabs.Insert(choice);
		};
		
		outItems = choices.Count();
		
		return true;
	};
	
	bool TryGetArsenalItemsFromCache(string cacheKey, out array<SCR_ArsenalItem> outValidItems, out int outItems) {
		if (!m_ArsenalItemTypes.Contains(cacheKey))
			return false;
		
		outValidItems = m_ArsenalItemTypes.Get(cacheKey);
		outItems = outValidItems.Count();
		
		return true;
	};

	int GetPrefabsForLoadoutAreaTypeSlot(Bacon_GunBuilderUI_SlotInfo slotInfo, string cacheKey, out array<ResourceName> outValidPrefabs) {
		int items;
		
		string loadoutSlotAreaTypeStr = Bacon_GunBuilderUI_Helpers.GetLoadoutSlotAreaTypeString(slotInfo.slot);
		cacheKey = string.Format("%1_%2", cacheKey, loadoutSlotAreaTypeStr);
		
		if (TryGetPrefabsFromCache(cacheKey, outValidPrefabs, items)) {
			Print(string.Format("Bacon_GunBuilderUI_Cache.GetPrefabsForLoadoutAreaTypeSlot | Fetched %1 items from Prefabs cache for %2", items, cacheKey), LogLevel.DEBUG);
			return items;
		};

		// nothing found in cache, regenerate...
		array<ResourceName> validPrefabs = {};
		
		BaseLoadoutClothComponent clothComponent;
		LoadoutAreaType itemAreaType;
		IEntity itemEntity;
		BaseWorld world = GetGame().GetWorld();
		
		// BaseInventoryStorageComponent slotStorage = slotInfo.slot.GetStorage();

		foreach (SCR_ArsenalItem arsenalItem : m_ArsenalItems) {
			if (arsenalItem.GetItemMode() != SCR_EArsenalItemMode.DEFAULT)
				continue;

			itemEntity = GetGame().SpawnEntityPrefabLocal(arsenalItem.GetItemResource(), world, null);
			if (!itemEntity)
				continue;
			
			clothComponent = BaseLoadoutClothComponent.Cast(itemEntity.FindComponent(BaseLoadoutClothComponent));
			if (clothComponent) {
				itemAreaType = clothComponent.GetAreaType();
				
				if (itemAreaType) {
					if (loadoutSlotAreaTypeStr == itemAreaType.Type().ToString()) {
						outValidPrefabs.Insert(arsenalItem.GetItemResourceName());
						validPrefabs.Insert(arsenalItem.GetItemResourceName());
						items += 1;
					}
				}
			}
			
			SCR_EntityHelper.DeleteEntityAndChildren(itemEntity);
		}
		
		m_SlotOptions.Set(cacheKey, validPrefabs);
		Print(string.Format("Bacon_GunBuilderUI_Cache.GetPrefabsForLoadoutAreaTypeSlot | Fetched %1 items for %2", items, cacheKey), LogLevel.DEBUG);
		return items;
	}

	int GetPrefabsForAttachmentSlotsTest(AttachmentSlotComponent slotComponent, string cacheKey, out array<ResourceName> outValidPrefabs) {
		int items;
		
		cacheKey = string.Format("%1_%2", cacheKey, slotComponent.GetAttachmentSlotType().Type().ToString());
		
		if (TryGetPrefabsFromCache(cacheKey, outValidPrefabs, items)) {
			Print(string.Format("Bacon_GunBuilderUI_Cache.GetPrefabsForWeaponAttachmentSlotsTest | Fetched %1 items from Prefabs cache for %2", items, cacheKey), LogLevel.DEBUG);
			return items;
		}
		
		array<ResourceName> validPrefabs = {};

		InventoryItemComponent itemComponent;
		IEntity itemEntity;
		BaseWorld world = GetGame().GetWorld();
		
		foreach (SCR_ArsenalItem arsenalItem : m_ArsenalItems) {
			// this might speed things up a little
			if (arsenalItem.GetItemMode() != SCR_EArsenalItemMode.ATTACHMENT || arsenalItem.GetItemType() != SCR_EArsenalItemType.WEAPON_ATTACHMENT)
				continue;

			itemEntity = GetGame().SpawnEntityPrefabLocal(arsenalItem.GetItemResource(), world, null);

			itemComponent = InventoryItemComponent.Cast(itemEntity.FindComponent(InventoryItemComponent));
			if (!itemComponent) {
				Print(string.Format("Prefab %1 has no InventoryItemComponent!", arsenalItem.GetItemResourceName()), LogLevel.WARNING);
				SCR_EntityHelper.DeleteEntityAndChildren(itemEntity);
				continue;
			}

			if (!slotComponent.CanSetAttachment(itemEntity)) {
				SCR_EntityHelper.DeleteEntityAndChildren(itemEntity);
				continue;
			}
			
			outValidPrefabs.Insert(arsenalItem.GetItemResourceName());
			validPrefabs.Insert(arsenalItem.GetItemResourceName());
			items += 1;
			
			SCR_EntityHelper.DeleteEntityAndChildren(itemEntity);
		}
		
		m_SlotOptions.Set(cacheKey, validPrefabs);
		Print(string.Format("Bacon_GunBuilderUI_Cache.GetPrefabsForWeaponAttachmentSlotsTest | Fetched %1 items for %2", items, cacheKey), LogLevel.DEBUG);
		return items;
	}
	
	int GetPrefabsForAttachmentSlotsWithStorage(Bacon_GunBuilderUI_SlotInfo slotInfo, string cacheKey, out array<ResourceName> outValidPrefabs) {
		int items;
		
		BaseInventoryStorageComponent storage = slotInfo.slot.GetStorage();
		AttachmentSlotComponent slotComponent = AttachmentSlotComponent.Cast(slotInfo.slot.GetParentContainer());
		
//		string baconAttachmentString = "BaconAttachmentSlotComponent";
//		typename baconAttachment = baconAttachmentString.ToType();
//		if (baconAttachment.Cast(slotInfo.slot.GetParentContainer())) {
//			
//		}
		
//		BaconAttachmentSlotComponent baconAttachment = BaconAttachmentSlotComponent.Cast(slot.GetParentContainer());
//		if (baconAttachment)
//			return baconAttachment.Bacon_CanSetAttachment(nextItem);
	
//#ifdef RISLASER_5ABD0CB57F7E9EB1
//		BaconAttachmentSlotComponent baconAttachment = BaconAttachmentSlotComponent.Cast(slot.GetParentContainer());
//		if (baconAttachment)
//			return baconAttachment.Bacon_CanSetAttachment(nextItem);
//
//		cacheKey = string.Format("%1_%2", cacheKey, slotComponent.GetAttachmentSlotType().Type().ToString());
//#else
//		cacheKey = string.Format("%1_%2", cacheKey, slotComponent.GetAttachmentSlotType().Type().ToString());
//#endif
		
		cacheKey = string.Format("%1_%2", cacheKey, slotComponent.GetAttachmentSlotType().Type().ToString());
		
		if (TryGetPrefabsFromCache(cacheKey, outValidPrefabs, items)) {
			Print(string.Format("Bacon_GunBuilderUI_Cache.GetPrefabsForWeaponAttachmentSlotsTest | Fetched %1 items from Prefabs cache for %2", items, cacheKey), LogLevel.DEBUG);
			return items;
		}
		
		array<ResourceName> validPrefabs = {};

		InventoryItemComponent itemComponent;
		IEntity itemEntity;
		BaseWorld world = GetGame().GetWorld();
		
		foreach (SCR_ArsenalItem arsenalItem : m_ArsenalItems) {
			// this might speed things up a little
			if (arsenalItem.GetItemMode() != SCR_EArsenalItemMode.ATTACHMENT || arsenalItem.GetItemType() != SCR_EArsenalItemType.WEAPON_ATTACHMENT)
				continue;

			itemEntity = GetGame().SpawnEntityPrefabLocal(arsenalItem.GetItemResource(), world, null);
			
			// Print(string.Format("Entity %1 can be attached: %2", arsenalItem.GetItemResource(), storage.CanReplaceItem(itemEntity, slotInfo.slot.GetID())));

			itemComponent = InventoryItemComponent.Cast(itemEntity.FindComponent(InventoryItemComponent));
			if (!itemComponent) {
				Print(string.Format("Prefab %1 has no InventoryItemComponent!", arsenalItem.GetItemResourceName()), LogLevel.WARNING);
				SCR_EntityHelper.DeleteEntityAndChildren(itemEntity);
				continue;
			}

			if (!slotComponent.CanSetAttachment(itemEntity)) {
				SCR_EntityHelper.DeleteEntityAndChildren(itemEntity);
				continue;
			}
//			if (!storage.CanReplaceItem(itemEntity, slotInfo.slot.GetID())) {
//				SCR_EntityHelper.DeleteEntityAndChildren(itemEntity);
//				continue;
//			}
//			if (!storage.CanStoreItem(itemEntity, slotInfo.slot.GetID())) {
//				SCR_EntityHelper.DeleteEntityAndChildren(itemEntity);
//				continue;
//			}
			
			outValidPrefabs.Insert(arsenalItem.GetItemResourceName());
			validPrefabs.Insert(arsenalItem.GetItemResourceName());
			items += 1;
			
			SCR_EntityHelper.DeleteEntityAndChildren(itemEntity);
		}
		
		m_SlotOptions.Set(cacheKey, validPrefabs);
		Print(string.Format("Bacon_GunBuilderUI_Cache.GetPrefabsForWeaponAttachmentSlotsTest | Fetched %1 items for %2", items, cacheKey), LogLevel.DEBUG);
		return items;
	}
	
	int GetPrefabsForMagazineWell(BaseMuzzleComponent muzzle, string cacheKey, out array<ResourceName> outValidPrefabs) {
		int items;
		
		string magazineWellString = muzzle.GetMagazineWell().Type().ToString();
		cacheKey = string.Format("%1_%2", cacheKey, magazineWellString);
		
		if (TryGetPrefabsFromCache(cacheKey, outValidPrefabs, items)) {
			Print(string.Format("Bacon_GunBuilderUI_Cache.GetPrefabsForMagazineWell | Fetched %1 items from Prefabs cache for %2", items, cacheKey), LogLevel.DEBUG);
			return items;
		}
		
		array<ResourceName> validPrefabs = {};
		
		InventoryItemComponent itemComponent;
		IEntity itemEntity;
		BaseWorld world = GetGame().GetWorld();
		
		foreach (SCR_ArsenalItem arsenalItem : m_ArsenalItems) {
			// this might speed things up a little
			if (arsenalItem.GetItemMode() != SCR_EArsenalItemMode.AMMUNITION)
				continue;
			
			itemEntity = GetGame().SpawnEntityPrefabLocal(arsenalItem.GetItemResource(), world, null);
			
			itemComponent = InventoryItemComponent.Cast(itemEntity.FindComponent(InventoryItemComponent));
			if (!itemComponent) {
				Print(string.Format("Prefab %1 has no InventoryItemComponent!", arsenalItem.GetItemResourceName()), LogLevel.WARNING);
				SCR_EntityHelper.DeleteEntityAndChildren(itemEntity);
				continue;
			}
			
			MagazineComponent mag = MagazineComponent.Cast(itemEntity.FindComponent(MagazineComponent));
			if (!mag) {
				Print(string.Format("Prefab %1 has no MagazineComponent!", arsenalItem.GetItemResourceName()), LogLevel.WARNING);
				SCR_EntityHelper.DeleteEntityAndChildren(itemEntity);
				continue;
			}
			
			BaseMagazineWell magWell = mag.GetMagazineWell();
			if (!magWell) {
				Print(string.Format("Prefab %1 has a MagazineComponent without a Magazine Well!", arsenalItem.GetItemResourceName()), LogLevel.WARNING);
				SCR_EntityHelper.DeleteEntityAndChildren(itemEntity);
				continue;
			}
			
			// here invisible + arsenal means we do not show them
			SCR_ItemAttributeCollection attributes = SCR_ItemAttributeCollection.Cast(itemComponent.GetAttributes());
			if (!attributes || !attributes.IsVisible()) {
				if (SCR_ArsenalComponent.Cast(itemEntity.FindComponent(SCR_ArsenalComponent))) {
					SCR_EntityHelper.DeleteEntityAndChildren(itemEntity);
					continue;
				}
			}
			
			if (magWell.Type().ToString() == magazineWellString) {
				outValidPrefabs.Insert(arsenalItem.GetItemResourceName());
				validPrefabs.Insert(arsenalItem.GetItemResourceName());
				items += 1;
			}
			
			SCR_EntityHelper.DeleteEntityAndChildren(itemEntity);
		}
		
		m_SlotOptions.Set(cacheKey, validPrefabs);
		
		Print(string.Format("Bacon_GunBuilderUI_Cache.GetPrefabsForMagazineWell | Fetched %1 items for %2", items, cacheKey), LogLevel.DEBUG);
		return items;
	}
	
	int GetPrefabsForCharacterEquipmentSlot(EquipmentStorageSlot slotComponent, string cacheKey, out array<ResourceName> outValidPrefabs) {
		int items;
		
		cacheKey = string.Format("%1_%2", cacheKey, slotComponent.GetSourceName());
		
		if (TryGetPrefabsFromCache(cacheKey, outValidPrefabs, items)) {
			Print(string.Format("Bacon_GunBuilderUI_Cache.GetPrefabsForCharacterEquipmentSlot | Fetched %1 items from Prefabs cache for %2", items, cacheKey), LogLevel.DEBUG);
			return items;
		}

		array<ResourceName> validPrefabs = {};

		InventoryItemComponent itemComponent;
		IEntity itemEntity;
		BaseWorld world = GetGame().GetWorld();
		
		foreach (SCR_ArsenalItem arsenalItem : m_ArsenalItems) {
			// this might speed things up a little
			if (arsenalItem.GetItemMode() != SCR_EArsenalItemMode.DEFAULT || arsenalItem.GetItemType() != SCR_EArsenalItemType.EQUIPMENT)
				continue;

			itemEntity = GetGame().SpawnEntityPrefabLocal(arsenalItem.GetItemResource(), world, null);
			if (!itemEntity) {
				Print(string.Format("Failed to spawn prefab: %1", arsenalItem.GetItemResource()), LogLevel.WARNING);
				continue;
			}

			itemComponent = InventoryItemComponent.Cast(itemEntity.FindComponent(InventoryItemComponent));
			if (!itemComponent) {
				Print(string.Format("Prefab %1 has no InventoryItemComponent!", arsenalItem.GetItemResourceName()), LogLevel.WARNING);
				SCR_EntityHelper.DeleteEntityAndChildren(itemEntity);
				continue;
			}

			if (!slotComponent.CanAttachItem(itemEntity)) {
				SCR_EntityHelper.DeleteEntityAndChildren(itemEntity);
				continue;
			}
			
			outValidPrefabs.Insert(arsenalItem.GetItemResourceName());
			validPrefabs.Insert(arsenalItem.GetItemResourceName());
			items += 1;
			
			SCR_EntityHelper.DeleteEntityAndChildren(itemEntity);
		}
		
		m_SlotOptions.Set(cacheKey, validPrefabs);
		
		Print(string.Format("Bacon_GunBuilderUI_Cache.GetPrefabsForCharacterEquipmentSlot | Fetched %1 items for %2", items, cacheKey), LogLevel.DEBUG);
		return items;
	}
	
	int GetPrefabsForCharacterWeaponSlot(BaseWeaponComponent weapon, string cacheKey, out array<ResourceName> outValidPrefabs) {
		int items;
		
		string weaponSlotType = Bacon_GunBuilderUI_Helpers.GetWeaponTypeStringFromWeaponSlot(weapon);
		
		cacheKey = string.Format("%1_%2", cacheKey, weaponSlotType);
		
		if (TryGetPrefabsFromCache(cacheKey, outValidPrefabs, items)) {
			Print(string.Format("Bacon_GunBuilderUI_Cache.GetPrefabsForCharacterWeaponSlot | Fetched %1 items from Prefabs cache for %2", items, cacheKey), LogLevel.DEBUG);
			return items;
		}
		
		SCR_EArsenalItemMode modes;
		SCR_EArsenalItemType types;
		
		if (!Bacon_GunBuilderUI_Helpers.GetArsenalItemTypesAndModesForWeaponSlot(weaponSlotType, modes, types)) {
			Print(string.Format("Bacon_GunBuilderUI_Cache.GetPrefabsForCharacterWeaponSlot | Unknown weapon slot type %1 cache key %2", weaponSlotType, cacheKey), LogLevel.WARNING);
			return 0;
		}
		
		array<ResourceName> validPrefabs = {};
		
		InventoryItemComponent itemComponent;
		IEntity itemEntity;
		BaseWorld world = GetGame().GetWorld();
		
		foreach (SCR_ArsenalItem arsenalItem : m_ArsenalItems) {
			// this might speed things up a little
			if (!SCR_Enum.HasPartialFlag(arsenalItem.GetItemMode(), modes) || !SCR_Enum.HasPartialFlag(arsenalItem.GetItemType(), types))
				continue;

			itemEntity = GetGame().SpawnEntityPrefabLocal(arsenalItem.GetItemResource(), world, null);

			itemComponent = InventoryItemComponent.Cast(itemEntity.FindComponent(InventoryItemComponent));
			if (!itemComponent) {
				Print(string.Format("Prefab %1 has no InventoryItemComponent!", arsenalItem.GetItemResourceName()), LogLevel.WARNING);
				SCR_EntityHelper.DeleteEntityAndChildren(itemEntity);
				continue;
			}
			
			outValidPrefabs.Insert(arsenalItem.GetItemResourceName());
			validPrefabs.Insert(arsenalItem.GetItemResourceName());
			items += 1;
			
			SCR_EntityHelper.DeleteEntityAndChildren(itemEntity);
		}
		
		m_SlotOptions.Set(cacheKey, validPrefabs);
		Print(string.Format("Bacon_GunBuilderUI_Cache.GetPrefabsForCharacterWeaponSlot | Fetched %1 items for %2", items, cacheKey), LogLevel.DEBUG);
		return items;
	}
	
	// --- sub arsenal stuff
	void UpdateSubArsenalItems(ResourceName prefabResource, SCR_ArsenalComponent arsenalComponent) {
		if (m_ArsenalItemsKnownSubArsenals.Contains(prefabResource))
			return;

//		if (m_ArsenalItemsSubArsenal.Contains(prefabResource))
//			return;
		
		array<SCR_ArsenalItem> subArsenalItems = {};
		arsenalComponent.GetFilteredArsenalItems(subArsenalItems);
		
		if (subArsenalItems.Count() < 1) { return; }
		
		m_ArsenalItemsKnownSubArsenals.Insert(prefabResource);
//		
//		set<ResourceName> resources = new set<ResourceName>;
//		
		foreach (SCR_ArsenalItem item : subArsenalItems) {
			m_ArsenalItemsKnownSubItems.Insert(item.GetItemResourceName());
		}
		
		Print(string.Format("Bacon_GunBuilderUI_Cache.UpdateSubArsenalItems | Added %1 items to cache from sub-arsenal %2", subArsenalItems.Count(), prefabResource), LogLevel.DEBUG);
		
//		m_ArsenalItemsSubArsenal.Set(prefabResource, resources);
	}
	bool IsItemInAnySubArsenal(ResourceName prefabResource) {
//		foreach (ResourceName key, set<ResourceName> value : m_ArsenalItemsSubArsenal) {
//			if (value.Contains(prefabResource))
//				return true;
//		}
		
		return m_ArsenalItemsKnownSubItems.Contains(prefabResource);
	}
	
	int GetArsenalItemsByMode(SCR_EArsenalItemMode itemMode, SCR_EArsenalItemType itemType, out array<SCR_ArsenalItem> outItems) {
		int items;
		
		outItems.Clear();
		
		// static string FlagsToString(typename e, int flags, string delimiter = ", ", string noValue = "N/A")
		string cacheKey = string.Format("ARSENAL_ITEMS_%1_%2", SCR_Enum.FlagsToString(SCR_EArsenalItemMode, itemMode, "_", ""), SCR_Enum.FlagsToString(SCR_EArsenalItemType, itemType, "_", ""));
		
		if (TryGetArsenalItemsFromCache(cacheKey, outItems, items)) {
			Print(string.Format("Bacon_GunBuilderUI_Cache.GetArsenalItemsByMode | Fetched %1 items from cache for %2", items, cacheKey), LogLevel.DEBUG);
			return items;
		}
		
		array<SCR_ArsenalItem> validItems = {};
		
		foreach (SCR_ArsenalItem arsenalItem : m_ArsenalItems) {
			if (!SCR_Enum.HasPartialFlag(arsenalItem.GetItemMode(), itemMode))
				continue;
			
			if (!SCR_Enum.HasPartialFlag(arsenalItem.GetItemType(), itemType))
				continue;
//			if (SCR_Enum.HasPartialFlag(arsenalItem.GetItemMode(), itemMode))
//				continue;

			outItems.Insert(arsenalItem);
			validItems.Insert(arsenalItem);
			items += 1;
		}
		
		m_ArsenalItemTypes.Set(cacheKey, validItems);
		Print(string.Format("Bacon_GunBuilderUI_Cache.GetArsenalItemsByMode | Fetched %1 items for %2", items, cacheKey), LogLevel.DEBUG);
		return items;
	}
	
	// will return only prefabs that are valid for this particular slot
	// will validate the prefabs are OK
	int GetChoicesForSlotType(Bacon_GunBuilderUI_SlotInfo slotInfo, out array<ResourceName> outValidPrefabs) {
		// string cacheKey = string.Format("%1_%2", SCR_Enum.GetEnumName(Bacon_GunBuilderUI_SlotInfo, slotInfo.storageType), 
		
		int items;
		string slotType = slotInfo.slot.Type().ToString();
		// string cacheKey = string.Format("%1_%2", SCR_Enum.GetEnumName(Bacon_GunBuilderUI_StorageType, slotInfo.storageType), slotType);
		string cacheKey = string.Format("%1_%2", SCR_Enum.GetEnumName(Bacon_GunBuilderUI_SlotType, slotInfo.slotType), slotType);
		
		switch (slotInfo.slotType) {
			case Bacon_GunBuilderUI_SlotType.CHARACTER_LOADOUT: {
				items = GetPrefabsForLoadoutAreaTypeSlot(slotInfo, cacheKey, outValidPrefabs);
				break;
			}
			case Bacon_GunBuilderUI_SlotType.ATTACHMENT: {
				// items = GetPrefabsForAttachmentSlotsTest(AttachmentSlotComponent.Cast(slotInfo.slot.GetParentContainer()), cacheKey, outValidPrefabs);
				items = GetPrefabsForAttachmentSlotsWithStorage(slotInfo, cacheKey, outValidPrefabs);
				break;
			}
			case Bacon_GunBuilderUI_SlotType.CHARACTER_WEAPON: {
				items = GetPrefabsForCharacterWeaponSlot(BaseWeaponComponent.Cast(slotInfo.slot.GetParentContainer()), cacheKey, outValidPrefabs);
				break;
			}
			case Bacon_GunBuilderUI_SlotType.CHARACTER_EQUIPMENT: {
				items = GetPrefabsForCharacterEquipmentSlot(EquipmentStorageSlot.Cast(slotInfo.slot), cacheKey, outValidPrefabs);
				break;
			}
			case Bacon_GunBuilderUI_SlotType.MAGAZINE: {
				items = GetPrefabsForMagazineWell(BaseMuzzleComponent.Cast(slotInfo.slot.GetParentContainer()), cacheKey, outValidPrefabs);
				break;
			}
		}
		
		Print(string.Format("Bacon_GunBuilderUI_Cache.GetChoicesForStorageSlot | Fetched %1 items", items), LogLevel.DEBUG);
		
		return items;
	};
}