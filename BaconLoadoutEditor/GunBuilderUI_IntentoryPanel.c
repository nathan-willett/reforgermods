class GunBuilderUI_InventoryPanelUIComponent: GunBuilderUI_SlotsUIComponent {
	ResourceName m_sResourceName_ArsenalItem = "{8F6B3BF38DC59A48}UI/BaconLoadoutEditor/GunBuilder_SlotItem_ArsenalItem.layout";

	ref ScriptInvoker m_OnItemAddRequested = new ScriptInvoker();
	
	// Inventory Panel specific stuff
	IEntity m_EditedCharacter;
	
	// magazine wells of player weapons for ammo highlight
	ref set<string> m_playerMagWells = new set<string>;
	
	ref Bacon_GunBuilderUI_Cache m_Cache;
	
	GunBuilderUI_CategoryButtonsUIComponent m_wCategorySelector;
	
	SizeLayoutWidget m_wSizeLayout;

	bool m_bIsInSubMenu = false;
		
	override void HandlerAttached(Widget w) {
		super.HandlerAttached(w);
		
		m_wCategorySelector = GunBuilderUI_CategoryButtonsUIComponent.Cast(w.FindAnyWidget("CategorySelectorInventory").FindHandler(GunBuilderUI_CategoryButtonsUIComponent));
		m_wCategorySelector.m_OnCategoryChangedInvoker.Insert(OnCategoryChanged);
		
		m_wSizeLayout = SizeLayoutWidget.Cast(w.FindAnyWidget("InventoryWrapSize"));
		m_wSizeLayout.EnableMaxDesiredHeight(true);
	}
	
	void ShowPanel(IEntity characterEntity, Bacon_GunBuilderUI_Cache cache) {
		m_EditedCharacter = characterEntity;
		m_Cache = cache;
		
		m_playerMagWells.Clear();
		Bacon_GunBuilderUI_Helpers.GetCharacterEquippedWeaponMagazineWells(characterEntity, m_playerMagWells);
		
		Print(string.Format("GunBuilderUI_InventoryPanelUIComponent.ShowPanel | Found %1 magazine wells", m_playerMagWells.Count()), LogLevel.DEBUG);
		
		m_wRoot.SetVisible(true);
	
		// ListAvailableItemsForMode(SCR_EArsenalItemMode.AMMUNITION);
		OnCategoryChanged();
	}
	void HidePanel() {
		m_wRoot.SetVisible(false);
	}
	void OnCategoryChanged() {
		SCR_EArsenalItemMode mode;
		SCR_EArsenalItemType type;
		m_wCategorySelector.GetCurrentArsenalFilter(mode, type);
		ListAvailableItemsForMode(mode, type);
	}
	
	bool IsMagazineValidForCurrentWeapons(IEntity itemEntity) {
		MagazineComponent mag = MagazineComponent.Cast(itemEntity.FindComponent(MagazineComponent));
		if (!mag)
			return false;
			
		BaseMagazineWell magWell = mag.GetMagazineWell();
		if (!magWell)
			return false;
		
		string magWellString = magWell.Type().ToString();
		if (m_playerMagWells.Contains(magWellString))
			return true;
		
		return false;
	}
	
	void ListAvailableItemsForMode(SCR_EArsenalItemMode mode, SCR_EArsenalItemType type) {
		array<SCR_ArsenalItem> availableItems = {};
		
		Clear();
		
		int numItems = m_Cache.GetArsenalItemsByMode(mode, type, availableItems);
		
		if (numItems == 0) 
			return;
		
		ItemPreviewManagerEntity previewManager = Bacon_GunBuilderUI_Helpers.GetPreviewManager();
		IEntity previewEntity;
		InventoryItemComponent itemComponent;
		
		foreach (SCR_ArsenalItem item : availableItems) {
			if (m_Cache.IsItemInAnySubArsenal(item.GetItemResourceName())) {
				Print(string.Format("ListAvailableItemsForMode | Skipping prefab %1 because it is contained in sub-arsenals", item.GetItemResourceName()), LogLevel.DEBUG);
				continue;
			}
			
			previewEntity = previewManager.ResolvePreviewEntityForPrefab(item.GetItemResourceName());
			if (!previewEntity) {
				Print(string.Format("ListAvailableItemsForMode | Failed to resolve preview entity for %1", item.GetItemResourceName()), LogLevel.WARNING);
				continue;
			}

			Bacon_GunBuilderUI_SlotChoice choice = new Bacon_GunBuilderUI_SlotChoice();
			choice.prefab = item.GetItemResourceName();
			
			if (!Bacon_GunBuilderUI_Helpers.GetItemNameFromEntityValidate_Arsenal(previewEntity, choice, m_Cache)) {
				SCR_EntityHelper.DeleteEntityAndChildren(previewEntity);
				continue;
			}

//			SCR_ArsenalComponent maybeArsenal = SCR_ArsenalComponent.Cast(previewEntity.FindComponent(SCR_ArsenalComponent));
//			if (maybeArsenal) {
//				choice.slotType = Bacon_GunBuilderUI_SlotType.ARSENAL_ITEM_ARSENAL;
//				
//				m_Cache.UpdateSubArsenalItems(choice.prefab, maybeArsenal);
//			}

			int child = AddItem(choice, m_sResourceName_ArsenalItem);
			
			if (mode == SCR_EArsenalItemMode.AMMUNITION && IsMagazineValidForCurrentWeapons(previewEntity)) {
				GunBuilderUI_MultifunctionSlotUIComponent.HighlightArsenalItem(GetItem(child));
			}
			
			if (choice.slotType == Bacon_GunBuilderUI_SlotType.ARSENAL_ITEM_ARSENAL) {
				GunBuilderUI_MultifunctionSlotUIComponent.HighlightArsenalItemArsenal(GetItem(child));
			}
			
			SCR_EntityHelper.DeleteEntityAndChildren(previewEntity);
		}
		
		int size = 64 * Math.Ceil(m_aElementComponents.Count() / 3);
		if (size < 64)
			size = 64;
		
		m_wSizeLayout.SetMaxDesiredHeight(size + 12 + 15);
	}

	override void OnClicked(GunBuilderUI_MultifunctionSlotUIComponent comp, int button) {
		// Bacon_GunBuilderUI_SlotChoice info = Bacon_GunBuilderUI_SlotChoice.Cast(comp.GetData());
		
		Bacon_GunBuilderUI_SlotChoice data = Bacon_GunBuilderUI_SlotChoice.Cast(comp.GetData());
		switch (data.slotType) {
			case Bacon_GunBuilderUI_SlotType.ARSENAL_ITEM_ARSENAL: {
				DisplaySubArsenalContent(data);
				return;
			}
			case Bacon_GunBuilderUI_SlotType.ARSENAL_ITEM_BACK: {
				ExitSubMenu();
				return;
			}
		}
		
		super.OnClicked(comp, button);
	}
	
	void DisplaySubArsenalContent(Bacon_GunBuilderUI_SlotChoice data) {
		ItemPreviewManagerEntity previewManager = Bacon_GunBuilderUI_Helpers.GetPreviewManager();
		
		IEntity mainEntity = previewManager.ResolvePreviewEntityForPrefab(data.prefab);
		if (!mainEntity) {
			Print(string.Format("DisplaySubArsenalContent | Failed to resolve preview entity for %1", data.prefab), LogLevel.WARNING);
			return;
		}
	
		SCR_ArsenalComponent subArsenal = SCR_ArsenalComponent.Cast(mainEntity.FindComponent(SCR_ArsenalComponent));
		
		array<SCR_ArsenalItem> subArsenalItems = {};
		subArsenal.GetFilteredArsenalItems(subArsenalItems);
		
		if (subArsenalItems.Count() < 1) {
			return;
		}
		
		m_bIsInSubMenu = true;
		
		Clear();
		
		IEntity previewEntity;
		
		Bacon_GunBuilderUI_SlotChoice choice = new Bacon_GunBuilderUI_SlotChoice();
		choice.slotType = Bacon_GunBuilderUI_SlotType.ARSENAL_ITEM_BACK;
		AddItem(choice, m_sResourceName_ArsenalItem);
		
		foreach (SCR_ArsenalItem item : subArsenalItems) {
			previewEntity = previewManager.ResolvePreviewEntityForPrefab(item.GetItemResourceName());
			if (!previewEntity) {
				Print(string.Format("DisplaySubArsenalContent | Failed to resolve preview entity for %1", item.GetItemResourceName()), LogLevel.WARNING);
				continue;
			}
			
			choice = new Bacon_GunBuilderUI_SlotChoice();
			choice.prefab = item.GetItemResourceName();
			// choice.slotType = Bacon_GunBuilderUI_SlotType.ARSENAL_ITEM;
			
			if (!Bacon_GunBuilderUI_Helpers.GetItemNameFromEntityValidate_Arsenal(previewEntity, choice)) {
				SCR_EntityHelper.DeleteEntityAndChildren(previewEntity);
				continue;
			}

			int child = AddItem(choice, m_sResourceName_ArsenalItem);
			
			if (item.GetItemMode() == SCR_EArsenalItemMode.AMMUNITION && IsMagazineValidForCurrentWeapons(previewEntity)) {
				GunBuilderUI_MultifunctionSlotUIComponent.HighlightArsenalItem(GetItem(child));
			}

			SCR_EntityHelper.DeleteEntityAndChildren(previewEntity);
		}
		
		int size = 64 * Math.Ceil(m_aElementComponents.Count() / 3);
		if (size < 64)
			size = 64;
		
		m_wSizeLayout.SetMaxDesiredHeight(size + 12 + 15);
	}
	
	bool IsInSubMenu() { return m_bIsInSubMenu; }
	void ExitSubMenu() { m_bIsInSubMenu = false; OnCategoryChanged(); }
}