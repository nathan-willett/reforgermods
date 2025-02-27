[EntityEditorProps(category: "GameScripted/UI/Inventory", description: "Inventory Item Info UI class")]

//------------------------------------------------------------------------------------------------
//! UI Script
//! Inventory Slot UI Layout
modded class SCR_InventoryItemInfoUI : ScriptedWidgetComponent
{
	
	private string m_sHintLayout = "{9996B50BE8DFED5F}UI/layouts/Menus/Inventory/InventoryItemHintElement.layout";	
	protected PreviewRenderAttributes m_PreviewAttributes;

	// ------------------------
	// ---- REAPER Modded ----- 
	// ------------------------	
	
	void SetPreviewItem(IEntity pEntity = null)
	{
		TextWidget m_wPreviewLoading = TextWidget.Cast(m_infoWidget.FindAnyWidget("previewLoading"));
		ItemPreviewWidget m_wPreviewItem = ItemPreviewWidget.Cast(m_infoWidget.FindAnyWidget("ItemPreviewRender"));
		m_wPreviewItem.SetVisible(false);
		m_wPreviewLoading.SetVisible(true);
				
		if(!pEntity)
			return;
				
		ChimeraWorld world = ChimeraWorld.CastFrom(GetGame().GetWorld());
		if (!world)
			return;
						
		ItemPreviewManagerEntity manager = world.GetItemPreviewManager();
		if (!manager)
			return;
		
		GetPreviewAttributes(pEntity, m_PreviewAttributes);			
		manager.SetPreviewItem(m_wPreviewItem, pEntity, m_PreviewAttributes, false);

		GetGame().GetCallqueue().CallLater(ShowPreview, 1000, false, m_wPreviewItem, m_wPreviewLoading);
	}

	void ShowPreview(ItemPreviewWidget wPreview, TextWidget wLoading)
	{
		wLoading.SetVisible(false);
		wPreview.SetVisible(true);		
	}
	
	protected void GetPreviewAttributes(IEntity pEntity, out PreviewRenderAttributes preview)
	{
		if (!pEntity)
			return;
		
		InventoryItemComponent invetoryItem = InventoryItemComponent.Cast(pEntity.FindComponent(InventoryItemComponent));		
		if (!invetoryItem)
			return;
		
		preview = PreviewRenderAttributes.Cast(invetoryItem.FindAttribute(PreviewRenderAttributes));
	}	
};
