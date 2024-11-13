enum Bacon_GunBuilderUI_SlotCategory {
	CHARACTER_CLOTHING,
	CHARACTER_GEAR,
	CHARACTER_ITEMS,
	LOADOUTS,
	SERVER_LOADOUTS,
	SETTINGS
}

enum Bacon_GunBuilderUI_ArsenalCategory {
	AMMUNITION,
	EQUIPMENT,
	THROWABLE,
	MEDICAL,
	WEAPON,
	ATTACHMENT,
	CHARACTER
}

enum Bacon_GunBuilderUI_CategoryUIComponent_Mode {
	CHARACTER,
	ARSENAL
}

class GunBuilderUI_CategoryButtonUIComponent: SCR_ButtonImageComponent {
	string m_sData;
	
	ImageWidget m_wActivePanel;
	
	TextWidget m_wCategoryLabel;
	
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
		
		m_wCategoryLabel = TextWidget.Cast(w.FindAnyWidget("CategoryLabel"));
		m_wActivePanel = ImageWidget.Cast(w.FindAnyWidget("ActivePanel"));
	}
	
	void SetData(string data) { m_sData = data; }
	string GetData() { return m_sData; }
	
	void SetLabel(string label) {
		m_wCategoryLabel.SetText(label); 
	}
	
	void SetActive(bool active) {
		m_wActivePanel.SetVisible(active);
		m_wCategoryLabel.SetVisible(active);
	}
	
	void SetImageFromImageSet(ResourceName imageSet, string imageName)
	{
		if (!m_wImage)
			return;
		

		m_wImage.SetVisible(true);
		m_wImage.LoadImageFromSet(0, imageSet, imageName);

		// Resize
//		int x, y;
//		m_wImage.GetImageSize(0, x, y);
//		m_wImage.SetSize(x, y);
		
		m_sTexture = imageSet;
		m_sImageName = imageName;
	}
}

class GunBuilderUI_CategoryButtonsUIComponent: ScriptedWidgetComponent {
	static ResourceName m_sResourceName_CategoryButton = "{8196382B6B9C8C14}UI/layouts/WidgetLibrary/Buttons/GunBuilderUI_ButtonCategory.layout";
	static ResourceName m_sResourceName_IconImageSet = "{D17288006833490F}UI/Textures/Icons/icons_wrapperUI-128.imageset";
	static ResourceName m_sResourceName_ArsenalIcons = "{ECC3919D420CDD2E}Assets/BaconLoadoutEditor/icons_arsenal.imageset";
	static ResourceName m_sResourceName_WeaponInfoImageSet = "{3BB05C675B05A74B}UI/Textures/WeaponInfo/icons_weaponInfo.imageset";
	
	Widget m_wRoot;
	HorizontalLayoutWidget m_layoutMain;
	
	ref array<GunBuilderUI_CategoryButtonUIComponent> m_wButtonComponents = {};
	
	[Attribute("0", UIWidgets.ComboBox, "Mode", "", ParamEnumArray.FromEnum(Bacon_GunBuilderUI_CategoryUIComponent_Mode))]
	Bacon_GunBuilderUI_CategoryUIComponent_Mode m_eCategoryMode;

	string m_sCurrentCategory;
	ref ScriptInvoker m_OnCategoryChangedInvoker = new ScriptInvoker();
	
	SCR_InputButtonComponent m_wButtonLeft;
	SCR_InputButtonComponent m_wButtonRight;
	
	bool m_bIsAdmin = false;
		
	override void HandlerAttached(Widget w) {
		super.HandlerAttached(w);
		
		m_wRoot = w;
		m_layoutMain = HorizontalLayoutWidget.Cast(m_wRoot.FindAnyWidget("ButtonContainer"));
		
		m_wButtonLeft = SCR_InputButtonComponent.Cast(m_wRoot.FindAnyWidget("NavigationButtonLeft").FindHandler(SCR_InputButtonComponent));
		m_wButtonRight = SCR_InputButtonComponent.Cast(m_wRoot.FindAnyWidget("NavigationButtonRight").FindHandler(SCR_InputButtonComponent));
		m_wButtonLeft.m_OnActivated.Insert(OnButtonPressed_Left);
		m_wButtonRight.m_OnActivated.Insert(OnButtonPressed_Right);
		
		m_bIsAdmin = Bacon_GunBuilderUI_Helpers.IsLocalPlayerAdmin();
		
		InitCategoryButtons();
		
		if (m_eCategoryMode == Bacon_GunBuilderUI_CategoryUIComponent_Mode.CHARACTER) {
			m_sCurrentCategory = "CHARACTER_CLOTHING"
		} else {
			m_sCurrentCategory = "AMMUNITION"
		}
		
		VisualizeSelection();
	}
	
