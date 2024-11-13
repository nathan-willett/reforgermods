//class Bacon_GunBuilderUI_ListBoxElementData {
//	// AttachmentSlotComponent slotComponent;
//	Bacon_GunBuilderUI_StorageType storageType;
//	
//	InventoryStorageSlot slot;
//	int slotId;
//	ResourceName prefab;
//	string name;
//	bool enabled = true;
//}

class Bacon_GunBuilderUI_ListBoxFocusComponent: SCR_ListBoxComponent
{
	ref ScriptInvoker m_OnFocusChanged = new ScriptInvoker();
	//ref ScriptInvoker m_OnEditButtonPressed = new ScriptInvoker();
	
	int m_iFocusedElement = -1;
	
	ScrollLayoutWidget m_wScroll;
	Widget m_wRoot;
	
	ResourceName m_ResourceNameInventoryItem = "{E0E9D7246E0417E4}UI/BaconLoadoutEditor/GunBuilder_InventorySlotItem.layout";
	ResourceName m_ResourceNameSlotItem = "{3EA30E5EA8768F73}UI/BaconLoadoutEditor/GunBuilder_SlotItem.layout";
	ResourceName m_ResourceNameLoadoutItem = "{E76BD3521B9F3224}UI/BaconLoadoutEditor/GunBuilder_LoadoutItem.layout";
	
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
		
		m_wRoot = w;
		m_wScroll = ScrollLayoutWidget.Cast(w.FindAnyWidget("ScrollLayout0"));
	}
	
	Widget GetRootWidget() {
		return m_wRoot;
	}
	
	void SetFocusOnList() {
		GetGame().GetWorkspace().SetFocusedWidget(m_wScroll);
	}
	
	int AddItem_LoadoutItem(string item, Managed data = null) {
		SCR_ListBoxElementComponent comp;
		
		int id = _AddItem(item, data, comp, m_ResourceNameLoadoutItem);
		
		return id;
	}

	int AddItem_InventoryItem(string item, Managed data = null) {
		SCR_ListBoxElementComponent comp;
		
		int id = _AddItem(item, data, comp, m_ResourceNameInventoryItem);
		
		return id;
	}
	
//	override int _AddItem(string item, Managed data, out SCR_ListBoxElementComponent compOut, ResourceName itemLayout = string.Empty) {
//		int itemId = super._AddItem(item, data, compOut, itemLayout);
//		
//		GunBuilderUI_MultifunctionSlotUIComponent.Cast(compOut).SetListBoxId(itemId, this);
//		
//		return itemId;
//	}
	
	override int _AddItem(string item, Managed data, out SCR_ListBoxElementComponent compOut, ResourceName itemLayout = string.Empty)
	{	
		// Create widget for this item
		// The layout can be provided either as argument or through attribute
		ResourceName selectedLayout = m_sElementLayout;
		if (!itemLayout.IsEmpty())
			selectedLayout = itemLayout;
		Widget newWidget = GetGame().GetWorkspace().CreateWidgets(selectedLayout, m_wList);
		
		SCR_ListBoxElementComponent comp = SCR_ListBoxElementComponent.Cast(newWidget.FindHandler(SCR_ListBoxElementComponent));
		
		comp.SetText(item);
		comp.SetToggleable(true);
		comp.SetData(data);
		
		// Pushback to internal arrays
		int id = m_aElementComponents.Insert(comp);
		
		// GunBuilderUI_MultifunctionSlotUIComponent.Cast(comp).SetListBoxId(id, this);
		// Setup event handlers
		comp.m_OnClicked.Insert(OnItemClick);
		
		// Set up explicit navigation rules for elements. Otherwise we can't navigate
		// Through separators when we are at the edge of scrolling if there is an element
		// directly above/below the list box which intercepts focus
		string widgetName = this.GetUniqueWidgetName();
		newWidget.SetName(widgetName);
		if (m_aElementComponents.Count() > 1)
		{
			Widget prevWidget = m_aElementComponents[m_aElementComponents.Count() - 2].GetRootWidget();
			prevWidget.SetNavigation(WidgetNavigationDirection.DOWN, WidgetNavigationRuleType.EXPLICIT, newWidget.GetName());
			newWidget.SetNavigation(WidgetNavigationDirection.UP, WidgetNavigationRuleType.EXPLICIT, prevWidget.GetName());
		}
		
		compOut = comp;
		
		return id;
	}
	
