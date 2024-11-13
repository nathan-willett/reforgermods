enum GunBuilderUI_StatusMessageUIComponent_Messagetype {
	OK,
	WAITING,
	ERROR
}

class GunBuilderUI_StatusMessageUIComponent : ScriptedWidgetComponent
{
	[Attribute("-0.5", desc: "Rotations per second")]
	protected float m_fLoaderSpeed;
	
	[Attribute("0.1", desc: "")]
	protected float m_fFadeSpeed;
	protected float m_fCurrentOpacity = 1.0;
	
	[Attribute("5", desc: "")]
	protected float m_fOkMessageTime;

	protected Widget m_wRoot;
	
	protected ImageWidget m_wIcon;
	protected TextWidget m_wText;
	
	protected bool m_bIsWaiting = false;
	protected bool m_bIsFading = false;
	
	protected string m_sImageLoading = "update";
	protected string m_sImageOK = "check";
	protected string m_sImageError = "warning";
	
	protected string m_imageset = "{FDD5423E69D007F8}UI/Textures/Icons/icons_wrapperUI-128.imageset";
	
	protected ref Color m_colorDefault = new Color(1.0, 1.0, 1.0, 1.0);
	protected ref Color m_colorError = new Color(1.0, 0.5, 0.5, 1.0);

	//---------------------------------------------------------------------------------------------
	override protected void HandlerAttached(Widget w)
	{
		m_wRoot = w;
		
		m_wIcon = ImageWidget.Cast(w.FindAnyWidget("Icon"));
		m_wText = TextWidget.Cast(w.FindAnyWidget("Text"));
	};
	
	// --- display an OK message
	void DisplayMessage(GunBuilderUI_StatusMessageUIComponent_Messagetype messageType, string messageContent) {
		m_wIcon.SetRotation(0);
		m_wRoot.SetColor(m_colorDefault);
		
		m_bIsWaiting = false;
		m_bIsFading = false;
		
		m_wRoot.SetOpacity(1.0);
		
		switch (messageType) {
			case GunBuilderUI_StatusMessageUIComponent_Messagetype.OK: {
				m_wIcon.LoadImageFromSet(0, m_imageset, m_sImageOK);
				m_bIsFading = true;
				break;
			}
			case GunBuilderUI_StatusMessageUIComponent_Messagetype.WAITING: {
				m_wIcon.LoadImageFromSet(0, m_imageset, m_sImageLoading);
				m_bIsWaiting = true;
				break;
			}
			case GunBuilderUI_StatusMessageUIComponent_Messagetype.ERROR: {
				m_wIcon.LoadImageFromSet(0, m_imageset, m_sImageError);
				m_wRoot.SetColor(m_colorError);
				break;
			}
		}

		m_wText.SetText(messageContent);
		m_wRoot.SetVisible(true);
	};

	// --- opacity and loading 
	void Update(float deltaTime)
	{
		if (m_bIsWaiting)
			RotateImage(m_wIcon, deltaTime, m_fLoaderSpeed);
		
		if (m_bIsFading) {
			m_fCurrentOpacity = m_wRoot.GetOpacity() - (m_fFadeSpeed * (m_fOkMessageTime * deltaTime));

			if (m_fCurrentOpacity < 0) {
				m_fCurrentOpacity = 0;
				m_bIsFading = false;
			};

			m_wRoot.SetOpacity(Math.Clamp(m_fCurrentOpacity, 0, 1));
		}
			
	};
	void RotateImage(ImageWidget w, float timeSlice, float speed)
	{
		float rotation = w.GetRotation() + (speed * timeSlice * 360);
		if (rotation > 360)
			rotation -= 360;
		
		w.SetRotation(rotation);
	};
};