	void SetEnabled(bool state) {
		m_layoutMain.SetEnabled(state);
		m_wButtonLeft.SetEnabled(state);
		m_wButtonRight.SetEnabled(state);
//		m_wButtonLeft.SetVisible(state);
//		m_wButtonRight.SetVisible(state);
	}
	
	void OnButtonPressed_Left() { Navigate(-1); }
	void OnButtonPressed_Right() { Navigate(1); }
	void Navigate(int step) {
		if (!m_layoutMain.IsEnabled())
			return;
		
		typename categoryEnum;
		if (m_eCategoryMode == Bacon_GunBuilderUI_CategoryUIComponent_Mode.CHARACTER) {
			categoryEnum = Bacon_GunBuilderUI_SlotCategory;
		} else {
			categoryEnum = Bacon_GunBuilderUI_ArsenalCategory;
		}

		int max = categoryEnum.GetVariableCount() - 1;
		
		if (!m_bIsAdmin) {
			if (m_sCurrentCategory == "SETTINGS" && step < 0) {
				step *= 2;
			}
			
			if (m_sCurrentCategory == "LOADOUTS" && step > 0) {
				step *= 2;
			}
		}

		int newCategory = typename.StringToEnum(categoryEnum, m_sCurrentCategory) + step;

		if (newCategory > max) {
			newCategory = 0;
		} else if (newCategory < 0) {
			newCategory = max;
		}
		
		// Print("New category: "+newCategory);
		
		// m_sCurrentCategory = m_wButtonComponents[newCategory].GetData();
		if (newCategory >= m_wButtonComponents.Count())
			newCategory -= 1;

		OnButtonClicked(m_wButtonComponents[newCategory]);
	}
	
	void VisualizeSelection() {
		foreach (GunBuilderUI_CategoryButtonUIComponent handler : m_wButtonComponents) {
			if (handler.GetData() == m_sCurrentCategory) {
				handler.SetActive(true);
				continue;
			}
			
			handler.SetActive(false);
		}
	}
	
	void OnButtonClicked(SCR_ButtonBaseComponent comp) {
		GunBuilderUI_CategoryButtonUIComponent button = GunBuilderUI_CategoryButtonUIComponent.Cast(comp);
		string selected = button.GetData();
		
		if (selected == m_sCurrentCategory)
			return;
		
		m_sCurrentCategory = selected;
		
		// Print(string.Format("GunBuilderUI_CategoryButtonsUIComponent | Button clicked: %1", m_sCurrentCategory));
		
		VisualizeSelection();
		
		m_OnCategoryChangedInvoker.Invoke();
	}
	
	void GetCategoryIcon(string categoryName, out ResourceName imageSet, out string iconName) {
		int asEnum;
		
		if (m_eCategoryMode == Bacon_GunBuilderUI_CategoryUIComponent_Mode.CHARACTER) {
			asEnum = typename.StringToEnum(Bacon_GunBuilderUI_SlotCategory, categoryName);
			imageSet = m_sResourceName_IconImageSet;
			
			switch (asEnum) {
				case Bacon_GunBuilderUI_SlotCategory.CHARACTER_CLOTHING: {
					iconName = "player";
					break;
				}
				case Bacon_GunBuilderUI_SlotCategory.CHARACTER_GEAR: {
					iconName = "weapons";
					break;
				}
				case Bacon_GunBuilderUI_SlotCategory.CHARACTER_ITEMS: {
					iconName = "radialSubMenu";
					break;
				}
				case Bacon_GunBuilderUI_SlotCategory.SETTINGS: {
					iconName = "settings";
					break;
				}
				case Bacon_GunBuilderUI_SlotCategory.LOADOUTS: {
					iconName = "save";
					break;
				}
				case Bacon_GunBuilderUI_SlotCategory.SERVER_LOADOUTS: {
					iconName = "save";
					break;
				}
			}
			
			return;
		}

		if (m_eCategoryMode == Bacon_GunBuilderUI_CategoryUIComponent_Mode.ARSENAL) {
			asEnum = typename.StringToEnum(Bacon_GunBuilderUI_ArsenalCategory, categoryName);

			imageSet = m_sResourceName_ArsenalIcons;
			iconName = categoryName;
			
			if (categoryName == "WEAPON") {
				imageSet = m_sResourceName_IconImageSet;
				iconName = "weapons";
			}

			return;
		}
	}
	
