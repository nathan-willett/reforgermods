
class Bacon_GunBuilderUI_InventoryItemUIComponent : SCR_ModularButtonComponent {
	static string m_imageset = "{FDD5423E69D007F8}UI/Textures/Icons/icons_wrapperUI-128.imageset";

	static string m_emptyIcon = "careerCircleOutline";
	
	void SetPreviewFromEntity(IEntity entity) {
		if (!entity) {
			SetImage(m_imageset, m_emptyIcon);
			return;
		}

		ItemPreviewManagerEntity previewManager = Bacon_GunBuilderUI_Helpers.GetPreviewManager();
		if (!previewManager)
			return;
		
		ItemPreviewWidget previewWidget = ItemPreviewWidget.Cast(m_wRoot.FindAnyWidget("SlotPreview"));
		if (!previewWidget)
			return;
		
		// clear first
		previewManager.SetPreviewItem(previewWidget, null, null, true);
		previewManager.SetPreviewItem(previewWidget, entity);
		previewWidget.SetVisible(true);
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
	
	void SetImage(ResourceName imageOrImageset, string iconName)
	{
		ImageWidget iw = ImageWidget.Cast(m_wRoot.FindAnyWidget("SlotImage"));
		if (iw) {
			iw.LoadImageFromSet(0, imageOrImageset, iconName);
			iw.SetVisible(true);
		}
	}
	
	void SetText(string text)
	{
		TextWidget tw = TextWidget.Cast(m_wRoot.FindAnyWidget("SlotText"));
		if (tw)
			tw.SetText(text);
	}
	
	override void SetData(Managed data) {
		super.SetData(data);

		CreatePreviewForInventoryItem(Bacon_GunBuilderUI_SlotInfoStorageItem.Cast(data));
	}
	
	override bool OnClick(Widget w, int x, int y, int button)
	{
		#ifdef DEBUG_MODULAR_BUTTON
		_print("OnClick");
		#endif
		
		// Auto focus is focusable
		if (m_wRoot.IsFocusable())
		{
			auto workspace = GetGame().GetWorkspace();
			Widget currentFocus = workspace.GetFocusedWidget();
			if (currentFocus != m_wRoot)
				workspace.SetFocusedWidget(m_wRoot);
		}
		
		bool eventReturnValue = m_eEventReturnValue & EModularButtonEventHandler.CLICK;
		
		if (m_bIgnoreStandardInputs)
			return eventReturnValue;
		
		if (m_bCanBeToggled && !m_bToggledOnlyThroughApi)
			Internal_SetToggled(!m_bToggled);
		
		InvokeEffectsEvent(EModularButtonEventFlag.EVENT_CLICKED);
		
		EModularButtonState state = GetCurrentState();
		InvokeEffectsEvent(state);
		
		m_OnClicked.Invoke(this, button);
			
		return eventReturnValue;
	}
}