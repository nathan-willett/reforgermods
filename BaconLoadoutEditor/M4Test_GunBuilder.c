modded enum ChimeraMenuPreset
{
	Bacon_GunBuilderUI,
};

enum Bacon_GunBuilderUI_Mode {
	CHARACTER,
	ENTITY,
	STORAGE,
	OPTIONS,
	LOADOUTS,
	SERVER_LOADOUTS
}
enum Bacon_GunBuilderUI_InputMode {
	VIEW_SLOTS,
	SWAP_SLOT,
	VIEW_OPTIONS,
	VIEW_ITEM_STORAGE_CONTENT,
	VIEW_ITEM_STORAGES,
	VIEW_ITEM_ARSENAL
}

enum Bacon_GunBuilderUI_StorageType {
	DEFAULT,
	CHARACTER_LOADOUT,
	CHARACTER_EQUIPMENT,
	CHARACTER_WEAPON,
	ATTACHMENT_STORAGE,
	WEAPON,
	
	LOADOUTSLOTINFO,
	ATTACHMENTSLOT,
	INVENTORY,
	OPTIONS,
	
	IGNORED
}

enum Bacon_GunBuilderUI_SlotType {
	CHARACTER_LOADOUT,
	CHARACTER_WEAPON,
	CHARACTER_EQUIPMENT,
	INVENTORY,
	ATTACHMENT,
	MAGAZINE,
	REMOVE,
	ARSENAL_ITEM,
	ARSENAL_ITEM_ARSENAL,
	ARSENAL_ITEM_BACK,
	ADD,
	OPTION,
	MESSAGE,
	LOADOUT_SAVE,
	LOADOUT_LOAD,
	UNKNOWN
}

// struct that stores slot information
class Bacon_GunBuilderUI_SlotInfo: Bacon_GunBuilderUI_SlotChoice {
	int storageRplId;
	int storageSlotId;
	// int listBoxChildId;
	
	Bacon_GunBuilderUI_StorageType storageType;
	
	InventoryStorageSlot slot;

	bool hasStorage = false;
}

class Bacon_GunBuilderUI_ItemListing {
	int numItems;
	Bacon_GunBuilderUI_SlotInfo slotInfo;
}

class Bacon_GunBuilderUI_SlotInfoStorageItem: Bacon_GunBuilderUI_SlotInfo {
	int numItems = 1;
}

class Bacon_GunBuilderUI_SlotItem {
	Bacon_GunBuilderUI_SlotType slotType;
	int listBoxChildId;
}

class Bacon_GunBuilderUI_SlotLoadout: Bacon_GunBuilderUI_SlotItem {
	string metadataClothes = "";
	string metadataWeapons = "";
	int loadoutSlotId = -1;
}

class Bacon_GunBuilderUI_SlotChoice: Bacon_GunBuilderUI_SlotItem {
	ResourceName prefab;

	string slotName = "";
	string itemName = "";
	
	bool slotEnabled = true;
	
	
}

enum Bacon_GunBuilderUI_Step {
	DEFAULT,
	MENU_OPEN,
	MENU_INIT,
	CREATE_SLOT
}

//------------------------------------------------------------------------------------------------
//! modded version for to be used with the inventory 2.0 
class Bacon_GunBuilderUI: ChimeraMenuBase {
	static ref Bacon_GunBuilderUI m_Instance;
	
	SCR_Faction m_arsenalFaction;

	InputManager m_pInputmanager;
	
	// widgets
	Widget m_root;
	// listbox
	OverlayWidget m_wSlotChoices;
	// Bacon_GunBuilderUI_ListBoxFocusComponent m_wSlotChoicesListbox;
	// listbox replacement
	GunBuilderUI_SlotsUIComponent m_wSlotChoicesListbox;
	
	// current slot widget
	// Bacon_GunBuilderUI_ListBoxElementComponent m_editedSlotInfo;
	GunBuilderUI_MultifunctionSlotUIComponent m_editedSlotInfo;
	Widget m_editedSlotInfoContainer;
	// status message widget
	GunBuilderUI_StatusMessageUIComponent m_wStatusMessageWidget;
	// navigation buttons
	SCR_InputButtonComponent m_swapButton;
	SCR_InputButtonComponent m_editButton;
	SCR_InputButtonComponent m_buttonPanCamera;
	SCR_InputButtonComponent m_buttonPanLight;
	SCR_InputButtonComponent m_removeItemButton;
	SCR_InputButtonComponent m_saveLoadoutButton;
	SCR_InputButtonComponent m_clearLoadoutButton;

	// current mode of operation (weapon, character...)
	Bacon_GunBuilderUI_Mode m_eCurrentMode;
	Bacon_GunBuilderUI_InputMode m_eCurrentInputMode;
	// current edited entity
	IEntity m_EditedEntity;
	
	// entities
	IEntity m_CharacterEntity;
	// player inventory manager
	SCR_InventoryStorageManagerComponent m_inventoryManager;
	// arsenal
	IEntity m_ArsenalEntity;
	SCR_ArsenalComponent m_arsenalComponent;
	RplId m_ArsenalComponentRplId;
	
	// information for currently edited entity
	// slot history, for going back and fetching current slot information
	ref array<ref Bacon_GunBuilderUI_SlotInfo> m_slotHistory = {};
	
	// store RplIds for known storages
	// map<int, BaseInventoryStorageComponent> m_Cache_RplId_Storage = new map<int, BaseInventoryStorageComponent>();
	// cache
	ref Bacon_GunBuilderUI_Cache m_Cache = new Bacon_GunBuilderUI_Cache();
	
	// player controller component
	Bacon_GunBuilderUI_PlayerControllerComponent m_pcComponent;
	
	// are we waiting on something?
	bool m_bIsActionInProgress = false;
	int m_iListBoxLastActionChild = -1;

	// UI components
	GunBuilderUI_PreviewUIComponent m_wPreviewWidgetComponent;
	GunBuilderUI_CategoryButtonsUIComponent m_SlotCategoryWidgetComponent;
	GunBuilderUI_InventoryPanelUIComponent m_InventoryPanelWidgetComponent;
	
	// editor options
	static ref map<string, GunBuilderUI_EditorOptionComponent> m_EditorOptions = new map<string, GunBuilderUI_EditorOptionComponent>;

	void OnCriticalError(Bacon_GunBuilderUI_Step step, string reason, string appendData = "") {
		string debugInformation = "";
		
		switch (step) {
			case Bacon_GunBuilderUI_Step.MENU_OPEN: {
				debugInformation += string.Format("m_root: %1\n", m_root);
				debugInformation += string.Format("m_wSlotChoices: %1\n", m_wSlotChoices);
				debugInformation += string.Format("m_wSlotChoicesListbox: %1\n", m_wSlotChoicesListbox);
				debugInformation += string.Format("m_editedSlotInfo: %1\n", m_editedSlotInfo);
				debugInformation += string.Format("m_editedSlotInfoContainer: %1\n", m_editedSlotInfoContainer);
				debugInformation += string.Format("m_wStatusMessageWidget: %1\n", m_wStatusMessageWidget);
				debugInformation += string.Format("m_swapButton: %1\n", m_swapButton);
				debugInformation += string.Format("m_editButton: %1\n", m_editButton);
				break;
			}
			case Bacon_GunBuilderUI_Step.MENU_INIT: {
				debugInformation += string.Format("m_CharacterEntity: %1\n", m_CharacterEntity);
				debugInformation += string.Format("m_ArsenalEntity: %1\n", m_ArsenalEntity);
				debugInformation += string.Format("m_inventoryManager: %1\n", m_inventoryManager);
				debugInformation += string.Format("m_arsenalComponent: %1\n", m_arsenalComponent);
				debugInformation += string.Format("m_arsenalFaction: %1\n", m_arsenalFaction);
				debugInformation += string.Format("m_pcComponent: %1\n", m_pcComponent);
				debugInformation += string.Format("m_pInputmanager: %1\n", m_pInputmanager);
				break;
			}
		}
		
		if (!appendData.IsEmpty()) { debugInformation += appendData; }
		
		Print(string.Format("Bacon_GunBuilderUI.OnCriticalError | Critical error:\n%1\n\n%2", reason, debugInformation), LogLevel.ERROR);

	    SCR_ConfigurableDialogUi dialog = Bacon_GunBuilderUI_Helpers.CreateDialog("error", string.Format(
			"Sorry, a critical error has occurred and this menu will close to avoid potential crashes.\n\nReason: %1", reason), debugInformation);

		dialog.m_OnConfirm.Insert(Destroy);
	}
	void ShowWarning(string message, string additionalData = "") {
		Print(string.Format("Bacon_GunBuilderUI.ShowWarning | Warning:\n%1\n\n%2", message, additionalData), LogLevel.WARNING);

	    SCR_ConfigurableDialogUi dialog = Bacon_GunBuilderUI_Helpers.CreateDialog("warning", message, additionalData);
	}
	
	static Bacon_GunBuilderUI GetInstance() {
		return m_Instance;
	}
	void RegisterOption(string optionName, GunBuilderUI_EditorOptionComponent optionComponent) {
		if (m_EditorOptions.Contains(optionName)) {
			Print(string.Format("Bacon_GunBuilderUI.RegisterOption | Component %1 tried to register option %2 but this option is already registered!", optionComponent, optionName), LogLevel.ERROR);
			return;
		}
		
		m_EditorOptions.Set(optionName, optionComponent);
	}

	override void OnMenuInit() {
		if (!m_Instance)
			m_Instance = this;
		
		m_EditorOptions.Clear();
	}