	void InitCategoryButtons() {
		int count;

		array<string> enumNames = {};
		if (m_eCategoryMode == Bacon_GunBuilderUI_CategoryUIComponent_Mode.CHARACTER) {
			count = SCR_Enum.GetEnumNames(Bacon_GunBuilderUI_SlotCategory, enumNames);
		} else {
			count = SCR_Enum.GetEnumNames(Bacon_GunBuilderUI_ArsenalCategory, enumNames);
		}
		
		ResourceName imageset;
		string iconName;
		string label;
		
		for (int i = 0; i < count; i++) {
			if (enumNames[i] == "SERVER_LOADOUTS" && !m_bIsAdmin) {
				continue;
			}
			
			Widget button = Widget.Cast(GetGame().GetWorkspace().CreateWidgets(m_sResourceName_CategoryButton, m_layoutMain));
			GunBuilderUI_CategoryButtonUIComponent handler = GunBuilderUI_CategoryButtonUIComponent.Cast(button.FindHandler(GunBuilderUI_CategoryButtonUIComponent));
			
			handler.m_OnClicked.Insert(OnButtonClicked);
			handler.SetData(enumNames[i]);
			
			GetCategoryIcon(enumNames[i], imageset, iconName);
			handler.SetImageFromImageSet(imageset, iconName);
			
			if (m_eCategoryMode == Bacon_GunBuilderUI_CategoryUIComponent_Mode.CHARACTER) {
				handler.SetLabel(GetCategoryLabelCharacter(enumNames[i]));

//				if (enumNames[i] == "LOADOUTS" && !SCR_Global.IsEditMode())
//					handler.SetEnabled(false);
			} else {
				handler.SetLabel(GetCategoryLabelArsenal(enumNames[i]));
			}
			
			m_wButtonComponents.Insert(handler);
		}
	}
	
	string GetCategoryLabelCharacter(string enumName) {
		int asEnum = typename.StringToEnum(Bacon_GunBuilderUI_SlotCategory, enumName);
		switch (asEnum) {
			case Bacon_GunBuilderUI_SlotCategory.CHARACTER_CLOTHING: {
				return "Clothing";
			}
			case Bacon_GunBuilderUI_SlotCategory.CHARACTER_GEAR: {
				return "Gear";
			}
			case Bacon_GunBuilderUI_SlotCategory.CHARACTER_ITEMS: {
				return "Inventory";
			}
			case Bacon_GunBuilderUI_SlotCategory.LOADOUTS: {
				return "Loadouts";
			}
			case Bacon_GunBuilderUI_SlotCategory.SERVER_LOADOUTS: {
				return "GM";
			}
			case Bacon_GunBuilderUI_SlotCategory.SETTINGS: {
				return "Settings";
			}
		}
		
		return "UNKNOWN";
	}
	
	string GetCategoryLabelArsenal(string enumName) {
		int asEnum = typename.StringToEnum(Bacon_GunBuilderUI_ArsenalCategory, enumName);
		
		switch (asEnum) {
			case Bacon_GunBuilderUI_ArsenalCategory.AMMUNITION: {
				return "Ammunition";
			}
			case Bacon_GunBuilderUI_ArsenalCategory.EQUIPMENT: {
				return "Equipment";
			}
			case Bacon_GunBuilderUI_ArsenalCategory.THROWABLE: {
				return "Throwables & Explosives";
			}
			case Bacon_GunBuilderUI_ArsenalCategory.MEDICAL: {
				return "Medical";
			}
			case Bacon_GunBuilderUI_ArsenalCategory.WEAPON: {
				return "Weapons";
			}
			case Bacon_GunBuilderUI_ArsenalCategory.ATTACHMENT: {
				return "Weapon Attachments";
			}
			case Bacon_GunBuilderUI_ArsenalCategory.CHARACTER: {
				return "Clothing";
			}
		}
		
		return "UNKNOWN";
	}
	
