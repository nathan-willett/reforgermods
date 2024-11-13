[BaseContainerProps(), SCR_BaseEditorAttributeCustomTitle()]
class Bacon_GunBuilder_ApplyLoadoutEditorAttribute : SCR_BaseFloatValueHolderEditorAttribute
{	
	override SCR_BaseEditorAttributeVar ReadVariable(Managed item, SCR_AttributesManagerEditorComponent manager)
	{		
		SCR_EditableEntityComponent editableEntity = SCR_EditableEntityComponent.Cast(item);
		if (!editableEntity || (editableEntity.GetEntityType() != EEditableEntityType.CHARACTER && editableEntity.GetEntityType() != EEditableEntityType.GROUP)) return null;
		if (editableEntity.HasEntityState(EEditableEntityState.PLAYER)) return null;
		
		SCR_EditableEntityComponent parentComponent = editableEntity.GetParentEntity();
		
		if (!parentComponent || !SCR_EditableGroupComponent.Cast(parentComponent))
			return null;
		
		return SCR_BaseEditorAttributeVar.CreateInt(-1);
	}	
	
	override void WriteVariable(Managed item, SCR_BaseEditorAttributeVar var, SCR_AttributesManagerEditorComponent manager, int playerID)
	{
		if (!var) return;
		SCR_EditableEntityComponent editableEntity = SCR_EditableEntityComponent.Cast(item);
		
		// Print("var: "+);
		
		int requestedSlot = var.GetInt()-1;
		
		// m_aValues[var.GetInt()].GetFloatValue();
		
		if (requestedSlot < 0 || requestedSlot >= Bacon_GunBuilder_PlayerFactionLoadoutStorage.MAX_ADMIN_LOADOUTS || !Replication.IsServer()) {
			return;
		}
		
		if (editableEntity.HasEntityState(EEditableEntityState.PLAYER)) 
			return;
		
		if (editableEntity.GetEntityType() != EEditableEntityType.CHARACTER)
			return;
		
		SCR_EditableEntityComponent parentComponent = editableEntity.GetParentEntity();
		if (!parentComponent || !SCR_EditableGroupComponent.Cast(parentComponent))
			return;
		
		SCR_AIGroup group = SCR_AIGroup.Cast(parentComponent.GetOwner());
		if (!group)
			return;
		
		if (!Bacon_GunBuilderUI_PlayerControllerComponent.ServerInstance) {
			PrintFormat("Bacon_GunBuilder_ApplyLoadoutEditorAttribute | Server instance of player controller component not found!", LogLevel.ERROR);
			return;
		}
		Bacon_GunBuilderUI_PlayerControllerComponent.ServerInstance.SetAILoadout(requestedSlot, editableEntity.GetOwner(), group);
	}
	
	override int GetEntries(notnull array<ref SCR_BaseEditorAttributeEntry> outEntries)
	{
		FillValues();
		outEntries.Insert(new SCR_BaseEditorAttributeFloatStringValues(m_aValues));
		return outEntries.Count();
	}
	
	protected void FillValues()
	{
		m_aValues.Clear();
		
		SCR_EditorAttributeFloatStringValueHolder value = new SCR_EditorAttributeFloatStringValueHolder();
		value.SetName("None");
		value.SetFloatValue(-1);
		m_aValues.Insert(value);
		
		for (int x = 0; x < Bacon_GunBuilderUI_PlayerControllerComponent.LocalInstance.AdminLoadoutMetadata.Count(); x++) {
			value = new SCR_EditorAttributeFloatStringValueHolder();
			value.SetName(string.Format("Slot %1", Bacon_GunBuilderUI_PlayerControllerComponent.LocalInstance.AdminLoadoutMetadata[x].slotId+1));
			value.SetFloatValue(x);

			m_aValues.Insert(value);
		}
		
//		for (int x = 0; x < Bacon_GunBuilder_PlayerFactionLoadoutStorage.MAX_ADMIN_LOADOUTS; x++) {
//			value = new SCR_EditorAttributeFloatStringValueHolder();
//			value.SetName(string.Format("Slot %1", x+1));
//			value.SetFloatValue(x);
//			m_aValues.Insert(value);
//		}
	}
};

/**
Dynamic description for daytime duration to display to compare the ingame time with realife time
*/
[BaseContainerProps(), BaseContainerCustomStringTitleField("derp")]
class Bacon_GunBuilder_ApplyLoadoutEditorAttributeDescription : SCR_BaseAttributeDynamicDescription
{
	//------------------------------------------------------------------------------------------------
	override void InitDynamicDescription(notnull SCR_BaseEditorAttribute attribute, notnull SCR_BaseEditorAttributeUIComponent attributeUi)
	{
		super.InitDynamicDescription(attribute);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool IsValid(notnull SCR_BaseEditorAttribute attribute, notnull SCR_BaseEditorAttributeUIComponent attributeUi)
	{
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override void GetDescriptionData(notnull SCR_BaseEditorAttribute attribute, notnull SCR_BaseEditorAttributeUIComponent attributeUi, out SCR_EditorAttributeUIInfo uiInfo, out string param1 = string.Empty, out string param2 = string.Empty, out string param3 = string.Empty)
	{
		SCR_BaseEditorAttributeVar var = attribute.GetVariableOrCopy();
		if (!var)
			return;
		
		int slotId = var.GetInt()-1;

		uiInfo = new SCR_EditorAttributeUIInfo();
		uiInfo.SetName("");
		
		if (slotId < 0) {
			uiInfo.SetDescription("None");
			return;
		}
		
		if (slotId > Bacon_GunBuilderUI_PlayerControllerComponent.LocalInstance.AdminLoadoutMetadata.Count()) {
			uiInfo.SetDescription("Empty slot");
			return;
		}
		
		//  || slotId > Bacon_GunBuilderUI_PlayerControllerComponent.LocalInstance.AdminLoadoutMetadata.Count())
		Bacon_GunBuilder_PlayerLoadout loadout = Bacon_GunBuilderUI_PlayerControllerComponent.LocalInstance.AdminLoadoutMetadata[slotId];
		
		if (loadout.metadata_clothes.IsEmpty() && loadout.metadata_weapons.IsEmpty()) {
			uiInfo.SetDescription("Empty slot");
			return;
		}
		
		string clothes = loadout.metadata_clothes;
		clothes.Replace("\n", ", ");
		
		string weapons = loadout.metadata_weapons;
		weapons.Replace("\n", ", ");
		
		uiInfo.SetDescription(string.Format("%1\n%2", clothes, weapons));
	}
};