	override void OnMenuOpen() {
		super.OnMenuOpen();

		// find all widgets on open
		m_root = GetRootWidget();
		if (!m_root) { OnCriticalError(Bacon_GunBuilderUI_Step.MENU_OPEN, "OnMenuOpen:1: null variable"); return; }	
		
		// slot choices listbox
		m_wSlotChoices = OverlayWidget.Cast(m_root.FindAnyWidget("ListBoxSlotChoices"));
		// m_wSlotChoicesListbox = Bacon_GunBuilderUI_ListBoxFocusComponent.Cast(m_wSlotChoices.FindHandler(Bacon_GunBuilderUI_ListBoxFocusComponent));
		m_wSlotChoicesListbox = GunBuilderUI_SlotsUIComponent.Cast(m_wSlotChoices.FindHandler(GunBuilderUI_SlotsUIComponent));
		
		// current slot indicator
		// m_editedSlotInfo = Bacon_GunBuilderUI_ListBoxElementComponent.Cast(m_root.FindAnyWidget("EditedSlotInfo").FindHandler(Bacon_GunBuilderUI_ListBoxElementComponent));
		m_editedSlotInfo = GunBuilderUI_MultifunctionSlotUIComponent.Cast(m_root.FindAnyWidget("EditedSlotInfo").FindHandler(GunBuilderUI_MultifunctionSlotUIComponent));
		m_editedSlotInfoContainer = m_root.FindAnyWidget("EditedSlotInfoContainer");
		
		// status message
		m_wStatusMessageWidget = GunBuilderUI_StatusMessageUIComponent.Cast(m_root.FindAnyWidget("StatusMessage").FindHandler(GunBuilderUI_StatusMessageUIComponent));
		
		if (!m_wSlotChoices || !m_wSlotChoicesListbox || !m_editedSlotInfo
			|| !m_editedSlotInfoContainer || !m_wStatusMessageWidget) {
			OnCriticalError(Bacon_GunBuilderUI_Step.MENU_OPEN, "OnMenuOpen:2: null variable"); return;
		};
		
		// buttons
		Widget backLayout = m_root.FindAnyWidget("BackLayout");
		Widget footer = m_root.FindAnyWidget("Footer");
		if (!backLayout || !footer) { OnCriticalError(Bacon_GunBuilderUI_Step.MENU_OPEN, "Footer or backLayout widget is null"); }
		
		SCR_InputButtonComponent backButton = SCR_InputButtonComponent.GetInputButtonComponent("BackButton", backLayout);
		m_swapButton = SCR_InputButtonComponent.GetInputButtonComponent("SwapButton", footer);
		m_editButton = SCR_InputButtonComponent.GetInputButtonComponent("EditButton", footer);
		m_buttonPanCamera = SCR_InputButtonComponent.GetInputButtonComponent("ButtonPanCamera", footer);
		m_buttonPanLight = SCR_InputButtonComponent.GetInputButtonComponent("ButtonPanSpotlight", footer);
		m_removeItemButton = SCR_InputButtonComponent.GetInputButtonComponent("RemoveItemButton", footer);
		m_saveLoadoutButton = SCR_InputButtonComponent.GetInputButtonComponent("SaveLoadoutButton", footer);
		m_clearLoadoutButton = SCR_InputButtonComponent.GetInputButtonComponent("ClearLoadoutButton", footer);
		if (!backButton || !m_swapButton || !m_editButton) { 
			OnCriticalError(Bacon_GunBuilderUI_Step.MENU_OPEN, "OnMenuOpen:3: null variable", string.Format("backButton: %1", backButton)); return;
		}
		
		backButton.m_OnActivated.Insert(OnButtonPressed_Back);
		//m_swapButton.m_OnActivated.Insert(OnButtonPressed_Swap);
		m_editButton.m_OnActivated.Insert(OnButtonPressed_Edit);
		m_removeItemButton.m_OnActivated.Insert(OnButtonPressed_RemoveItem);
		m_saveLoadoutButton.m_OnActivated.Insert(OnButtonPressed_SaveLoadout);
		m_clearLoadoutButton.m_OnActivated.Insert(OnButtonPressed_ClearLoadout);

		m_wSlotChoicesListbox.m_OnItemClicked.Insert(OnEditedSlotChanged);
		m_wSlotChoicesListbox.m_OnItemFocused.Insert(OnSlotFocusChanged);
		m_wSlotChoicesListbox.m_OnPanelFocused.Insert(OnSlotsPanelFocused);

		m_wPreviewWidgetComponent = GunBuilderUI_PreviewUIComponent.Cast(m_root.FindAnyWidget("ItemPreviewContainer").FindHandler(GunBuilderUI_PreviewUIComponent));
		m_SlotCategoryWidgetComponent = GunBuilderUI_CategoryButtonsUIComponent.Cast(m_root.FindAnyWidget("CategorySelector").FindHandler(GunBuilderUI_CategoryButtonsUIComponent));
		m_SlotCategoryWidgetComponent.m_OnCategoryChangedInvoker.Insert(OnSlotCategoryChanged);
		
		m_InventoryPanelWidgetComponent = GunBuilderUI_InventoryPanelUIComponent.Cast(m_root.FindAnyWidget("InventoryPanelMain").FindHandler(GunBuilderUI_InventoryPanelUIComponent));
		m_InventoryPanelWidgetComponent.m_OnItemClicked.Insert(OnItemAddRequested);
		m_InventoryPanelWidgetComponent.m_OnItemFocused.Insert(OnInventoryPanelItemFocusChanged);
		
	}
	
	override void OnMenuUpdate(float tDelta) {
		m_wPreviewWidgetComponent.Update(tDelta);
		m_wStatusMessageWidget.Update(tDelta);
	}
	
	void SetCameraControlEnabled(bool enabled) {
		m_buttonPanLight.SetEnabled(enabled);
		m_buttonPanLight.SetVisible(enabled);
		m_buttonPanCamera.SetEnabled(enabled);
		m_buttonPanCamera.SetVisible(enabled);
		m_wPreviewWidgetComponent.SetPanningEnabled(enabled);
	}
	
	void TryUnequipHeldItem(IEntity character) {
		SCR_InventoryStorageManagerComponent storageManager = SCR_InventoryStorageManagerComponent.Cast(character.FindComponent(SCR_InventoryStorageManagerComponent));
		if (!storageManager)
			return;

		SCR_CharacterInventoryStorageComponent storage = storageManager.GetCharacterStorage();
		if (storage)
			storage.UnequipCurrentItem();
	}

	// initialization when the menu is accessed via action
	void Init(IEntity arsenalEntity, IEntity characterEntity) {		
		m_CharacterEntity = characterEntity;
		m_ArsenalEntity = arsenalEntity;
		if (!characterEntity || !arsenalEntity) { OnCriticalError(Bacon_GunBuilderUI_Step.MENU_INIT, "Init:1: null variable"); return; }
		
		TryUnequipHeldItem(characterEntity);
		
		m_inventoryManager = SCR_InventoryStorageManagerComponent.Cast(characterEntity.FindComponent(SCR_InventoryStorageManagerComponent));
		m_arsenalComponent = SCR_ArsenalComponent.Cast(arsenalEntity.FindComponent(SCR_ArsenalComponent));
		m_ArsenalComponentRplId = Replication.FindId(m_arsenalComponent);
		
		FactionAffiliationComponent factionComp = FactionAffiliationComponent.Cast(arsenalEntity.FindComponent(FactionAffiliationComponent));
		m_pcComponent = Bacon_GunBuilderUI_Helpers.GetPlayerControllerComponent();
		m_pInputmanager = GetGame().GetInputManager();
		if (!m_inventoryManager || !m_arsenalComponent || !factionComp || !m_pcComponent || !m_pInputmanager) { 
			OnCriticalError(Bacon_GunBuilderUI_Step.MENU_INIT, "Init:2: null variable", string.Format("factionComp: %1", factionComp)); return; }
		
		m_arsenalFaction = SCR_Faction.Cast(factionComp.GetAffiliatedFaction());
		m_pcComponent.m_OnResponse_Storage.Insert(OnServerResponse_Storage);
		m_pcComponent.m_OnResponse_Loadout.Insert(OnServerResponse_Loadout);
		// m_pcComponent.m_OnSwapRequestResponseRplId.Insert(OnSwapUpdateResponse);
		
		if (!m_Cache.Init(m_arsenalComponent)) { ShowWarning("This arsenal seems to have no items!"); }
		
		// start in Character mode
		SetUIWaiting(true);
		SetLoadoutEditorMode(Bacon_GunBuilderUI_Mode.CHARACTER);
		
		m_wPreviewWidgetComponent.Init(m_CharacterEntity);
		m_wPreviewWidgetComponent.SetPreviewedEntity(Bacon_GunBuilderUI_Mode.CHARACTER, m_CharacterEntity, null);
		
		SCR_PlayerController.Cast(GetGame().GetPlayerController()).m_OnControlledEntityChanged.Insert(OnControlledEntityChanged);
	}
	
	void DelayedUpdatePlayerCharacter() {
		m_wPreviewWidgetComponent.Init(m_CharacterEntity);
		m_wPreviewWidgetComponent.SetPreviewedEntity(Bacon_GunBuilderUI_Mode.CHARACTER, m_CharacterEntity, null);
		
		if (IsUIWaiting())
			SetUIWaiting(false);
	}
	void OnControlledEntityChanged(IEntity from, IEntity to) {
		if (!to) {
			Destroy();
			return;
		}
		
		Print(string.Format("Bacon_GunBuilderUI.OnControlledEntityChanged | New player character entity: %1", to), LogLevel.DEBUG);

		m_CharacterEntity = to;
		
		GetGame().GetCallqueue().CallLater(DelayedUpdatePlayerCharacter, 500, false);
	}
	
	// block interface until ok to proceed
	void SetUIWaiting(bool state) {
		m_bIsActionInProgress = state;
		
		if (state == false && m_iListBoxLastActionChild > -1) {
			m_wSlotChoicesListbox.SetFocus(m_iListBoxLastActionChild);
			m_iListBoxLastActionChild = -1;
		}
		
		m_wSlotChoices.SetEnabled(!state);
		m_SlotCategoryWidgetComponent.SetEnabled(!state);
		m_swapButton.SetEnabled(!state);
		m_editButton.SetEnabled(!state);
	}
	bool IsUIWaiting() {
		return m_bIsActionInProgress;
	}