	// is valid storage for this category? used for slot checks
	bool ShouldViewCharacterStorageCategory(Bacon_GunBuilderUI_StorageType storageType) {
		int asEnum = typename.StringToEnum(Bacon_GunBuilderUI_SlotCategory, m_sCurrentCategory);
		
		if (asEnum == Bacon_GunBuilderUI_SlotCategory.CHARACTER_CLOTHING) {
			if (storageType == Bacon_GunBuilderUI_StorageType.CHARACTER_LOADOUT)
				return true;
			
			return false;
		}
		
		if (storageType == Bacon_GunBuilderUI_StorageType.CHARACTER_LOADOUT)
			return false;
		
		return true;
	}
	Bacon_GunBuilderUI_SlotCategory GetCurrentCategoryCharacter() {
		return typename.StringToEnum(Bacon_GunBuilderUI_SlotCategory, m_sCurrentCategory);
	}
	
	// 
	void GetCurrentArsenalFilter(out SCR_EArsenalItemMode outMode, out SCR_EArsenalItemType outType) {
		int asEnum = typename.StringToEnum(Bacon_GunBuilderUI_ArsenalCategory, m_sCurrentCategory);
		
		switch (asEnum) {
			case Bacon_GunBuilderUI_ArsenalCategory.AMMUNITION: {
				outMode = SCR_EArsenalItemMode.AMMUNITION;
				outType = SCR_EArsenalItemType.PISTOL | SCR_EArsenalItemType.RIFLE | SCR_EArsenalItemType.SNIPER_RIFLE | SCR_EArsenalItemType.ROCKET_LAUNCHER | SCR_EArsenalItemType.EXPLOSIVES | SCR_EArsenalItemType.MACHINE_GUN;
				break;
			}
			case Bacon_GunBuilderUI_ArsenalCategory.EQUIPMENT: {
				outMode = SCR_EArsenalItemMode.DEFAULT | SCR_EArsenalItemMode.SUPPORT_STATION;
				outType = SCR_EArsenalItemType.EQUIPMENT;
				break;
			}
			case Bacon_GunBuilderUI_ArsenalCategory.THROWABLE: {
				outMode = SCR_EArsenalItemMode.DEFAULT | SCR_EArsenalItemMode.CONSUMABLE;
				outType = SCR_EArsenalItemType.LETHAL_THROWABLE | SCR_EArsenalItemType.NON_LETHAL_THROWABLE | SCR_EArsenalItemType.EXPLOSIVES;
				break;
			}
			case Bacon_GunBuilderUI_ArsenalCategory.MEDICAL: {
				outMode = SCR_EArsenalItemMode.DEFAULT | SCR_EArsenalItemMode.CONSUMABLE | SCR_EArsenalItemMode.SUPPORT_STATION;
				outType = SCR_EArsenalItemType.HEAL;
				break;
			}
			case Bacon_GunBuilderUI_ArsenalCategory.WEAPON: {
				outMode = SCR_EArsenalItemMode.WEAPON | SCR_EArsenalItemMode.WEAPON_VARIANTS;
				outType = SCR_EArsenalItemType.PISTOL | SCR_EArsenalItemType.RIFLE | SCR_EArsenalItemType.SNIPER_RIFLE | SCR_EArsenalItemType.ROCKET_LAUNCHER | SCR_EArsenalItemType.EXPLOSIVES | SCR_EArsenalItemType.MACHINE_GUN;
				break;
			}
			case Bacon_GunBuilderUI_ArsenalCategory.ATTACHMENT: {
				outMode = SCR_EArsenalItemMode.ATTACHMENT;
				outType = SCR_EArsenalItemType.WEAPON_ATTACHMENT;
				break;
			}
			case Bacon_GunBuilderUI_ArsenalCategory.CHARACTER: {
				outMode = SCR_EArsenalItemMode.DEFAULT;
				outType = SCR_EArsenalItemType.BACKPACK | SCR_EArsenalItemType.RADIO_BACKPACK | SCR_EArsenalItemType.HEADWEAR | SCR_EArsenalItemType.TORSO | SCR_EArsenalItemType.VEST_AND_WAIST | SCR_EArsenalItemType.LEGS | SCR_EArsenalItemType.FOOTWEAR;
				break;
			}
		}
	}
	
