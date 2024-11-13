class GunBuilderUI_EditorOptionVisuals {
	string imageSet;
	string iconName;
	ref Color iconColor;
	string optionName;
	string optionValue;
}

class GunBuilderUI_EditorOptionData : Bacon_GunBuilderUI_SlotInfo {
	string iconImageSet;
	string iconName;
	ref Color iconColor;
	string editorOptionLabel;
	string optionLabel;
	string optionValue;
	
	void GunBuilderUI_EditorOptionData(
		string optionName = "Dummy Option",
		string imageSet = "{FDD5423E69D007F8}UI/Textures/Icons/icons_wrapperUI-128.imageset", 
		string icon = "weapons",
	    Color color = Color.Red,
		string label = "Dummy Value",
		string value = "value_1") {
		editorOptionLabel = optionName;
		iconImageSet = imageSet;
		iconName = icon;
		iconColor = color;
		optionLabel = label;
		optionValue = value;
		
		slotType = Bacon_GunBuilderUI_SlotType.OPTION;
	}
}

class GunBuilderUI_EditorOptionComponentClass: ScriptComponentClass {}
class GunBuilderUI_EditorOptionComponent: ScriptComponent {
	[Attribute("{FDD5423E69D007F8}UI/Textures/Icons/icons_wrapperUI-128.imageset", UIWidgets.FileNamePicker, params:"imageset")]
	string m_sIconImageset;
	
	[Attribute("weapons")]
	string m_sIconName;
	
//	[Attribute("1 1 1 1", UIWidgets.ColorPicker)]
//	ref Color m_Color;
	
	[Attribute("Editor Option")]
	string m_sOptionName;
	
	[Attribute("Default Value")]
	string m_sOptionDefaultValue;
	
	ref map<string, ref GunBuilderUI_EditorOptionData> m_aOptionChoices = new map<string, ref GunBuilderUI_EditorOptionData>;
	string m_sCurrentValue;
	
	static ref map<string, string> m_sSelecedEditorOptions = new map<string, string>;
	
	IEntity m_owner;
	
	override void OnPostInit(IEntity owner) { SetEventMask(owner, EntityEvent.INIT); }
	
	override void EOnInit(IEntity owner) {
		if (SCR_Global.IsEditMode())
			return;
		
		Bacon_GunBuilderUI editor = Bacon_GunBuilderUI.GetInstance();
		if (!editor) {
			Print("GunBuilderUI_EditorOptionComponent | No editor instance!", LogLevel.ERROR);
			return;
		}
		
		m_owner = owner;
		
		m_aOptionChoices.Clear();
		FillOptions();
		
		// register in editor
		editor.RegisterOption(m_sOptionName, this);
		
		if (!m_sSelecedEditorOptions.Contains(m_sOptionName))
			OnOptionSelected(m_sOptionDefaultValue);
		else
			OnOptionSelected(m_sSelecedEditorOptions.Get(m_sOptionName));
		
		Print(string.Format("GunBuilderUI_EditorOptionComponent | Registered option %1", m_sOptionName), LogLevel.DEBUG);
	}
	
	GunBuilderUI_EditorOptionData GetSelectedOption() {
		return m_aOptionChoices.Get(m_sCurrentValue);
	}
	
	void FillOptions() {
		m_aOptionChoices.Set("option_value_1", new GunBuilderUI_EditorOptionData(optionName: m_sOptionName, icon: "careerCircleSelected", label: "value 1"));
		m_aOptionChoices.Set("option_value_2", new GunBuilderUI_EditorOptionData(optionName: m_sOptionName, icon: "careerCircleSelected", label: "value 2"));
		m_aOptionChoices.Set("option_value_3", new GunBuilderUI_EditorOptionData(optionName: m_sOptionName, icon: "careerCircleSelected", label: "value 3"));
	}
	
	void OnOptionSelected(string key) {
		m_sCurrentValue = key;
		m_sSelecedEditorOptions.Set(m_sOptionName, key);
		
		ApplyOption(m_aOptionChoices.Get(key));
	}
	
	void ApplyOption(GunBuilderUI_EditorOptionData data) {
	
	}
	
	int GetChoices(out array<GunBuilderUI_EditorOptionData> outChoices) {
		outChoices.Clear();
		
		foreach (string key, GunBuilderUI_EditorOptionData value: m_aOptionChoices) {
			outChoices.Insert(value);
		}
		
		return outChoices.Count();
	}
	
//	void GetOptionVisuals(out string iconImageSet, out string iconName, out Color iconColor) {
//		GunBuilderUI_EditorOptionVisuals visuals = new GunBuilderUI_EditorOptionVisuals();
//	}
}