	// return currently edited slot
	private Bacon_GunBuilderUI_SlotInfo GetCurrentlyEditedSlot() {
		if (m_slotHistory.Count() == 0)
			return null;
		
		return m_slotHistory.Get(m_slotHistory.Count()-1);
	}
	
	// displayed message handler
	// button handlers
	protected void OnButtonPressed_Back()
	{
		if (!IsFocused() || IsUIWaiting())
			return;
		
		int historyCount = m_slotHistory.Count();
		
		if (m_InventoryPanelWidgetComponent.IsInSubMenu()) {
			m_InventoryPanelWidgetComponent.ExitSubMenu();
			return;
		}
		
		SetCameraControlEnabled(true);

		// go back in slots
		if (historyCount > 0) {
			if (m_InventoryPanelWidgetComponent.GetRootWidget().IsVisible()) {
				m_InventoryPanelWidgetComponent.HidePanel();
				m_wSlotChoicesListbox.SetFocus(0);
				return;
			}
			
			// b4 child id 
			int focusWanted = m_slotHistory[historyCount-1].listBoxChildId;
			
			historyCount -= 1;
			m_slotHistory.Remove(historyCount);

			// if we are at root, set mode to character
			if (m_slotHistory.Count() == 0) {
				if (m_eCurrentMode == Bacon_GunBuilderUI_Mode.STORAGE) {
					SetLoadoutEditorMode(Bacon_GunBuilderUI_Mode.STORAGE);
				} else if (m_eCurrentMode == Bacon_GunBuilderUI_Mode.OPTIONS) {
					SetLoadoutEditorMode(Bacon_GunBuilderUI_Mode.OPTIONS);
				} else {
					SetLoadoutEditorMode(Bacon_GunBuilderUI_Mode.CHARACTER);
				}
				
				m_wSlotChoicesListbox.SetFocus(focusWanted);
				return;
			}
			
			// Bacon_GunBuilderUI_SlotInfo slotInfo = m_slotHistory[historyCount];
			SetViewingStorageSlots(m_slotHistory[historyCount-1]);
			
			// if (slotInfo.slotType.CHARACTER_LOADOUT
			
			UpdateEditedSlotInfo();
			CreateSlotsForCurrentEntity();
			
			m_wSlotChoicesListbox.SetFocus(focusWanted);
			
			return;
		}

		Destroy();
	}
	override void OnMenuClose() { Destroy(); }
	override void OnMenuHide() { Destroy();	}

	void OnButtonPressed_Swap() {
		if (IsUIWaiting())
			return;
		
		if (m_pInputmanager.GetLastUsedInputDevice() == EInputDeviceType.GAMEPAD)
			return;

		m_wSlotChoicesListbox.SetFocusedItemSelected();
	};
	
	void OnButtonPressed_RemoveItem() {
		if (IsUIWaiting())
			return;

		Managed data = m_wSlotChoicesListbox.GetItemData(m_wSlotChoicesListbox.GetFocusedItem());

		// inventory mode - clicked inventory item, probably
		if (m_eCurrentMode == Bacon_GunBuilderUI_Mode.STORAGE && Bacon_GunBuilderUI_SlotInfoStorageItem.Cast(data)) {
			SetUIWaiting(true);
			HandleClickedInventoryItem(Bacon_GunBuilderUI_SlotInfoStorageItem.Cast(data), 1);
		}
	}
	
	void OnButtonPressed_SaveLoadout() {
		if (IsUIWaiting())
			return;

		Managed data = m_wSlotChoicesListbox.GetItemData(m_wSlotChoicesListbox.GetFocusedItem());
		
		if (Bacon_GunBuilderUI_SlotLoadout.Cast(data)) {
			if (m_eCurrentMode == Bacon_GunBuilderUI_Mode.LOADOUTS) {
				HandleChangedLoadoutOption(Bacon_GunBuilderUI_SlotLoadout.Cast(data), 1);
			}
			if (m_eCurrentMode == Bacon_GunBuilderUI_Mode.SERVER_LOADOUTS) {
				HandleChangedLoadoutOption(Bacon_GunBuilderUI_SlotLoadout.Cast(data), 3);
			}
		}

//		if (m_eCurrentMode == Bacon_GunBuilderUI_Mode.LOADOUTS && Bacon_GunBuilderUI_SlotLoadout.Cast(data)) {
//			HandleChangedLoadoutOption(Bacon_GunBuilderUI_SlotLoadout.Cast(data), 1);
//		}
	}
	
	void OnButtonPressed_ClearLoadout() {
		if (IsUIWaiting())
			return;

		Managed data = m_wSlotChoicesListbox.GetItemData(m_wSlotChoicesListbox.GetFocusedItem());
		
		if (Bacon_GunBuilderUI_SlotLoadout.Cast(data)) {
			if (m_eCurrentMode == Bacon_GunBuilderUI_Mode.LOADOUTS) {
				HandleChangedLoadoutOption(Bacon_GunBuilderUI_SlotLoadout.Cast(data), 100);
			}
			if (m_eCurrentMode == Bacon_GunBuilderUI_Mode.SERVER_LOADOUTS) {
				HandleChangedLoadoutOption(Bacon_GunBuilderUI_SlotLoadout.Cast(data), 101);
			}
		}
		
//		if (m_eCurrentMode == Bacon_GunBuilderUI_Mode.LOADOUTS && Bacon_GunBuilderUI_SlotLoadout.Cast(data)) {
//			HandleChangedLoadoutOption(Bacon_GunBuilderUI_SlotLoadout.Cast(data), 100);
//		}
	}

	void OnButtonPressed_Edit() {
		if (IsUIWaiting())
			return;

//		if (m_pInputmanager.GetLastUsedInputDevice() == EInputDeviceType.GAMEPAD)
//			return;

		Bacon_GunBuilderUI_SlotInfo slotInfo = Bacon_GunBuilderUI_SlotInfo.Cast(m_wSlotChoicesListbox.GetItemData(m_wSlotChoicesListbox.GetFocusedItem()));
		IEntity ent = slotInfo.slot.GetAttachedEntity();
		
		if (!ent || !slotInfo.hasStorage)
			return;
		
		m_slotHistory.Insert(slotInfo);
		SetViewingStorageSlots(slotInfo);
	};

	void SetViewingStorageSlots(Bacon_GunBuilderUI_SlotInfo slotInfo) {
		if (slotInfo.slotType == Bacon_GunBuilderUI_SlotType.CHARACTER_LOADOUT) {
			SetLoadoutEditorMode(Bacon_GunBuilderUI_Mode.ENTITY, slotInfo.slot.GetAttachedEntity());

			m_wPreviewWidgetComponent.SetPreviewedEntity(Bacon_GunBuilderUI_Mode.CHARACTER, slotInfo.slot.GetAttachedEntity(), null);
			return;
		}

		if (slotInfo.slotType == Bacon_GunBuilderUI_SlotType.CHARACTER_WEAPON) {
			SetLoadoutEditorMode(Bacon_GunBuilderUI_Mode.ENTITY, slotInfo.slot.GetAttachedEntity());

			m_wPreviewWidgetComponent.SetPreviewedEntity(Bacon_GunBuilderUI_Mode.ENTITY, m_slotHistory[0].slot.GetAttachedEntity(), null);
			return;
		}
		
		if (slotInfo.slotType == Bacon_GunBuilderUI_SlotType.ATTACHMENT) {
			SetLoadoutEditorMode(Bacon_GunBuilderUI_Mode.ENTITY, slotInfo.slot.GetAttachedEntity());

			m_wPreviewWidgetComponent.SetPreviewedEntity(Bacon_GunBuilderUI_Mode.ENTITY, m_slotHistory[0].slot.GetAttachedEntity(), slotInfo);
			return;
		}
	};
	void OnSlotCategoryChanged() {
		if (m_slotHistory.Count() > 0)
			return;
		
		Bacon_GunBuilderUI_SlotCategory newCategory = m_SlotCategoryWidgetComponent.GetCurrentCategoryCharacter();
		SetRemoveItemButtonActive(false);
		SetLoadoutButtonsActive(false);
		
		switch (newCategory) {
			case Bacon_GunBuilderUI_SlotCategory.CHARACTER_CLOTHING: {
				m_swapButton.SetLabel("Swap");
				m_editButton.SetLabel("Edit");
				
				SetLoadoutEditorMode(Bacon_GunBuilderUI_Mode.CHARACTER, m_CharacterEntity);
				break;
			}
			case Bacon_GunBuilderUI_SlotCategory.CHARACTER_GEAR: {
				
				SetLoadoutEditorMode(Bacon_GunBuilderUI_Mode.CHARACTER, m_CharacterEntity);
				break;
			}
			case Bacon_GunBuilderUI_SlotCategory.CHARACTER_ITEMS: {
				m_swapButton.SetLabel("Open");
				
				SetLoadoutEditorMode(Bacon_GunBuilderUI_Mode.STORAGE, m_CharacterEntity);
				break;
			}
			case Bacon_GunBuilderUI_SlotCategory.SETTINGS: {
				m_swapButton.SetLabel("Change");
				SetLoadoutEditorMode(Bacon_GunBuilderUI_Mode.OPTIONS, m_CharacterEntity);
				break;
			}
			case Bacon_GunBuilderUI_SlotCategory.SERVER_LOADOUTS: {
				SetUIWaiting(true);
				SetLoadoutEditorMode(Bacon_GunBuilderUI_Mode.SERVER_LOADOUTS, m_CharacterEntity);
				break;
			}
			case Bacon_GunBuilderUI_SlotCategory.LOADOUTS: {
				SetUIWaiting(true);
				SetLoadoutEditorMode(Bacon_GunBuilderUI_Mode.LOADOUTS, m_CharacterEntity);
				break;
			}
		}
	}
	