	void SetWidgetEnabled(bool enabled) {
		m_wButtonLeft.SetEnabled(enabled);
		m_wButtonRight.SetEnabled(enabled);
		m_wRoot.SetVisible(enabled);
		m_wRoot.SetEnabled(enabled);
	}
}

//class GunBuilderUI_EnumCategoryUIComponent: ScriptedWidgetComponent {
//	static ResourceName m_sResourceName_CategoryDot = "{FEF4C810C60C2731}UI/BaconLoadoutEditor/GunBuilderUI_CategoryCircle.layout";
//	static ResourceName m_sResourceName_IconImageSet = "{D17288006833490F}UI/Textures/Icons/icons_wrapperUI-128.imageset";
//	static ResourceName m_sResourceName_WeaponInfoImageSet = "{3BB05C675B05A74B}UI/Textures/WeaponInfo/icons_weaponInfo.imageset";
//	static string m_dotImageActive = "careerCircleSelected";
//	static string m_dotImageInactive = "careerCircleOutline";
//	
//	ImageWidget m_image;
//	TextWidget m_text;
//	
//	int m_eCurrentCategory = 0;
//	
//	SCR_InputButtonComponent m_wButtonLeft;
//	SCR_InputButtonComponent m_wButtonRight;
//	
//	Widget m_wRoot;
//	HorizontalLayoutWidget m_layoutMain;
//	
//	ref ScriptInvoker m_OnCategoryChangedInvoker = new ScriptInvoker();
//	
//	[Attribute("0", UIWidgets.ComboBox, "Mode", "", ParamEnumArray.FromEnum(Bacon_GunBuilderUI_CategoryUIComponent_Mode))]
//	Bacon_GunBuilderUI_CategoryUIComponent_Mode m_eCategoryMode;
//	
//	ref array<ImageWidget> m_CategoryCircles = {};
//	ref array<string> m_categoryChoices = {};
//	
//	string m_sCurrentCategory;
//	
//	override void HandlerAttached(Widget w) {
//		super.HandlerAttached(w);
//		
//		m_wRoot = w;
//		
//		m_image = ImageWidget.Cast(m_wRoot.FindAnyWidget("SlotImage"));
//		m_text = TextWidget.Cast(m_wRoot.FindAnyWidget("SlotText"));
//	
//		m_layoutMain = HorizontalLayoutWidget.Cast(m_wRoot.FindAnyWidget("HorizontalLayoutMain"));
//		
//		InitCategories();
//		
//		Print(string.Format("GunBuilderUI_SlotCategoryUIComponent | Found %1 dot widgets", m_CategoryCircles.Count()), LogLevel.DEBUG);
//		// UpdateDotWidgets();
//		
//		// setup buttons
//		m_wButtonLeft = SCR_InputButtonComponent.Cast(m_layoutMain.FindAnyWidget("NavigationButtonLeft").FindHandler(SCR_InputButtonComponent));
//		m_wButtonRight = SCR_InputButtonComponent.Cast(m_layoutMain.FindAnyWidget("NavigationButtonRight").FindHandler(SCR_InputButtonComponent));
//		
//		m_wButtonLeft.m_OnActivated.Insert(OnButtonPressed_Left);
//		m_wButtonRight.m_OnActivated.Insert(OnButtonPressed_Right);
//		
//		SetSelectedCategory(0);
//		
//		// UpdateCategoryImageAndText();
//	}
//	
//	void OnButtonPressed_Left() {
//		SetSelectedCategory(m_eCurrentCategory - 1);
//	}
//	void OnButtonPressed_Right() {
//		SetSelectedCategory(m_eCurrentCategory + 1);
//	}
//	
//	void InitCategories() {
//		int count;
//
//		if (m_eCategoryMode == Bacon_GunBuilderUI_CategoryUIComponent_Mode.CHARACTER) {
//			count = SCR_Enum.GetEnumNames(Bacon_GunBuilderUI_SlotCategory, m_categoryChoices);
//		} else {
//			count = SCR_Enum.GetEnumNames(Bacon_GunBuilderUI_ArsenalCategory, m_categoryChoices);
//		}
//		
//		HorizontalLayoutWidget dotsLayout = HorizontalLayoutWidget.Cast(m_layoutMain.FindAnyWidget("HorizontalLayoutDots"));
//		
//		for (int i = 0; i < count; i++) {
//			ImageWidget iw = ImageWidget.Cast(GetGame().GetWorkspace().CreateWidgets(m_sResourceName_CategoryDot, dotsLayout));
//			iw.SetZOrder(i);
//			m_CategoryCircles.Insert(iw);
//		}
//		
//		m_CategoryCircles[0].LoadImageFromSet(0, m_sResourceName_IconImageSet, m_dotImageActive);
//	}
//	
//	void SetSelectedCategory(int id) {
//		if (id >= m_categoryChoices.Count()) {
//			id = 0;
//		} else if (id < 0) {
//			id = m_categoryChoices.Count() - 1;
//		}
//		
//		m_eCurrentCategory = id;
//		
//		m_CategoryCircles[0].SetZOrder(id);
//		//
//		
//		if (m_eCategoryMode == Bacon_GunBuilderUI_CategoryUIComponent_Mode.CHARACTER) {
//			UpdateWidget_CharacterCategories();
//		} else {
//			UpdateWidget_ArsenalItemMode();
//		}
//		
//		m_OnCategoryChangedInvoker.Invoke();
//	}
//	
//	void UpdateWidget_CharacterCategories() {
//		string imageName = "";
//		string text = "";
//		
//		int asEnum = typename.StringToEnum(Bacon_GunBuilderUI_SlotCategory, m_categoryChoices[m_eCurrentCategory]);
//		
//		switch (asEnum) {
//			case Bacon_GunBuilderUI_SlotCategory.CHARACTER_CLOTHING: {
//				imageName = "player";
//				text = "Character Slots";
//				break;
//			}
//			case Bacon_GunBuilderUI_SlotCategory.CHARACTER_GEAR: {
//				imageName = "weapons";
//				text = "Gear Slots";
//				break;
//			}
//			case Bacon_GunBuilderUI_SlotCategory.CHARACTER_ITEMS: {
//				imageName = "radialSubMenu";
//				text = "Item Inventory";
//				break;
//			}
//		}
//		
//		m_image.LoadImageFromSet(0, m_sResourceName_IconImageSet, imageName);
//		m_text.SetText(text);
//	}
//	
//	void UpdateWidget_ArsenalItemMode() {
//		ResourceName imageset = "";
//		string imageName = "";
//		string text = "";
//		
//		int asEnum = typename.StringToEnum(Bacon_GunBuilderUI_ArsenalCategory, m_categoryChoices[m_eCurrentCategory]);
//		
//		switch (asEnum) {
//			case Bacon_GunBuilderUI_ArsenalCategory.AMMUNITION: {
//				imageset = m_sResourceName_WeaponInfoImageSet;
//				imageName = "firemode-rifle-auto";
//				text = "Ammunition";
//				break;
//			}
//			case Bacon_GunBuilderUI_ArsenalCategory.ITEMS: {
//				imageset = m_sResourceName_IconImageSet;
//				imageName = "supplies";
//				text = "Consumables";
//				break;
//			}
//			case Bacon_GunBuilderUI_ArsenalCategory.WEAPON: {
//				imageset = m_sResourceName_IconImageSet;
//				imageName = "weapons";
//				text = "Weapons";
//				break;
//			}
//			case Bacon_GunBuilderUI_ArsenalCategory.ATTACHMENT: {
//				imageset = m_sResourceName_IconImageSet;
//				imageName = "props";
//				text = "Weapon Attachments";
//				break;
//			}
//			case Bacon_GunBuilderUI_ArsenalCategory.CHARACTER: {
//				imageset = m_sResourceName_IconImageSet;
//				imageName = "player";
//				text = "Character";
//				break;
//			}
//		}
//
//		m_image.LoadImageFromSet(0, imageset, imageName);
//		m_text.SetText(text);
//	}
//	
//	// is valid stora
//	bool ShouldViewCharacterStorageCategory(Bacon_GunBuilderUI_StorageType storageType) {
//		int asEnum = typename.StringToEnum(Bacon_GunBuilderUI_SlotCategory, m_categoryChoices[m_eCurrentCategory]);
//		
//		if (asEnum == Bacon_GunBuilderUI_SlotCategory.CHARACTER_CLOTHING) {
//			if (storageType == Bacon_GunBuilderUI_StorageType.CHARACTER_LOADOUT)
//				return true;
//			
//			return false;
//		}
//		
//		if (storageType == Bacon_GunBuilderUI_StorageType.CHARACTER_LOADOUT)
//			return false;
//		
//		return true;
//	}
//	Bacon_GunBuilderUI_SlotCategory GetCurrentCategoryCharacter() {
//		return typename.StringToEnum(Bacon_GunBuilderUI_SlotCategory, m_categoryChoices[m_eCurrentCategory]);
//	}
//	
//	// 
//	SCR_EArsenalItemMode GetCurrentArsenalFilter() {
//		int asEnum = typename.StringToEnum(Bacon_GunBuilderUI_ArsenalCategory, m_categoryChoices[m_eCurrentCategory]);
//		
//		switch (asEnum) {
//			case Bacon_GunBuilderUI_ArsenalCategory.AMMUNITION: {
//				return SCR_EArsenalItemMode.AMMUNITION;
//			}
//			case Bacon_GunBuilderUI_ArsenalCategory.ITEMS: {
//				return SCR_EArsenalItemMode.CONSUMABLE | SCR_EArsenalItemMode.SUPPORT_STATION;
//			}
//			case Bacon_GunBuilderUI_ArsenalCategory.WEAPON: {
//				return SCR_EArsenalItemMode.WEAPON | SCR_EArsenalItemMode.WEAPON_VARIANTS;
//			}
//			case Bacon_GunBuilderUI_ArsenalCategory.ATTACHMENT: {
//				return SCR_EArsenalItemMode.ATTACHMENT;
//			}
//			case Bacon_GunBuilderUI_ArsenalCategory.CHARACTER: {
//				return SCR_EArsenalItemMode.DEFAULT;
//			}
//		}
//		
//		return SCR_EArsenalItemMode.DEFAULT;
//	}
//	
//	void SetWidgetEnabled(bool enabled) {
//		m_wButtonLeft.SetEnabled(enabled);
//		m_wButtonRight.SetEnabled(enabled);
//		m_wRoot.SetVisible(enabled);
//		m_wRoot.SetEnabled(enabled);
//	}
//}