//	override void OnItemClick(GunBuilderUI_MultifunctionSlotUIComponent comp, int button)
//	{
//		int id = comp.m_iParentChildId;
//		
//		if (id == -1)
//			return;
//		
//		// Behaviour depends on multi selection
//		if (m_bMultiSelection)
//		{
//			// If multi selection is enabled, inverse the selection state for this item
//			bool selected = m_aElementComponents[id].GetToggled();
//			_SetItemSelected(id, !selected, true);
//		}
//		else
//		{
//			// Unselect previous item
//			if (id != m_iCurrentItem && m_iCurrentItem >= 0 && m_iCurrentItem < m_aElementComponents.Count())
//				this.VisualizeSelection(m_iCurrentItem, false);
//			
//			// Select new item
//			_SetItemSelected(id, true, true);
//		}
//	}
	
	override void _SetItemSelected(int item, bool selected, bool invokeOnChanged, bool instant = false)
	{
		// Set m_iCurrentItem, if multi selection is not used
		if (!m_bMultiSelection)
		{
			if (selected)
				m_iCurrentItem = item;
//			else
//			{
//				// Nothing will be selected
//				if (item == m_iCurrentItem)
//					m_iCurrentItem = -1;
//			}
		}
				
		bool oldSelected = m_aElementComponents[item].GetToggled();
		this.VisualizeSelection(item, selected, instant);
		
		// if (invokeOnChanged && oldSelected != selected) // Only invoke if value actually changed
		if (invokeOnChanged)
			m_OnChanged.Invoke(this, item, selected);
	}
	
	int GetFocusedItem() {
		return m_iFocusedElement;
	}
	
	void SetFocusedItemSelected() {
		SetItemSelected(m_iFocusedElement, true, true);
	}
	
	void SetFocus(int childId) {
		
		GetGame().GetWorkspace().SetFocusedWidget(m_aElementComponents[childId].GetRootWidget());
		m_aElementComponents[childId].SetToggled(true);
		// SetItemSelected(m_iFocusedElement, true, true);
	}
	
	void FocusChanged(int listBoxId) {
		m_iFocusedElement = listBoxId;
		m_OnFocusChanged.Invoke(listBoxId);
	}
	
//	void EditButtonPressed(int listBoxId) {
//		m_OnEditButtonPressed.Invoke(listBoxId);
//	}
	
	override void Clear() {
		super.Clear();
		
		m_wScroll.SetSliderPos(0,0,false);
	}
}

class Bacon_GunBuilderUI_ListBoxElementComponent : SCR_ListBoxElementComponent
{
	[Attribute("Preview")]
	protected string m_sWidgetPreviewName;
	
//	[Attribute("true")]
//	protected bool m_bEnableStorageButton;
	
	static string m_imageset = "{FDD5423E69D007F8}UI/Textures/Icons/icons_wrapperUI-128.imageset";
	static string m_removeIcon = "cancel";
	static string m_emptyIcon = "careerCircleOutline";
	static string m_invalidIcon = "cancelCircle";
	static string m_addIcon = "addCircle";
	
	int m_iListBoxId;
	
	ItemPreviewWidget m_itemPreview;
	ImageWidget m_image;
	ImageWidget m_imageHasStorage;
	TextWidget m_text;
	ItemPreviewManagerEntity m_previewManager;
	
	Bacon_GunBuilderUI_ListBoxFocusComponent m_parent;
	