	void OnEditedSlotChanged(GunBuilderUI_SlotsUIComponent comp, Managed data, int button) {
		if (IsUIWaiting())
			return;

		// inventory mode - clicked inventory item, probably
		if (m_eCurrentMode == Bacon_GunBuilderUI_Mode.STORAGE && Bacon_GunBuilderUI_SlotInfoStorageItem.Cast(data)) {
			HandleClickedInventoryItem(Bacon_GunBuilderUI_SlotInfoStorageItem.Cast(data), button);
			return;
		}
		
		if (m_eCurrentMode == Bacon_GunBuilderUI_Mode.OPTIONS && m_slotHistory.Count() > 0) {
			HandleChangedEditorOption(GunBuilderUI_EditorOptionData.Cast(data));
			return;
		}
		
		if (m_eCurrentMode == Bacon_GunBuilderUI_Mode.LOADOUTS) {
			HandleChangedLoadoutOption(Bacon_GunBuilderUI_SlotLoadout.Cast(data), button);
			return;
		}
		
		if (m_eCurrentMode == Bacon_GunBuilderUI_Mode.SERVER_LOADOUTS) {
			HandleChangedLoadoutOption(Bacon_GunBuilderUI_SlotLoadout.Cast(data), button+2);
			return;
		}
				
		Bacon_GunBuilderUI_SlotInfo slotInfo = Bacon_GunBuilderUI_SlotInfo.Cast(data);
		if (slotInfo) {
			HandleChangedSlot(slotInfo);
			return;
		}

		Bacon_GunBuilderUI_SlotChoice slotChoice = Bacon_GunBuilderUI_SlotChoice.Cast(data);
		if (slotChoice) {
			HandleChangedOption(slotChoice);
			return;
		}
	}
	
	// button: 0 means add, 1 means remove
	void HandleClickedInventoryItem(Bacon_GunBuilderUI_SlotInfoStorageItem itemInfo, int button) {
		// add item under pointer
		Bacon_GunBuilderUI_Network_StorageRequest request = new Bacon_GunBuilderUI_Network_StorageRequest();

		request.storageRplId = m_Cache.GetStorageRplId(itemInfo.slot.GetStorage());
		
		// m_iListBoxLastActionChild = itemInfo.listBoxChildId;
		m_iListBoxLastActionChild = m_wSlotChoicesListbox.GetItem(m_wSlotChoicesListbox.GetFocusedItem()).GetZOrder();
		

		switch (button) {
			case 0: {
				request.storageSlotId = -1;
				request.actionType = Bacon_GunBuilderUI_ActionType.ADD_ITEM;
				
				Bacon_GunBuilderUI_Helpers.GetResourceNameFromEntity(itemInfo.slot.GetAttachedEntity(), request.prefab);
				break;
			}
			case 1: {
				request.storageSlotId = itemInfo.storageSlotId;
				request.actionType = Bacon_GunBuilderUI_ActionType.REMOVE_ITEM;
//				if (itemInfo.numItems == 1 && m_iListBoxLastActionChild > 0)
//					m_iListBoxLastActionChild = m_iListBoxLastActionChild - 1;
					// m_iListBoxLastActionChild = itemInfo.listBoxChildId - 1;

				break;
			}
		}
		
		// m_iListBoxLastActionChild = ;
		// test 

		m_pcComponent.RequestAction(request);
	}
	
	void HandleChangedEditorOption(GunBuilderUI_EditorOptionData selected) {
		GunBuilderUI_EditorOptionComponent optionComponent = m_EditorOptions.Get(selected.editorOptionLabel);
		optionComponent.OnOptionSelected(selected.optionValue);
		
//		Print("Listbox child id: "+selected.optionLabel);
//		Print("Listbox child id: "+selected.optionValue);
//		Print("Listbox child id: "+selected.listBoxChildId);
		
		m_iListBoxLastActionChild = m_wSlotChoicesListbox.GetFocusedItem();
		
		// UpdateEditedSlotInfo();
		m_editedSlotInfo.SetData(selected);
		CreateOptionsForSlot();
		SetUIWaiting(false);
	}
	
	void HandleChangedSlot(Bacon_GunBuilderUI_SlotInfo slotInfo) {
		if (!slotInfo.slotEnabled) {
			Bacon_GunBuilderUI_Helpers.PlaySound("blocked");
			SetUIWaiting(false);
			return;
		}

		m_slotHistory.Insert(slotInfo);

		switch (slotInfo.slotType) {
			case Bacon_GunBuilderUI_SlotType.MAGAZINE:
			case Bacon_GunBuilderUI_SlotType.ATTACHMENT: {
				m_wPreviewWidgetComponent.SetPreviewedEntity(Bacon_GunBuilderUI_Mode.ENTITY, m_slotHistory[0].slot.GetAttachedEntity(), slotInfo);
				break;
			}
			case Bacon_GunBuilderUI_SlotType.CHARACTER_LOADOUT: {
				if (m_eCurrentMode == Bacon_GunBuilderUI_Mode.STORAGE) {
					m_wPreviewWidgetComponent.SetPreviewedEntity(Bacon_GunBuilderUI_Mode.ENTITY, m_slotHistory[0].slot.GetAttachedEntity(), slotInfo);
				} else {				
					m_wPreviewWidgetComponent.SetPreviewedEntity(Bacon_GunBuilderUI_Mode.CHARACTER, m_CharacterEntity, slotInfo);
				}
				break;
			}
			case Bacon_GunBuilderUI_SlotType.OPTION: {
				break;
			}
			default: {
				m_wPreviewWidgetComponent.SetPreviewedEntity(Bacon_GunBuilderUI_Mode.CHARACTER, m_CharacterEntity, slotInfo);
				break;
			}
		}
		
		UpdateEditedSlotInfo();
		CreateOptionsForSlot();
		SetUIWaiting(false);
	}
	
	void SetEditButtonActive(bool active) {
		if (active == m_editButton.IsVisible())
			return;

		m_editButton.SetEnabled(active);
		m_editButton.SetVisible(active);
	}
	// void OnSlotFocusChanged(int focusedListBoxItem) {
	void OnSlotFocusChanged(GunBuilderUI_SlotsUIComponent component, Managed data) {
		if (Bacon_GunBuilderUI_SlotInfoStorageItem.Cast(data)) {
			SetRemoveItemButtonActive(true);
			return;
		}
		
		SetRemoveItemButtonActive(false);
		
		Bacon_GunBuilderUI_SlotInfo slotInfo = Bacon_GunBuilderUI_SlotInfo.Cast(data);
		if (!slotInfo) {
			SetEditButtonActive(false);
			return;
		}
		
		SetEditButtonActive(slotInfo.hasStorage);
	}
	
	void SetRemoveItemButtonActive(bool active) {
		if (m_removeItemButton.IsEnabled() == active)
			return;
		
		m_removeItemButton.SetEnabled(active);
		m_removeItemButton.SetVisible(active);
	}
	void SetLoadoutButtonsActive(bool active) {
		if (m_saveLoadoutButton.IsEnabled() == active)
			return;
		
		m_saveLoadoutButton.SetEnabled(active);
		m_saveLoadoutButton.SetVisible(active);
		m_clearLoadoutButton.SetEnabled(active);
		m_clearLoadoutButton.SetVisible(active);
	}
	
	// before menu terminates
	protected void Destroy() {
		SCR_PlayerController.Cast(GetGame().GetPlayerController()).m_OnControlledEntityChanged.Remove(OnControlledEntityChanged);
		
		MenuManager menuManager = GetGame().GetMenuManager();
		menuManager.CloseMenuByPreset(ChimeraMenuPreset.Bacon_GunBuilderUI);
		m_Instance = null;
	};
	
	// set mode
	void SetLoadoutEditorMode(Bacon_GunBuilderUI_Mode newMode, IEntity editedEntity = null) {
		m_eCurrentMode = newMode;
		
		m_InventoryPanelWidgetComponent.HidePanel();
		
		m_swapButton.SetLabel("Swap");
		m_editButton.SetLabel("Edit");
		
		if (newMode == Bacon_GunBuilderUI_Mode.ENTITY) {
			if (!editedEntity) {
				OnCriticalError(Bacon_GunBuilderUI_Step.DEFAULT, "SetLoadoutEditorMode:1: null entity", string.Format("editedEntity: %1", editedEntity)); return; }

			m_EditedEntity = editedEntity;
		} else {
			m_EditedEntity = m_CharacterEntity;
			m_wPreviewWidgetComponent.SetPreviewedEntity(Bacon_GunBuilderUI_Mode.CHARACTER, m_CharacterEntity, null);
		}

		switch (newMode) {
			case Bacon_GunBuilderUI_Mode.STORAGE: {
				m_swapButton.SetLabel("Open");
				break;
			}
			case Bacon_GunBuilderUI_Mode.OPTIONS: {
				m_swapButton.SetLabel("Change");
				break;
			}
			case Bacon_GunBuilderUI_Mode.LOADOUTS:
			case Bacon_GunBuilderUI_Mode.SERVER_LOADOUTS: {
				m_swapButton.SetLabel("Load Loadout");
				SetLoadoutButtonsActive(true);
				break;
			}
		}

		CreateSlotsForCurrentEntity();
		UpdateEditedSlotInfo();
	};
	
	void OnInventoryPanelItemFocused() {
		Print("OnInventoryPanelFocused");
	}
	void OnSlotsPanelFocused() {
		Print("OnSlotsPanelFocused");
	}
	void OnInventoryPanelItemFocusChanged(GunBuilderUI_SlotsUIComponent component, Managed data) {
		SetRemoveItemButtonActive(false);
	}

