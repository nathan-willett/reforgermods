
modded class SCR_InventoryStoragesListUI : SCR_InventoryStorageBaseUI
{
	protected const ResourceName REAPER_INVENTORY_CONFIG = "{AD415B6656453D2B}Configs/Inventory/REAPER_InventoryUI.conf";
	protected ref SCR_InventoryConfig m_pInventoryUIConfig;
	
	protected const int MAX_ENTRIES_EACH_COLUMN = 13;
				
	//------------------------------------------------------------------------------------------------
	
	void SCR_InventoryStoragesListUI(BaseInventoryStorageComponent storage, LoadoutAreaType slotID = null, SCR_InventoryMenuUI menuManager = null, int iPage = 0, array<BaseInventoryStorageComponent> aTraverseStorage = null )
	{
		m_MenuHandler = menuManager;
		
		Resource resource = BaseContainerTools.LoadContainer(REAPER_INVENTORY_CONFIG);
		if (!resource || !resource.IsValid())
		{
			Print("Cannot load " + REAPER_INVENTORY_CONFIG + " | " + FilePath.StripPath(__FILE__) + ":" + __LINE__, LogLevel.WARNING);
			return;
		}
		
		m_pInventoryUIConfig = SCR_InventoryConfig.Cast(BaseContainerTools.CreateInstanceFromContainer(resource.GetResource().ToBaseContainer()));
		
		//~ Get items in row
		if (m_pInventoryUIConfig)
			//m_iMaxRows = m_pInventoryUIConfig.GetValidLoadoutAreaCount();
			m_iMaxRows = m_pInventoryUIConfig.GetLoadoutAreaCount();
		
		int tempMaxRowCount = m_iMaxRows;
		m_iMaxColumns = 0;
		
		//~ Set ammount of columns
		if (MAX_ENTRIES_EACH_COLUMN > 1)
		{
			while (tempMaxRowCount > 0)
			{
				tempMaxRowCount -= MAX_ENTRIES_EACH_COLUMN;
				m_iMaxColumns++;
			}
		}
		else 
		{
			m_iMaxColumns = 1;
		}
		
		m_iMatrix = new SCR_Matrix( m_iMaxColumns, m_iMaxRows );
		m_sGridPath = "CharacterGrid";
		m_Storage = storage;
	}
	
}