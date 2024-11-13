class GunBuilderUI_SlotsUIComponent: ScriptedWidgetComponent {
	// layouts
	static ResourceName m_sResourceName_InventoryItem = "{E0E9D7246E0417E4}UI/BaconLoadoutEditor/GunBuilder_InventorySlotItem.layout";
	static ResourceName m_sResourceName_SlotItem = "{3EA30E5EA8768F73}UI/BaconLoadoutEditor/GunBuilder_SlotItem.layout";
	static ResourceName m_sResourceName_LoadoutItem = "{E76BD3521B9F3224}UI/BaconLoadoutEditor/GunBuilder_LoadoutItem.layout";
	
	[Attribute("SlotsWidget")]
	string m_slotsWidgetName;
	
	Widget m_wRoot;
	Widget m_wItemsLayout;
	
	ref ScriptInvoker m_OnItemClicked = new ScriptInvoker();
	ref ScriptInvoker m_OnItemFocused = new ScriptInvoker();
	ref ScriptInvoker m_OnPanelFocused = new ScriptInvoker();
	
	int m_focused = -1;
	
	ref array<GunBuilderUI_MultifunctionSlotUIComponent> m_aElementComponents = new array<GunBuilderUI_MultifunctionSlotUIComponent>;
	
	ScrollLayoutWidget m_wScrollLayout;
	
	override bool OnFocus(Widget w, int x, int y) {
		m_OnPanelFocused.Invoke();
		return super.OnFocus(w,x,y);
	}
	Widget GetRootWidget() {
		return m_wRoot;
	}
	
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
		
		m_wRoot = w;
		m_wItemsLayout = w.FindAnyWidget(m_slotsWidgetName);
		m_wScrollLayout = ScrollLayoutWidget.Cast(w.FindAnyWidget("ScrollLayout0"));
	}
	
	int AddItem_Slot(Bacon_GunBuilderUI_SlotInfo slotInfo) {
		return AddItem(slotInfo, m_sResourceName_SlotItem);
	}
	
	int AddItem_Choice(Bacon_GunBuilderUI_SlotChoice slotInfo) {
		return AddItem(slotInfo, m_sResourceName_SlotItem);
	}
	
	int AddItem_Loadout(Bacon_GunBuilderUI_SlotLoadout slotInfo) {
		return AddItem(slotInfo, m_sResourceName_LoadoutItem);
	}
	
	int AddItem_InventoryItem(Bacon_GunBuilderUI_SlotInfoStorageItem slotInfo) {
		return AddItem(slotInfo, m_sResourceName_InventoryItem);
	}
	
	int AddItem_Option(GunBuilderUI_EditorOptionData optionData) {
		return AddItem(optionData, m_sResourceName_InventoryItem);
	}
	
	void SetFocusedItemSelected(int button = 0) {
		if (m_focused == -1)
			return;

		OnClicked(m_aElementComponents[m_focused], button);
	}
	void SetFocus(int child) {
		if (child >= m_aElementComponents.Count())
			return;

		GetGame().GetCallqueue().CallLater(_SetFocusDelayed, 33, false, child);
	}
	void SetFocusByZOrder(int z) {
		if (z > m_aElementComponents.Count()-2)
			z = m_aElementComponents.Count()-2;

		foreach (GunBuilderUI_MultifunctionSlotUIComponent comp : m_aElementComponents) {
			if (comp.GetZOrder() == z) {
				GetGame().GetCallqueue().CallLater(_SetFocusDelayed, 33, false, comp.m_iParentChildId);
				return;
			}
		}
	}
	void _SetFocusDelayed(int child) {
		GetGame().GetWorkspace().SetFocusedWidget(m_aElementComponents[child].GetRootWidget());
	}
	void FocusChanged(int child) {
		m_focused = child;
		
		m_OnItemFocused.Invoke(this, m_aElementComponents[child].GetData());
	}
	int GetFocusedItem() {
		return m_focused;
	}
	
	Managed GetItemData(int child) {
		return m_aElementComponents[child].GetData();
	}
	
	// todo: more efficient sorting
	void Sort() {
		map<string, Widget> nameToWidget = new map<string, Widget>;
		array<string> names = {};
		
		string nameWithSlot;
		
		foreach (GunBuilderUI_MultifunctionSlotUIComponent comp : m_aElementComponents) {
			if (comp.GetRootWidget().GetZOrder() < 0)
				continue;
			
			Bacon_GunBuilderUI_SlotChoice slotInfo = Bacon_GunBuilderUI_SlotChoice.Cast(comp.GetData());
			nameWithSlot = string.Format("%1_child:%2", WidgetManager.Translate(slotInfo.slotName), comp.m_iParentChildId);
			
			names.Insert(nameWithSlot);
			nameToWidget.Set(nameWithSlot, comp.GetRootWidget());
		}
		
		names.Sort();
		
		for (int i = 0; i < names.Count(); i++) {
			Widget w = nameToWidget.Get(names[i]);

			w.SetZOrder(i);
		}
	}
	
	GunBuilderUI_MultifunctionSlotUIComponent GetItem(int childId) {
		return m_aElementComponents[childId];
	}
	
	int AddItem(Managed data, ResourceName layout) {
		Widget newWidget = GetGame().GetWorkspace().CreateWidgets(layout, m_wItemsLayout);
		
		GunBuilderUI_MultifunctionSlotUIComponent comp = GunBuilderUI_MultifunctionSlotUIComponent.Cast(newWidget.FindHandler(GunBuilderUI_MultifunctionSlotUIComponent));
		
		comp.SetToggleable(true);
		
		comp.m_OnClicked.Insert(OnClicked);

		int id = m_aElementComponents.Insert(comp);
		Bacon_GunBuilderUI_SlotItem slotItem = Bacon_GunBuilderUI_SlotItem.Cast(data);
		slotItem.listBoxChildId = id;
		
		comp.SetData(slotItem);
		comp.SetListBoxId(id, this);

		return id;
	}

	void Clear()
	{
		m_focused = 0;
		while (m_wItemsLayout.GetChildren())
			m_wItemsLayout.GetChildren().RemoveFromHierarchy();
		
		m_aElementComponents.Clear();
		m_wScrollLayout.SetSliderPos(0, 0, false);		
	}
	
	void OnClicked(GunBuilderUI_MultifunctionSlotUIComponent comp, int button) {
		// Bacon_GunBuilderUI_SlotChoice info = Bacon_GunBuilderUI_SlotChoice.Cast(comp.GetData());
		
		m_OnItemClicked.Invoke(this, comp.GetData(), button);
	}
	
	
}