	// SCR_ButtonImageComponent m_Button;
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);

		m_bToggledOnlyThroughApi = true;
	}
	
	void SetListBoxId(int id, Bacon_GunBuilderUI_ListBoxFocusComponent parent) {
		m_iListBoxId = id;
		m_parent = parent;
	}
	
	override bool OnFocus(Widget w, int x, int y) {
		bool state = super.OnFocus(w, x, y);
		
		if (!m_parent)
			return state;

		m_parent.FocusChanged(m_iListBoxId);
		
		return state;
	}
	
	// helper functions
	void SetPreviewFromEntity(IEntity entity) {
		if (!entity) {
			SetImage(m_imageset, m_emptyIcon);
			return;
		}

		ItemPreviewManagerEntity previewManager = Bacon_GunBuilderUI_Helpers.GetPreviewManager();
		if (!previewManager)
			return;
		
		ItemPreviewWidget previewWidget = ItemPreviewWidget.Cast(m_wRoot.FindAnyWidget(m_sWidgetPreviewName));
		if (!previewWidget)
			return;
		
		// clear first
		previewManager.SetPreviewItem(previewWidget, null, null, true);
		previewManager.SetPreviewItem(previewWidget, entity);
		previewWidget.SetVisible(true);
	}
	
	void SetPreviewFromResourceName(ResourceName res) {		
		ItemPreviewManagerEntity previewManager = Bacon_GunBuilderUI_Helpers.GetPreviewManager();
		if (!previewManager)
			return;
		
		ItemPreviewWidget previewWidget = ItemPreviewWidget.Cast(m_wRoot.FindAnyWidget(m_sWidgetPreviewName));
		if (!previewWidget)
			return;
		
		previewManager.SetPreviewItemFromPrefab(previewWidget, res);
		previewWidget.SetVisible(true);
	}
	
	// slot type handlers
	void CreatePreviewForSlot(Bacon_GunBuilderUI_SlotInfo slotInfo) {
		SetText(string.Format("%1\n%2", slotInfo.slotName, slotInfo.itemName));
		SetPreviewFromEntity(slotInfo.slot.GetAttachedEntity());
		
		if (slotInfo.hasStorage) {
			ImageWidget iw = ImageWidget.Cast(m_wRoot.FindAnyWidget("ImageHasStorage"));
			if (iw)
				iw.SetVisible(true);
		}
		
		HandleEnabled(slotInfo.slotEnabled);
	}
	
	void CreatePreviewForChoice(Bacon_GunBuilderUI_SlotChoice slotChoice) {
		if (CreateImageItem(slotChoice.slotType))
			return;

		SetText(string.Format("%1\n%2", slotChoice.slotName, slotChoice.itemName));
		SetPreviewFromResourceName(slotChoice.prefab);
		HandleEnabled(slotChoice.slotEnabled);
	}
	
	bool CreateImageItem(Bacon_GunBuilderUI_SlotType slotType) {
		switch (slotType) {
			case Bacon_GunBuilderUI_SlotType.REMOVE: {
				SetText("Remove");
				SetImage(m_imageset, m_removeIcon);
				return true;
			}
			case Bacon_GunBuilderUI_SlotType.ADD: {
				SetText("Add");
				SetImage(m_imageset, m_addIcon);
				return true;
			}
		}
		
		return false;
	}
	
	void CreatePreviewForInventoryItem(Bacon_GunBuilderUI_SlotInfoStorageItem slotInfo) {
		SetText(string.Format("%1\n%2", slotInfo.slotName, slotInfo.itemName));
		SetPreviewFromEntity(slotInfo.slot.GetAttachedEntity());
		
		TextWidget tw = TextWidget.Cast(m_wRoot.FindAnyWidget("ItemCount"));
		if (tw && slotInfo.numItems > 1) {
			tw.SetText(string.Format("%1x", slotInfo.numItems));
			tw.SetVisible(true);
		}
	}

	override void SetImage(ResourceName imageOrImageset, string iconName)
	{
		ImageWidget iw = ImageWidget.Cast(m_wRoot.FindAnyWidget(m_sWidgetImageName));
		if (iw) {
			iw.LoadImageFromSet(0, imageOrImageset, iconName);
			iw.SetVisible(true);
		}
	}
	
	override void SetText(string text)
	{
		TextWidget tw = TextWidget.Cast(m_wRoot.FindAnyWidget(m_sWidgetTextName));
		if (tw)
			tw.SetText(text);
	}
	
	override void SetData(Managed data) {
		super.SetData(data);

		Bacon_GunBuilderUI_SlotItem slotInfo = Bacon_GunBuilderUI_SlotItem.Cast(data);
		
		if (Bacon_GunBuilderUI_SlotInfoStorageItem.Cast(data)) {
			CreatePreviewForInventoryItem(Bacon_GunBuilderUI_SlotInfoStorageItem.Cast(data));
			return;
		}
		
		if (Bacon_GunBuilderUI_SlotInfo.Cast(data)) {
			CreatePreviewForSlot(Bacon_GunBuilderUI_SlotInfo.Cast(data));
			return;
		}
		
		CreatePreviewForChoice(Bacon_GunBuilderUI_SlotChoice.Cast(data));
	}
	
	void HandleEnabled(bool enabled) {
		ImageWidget panel = ImageWidget.Cast(m_wRoot.FindAnyWidget("BlockedPanel"));
		if (!panel)
			return;

		if (enabled) {
			panel.SetVisible(false);
		} else {
			panel.SetVisible(true);
		}		
	}
}