	// ------------ creating slots
	void CreateSlotsForCurrentEntity() {
		if (!m_EditedEntity) { 
			OnCriticalError(Bacon_GunBuilderUI_Step.DEFAULT, "CreateSlotsForCurrentEntity:1: null variable", string.Format("m_EditedEntity: %1", m_EditedEntity)); return; }
	
		switch (m_eCurrentMode) {
			case Bacon_GunBuilderUI_Mode.CHARACTER: {
				CreateSlotsForMultipleStorages();
				break;
			}
			case Bacon_GunBuilderUI_Mode.ENTITY: {
				CreateSlotsForStorage(Bacon_GunBuilderUI_StorageType.WEAPON, 
					BaseInventoryStorageComponent.Cast(m_EditedEntity.FindComponent(BaseInventoryStorageComponent)));
				break;
			}
			case Bacon_GunBuilderUI_Mode.STORAGE: {
				CreateSlotsForCharacterStorages();
				break;
			}
			case Bacon_GunBuilderUI_Mode.OPTIONS: {
				CreateSlotsForEditorOptions();
				break;
			}
			case Bacon_GunBuilderUI_Mode.LOADOUTS: {
				RequestLoadoutSlotOptions();
				return;
			}
			case Bacon_GunBuilderUI_Mode.SERVER_LOADOUTS: {
				RequestLoadoutSlotOptions(true);
				return;
			}
		}
		
		SetUIWaiting(false);
	};
	
	void CreateSlotsForMultipleStorages() {		
		array<BaseInventoryStorageComponent> storages = {};
		int numStorages = Bacon_GunBuilderUI_Helpers.GetAllEntityStorages(m_EditedEntity, storages);
		
		if (numStorages < 1) {
			ShowWarning("No storage component with slots found in entity", string.Format("Entity: %1", m_EditedEntity)); return;
		}
		
		m_wSlotChoicesListbox.Clear();
		
		foreach (BaseInventoryStorageComponent storage : storages) {
			Bacon_GunBuilderUI_StorageType storageType = m_Cache.GetStorageType(storage);
			if (storageType == Bacon_GunBuilderUI_StorageType.IGNORED)
				continue;
			
			if (!m_SlotCategoryWidgetComponent.ShouldViewCharacterStorageCategory(storageType))
				continue;
			
			CreateSlotsForStorage(storageType, storage, false);
		}
	};
	
	void CreateSlotsForCharacterStorages() {
		array<InventoryStorageSlot> storageSlots = {};
		int numStorageSlots = Bacon_GunBuilderUI_Helpers.GetAllCharacterItemStorageSlots(m_CharacterEntity, storageSlots);
		
		m_wSlotChoicesListbox.Clear();
		
		if (numStorageSlots < 1) {
			CreateDummy("No storage found");
			return;
			// ShowWarning("No storage component with slots found in entity", string.Format("Entity: %1", m_EditedEntity)); return;
		}

		foreach (InventoryStorageSlot storageSlot : storageSlots) {
			CreateStorageSlot(Bacon_GunBuilderUI_StorageType.CHARACTER_LOADOUT, storageSlot.GetStorage(), storageSlot.GetID());
		}
	}
	
	void CreateSlotsForStorage(Bacon_GunBuilderUI_StorageType storageType, BaseInventoryStorageComponent storage, bool clearFirst = true) {
		if (clearFirst)
			m_wSlotChoicesListbox.Clear();

		int count = storage.GetSlotsCount();

		for (int i = 0; i < count; ++i) {
			CreateStorageSlot(storageType, storage, i);
		}
	};
	
	void CreateSlotsForEditorOptions(Bacon_GunBuilderUI_SlotInfo optionData = null) {
		m_wSlotChoicesListbox.Clear();
		
		if (optionData) {
			GunBuilderUI_EditorOptionData selected = GunBuilderUI_EditorOptionData.Cast(optionData);
			GunBuilderUI_EditorOptionComponent optionComponent = m_EditorOptions.Get(selected.editorOptionLabel);
			
			array<GunBuilderUI_EditorOptionData> optionChoices = {};
			
			int choices = optionComponent.GetChoices(optionChoices);
			
			for (int i = 0; i < choices; i++) {
				m_wSlotChoicesListbox.AddItem_Option(optionChoices[i]);
			}
			
			return;
		}
		
		foreach (string key, GunBuilderUI_EditorOptionComponent optionComponent : m_EditorOptions) {
			GunBuilderUI_EditorOptionData selected = optionComponent.GetSelectedOption();
			m_wSlotChoicesListbox.AddItem_Option(selected);
		}
	}
	
	// ---- loadout options
	void CreateSlotsForLoadoutOptions(string payload) {
		m_wSlotChoicesListbox.Clear();

		SCR_JsonLoadContext ctx = new SCR_JsonLoadContext();
		ctx.ImportFromString(payload);
		array<ref Bacon_GunBuilder_PlayerLoadout> loadouts = {};

		ctx.ReadValue("loadouts", loadouts);

		foreach (Bacon_GunBuilder_PlayerLoadout loadout : loadouts) {
			Bacon_GunBuilderUI_SlotLoadout choice = new Bacon_GunBuilderUI_SlotLoadout();
			
			if (loadout.metadata_clothes.IsEmpty()) {
				choice.metadataClothes = "Empty Slot";
			} else {
				choice.metadataClothes = loadout.metadata_clothes;
			}

			choice.metadataWeapons = loadout.metadata_weapons;
			choice.loadoutSlotId = loadout.slotId;

			m_wSlotChoicesListbox.AddItem_Loadout(choice);
		}
		
		m_wSlotChoicesListbox.SetFocus(0);
		
		SetUIWaiting(false);
	}
	void RequestLoadoutSlotOptions(bool adminLoadouts = false) {
		SetUIWaiting(true);
		
		Bacon_GunBuilderUI_Network_LoadoutRequest request = new Bacon_GunBuilderUI_Network_LoadoutRequest();
		request.arsenalComponentRplId = m_ArsenalComponentRplId;
		
		if (adminLoadouts) {
			request.actionType = Bacon_GunBuilderUI_ActionType.GET_ADMIN_LOADOUTS;
		} else {
			request.actionType = Bacon_GunBuilderUI_ActionType.GET_LOADOUTS;
		}
		
		m_pcComponent.RequestLoadoutAction(request);
	}
	void HandleChangedLoadoutOption(Bacon_GunBuilderUI_SlotLoadout slotInfo, int button) {
		if (!slotInfo) {
			Bacon_GunBuilderUI_Helpers.PlaySound("blocked");
			SetUIWaiting(false);
			return;
		}
		
		SetUIWaiting(true);
		
		Bacon_GunBuilderUI_Network_LoadoutRequest request = new Bacon_GunBuilderUI_Network_LoadoutRequest();
		
		m_iListBoxLastActionChild = m_wSlotChoicesListbox.GetFocusedItem();

		switch (button) {
			case 0: {
				if (slotInfo.metadataWeapons == "") {
					Bacon_GunBuilderUI_Helpers.PlaySound("blocked");
					SetUIWaiting(false);
					return;
				}
				request.actionType = Bacon_GunBuilderUI_ActionType.APPLY_LOADOUT;
				request.loadoutSlotId = slotInfo.loadoutSlotId;
				break;
			}
			case 1: {
				request.actionType = Bacon_GunBuilderUI_ActionType.SAVE_LOADOUT;
				request.loadoutSlotId = slotInfo.loadoutSlotId;
				break;
			}
			case 2: {
				if (slotInfo.metadataWeapons == "") {
					Bacon_GunBuilderUI_Helpers.PlaySound("blocked");
					SetUIWaiting(false);
					return;
				}
				request.actionType = Bacon_GunBuilderUI_ActionType.APPLY_LOADOUT_ADMIN;
				request.loadoutSlotId = slotInfo.loadoutSlotId;
				break;
			}
			case 3: {
				request.actionType = Bacon_GunBuilderUI_ActionType.SAVE_LOADOUT_ADMIN;
				request.loadoutSlotId = slotInfo.loadoutSlotId;
				break;
			}
			case 100: {
				request.actionType = Bacon_GunBuilderUI_ActionType.CLEAR_LOADOUT;
				request.loadoutSlotId = slotInfo.loadoutSlotId;
				break;
			}
			case 101: {
				request.actionType = Bacon_GunBuilderUI_ActionType.CLEAR_LOADOUT_ADMIN;
				request.loadoutSlotId = slotInfo.loadoutSlotId;
				break;
			}
		}

		request.arsenalComponentRplId = m_ArsenalComponentRplId;
		m_pcComponent.RequestLoadoutAction(request);
	}

	// ---------- create a slot
	void CreateStorageSlot(Bacon_GunBuilderUI_StorageType storageType, BaseInventoryStorageComponent storage, int storageSlotId) {
		InventoryStorageSlot storageSlot = storage.GetSlot(storageSlotId);

		Bacon_GunBuilderUI_SlotInfo slotInfo = new Bacon_GunBuilderUI_SlotInfo();
		if (!Bacon_GunBuilderUI_Helpers.ValidateAndFillSlotInfo(storageSlot, slotInfo)) {
			slotInfo.slotEnabled = false;
		} else {
			IEntity ent = storageSlot.GetAttachedEntity();
			
			if (m_eCurrentMode == Bacon_GunBuilderUI_Mode.CHARACTER || m_eCurrentMode == Bacon_GunBuilderUI_Mode.ENTITY)
				slotInfo.hasStorage = Bacon_GunBuilderUI_Helpers.IsItemInSlotEditable(ent);

			slotInfo.itemName = Bacon_GunBuilderUI_Helpers.GetItemNameFromEntity(ent);
		}

		slotInfo.storageType = storageType;
		slotInfo.slot = storageSlot;

		// slotInfo.listBoxChildId = m_wSlotChoicesListbox.AddItem("storageSlot", slotInfo);
		slotInfo.listBoxChildId = m_wSlotChoicesListbox.AddItem_Slot(slotInfo);
	};

