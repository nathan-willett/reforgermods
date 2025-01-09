[BaseContainerProps()]
class SCR_BasePlayerLoadout
{
	//------------------------------------------------------------------------------------------------
	//! \return
	ResourceName GetLoadoutResource()
	{
		return ResourceName.Empty;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	string GetLoadoutName()
	{
		return string.Empty;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	ResourceName GetLoadoutImageResource()
	{
		return ResourceName.Empty;
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] playerId
	//! \return
	bool IsLoadoutAvailable(int playerId)
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \return
	bool IsLoadoutAvailableClient()
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] pOwner
	//! \param[in] playerId
	void OnLoadoutSpawned(GenericEntity pOwner, int playerId)
	{
	}
}
