

// --------------------------------------------------------------------------------------------------------------------
// ------           This script is part of REAPER CORE | an Arma Reforger SP/COOP Modification                   ------
// ------   You are not allowed to use this script or parts of it in your mod or pass it off as your own work.   ------
// ------                                                                                                        ------
// ------                         Written by REAPER 2024 - www.reaper-as.de                                      ------
// --------------------------------------------------------------------------------------------------------------------

// Modded, to show Attachment Names in inspection UI

modded class SCR_InventoryGearInspectionPointUI : SCR_InventoryAttachmentPointUI
{	
	override protected typename GetSlotType(InventoryStorageSlot slot, out string name)
	{
		typename type = slot.GetParentContainer().Type();
		
		if(type.IsInherited(BaseMuzzleComponent)) {
			name = "#AR-Magazine_Name";
			BaseMagazineWell magwell = BaseMuzzleComponent.Cast(slot.GetParentContainer()).GetMagazineWell();
			
			if(magwell) return magwell.Type();
			
		} else if (type == AttachmentSlotComponent) {
			
			AttachmentSlotComponent attachment = AttachmentSlotComponent.Cast(slot.GetParentContainer());
			if(!attachment || !attachment.GetAttachmentSlotType()) {
				name = string.Empty;
			 	return typename.Empty;
			}

			type = attachment.GetAttachmentSlotType().Type();
			
			if(type.IsInherited(AttachmentOptics)) name = "Optic";
			
			if(name.IsEmpty() && type.IsInherited(REAPER_AttachmentTopRail)) name = "TopRail";
			if(name.IsEmpty() && type.IsInherited(REAPER_AttachmentSideRail)) name = "SideRail";
			if(name.IsEmpty() && type.IsInherited(REAPER_AttachmentRearSight)) name = "RearSight";
			if(name.IsEmpty() && type.IsInherited(REAPER_AttachmentFrontSight)) name = "FrontSight";
			if(name.IsEmpty() && type.IsInherited(REAPER_Attachment45degSight)) name = "OffsetSight";
			if(name.IsEmpty() && type.IsInherited(REAPER_AttachmentSuppressor762)) name = "Muzzle 7.62 | 300 Win";
			if(name.IsEmpty() && type.IsInherited(REAPER_AttachmentSuppressor556)) name = "Muzzle 5.56";
			if(name.IsEmpty() && type.IsInherited(REAPER_AttachmentSuppressor45ACP)) name = "Muzzle .45 ACP";
			if(name.IsEmpty() && type.IsInherited(REAPER_AttachmentSuppressor545AK)) name = "Muzzle 5.45";
						
			if(name.IsEmpty() && type.IsInherited(AttachmentUnderBarrel)) name = "#AR-AttachmentType_Underbarrel";
			
			if(name.IsEmpty() && type.IsInherited(AttachmentHandGuard)) {
				name = string.Empty;
				return typename.Empty;
			}
		}
		return type;
	}

	//--------------------------------------------------------------------------------------------------------------------
			
	override protected void UpdateOwnedSlots(notnull array<IEntity> pItemsInStorage)
	{
		int count = pItemsInStorage.Count();
		
		if (count < m_aSlots.Count())
		{
			for (int i = m_aSlots.Count() - count; i > 0; i--)
			{
				SCR_InventorySlotUI slotUI = m_aSlots[m_aSlots.Count() - 1];
				if (slotUI)
					slotUI.Destroy();
			}
		}
		m_aSlots.Resize(count);
		for (int i = 0; i < count; i++)
		{
			InventoryItemComponent pComponent = GetItemComponentFromEntity( pItemsInStorage[i] );

			if (m_aSlots[i])
				m_aSlots[i].UpdateReferencedComponent(pComponent);
			else
			{
				m_aSlots[i] = CreateSlotUI(pComponent);
			}

			string slotName;
			typename slotType = GetSlotType(m_Storage.GetSlot(i), slotName);
			if (!slotName.Empty && slotType)
			{
				// Modded
				SCR_InventorySlotGearInspectionUI slot = SCR_InventorySlotGearInspectionUI.Cast(m_aSlots[i]);
				if(slot) {
					slot.m_tAttachmentType = slotType;
					slot.m_AttachmentSlotName = slotName;
				}
					
				m_MenuHandler.AddItemToAttachmentSelection(slotName, slot);
			}
		}
	}

	//--------------------------------------------------------------------------------------------------------------------
	
	void REAPER_OnlyShowSelectedSlot(SCR_InventorySlotUI selectedSlot, bool showAllSlots = false)
	{
		foreach(SCR_InventorySlotUI slot : m_aSlots) {
			
			if(showAllSlots) {
				slot.SetSlotVisible(true);
			} else {
				if(slot == selectedSlot) {
					slot.SetSlotVisible(true);
				} else {
					slot.SetSlotVisible(false);
				}		
			}
		}
	}
		
}