	void HandleChangedOption(Bacon_GunBuilderUI_SlotChoice slotChoice) {
		// activate choices mode for items!
		if (slotChoice.slotType == Bacon_GunBuilderUI_SlotType.ADD && m_eCurrentMode == Bacon_GunBuilderUI_Mode.STORAGE) {
			m_InventoryPanelWidgetComponent.ShowPanel(m_CharacterEntity, m_Cache);
			SetCameraControlEnabled(false);
			SetUIWaiting(false);
			return;
		}
		
		if (!slotChoice.slotEnabled) {
			Bacon_GunBuilderUI_Helpers.PlaySound("blocked");
			SetUIWaiting(false);
			return;
		}
		
		if (slotChoice.slotType == Bacon_GunBuilderUI_SlotType.MESSAGE) {
			Bacon_GunBuilderUI_Helpers.PlaySound("blocked");
			SetUIWaiting(false);
			return;
		}
		
		Bacon_GunBuilderUI_SlotInfo editedSlot = GetCurrentlyEditedSlot();
		IEntity attached = editedSlot.slot.GetAttachedEntity();
		if (slotChoice.prefab == "empty" && !attached) {
			Bacon_GunBuilderUI_Helpers.PlaySound("blocked");
			SetUIWaiting(false);
			return;
		}
		
		ResourceName resourceName;
		Bacon_GunBuilderUI_Helpers.GetResourceNameFromEntity(attached, resourceName);
		if (resourceName == slotChoice.prefab) {
			Bacon_GunBuilderUI_Helpers.PlaySound("blocked");
			SetUIWaiting(false);
			return;
		}

		m_iListBoxLastActionChild = slotChoice.listBoxChildId;
		
		Bacon_GunBuilderUI_Network_StorageRequest request = new Bacon_GunBuilderUI_Network_StorageRequest();
		
		if (slotChoice.prefab == "empty")
			request.actionType = Bacon_GunBuilderUI_ActionType.REMOVE_ITEM;
		else
			request.actionType = Bacon_GunBuilderUI_ActionType.REPLACE_ITEM;
		
		request.storageRplId = m_Cache.GetStorageRplId(editedSlot.slot.GetStorage());
		request.storageSlotId = editedSlot.storageSlotId;
		request.prefab = slotChoice.prefab;
		
		m_pcComponent.RequestAction(request);

		// m_pcComponent.RequestAttachmentSwapByRplId(m_Cache.GetStorageRplId(editedSlot.slot.GetStorage()), editedSlot.storageSlotId, slotChoice.prefab);
	}
	
//	void SetButtonsVisible(bool camera, bool removeItem) {
//		if (camera != m_buttonPanLight.IsVisible()) {
//			m_buttonPanLight.SetEnabled(camera);
//			m_buttonPanLight.SetVisible(camera);
//			m_buttonPanCamera.SetEnabled(camera);
//			m_buttonPanCamera.SetVisible(camera);
//		}
//		
//		if (removeItem != m_removeItemButton.IsVisible()) {
//			m_removeItemButton.SetEnabled(removeItem);
//			m_removeItemButton.SetVisible(removeItem);
//			if (removeItem) {
//				m_editButton.SetEnabled(false);
//				m_editButton.SetVisible(false);
//			}
//		}
//	}

	// --------- response received
//	void OnSwapUpdateResponse(bool success, RplId storageRplId, int slotId, string message) {
//		HandleMessage(success, message);
//		GetGame().GetCallqueue().CallLater(RefreshUpdatedSlot, 66, false, success, storageRplId, slotId);
//	}
	void OnServerResponse_Storage(Bacon_GunBuilderUI_Network_Response response, Bacon_GunBuilderUI_Network_StorageRequest request) {
		HandleMessage(response.success, response.message);	
		GetGame().GetCallqueue().CallLater(RefreshUpdatedSlot, 66, false, response.success, request.storageRplId, request.storageSlotId);
	}
	void OnServerResponse_Loadout(Bacon_GunBuilderUI_Network_Response response, Bacon_GunBuilderUI_Network_LoadoutRequest request) {
		if (!response.success) {
			HandleMessage(response.success, response.message);
			
			m_wSlotChoicesListbox.Clear();
			CreateDummy("Loadouts unavailable");
			
			SetUIWaiting(false);
			return;
		}
		
		switch (request.actionType) {
			case Bacon_GunBuilderUI_ActionType.GET_ADMIN_LOADOUTS:
			case Bacon_GunBuilderUI_ActionType.GET_LOADOUTS: {
				CreateSlotsForLoadoutOptions(response.message);
				break;
			}
			case Bacon_GunBuilderUI_ActionType.SAVE_LOADOUT_ADMIN:
			case Bacon_GunBuilderUI_ActionType.SAVE_LOADOUT: {
				CreateSlotsForLoadoutOptions(response.message);
				HandleMessage(true, "Saved");
				Bacon_GunBuilderUI_Helpers.PlaySoundForSlotType(Bacon_GunBuilderUI_SlotType.CHARACTER_LOADOUT);
				break;
			}
			case Bacon_GunBuilderUI_ActionType.CLEAR_LOADOUT_ADMIN:
			case Bacon_GunBuilderUI_ActionType.CLEAR_LOADOUT: {
				CreateSlotsForLoadoutOptions(response.message);
				HandleMessage(true, "Cleared");
				Bacon_GunBuilderUI_Helpers.PlaySoundForSlotType(Bacon_GunBuilderUI_SlotType.CHARACTER_LOADOUT);
				break;
			}
			case Bacon_GunBuilderUI_ActionType.APPLY_LOADOUT_ADMIN:
			case Bacon_GunBuilderUI_ActionType.APPLY_LOADOUT: {
				HandleMessage(true, "Applied");
				Bacon_GunBuilderUI_Helpers.PlaySoundForSlotType(Bacon_GunBuilderUI_SlotType.CHARACTER_LOADOUT);
				break;
			}
		}
	}

	void HandleMessage(bool success, string message) {
		if (success) {
			m_wStatusMessageWidget.DisplayMessage(GunBuilderUI_StatusMessageUIComponent_Messagetype.OK, message);
			return;
		}
		
		m_wStatusMessageWidget.DisplayMessage(GunBuilderUI_StatusMessageUIComponent_Messagetype.ERROR, message);
	}
	
	// refresh a slot in a storage
	void RefreshUpdatedSlot(bool success, RplId storageRplId, int slotId) {
		// current slot info
		Bacon_GunBuilderUI_SlotInfo slotInfo = GetCurrentlyEditedSlot();
		slotInfo.itemName = "";
		
		if (success) {
			Bacon_GunBuilderUI_Helpers.PlaySoundForSlotType(slotInfo.slotType);
		}
				
		Bacon_GunBuilderUI_Helpers.GetItemNameFromEntityValidate(slotInfo.slot.GetAttachedEntity(), slotInfo.itemName);
		
		// update entity in slot
		m_editedSlotInfo.SetData(slotInfo);
		
		// update preview
		if (m_eCurrentMode == Bacon_GunBuilderUI_Mode.CHARACTER) {
			m_wPreviewWidgetComponent.SetPreviewedEntity(Bacon_GunBuilderUI_Mode.CHARACTER, m_CharacterEntity, slotInfo, false);
		}
		else if (m_eCurrentMode == Bacon_GunBuilderUI_Mode.STORAGE) {
			ListItemsForStorage();
		}
		else
		{
			// if (slotInfo.slotType == Bacon_GunBuilderUI_SlotType.CHARACTER_WEAPON || slotInfo.slotType == Bacon_GunBuilderUI_SlotType.CHARACTER_WEAPON || 
			switch (slotInfo.slotType) {
				case Bacon_GunBuilderUI_SlotType.CHARACTER_WEAPON:
				case Bacon_GunBuilderUI_SlotType.ATTACHMENT:
				case Bacon_GunBuilderUI_SlotType.MAGAZINE: {
					m_wPreviewWidgetComponent.SetPreviewedEntity(Bacon_GunBuilderUI_Mode.ENTITY, m_slotHistory[0].slot.GetAttachedEntity(), slotInfo);
					break;
				}
				default: {
					m_wPreviewWidgetComponent.SetPreviewedEntity(Bacon_GunBuilderUI_Mode.CHARACTER, m_CharacterEntity, slotInfo);
				}
			}
		}

		SetUIWaiting(false);
	}

	void UpdateEditedSlotInfo() {
		if (m_slotHistory.Count() == 0) {
			m_editedSlotInfoContainer.SetVisible(false); 
			m_SlotCategoryWidgetComponent.SetWidgetEnabled(true);
			return; 
		}
		m_SlotCategoryWidgetComponent.SetWidgetEnabled(false);
		
		Bacon_GunBuilderUI_SlotInfo slotInfo = m_slotHistory.Get(m_slotHistory.Count() - 1);
		
		m_editedSlotInfo.SetData(slotInfo);
		
		m_editedSlotInfoContainer.SetVisible(true);
	}

	// -------- create choices for slot
	void CreateRemoveSlotOption(InventoryStorageSlot slot) {
		Bacon_GunBuilderUI_SlotChoice slotChoice = new Bacon_GunBuilderUI_SlotChoice();
		slotChoice.slotType = Bacon_GunBuilderUI_SlotType.REMOVE;
		slotChoice.prefab = "empty";

		int childId = m_wSlotChoicesListbox.AddItem_Choice(slotChoice);
		m_wSlotChoicesListbox.GetItem(childId).GetRootWidget().SetZOrder(-1);

	}
	void CreateAddSlotOption(InventoryStorageSlot slot) {
		Bacon_GunBuilderUI_SlotChoice slotChoice = new Bacon_GunBuilderUI_SlotChoice();
		slotChoice.slotType = Bacon_GunBuilderUI_SlotType.ADD;
		slotChoice.prefab = "add";

		int childId = m_wSlotChoicesListbox.AddItem_Choice(slotChoice);
		m_wSlotChoicesListbox.GetItem(childId).GetRootWidget().SetZOrder(-1);
	}
	void CreateDummy(string text) {
		Bacon_GunBuilderUI_SlotChoice slotChoice = new Bacon_GunBuilderUI_SlotChoice();
		slotChoice.slotType = Bacon_GunBuilderUI_SlotType.MESSAGE;
		slotChoice.slotName = text;

		int childId = m_wSlotChoicesListbox.AddItem_Choice(slotChoice);
	}
	
