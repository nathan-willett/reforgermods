[BaseContainerProps(configRoot: true), BaseContainerCustomTitleField("m_sLoadoutName")]
class SCR_PlayerArsenalLoadout : SCR_FactionPlayerLoadout
{	
	static const string ARSENALLOADOUT_FACTIONKEY_NONE = "none";
	static const string ARSENALLOADOUT_KEY = "arsenalLoadout";
	static const string ARSENALLOADOUT_FACTION_KEY = "faction";
	
	//------------------------------------------------------------------------------------------------
	override bool IsLoadoutAvailable(int playerId)
	{
		SCR_ArsenalManagerComponent arsenalManager;
		if (SCR_ArsenalManagerComponent.GetArsenalManager(arsenalManager))
		{
			SCR_ArsenalPlayerLoadout arsenalLoadout;
			return arsenalManager.GetPlayerArsenalLoadout(GetGame().GetBackendApi().GetPlayerIdentityId(playerId), arsenalLoadout);
		}
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool IsLoadoutAvailableClient()
	{
		SCR_ArsenalManagerComponent arsenalManager;
		if (SCR_ArsenalManagerComponent.GetArsenalManager(arsenalManager))
			return arsenalManager.GetLocalPlayerLoadoutAvailable();

		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] playerUID
	//! \return the cost of the provided id player's loadout or 0 on error or player not found
	static float GetLoadoutSuppliesCost(string playerUID)
	{
		SCR_ArsenalManagerComponent arsenalManager;
 		if (SCR_ArsenalManagerComponent.GetArsenalManager(arsenalManager))
 		{
			float supplyCostMulti = arsenalManager.GetCalculatedLoadoutSpawnSupplyCostMultiplier();
				
			//~ No need to get supply cost if multiplier is 0
			if (supplyCostMulti <= 0)
				return 0;
			
			SCR_ArsenalPlayerLoadout arsenalLoadout;
			if (arsenalManager.GetPlayerArsenalLoadout(playerUID, arsenalLoadout))
				return arsenalLoadout.suppliesCost * supplyCostMulti;
		}
 		
		return 0;
	}
	
		//------------------------------------------------------------------------------------------------
	//! \param[in] playerId
	//! \return the cost of the provided id player's loadout or 0 on error or player not found
	static float GetLoadoutSuppliesCost(int playerID)
	{
		string playerUID = GetGame().GetBackendApi().GetPlayerIdentityId(playerID);
		return GetLoadoutSuppliesCost(playerUID);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnLoadoutSpawned(GenericEntity pOwner, int playerId)
	{
		super.OnLoadoutSpawned(pOwner, playerId);
		SCR_ArsenalManagerComponent arsenalManager;
		if (!SCR_ArsenalManagerComponent.GetArsenalManager(arsenalManager))
			return;
		
		string playerUID = GetGame().GetBackendApi().GetPlayerIdentityId(playerId);
		GameEntity playerEntity = GameEntity.Cast(pOwner);
		SCR_ArsenalPlayerLoadout playerArsenalItems;
		if (!playerEntity || !arsenalManager.GetPlayerArsenalLoadout(playerUID, playerArsenalItems))
			return;
		
		FactionAffiliationComponent factionComponent = FactionAffiliationComponent.Cast(playerEntity.FindComponent(FactionAffiliationComponent));
		if (!factionComponent)
			return;
		
		SCR_JsonLoadContext context = new SCR_JsonLoadContext();
		bool loadSuccess = true;
		loadSuccess &= context.ImportFromString(playerArsenalItems.loadout);
		// Read faction key and ensure same faction, otherwise delete saved arsenal loadout
		string factionKey;
		loadSuccess &= context.ReadValue(ARSENALLOADOUT_FACTION_KEY, factionKey) && factionKey != ARSENALLOADOUT_FACTIONKEY_NONE;
		loadSuccess &= factionKey == factionComponent.GetAffiliatedFaction().GetFactionKey();
		loadSuccess &= context.ReadValue(ARSENALLOADOUT_KEY, playerEntity);
		
		// Deserialisation failed, delete saved arsenal loadout
		if (!loadSuccess)
			arsenalManager.SetPlayerArsenalLoadout(playerId, null, null, SCR_EArsenalSupplyCostType.RESPAWN_COST);
	}
}