//--------------------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------------------


modded class SCR_InventorySlotGearInspectionUI : SCR_InventorySlotUI
{
	protected static const float SLOT_SIZE_DEFAULT = 32;
	protected static const float SLOT_SIZE_HIGHLIGHTED = 100; // Original is 64
	
	protected TextWidget m_wSlotNameWidget = null;
	
	string m_AttachmentSlotName;
	
	//-----------------------------------------------------------------
	
	override void UpdateReferencedComponent(InventoryItemComponent pComponent, SCR_ItemAttributeCollection attributes = null)
	{
		super.UpdateReferencedComponent(pComponent, attributes);
		
		if(m_widget) {
			
			m_wSlotNameWidget = TextWidget.Cast(m_widget.FindAnyWidget("slotName"));
			
			if(m_wSlotNameWidget) {
				m_wSlotNameWidget.SetText(m_AttachmentSlotName);
				m_wSlotNameWidget.SetVisible(false);
			} 
		}
	}
	
	//------------------------------------------------------------------------------------------------
	
	override bool OnMouseEnter(Widget w, int x, int y)
	{
		m_pStorageUI.GetInventoryMenuHandler().REAPER_ShowCompatibleAttachments(m_tAttachmentType, false);
		
		Highlight(true);

		return super.OnMouseEnter(w, x, y);
	}

	//------------------------------------------------------------------------------------------------
	override bool OnMouseLeave(Widget w, Widget enterW, int x, int y)
	{
		if(!m_bIsSelected) {
			m_pStorageUI.GetInventoryMenuHandler().REAPER_ShowCompatibleAttachments(m_tAttachmentType, true);
			Highlight(false);
		}

		return super.OnMouseLeave(w, enterW, x, y);
	}
	
	//------------------------------------------------------------------------------------------------
			
	override void Highlight(bool highlight)
	{			
		if (m_widget) {
			
			if(highlight) {
				FrameSlot.SetSize(m_widget, SLOT_SIZE_HIGHLIGHTED, SLOT_SIZE_HIGHLIGHTED);
			} else {
				FrameSlot.SetSize(m_widget, SLOT_SIZE_DEFAULT, SLOT_SIZE_DEFAULT);
			}			
		}
		
		if(m_wSlotNameWidget) m_wSlotNameWidget.SetVisible(highlight);		
		if(m_wItemWidget) m_wItemWidget.SetVisible(highlight);	
		if(m_wIconWidget) m_wIconWidget.SetVisible(!highlight);
		
		SCR_InventoryGearInspectionPointUI InspectionPointUI = SCR_InventoryGearInspectionPointUI.Cast(m_pStorageUI);
		if(InspectionPointUI) {
			if(highlight) {
				InspectionPointUI.REAPER_OnlyShowSelectedSlot(this, false);
			} else {
				InspectionPointUI.REAPER_OnlyShowSelectedSlot(this, true);
			}
		}
	}
}