	void CreateOptionsForSlot() {
		Bacon_GunBuilderUI_SlotInfo slotInfo = m_slotHistory.Get(m_slotHistory.Count() - 1);
		m_wSlotChoicesListbox.Clear();
		
		if (m_eCurrentMode == Bacon_GunBuilderUI_Mode.STORAGE) {
			ListItemsForStorage();
			m_swapButton.SetLabel("Add");
			m_editButton.SetLabel("Remove");
			return;
		}
		
		if (m_eCurrentMode == Bacon_GunBuilderUI_Mode.OPTIONS) {
			CreateSlotsForEditorOptions(slotInfo);
			return;
		}

		array<ResourceName> prefabChoices = {};
		int numChoices = m_Cache.GetChoicesForSlotType(slotInfo, prefabChoices);
		
//		if (numChoices == 0)
//			return;
		
		switch (slotInfo.slotType) {
			case Bacon_GunBuilderUI_SlotType.CHARACTER_LOADOUT: {
				CreateOptionsForCharacterLoadoutSlot(slotInfo, prefabChoices);
				break;
			}
			case Bacon_GunBuilderUI_SlotType.CHARACTER_WEAPON: {
				CreateOptionsForCharacterWeaponSlot(slotInfo, prefabChoices);
				break;
			}
			case Bacon_GunBuilderUI_SlotType.ATTACHMENT: {
				CreateOptionsForAttachmentSlot(slotInfo, prefabChoices);
				break;
			}
			case Bacon_GunBuilderUI_SlotType.CHARACTER_EQUIPMENT: {
				CreateOptionsForEquipmentSlot(slotInfo, prefabChoices);
				break;
			}
			case Bacon_GunBuilderUI_SlotType.MAGAZINE: {
				CreateOptionsForMagazine(slotInfo, prefabChoices);
				break;
			}
		}
		
		if (numChoices == 0)
			CreateDummy("No items found for slot");
		
		if (m_wSlotChoicesListbox.GetFocusedItem() == -1)
			m_wSlotChoicesListbox.SetFocus(0);
		
		// m_wSlotChoicesListbox.SetFocusOnFirstItem();
		// m_wSlotChoicesListbox.SetFocus(0);
	};
	
	// ------------ inventory handling
	void OnItemAddRequested(GunBuilderUI_InventoryPanelUIComponent comp, Bacon_GunBuilderUI_SlotChoice choiceInfo, int button) {
		Bacon_GunBuilderUI_SlotInfo editedSlot = GetCurrentlyEditedSlot();
		
		IEntity attachedEntity = editedSlot.slot.GetAttachedEntity();
		if (!attachedEntity) {
			OnCriticalError(Bacon_GunBuilderUI_Step.DEFAULT, "Add item requested but attached entity is null"); return; }
		
		BaseInventoryStorageComponent storage = BaseInventoryStorageComponent.Cast(attachedEntity.FindComponent(BaseInventoryStorageComponent));
		if (!storage) {
			OnCriticalError(Bacon_GunBuilderUI_Step.DEFAULT, "Add item requested but attached entity storage component is null", string.Format("Attached entity: %1", attachedEntity)); return; }

		Bacon_GunBuilderUI_Network_StorageRequest request = new Bacon_GunBuilderUI_Network_StorageRequest();

		request.actionType = Bacon_GunBuilderUI_ActionType.ADD_ITEM;
		request.storageRplId = m_Cache.GetStorageRplId(storage);
		request.storageSlotId = -1;
		request.prefab = choiceInfo.prefab;

		m_pcComponent.RequestAction(request);
	}
	void CreateInventoryItemListingFromEntity(InventoryStorageSlot slot, inout map<ResourceName, ref Bacon_GunBuilderUI_SlotInfoStorageItem> itemlisting) {
		if (!slot.GetAttachedEntity() || LoadoutSlotInfo.Cast(slot))
			return;
		
		bool notAPrefab = false;
		
		ResourceName attachedResourceName;
		if (!Bacon_GunBuilderUI_Helpers.GetResourceNameFromEntity(slot.GetAttachedEntity(), attachedResourceName)) {
			attachedResourceName = string.Format("UNKNOWN_%1", slot.GetID());
			notAPrefab = true;
		}
		
		Bacon_GunBuilderUI_SlotInfoStorageItem currentListing = itemlisting.Get(attachedResourceName);
		if (currentListing) {
			currentListing.numItems += 1;
			return;
		}
		
		currentListing = new Bacon_GunBuilderUI_SlotInfoStorageItem();
				
		currentListing.slotName = Bacon_GunBuilderUI_Helpers.GetItemNameFromEntity(slot.GetAttachedEntity());
		currentListing.storageSlotId = slot.GetID();
		currentListing.storageType = Bacon_GunBuilderUI_StorageType.INVENTORY;
		currentListing.slotType = Bacon_GunBuilderUI_SlotType.INVENTORY;
		currentListing.slot = slot;
	
		if (notAPrefab) {
			currentListing.itemName = "Warning: Not a prefab";
		}
				
		itemlisting.Set(attachedResourceName, currentListing);
	}
	void ListItemsForStorage() {
		m_wSlotChoicesListbox.Clear();
		
		Bacon_GunBuilderUI_SlotInfo currentSlot = GetCurrentlyEditedSlot();
		BaseInventoryStorageComponent storage = BaseInventoryStorageComponent.Cast(currentSlot.slot.GetAttachedEntity().FindComponent(BaseInventoryStorageComponent));
		
		CreateAddSlotOption(currentSlot.slot);
		
		int slots = storage.GetSlotsCount();
		
		// prefab - listing info
		map<ResourceName, ref Bacon_GunBuilderUI_SlotInfoStorageItem> itemlisting = new map<ResourceName, ref Bacon_GunBuilderUI_SlotInfoStorageItem>;
		
		InventoryStorageSlot storageSlot;
		IEntity attachedEntity;
		
		array<InventoryItemComponent> outItemsComponents = {};
		storage.GetOwnedItems(outItemsComponents);
		
		foreach (InventoryItemComponent item : outItemsComponents) {
			CreateInventoryItemListingFromEntity(item.GetParentSlot(), itemlisting);

			// if this is a storage I need to iterate over it
			BaseInventoryStorageComponent itemStorage = BaseInventoryStorageComponent.Cast(item);
			if (itemStorage && SCR_Enum.HasPartialFlag(itemStorage.GetPurpose(), EStoragePurpose.PURPOSE_DEPOSIT | EStoragePurpose.PURPOSE_EQUIPMENT_ATTACHMENT)) {
				int subSlots = itemStorage.GetSlotsCount();
				
				for (int x = 0; x < subSlots; ++x) {
					CreateInventoryItemListingFromEntity(itemStorage.GetSlot(x), itemlisting);
				}
				
				continue;
			}

			
		}

		foreach (ResourceName resourceName, Bacon_GunBuilderUI_SlotInfoStorageItem slotInfo : itemlisting) {
			// m_wSlotChoicesListbox.AddItem_InventoryItem("inventoryItemSlot", slotInfo);
			m_wSlotChoicesListbox.AddItem_InventoryItem(slotInfo);
		}
		
		m_wSlotChoicesListbox.Sort();
		
		if (m_iListBoxLastActionChild > -1) {
			// GetGame().GetCallqueue().CallLater(m_wSlotChoicesListbox.SetFocus, 33, false, m_iListBoxLastActionChild);
			m_wSlotChoicesListbox.SetFocusByZOrder(m_iListBoxLastActionChild);
			m_iListBoxLastActionChild = -1;
		}
	}
	// -------------- slots handling
	void CreateOptionsForCharacterLoadoutSlot(Bacon_GunBuilderUI_SlotInfo slotInfo, array<ResourceName> prefabChoices) {
		CreateRemoveSlotOption(slotInfo.slot);
		
		ResourceName attachedItemResourceName;
		Bacon_GunBuilderUI_Helpers.GetResourceNameFromEntity(slotInfo.slot.GetAttachedEntity(), attachedItemResourceName);
		
		int listBoxChildId;
		IEntity itemEntity;
		
		BaseLoadoutClothComponent itemClothComponent;
		LoadoutAreaType itemAreaType;
		
		array<typename> blockedSlots = {};
		int blockedSlotsCount;

		Resource loadedResource;
		BaseWorld world = GetGame().GetWorld();
		
		BaseInventoryStorageComponent slotStorage = slotInfo.slot.GetStorage();
		SCR_CharacterInventoryStorageComponent characterStorage = SCR_CharacterInventoryStorageComponent.Cast(slotStorage);
		
		foreach (ResourceName resourceName : prefabChoices) {
			loadedResource = Resource.Load(resourceName);
			itemEntity = GetGame().SpawnEntityPrefabLocal(loadedResource, world, null);

			// Bacon_GunBuilderUI_SlotInfo optionSlotInfo = new Bacon_GunBuilderUI_SlotInfo();
			Bacon_GunBuilderUI_SlotChoice optionSlotInfo = new Bacon_GunBuilderUI_SlotChoice();

			if (!Bacon_GunBuilderUI_Helpers.GetItemNameFromEntityValidate(itemEntity, optionSlotInfo.slotName)) {
				Print(string.Format("Prefab %1 is invalid!", resourceName), LogLevel.WARNING);
				SCR_EntityHelper.DeleteEntityAndChildren(itemEntity);
				continue;
			}
			
			// optionSlotInfo.slot = slotInfo.slot;
			optionSlotInfo.prefab = resourceName;
			
			itemClothComponent = BaseLoadoutClothComponent.Cast(itemEntity.FindComponent(BaseLoadoutClothComponent));

			if (!slotStorage.CanReplaceItem(itemEntity, slotInfo.storageSlotId)) {
				SCR_EntityHelper.DeleteEntityAndChildren(itemEntity);
				continue;
			}
			
			if (characterStorage) {
				blockedSlots.Clear();
				blockedSlotsCount = itemClothComponent.GetBlockedSlots(blockedSlots);
				if (blockedSlotsCount == 0) {
					optionSlotInfo.slotEnabled = true;
				} else {
					foreach (typename blockedAreaType : blockedSlots) {
						if (characterStorage.GetClothFromArea(blockedAreaType) || characterStorage.IsAreaBlocked(blockedAreaType)) {
							optionSlotInfo.slotEnabled = false;
							optionSlotInfo.itemName = "(blocked)";
							break;
						}
					}
				}
			}
			
			// listBoxChildId = m_wSlotChoicesListbox.AddItem("optionSlot", optionSlotInfo);
			listBoxChildId = m_wSlotChoicesListbox.AddItem_Choice(optionSlotInfo);
			if (attachedItemResourceName == resourceName)
				m_wSlotChoicesListbox.SetFocus(listBoxChildId);
			
			SCR_EntityHelper.DeleteEntityAndChildren(itemEntity);
		}
	}
	
