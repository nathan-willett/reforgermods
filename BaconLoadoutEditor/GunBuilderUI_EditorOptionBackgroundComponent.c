class GunBuilderUI_EditorOptionBackgroundComponentClass: GunBuilderUI_EditorOptionComponentClass {}
class GunBuilderUI_EditorOptionBackgroundComponent: GunBuilderUI_EditorOptionComponent {
	override void FillOptions() {
		m_aOptionChoices.Set("2", new GunBuilderUI_EditorOptionData(m_sOptionName, icon: "careerCircleSelected", label: "Dark", value: "2", color: new Color(0.15, 0.15, 0.15, 1.000000)));
		m_aOptionChoices.Set("3", new GunBuilderUI_EditorOptionData(m_sOptionName, icon: "careerCircleSelected", label: "Light", value: "3", color: new Color(0.6, 0.6, 0.6, 1.000000)));
	}
	
	// 0.007172 0.008209 0.008576 1.000000
	
	override void ApplyOption(GunBuilderUI_EditorOptionData data) {
		ParametricMaterialInstanceComponent comp = ParametricMaterialInstanceComponent.Cast(m_owner.FindComponent(ParametricMaterialInstanceComponent));
		if (!comp) {
			Print("GunBuilderUI_EditorOptionBackgroundComponent | No ParametricMaterialInstanceComponent!", LogLevel.ERROR);
			return;
		}

		comp.SetEmissiveColor(data.iconColor.PackToInt());
		comp.SetEmissiveMultiplier(1);
	}
}