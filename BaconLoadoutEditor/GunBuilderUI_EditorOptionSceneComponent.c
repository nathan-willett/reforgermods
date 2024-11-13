class GunBuilderUI_EditorOptionSceneComponentClass: GunBuilderUI_EditorOptionComponentClass {}
class GunBuilderUI_EditorOptionSceneComponent: GunBuilderUI_EditorOptionComponent {
	override void FillOptions() {
		m_aOptionChoices.Set("asphalt", new GunBuilderUI_EditorOptionData(m_sOptionName, icon: "careerCircleSelected", label: "Default Dark", value: "asphalt"));
		m_aOptionChoices.Set("asphalt", new GunBuilderUI_EditorOptionData(m_sOptionName, icon: "careerCircleSelected", label: "Default Dark", value: "asphalt"));
	}
	
	override void ApplyOption(GunBuilderUI_EditorOptionData data) {
		LightEntity light = LightEntity.Cast(m_owner);
		if (!light) {
			Print("GunBuilderUI_EditorOptionLightColorComponent | No light entity!", LogLevel.ERROR);
			return;
		}
		
		light.SetColor(data.iconColor, 8.0);
	}
}

// 