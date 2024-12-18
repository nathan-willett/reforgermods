//!Cache world save manifest and provide info to edited value
class SCR_EditedSaveManifest
{
	protected ref WorldSaveManifest m_Manifest;
	protected string m_sEditingValue;
	protected string m_Value;
	
	//----------------------------------------------------------------------------------------
	WorldSaveManifest GetManifest()
	{
		return m_Manifest;
	}
	
	//----------------------------------------------------------------------------------------
	string GetEditingValue()
	{
		return m_sEditingValue;
	}
	
	void SetEditingValue(string value)
	{
		m_sEditingValue = value;
	}
	
	//----------------------------------------------------------------------------------------
	string GetValue()
	{
		return m_Value;
	}
	
	//----------------------------------------------------------------------------------------
	void SetValue(string value)
	{
		m_Value = value;
	}
	
	//----------------------------------------------------------------------------------------
	void SCR_EditedSaveManifest(WorldSaveManifest manifest = null)
	{
		if (manifest)
			m_Manifest = manifest;
		else
			m_Manifest = new WorldSaveManifest();
	}
}