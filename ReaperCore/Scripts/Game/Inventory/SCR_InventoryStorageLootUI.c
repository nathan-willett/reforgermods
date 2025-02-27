
modded class SCR_InventoryStorageLootUI : SCR_InventoryStorageBaseUI
{
	// Default = int rows = 8, i like to have 10 rows!
	
	void SCR_InventoryStorageLootUI(BaseInventoryStorageComponent storage, LoadoutAreaType slotID = null, SCR_InventoryMenuUI menuManager = null, int iPage = 0, array<BaseInventoryStorageComponent> aTraverseStorage = null, GenericEntity character = null, int rows = 10, int cols = 6)
	{
		m_iMaxRows = rows;
		m_iMaxColumns = cols;
		m_iMatrix = new SCR_Matrix( m_iMaxColumns, m_iMaxRows );
		SetSlotAreaType( slotID );
		m_Character = character;
		m_bEnabled = true;
	}	

}