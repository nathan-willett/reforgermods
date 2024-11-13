class GunBuilder_PrefabUIComponent: SCR_ModularButtonComponent
{
	ItemPreviewWidget m_itemPreview;
	TextWidget m_text;
	
	override static GunBuilder_PrefabUIComponent FindComponent(Widget w)
	{
		return GunBuilder_PrefabUIComponent.Cast(w.FindHandler(GunBuilder_PrefabUIComponent));
	}
	
	override void HandlerAttached(Widget w) {
		super.HandlerAttached(w);
		
		m_itemPreview = ItemPreviewWidget.Cast(m_wRoot.FindAnyWidget("SlotPreview"));
		m_text = TextWidget.Cast(m_wRoot.FindAnyWidget("SlotText"));
	}
	
	void SetData(Bacon_GunBuilderUI_SlotChoice data) {
		m_UserData = data;
		
		m_itemPreview.SetVisible(true);
		
		ItemPreviewManagerEntity previewManager = Bacon_GunBuilderUI_Helpers.GetPreviewManager();
		if (!previewManager)
			return;

		previewManager.SetPreviewItemFromPrefab(m_itemPreview, data.prefab);
		
		m_text.SetText(string.Format("%1\n%2", data.slotName, data.itemName));
	}
	
	override Bacon_GunBuilderUI_SlotChoice GetData() {
		return m_UserData;
	}
}