	void CreateOptionsForCharacterWeaponSlot(Bacon_GunBuilderUI_SlotInfo slotInfo, array<ResourceName> prefabChoices) {
		ResourceName attachedItemResourceName;
		Bacon_GunBuilderUI_Helpers.GetResourceNameFromEntity(slotInfo.slot.GetAttachedEntity(), attachedItemResourceName);
		
		CreateRemoveSlotOption(slotInfo.slot);
		
		int listBoxChildId;
		
		IEntity itemEntity;
		InventoryItemComponent itemComponent;
		Resource loadedResource;
		BaseWorld world = GetGame().GetWorld();
		
		SCR_CharacterInventoryStorageComponent characterStorage = SCR_CharacterInventoryStorageComponent.Cast(slotInfo.slot.GetStorage());
		
		foreach (ResourceName resourceName : prefabChoices) {
			loadedResource = Resource.Load(resourceName);
			itemEntity = GetGame().SpawnEntityPrefabLocal(loadedResource, world, null);
			
			// Bacon_GunBuilderUI_SlotInfo optionSlotInfo = new Bacon_GunBuilderUI_SlotInfo();
			Bacon_GunBuilderUI_SlotChoice optionSlotInfo = new Bacon_GunBuilderUI_SlotChoice();

			if (!Bacon_GunBuilderUI_Helpers.GetItemNameFromEntityValidate(itemEntity, optionSlotInfo.slotName)) {
				Print(string.Format("Prefab %1 is invalid!", resourceName), LogLevel.WARNING);
				SCR_EntityHelper.DeleteEntityAndChildren(itemEntity);
				continue;
			}
			
			// optionSlotInfo.slot = slotInfo.slot;
			optionSlotInfo.prefab = resourceName;
			
			// listBoxChildId = m_wSlotChoicesListbox.AddItem("optionSlot", optionSlotInfo);
			listBoxChildId = m_wSlotChoicesListbox.AddItem_Choice(optionSlotInfo);
			if (attachedItemResourceName == resourceName)
				m_wSlotChoicesListbox.SetFocus(listBoxChildId);
			
			SCR_EntityHelper.DeleteEntityAndChildren(itemEntity);
		}
	}
	
	void CreateOptionsForAttachmentSlot(Bacon_GunBuilderUI_SlotInfo slotInfo, array<ResourceName> prefabChoices) {
		ResourceName attachedItemResourceName;
		Bacon_GunBuilderUI_Helpers.GetResourceNameFromEntity(slotInfo.slot.GetAttachedEntity(), attachedItemResourceName);
		
		AttachmentSlotComponent attachmentSlot = AttachmentSlotComponent.Cast(slotInfo.slot.GetParentContainer());
		if (attachmentSlot.ShouldShowInInspection())
			CreateRemoveSlotOption(slotInfo.slot);
		
		int listBoxChildId;
		IEntity itemEntity;
		InventoryItemComponent itemComponent;
		Resource loadedResource;
		BaseWorld world = GetGame().GetWorld();
		
		foreach (ResourceName resourceName : prefabChoices) {
			loadedResource = Resource.Load(resourceName);
			itemEntity = GetGame().SpawnEntityPrefabLocal(loadedResource, world, null);
			
			itemComponent = InventoryItemComponent.Cast(itemEntity.FindComponent(InventoryItemComponent));
			// Bacon_GunBuilderUI_SlotInfo optionSlotInfo = new Bacon_GunBuilderUI_SlotInfo();
			Bacon_GunBuilderUI_SlotChoice optionSlotInfo = new Bacon_GunBuilderUI_SlotChoice();

			if (!Bacon_GunBuilderUI_Helpers.GetItemNameFromEntityValidate(itemEntity, optionSlotInfo.slotName)) {
				Print(string.Format("Prefab %1 is invalid!", resourceName), LogLevel.WARNING);
				SCR_EntityHelper.DeleteEntityAndChildren(itemEntity);
				continue;
			}
			
			// optionSlotInfo.slot = slotInfo.slot;
			optionSlotInfo.prefab = resourceName;
			
			// listBoxChildId = m_wSlotChoicesListbox.AddItem("optionSlot", optionSlotInfo);
			listBoxChildId = m_wSlotChoicesListbox.AddItem_Choice(optionSlotInfo);
			if (attachedItemResourceName == resourceName)
				m_wSlotChoicesListbox.SetFocus(listBoxChildId);
			
			SCR_EntityHelper.DeleteEntityAndChildren(itemEntity);
		}
	}
	
	void CreateOptionsForEquipmentSlot(Bacon_GunBuilderUI_SlotInfo slotInfo, array<ResourceName> prefabChoices) {
		ResourceName attachedItemResourceName;
		Bacon_GunBuilderUI_Helpers.GetResourceNameFromEntity(slotInfo.slot.GetAttachedEntity(), attachedItemResourceName);
		
		CreateRemoveSlotOption(slotInfo.slot);
		
		int listBoxChildId;
		IEntity itemEntity;
		InventoryItemComponent itemComponent;
		Resource loadedResource;
		BaseWorld world = GetGame().GetWorld();
		
		foreach (ResourceName resourceName : prefabChoices) {
			loadedResource = Resource.Load(resourceName);
			itemEntity = GetGame().SpawnEntityPrefabLocal(loadedResource, world, null);
			
			itemComponent = InventoryItemComponent.Cast(itemEntity.FindComponent(InventoryItemComponent));
			// Bacon_GunBuilderUI_SlotInfo optionSlotInfo = new Bacon_GunBuilderUI_SlotInfo();
			Bacon_GunBuilderUI_SlotChoice optionSlotInfo = new Bacon_GunBuilderUI_SlotChoice();

			if (!Bacon_GunBuilderUI_Helpers.GetItemNameFromEntityValidate(itemEntity, optionSlotInfo.slotName)) {
				Print(string.Format("Prefab %1 is invalid!", resourceName), LogLevel.WARNING);
				SCR_EntityHelper.DeleteEntityAndChildren(itemEntity);
				continue;
			}
			
			// optionSlotInfo.slot = slotInfo.slot;
			optionSlotInfo.prefab = resourceName;
			
			// listBoxChildId = m_wSlotChoicesListbox.AddItem("optionSlot", optionSlotInfo);
			listBoxChildId = m_wSlotChoicesListbox.AddItem_Choice(optionSlotInfo);
			if (attachedItemResourceName == resourceName)
				m_wSlotChoicesListbox.SetFocus(listBoxChildId);
			
			SCR_EntityHelper.DeleteEntityAndChildren(itemEntity);
		}
	}
	
	void CreateOptionsForMagazine(Bacon_GunBuilderUI_SlotInfo slotInfo, array<ResourceName> prefabChoices) {
		ResourceName attachedItemResourceName;
		Bacon_GunBuilderUI_Helpers.GetResourceNameFromEntity(slotInfo.slot.GetAttachedEntity(), attachedItemResourceName);
		
		CreateRemoveSlotOption(slotInfo.slot);
		
		int listBoxChildId;
		IEntity itemEntity;
		InventoryItemComponent itemComponent;
		Resource loadedResource;
		BaseWorld world = GetGame().GetWorld();
		
		foreach (ResourceName resourceName : prefabChoices) {
			loadedResource = Resource.Load(resourceName);
			itemEntity = GetGame().SpawnEntityPrefabLocal(loadedResource, world, null);
			
			itemComponent = InventoryItemComponent.Cast(itemEntity.FindComponent(InventoryItemComponent));
			Bacon_GunBuilderUI_SlotChoice optionSlotInfo = new Bacon_GunBuilderUI_SlotChoice();

			if (!Bacon_GunBuilderUI_Helpers.GetItemNameFromEntityValidate(itemEntity, optionSlotInfo.slotName)) {
				Print(string.Format("Prefab %1 is invalid!", resourceName), LogLevel.WARNING);
				SCR_EntityHelper.DeleteEntityAndChildren(itemEntity);
				continue;
			}

			optionSlotInfo.prefab = resourceName;
			
			//listBoxChildId = m_wSlotChoicesListbox.AddItem("optionSlot", optionSlotInfo);
			listBoxChildId = m_wSlotChoicesListbox.AddItem_Choice(optionSlotInfo);
			if (attachedItemResourceName == resourceName)
				m_wSlotChoicesListbox.SetFocus(listBoxChildId);
			
			SCR_EntityHelper.DeleteEntityAndChildren(itemEntity);
		}
	}
}