class GunBuilderUI_SlotCategoryUIComponent: ScriptedWidgetComponent {
	ImageWidget m_image;
	TextWidget m_text;

	Bacon_GunBuilderUI_SlotCategory m_eCurrentCategory = Bacon_GunBuilderUI_SlotCategory.CHARACTER_CLOTHING;

	SCR_InputButtonComponent m_wButtonLeft;
	SCR_InputButtonComponent m_wButtonRight;
	
	ref array<ImageWidget> m_CategoryCircles = {};
	
	// array<Bacon_GunBuilderUI_SlotCategory> m_AvailableCategories = {Bacon_GunBuilderUI_SlotCategory.CHARACTER_CLOTHING, Bacon_GunBuilderUI_SlotCategory.CHARACTER_GEAR};
	
	string m_iconImageSet = "{D17288006833490F}UI/Textures/Icons/icons_wrapperUI-64.imageset";
	string m_dotImageActive = "careerCircleSelected";
	string m_dotImageInactive = "careerCircleOutline";
	
	Widget m_wRoot;
	
	ref ScriptInvoker m_OnCategoryChangedInvoker = new ScriptInvoker();
	
	override void HandlerAttached(Widget w) {
		super.HandlerAttached(w);
		
		m_wRoot = w;
		
		m_image = ImageWidget.Cast(m_wRoot.FindAnyWidget("SlotImage"));
		m_text = TextWidget.Cast(m_wRoot.FindAnyWidget("SlotText"));
	
		HorizontalLayoutWidget layoutMain = HorizontalLayoutWidget.Cast(m_wRoot.FindAnyWidget("HorizontalLayoutMain"));
		
		HorizontalLayoutWidget dotsLayout = HorizontalLayoutWidget.Cast(layoutMain.FindAnyWidget("HorizontalLayoutDots"));
		// setup category circles
		Widget dot = dotsLayout.GetChildren();
		int i = 0;
		while (dot && i++ < 10) {
			ImageWidget dotImage = ImageWidget.Cast(dot);
			if (dotImage)
				m_CategoryCircles.Insert(dotImage);
			
			dot = dot.GetSibling();
		};
		
		Print(string.Format("GunBuilderUI_SlotCategoryUIComponent | Found %1 dot widgets", m_CategoryCircles.Count()), LogLevel.DEBUG);
		UpdateDotWidgets();
		
		// setup buttons
		m_wButtonLeft = SCR_InputButtonComponent.Cast(layoutMain.FindAnyWidget("NavigationButtonLeft").FindHandler(SCR_InputButtonComponent));
		m_wButtonRight = SCR_InputButtonComponent.Cast(layoutMain.FindAnyWidget("NavigationButtonRight").FindHandler(SCR_InputButtonComponent));
		
		m_wButtonLeft.m_OnActivated.Insert(OnButtonPressed_Left);
		m_wButtonRight.m_OnActivated.Insert(OnButtonPressed_Right);
		
		UpdateCategoryImageAndText();
	}
	
	void SetWidgetEnabled(bool enabled) {
		m_wButtonLeft.SetEnabled(enabled);
		m_wButtonRight.SetEnabled(enabled);
		m_wRoot.SetVisible(enabled);
		m_wRoot.SetEnabled(enabled);
	}
	
	void SetCategory(bool next) {
		int category;
		if (next)
			category = m_eCurrentCategory + 1;
		else
			category = m_eCurrentCategory - 1;
		
		string diyValidation = SCR_Enum.GetEnumName(Bacon_GunBuilderUI_SlotCategory, category);
		if (diyValidation.IsEmpty() || diyValidation == "unknown") {
			if (next)
				category = 1;
			else
				category = 3;
		}

		m_eCurrentCategory = category;

		UpdateCategoryImageAndText();
		UpdateDotWidgets();
		
		m_OnCategoryChangedInvoker.Invoke(m_eCurrentCategory);
	}
	
	void UpdateCategoryImageAndText() {
		string imageName = "";
		string text = "";
		
		switch (m_eCurrentCategory) {
			case Bacon_GunBuilderUI_SlotCategory.CHARACTER_CLOTHING: {
				imageName = "player";
				text = "Character Slots";
				break;
			}
			case Bacon_GunBuilderUI_SlotCategory.CHARACTER_GEAR: {
				imageName = "weapons";
				text = "Gear Slots";
				break;
			}
			case Bacon_GunBuilderUI_SlotCategory.CHARACTER_ITEMS: {
				imageName = "radialSubMenu";
				text = "Item Inventory";
				break;
			}
		}
		
		m_image.LoadImageFromSet(0, m_iconImageSet, imageName);
		m_text.SetText(text);
	}
	
	void UpdateDotWidgets() {
		for (int i = 0; i < m_CategoryCircles.Count(); i++) {
			if (i+1 == m_eCurrentCategory)
				m_CategoryCircles[i].LoadImageFromSet(0, m_iconImageSet, m_dotImageActive);
			else
				m_CategoryCircles[i].LoadImageFromSet(0, m_iconImageSet, m_dotImageInactive);
		}
	}

	void OnButtonPressed_Right() {
		SetCategory(true);
	}
	void OnButtonPressed_Left() {
		SetCategory(false);
	}
	
	bool ShouldViewCharacterStorageCategory(Bacon_GunBuilderUI_StorageType storageType) {
		if (m_eCurrentCategory == Bacon_GunBuilderUI_SlotCategory.CHARACTER_CLOTHING) {
			if (storageType == Bacon_GunBuilderUI_StorageType.CHARACTER_LOADOUT)
				return true;
			
			return false;
		}
		
		if (storageType == Bacon_GunBuilderUI_StorageType.CHARACTER_LOADOUT)
			return false;
		
		return true;
	}
}