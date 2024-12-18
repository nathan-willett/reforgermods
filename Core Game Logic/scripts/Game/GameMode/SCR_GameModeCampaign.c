//------------------------------------------------------------------------------------------------
class SCR_GameModeCampaignClass : SCR_BaseGameModeClass
{
}

//------------------------------------------------------------------------------------------------
class SCR_GameModeCampaign : SCR_BaseGameMode
{
	[Attribute("1", desc: "Run Conflict automatically at scenario start. If disabled, use RequestStart() method on server.", category: "Campaign")]
	protected bool m_bAutostart;

	[Attribute("1", desc: "Terminate the scenario when Conflict is finished.", category: "Campaign")]
	protected bool m_bTerminateScenario;

	[Attribute("2", desc: "How many control points does a faction need to win", params: "1 inf 1", category: "Campaign")]
	protected int m_iControlPointsThreshold;

	[Attribute("300", desc: "How long does a faction need to hold the control points to win (seconds).", params: "0 inf 1", category: "Campaign")]
	protected float m_fVictoryTimer;

	[Attribute("1000", desc: "Supplies will be autoreplenished in bases until this limit is reached.", params: "0 inf 1", category: "Campaign")]
	protected int m_iSuppliesReplenishThreshold;
	
	[Attribute("200", desc: "Supplies will be autoreplenished in bases quickly until this limit is reached (HQs are not affected).", params: "0 inf 1", category: "Campaign")]
	protected int m_iQuickSuppliesReplenishThreshold;
	
	[Attribute("2", desc: "Supplies income will be multiplied by this number unless the quick replenish threshold has been reached.", params: "1 inf 0.05", category: "Campaign")]
	protected float m_fQuickSuppliesReplenishMultiplier;

	[Attribute("5", desc: "How often are supplies replenished in bases (seconds).", params: "1 inf 1", category: "Campaign")]
	protected int m_iSuppliesArrivalInterval;

	[Attribute("40", desc: "How many supplies are periodically replenished in HQs.", params: "0 inf 1", category: "Campaign")]
	protected int m_iRegularSuppliesIncome;

	[Attribute("4", desc: "How many supplies are periodically replenished in non-HQ bases.", params: "0 inf 1", category: "Campaign")]
	protected int m_iRegularSuppliesIncomeBase;

	[Attribute("4", desc: "How many extra supplies are periodically replenished in non-HQ bases per base connected via radio.", params: "0 inf 1", category: "Campaign")]
	protected int m_iRegularSuppliesIncomeExtra;
	
	[Attribute("600", desc: "The amount of supplies in HQs.", params: "0 inf 1", category: "Campaign")]
	protected int m_iHQStartingSupplies;

	[Attribute("100", desc: "When randomized, the least supplies a base can hold at the start.", params: "0 inf 1", category: "Campaign")]
	protected int m_iMinStartingSupplies;

	[Attribute("500", desc: "When randomized, the most supplies a base can hold at the start.", params: "0 inf 1", category: "Campaign")]
	protected int m_iMaxStartingSupplies;

	[Attribute("25", desc: "The step by which randomized supplies will be added in randomization. Min and Max limits should be divisible by this.", params: "1 inf 1", category: "Campaign")]
	protected int m_iStartingSuppliesInterval;

	[Attribute("US", category: "Campaign")]
	protected FactionKey m_sBLUFORFactionKey;

	[Attribute("USSR", category: "Campaign")]
	protected FactionKey m_sOPFORFactionKey;

	[Attribute("FIA", category: "Campaign")]
	protected FactionKey m_sINDFORFactionKey;

	[Attribute("1", UIWidgets.CheckBox, "Randomized starting supplies in small bases", category: "Campaign")]
	protected bool m_bRandomizeSupplies;

	[Attribute("1200", UIWidgets.EditBox, "The furthest an independent supply depot can be from the nearest base to still be visible in the map.", params: "0 inf 1", category: "Campaign")]
	protected int m_iSupplyDepotIconThreshold;

	[Attribute("{B3E7B8DC2BAB8ACC}Prefabs/AI/Waypoints/AIWaypoint_SearchAndDestroy.et", category: "Campaign")]
	protected ResourceName m_sSeekDestroyWaypointPrefab;

	[Attribute("1800", UIWidgets.EditBox, "If suicide is committed more than once in this time (seconds), a respawn penalty is issued.", params: "0 inf 1", category: "Campaign")]
	protected int m_iSuicidePenaltyCooldown;

	[Attribute("30", UIWidgets.EditBox, "Stacking extra deploy cooldown after suicide (seconds). Gets deducted over time.", params: "0 inf 1", category: "Campaign")]
	protected int m_iSuicideRespawnDelay;

	[Attribute("600", UIWidgets.EditBox, "How often is the post-suicide deploy cooldown penalty deducted (seconds).", params: "0 inf 1", category: "Campaign")]
	protected int m_iSuicideForgiveCooldown;

	static const int MINIMUM_DELAY = 100;
	static const int UI_UPDATE_DELAY = 250;
	static const int MEDIUM_DELAY = 1000;
	static const int DEFAULT_DELAY = 2000;
	static const int BACKEND_DELAY = 25000;

	protected ref ScriptInvoker m_OnFactionAssignedLocalPlayer;
	protected ref ScriptInvoker m_OnStarted;
	protected ref ScriptInvoker m_OnMatchSituationChanged;
	protected ref ScriptInvoker m_OnCallsignOffsetChanged;

	protected ref map<int, Faction> m_mUnprocessedFactionAssignments = new map<int, Faction>();

	protected ref array<SCR_PlayerRadioSpawnPointCampaign> m_aRadioSpawnPoints = {};
	protected ref array<ref SCR_CampaignClientData> m_aRegisteredClients = {};

	protected bool m_bIgnoreMinimumVehicleRank;
	protected bool m_bIsTutorial;
	protected bool m_bMatchOver;
	protected bool m_bWorldPostProcessDone;
	protected bool m_bRemnantsStateLoaded;
	protected bool m_bIsSessionLoadInProgress;

	protected ref SCR_CampaignMilitaryBaseManager m_BaseManager = new SCR_CampaignMilitaryBaseManager(this);

	protected SCR_CampaignStruct m_LoadedData;

	[RplProp(onRplName: "OnStarted")]
	protected bool m_bStarted;

	[RplProp(onRplName: "OnMatchSituationChanged")]
	protected WorldTimestamp m_fVictoryTimestamp;

	[RplProp(onRplName: "OnMatchSituationChanged")]
	protected WorldTimestamp m_fVictoryPauseTimestamp;

	[RplProp(onRplName: "OnMatchSituationChanged")]
	protected int m_iWinningFactionId = SCR_CampaignMilitaryBaseComponent.INVALID_FACTION_INDEX;

	[RplProp(onRplName: "OnCallsignOffsetChanged")]
	protected int m_iCallsignOffset = SCR_MilitaryBaseComponent.INVALID_BASE_CALLSIGN;

	//------------------------------------------------------------------------------------------------
	//! Triggered when the local player picks a faction
	ScriptInvoker GetOnFactionAssignedLocalPlayer()
	{
		if (!m_OnFactionAssignedLocalPlayer)
			m_OnFactionAssignedLocalPlayer = new ScriptInvoker();

		return m_OnFactionAssignedLocalPlayer;
	}

	//------------------------------------------------------------------------------------------------
	//! Triggered when Conflict gamemode has started
	ScriptInvoker GetOnStarted()
	{
		if (!m_OnStarted)
			m_OnStarted = new ScriptInvoker();

		return m_OnStarted;
	}

	//------------------------------------------------------------------------------------------------
	//! Triggered when an event happened which should be communicated to players (i.e. amount of control points held etc.)
	ScriptInvoker GetOnMatchSituationChanged()
	{
		if (!m_OnMatchSituationChanged)
			m_OnMatchSituationChanged = new ScriptInvoker();

		return m_OnMatchSituationChanged;
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnCallsignOffsetChanged()
	{
		if (!m_OnCallsignOffsetChanged)
			m_OnCallsignOffsetChanged = new ScriptInvoker();

		return m_OnCallsignOffsetChanged;
	}

	//------------------------------------------------------------------------------------------------
	int GetSuppliesReplenishThreshold()
	{
		return m_iSuppliesReplenishThreshold;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetQuickSuppliesReplenishThreshold()
	{
		return m_iQuickSuppliesReplenishThreshold;
	}
	
	//------------------------------------------------------------------------------------------------
	float GetQuickSuppliesReplenishMultiplier()
	{
		return m_fQuickSuppliesReplenishMultiplier;
	}

	//------------------------------------------------------------------------------------------------
	int GetSuppliesArrivalInterval()
	{
		return m_iSuppliesArrivalInterval;
	}

	//------------------------------------------------------------------------------------------------
	int GetRegularSuppliesIncome()
	{
		return m_iRegularSuppliesIncome;
	}

	//------------------------------------------------------------------------------------------------
	int GetRegularSuppliesIncomeBase()
	{
		return m_iRegularSuppliesIncomeBase;
	}

	//------------------------------------------------------------------------------------------------
	int GetRegularSuppliesIncomeExtra()
	{
		return m_iRegularSuppliesIncomeExtra;
	}

	//------------------------------------------------------------------------------------------------
	int GetSupplyDepotIconThreshold()
	{
		return m_iSupplyDepotIconThreshold;
	}

	//------------------------------------------------------------------------------------------------
	SCR_CampaignMilitaryBaseManager GetBaseManager()
	{
		return m_BaseManager;
	}

	//------------------------------------------------------------------------------------------------
	float GetVictoryTimer()
	{
		return m_fVictoryTimer;
	}

	//------------------------------------------------------------------------------------------------
	int GetControlPointTreshold()
	{
		return m_iControlPointsThreshold;
	}

	//------------------------------------------------------------------------------------------------
	int GetMinStartingSupplies()
	{
		return m_iMinStartingSupplies;
	}

	//------------------------------------------------------------------------------------------------
	int GetMaxStartingSupplies()
	{
		return m_iMaxStartingSupplies;
	}

	//------------------------------------------------------------------------------------------------
	int GetStartingSuppliesInterval()
	{
		return m_iStartingSuppliesInterval;
	}

	//------------------------------------------------------------------------------------------------
	bool WasRemnantsStateLoaded()
	{
		return m_bRemnantsStateLoaded;
	}

	//------------------------------------------------------------------------------------------------
	bool IsSessionLoadInProgress()
	{
		return m_bIsSessionLoadInProgress;
	}

	//------------------------------------------------------------------------------------------------
	ResourceName GetSeekDestroyWaypointPrefab()
	{
		return m_sSeekDestroyWaypointPrefab;
	}

	//------------------------------------------------------------------------------------------------
	int GetWinningFactionId()
	{
		return m_iWinningFactionId;
	}

	//------------------------------------------------------------------------------------------------
	WorldTimestamp GetVictoryTimestamp()
	{
		return m_fVictoryTimestamp;
	}

	//------------------------------------------------------------------------------------------------
	WorldTimestamp GetVictoryPauseTimestamp()
	{
		return m_fVictoryPauseTimestamp;
	}

	//------------------------------------------------------------------------------------------------
	bool IsTutorial()
	{
		return m_bIsTutorial;
	}

	//------------------------------------------------------------------------------------------------
	bool GetIsMatchOver()
	{
		return m_bMatchOver;
	}

	//------------------------------------------------------------------------------------------------
	bool HasStarted()
	{
		return m_bStarted;
	}

	//------------------------------------------------------------------------------------------------
	int GetCallsignOffset()
	{
		return m_iCallsignOffset;
	}

	//------------------------------------------------------------------------------------------------
	void OnMatchSituationChanged()
	{
		if (m_OnMatchSituationChanged)
			m_OnMatchSituationChanged.Invoke();
	}

	//------------------------------------------------------------------------------------------------
	override bool RplSave(ScriptBitWriter writer)
	{
		// Sync respawn radios & control points amount
		writer.WriteInt(m_BaseManager.GetTargetActiveBasesCount());

		int controlPointsHeldBLUFOR = GetFactionByEnum(SCR_ECampaignFaction.BLUFOR).GetControlPointsHeld();
		int controlPointsHeldOPFOR = GetFactionByEnum(SCR_ECampaignFaction.OPFOR).GetControlPointsHeld();

		RplId primaryTargetBLUFOR = Replication.FindId(GetFactionByEnum(SCR_ECampaignFaction.BLUFOR).GetPrimaryTarget());
		RplId primaryTargetOPFOR = Replication.FindId(GetFactionByEnum(SCR_ECampaignFaction.OPFOR).GetPrimaryTarget());

		writer.WriteInt(controlPointsHeldBLUFOR);
		writer.WriteInt(controlPointsHeldOPFOR);

		writer.WriteInt(primaryTargetBLUFOR);
		writer.WriteInt(primaryTargetOPFOR);

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool RplLoad(ScriptBitReader reader)
	{
		// Sync respawn radios & control points amount
		int activeBasesTotal;

		reader.ReadInt(activeBasesTotal);

		m_BaseManager.SetTargetActiveBasesCount(activeBasesTotal);

		if (m_BaseManager.GetActiveBasesCount() == activeBasesTotal)
			m_BaseManager.OnAllBasesInitialized();

		int controlPointsHeldBLUFOR, controlPointsHeldOPFOR, primaryTargetBLUFOR, primaryTargetOPFOR;

		reader.ReadInt(controlPointsHeldBLUFOR);
		reader.ReadInt(controlPointsHeldOPFOR);

		reader.ReadInt(primaryTargetBLUFOR);
		reader.ReadInt(primaryTargetOPFOR);

		GetFactionByEnum(SCR_ECampaignFaction.BLUFOR).SetControlPointsHeld(controlPointsHeldBLUFOR);
		GetFactionByEnum(SCR_ECampaignFaction.OPFOR).SetControlPointsHeld(controlPointsHeldOPFOR);

		GetFactionByEnum(SCR_ECampaignFaction.BLUFOR).SetPrimaryTarget(SCR_CampaignMilitaryBaseComponent.Cast(Replication.FindItem(primaryTargetBLUFOR)));
		GetFactionByEnum(SCR_ECampaignFaction.OPFOR).SetPrimaryTarget(SCR_CampaignMilitaryBaseComponent.Cast(Replication.FindItem(primaryTargetOPFOR)));

		return true;
	}

	//------------------------------------------------------------------------------------------------
	static SCR_GameModeCampaign GetInstance()
	{
		return SCR_GameModeCampaign.Cast(GetGame().GetGameMode());
	}

	//------------------------------------------------------------------------------------------------
	void SetControlPointsHeld(SCR_CampaignFaction faction, int newCount)
	{
		if (faction.GetControlPointsHeld() == newCount)
			return;

		int index = GetGame().GetFactionManager().GetFactionIndex(faction);

		Rpc(RPC_DoSetControlPointsHeld, index, newCount);
		RPC_DoSetControlPointsHeld(index, newCount)
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void RPC_DoSetControlPointsHeld(int factionIndex, int count)
	{
		SCR_CampaignFaction faction = SCR_CampaignFaction.Cast(GetGame().GetFactionManager().GetFactionByIndex(factionIndex));

		if (!faction)
			return;

		faction.SetControlPointsHeld(count);
		OnMatchSituationChanged();
	}

	//------------------------------------------------------------------------------------------------
	void SetPrimaryTarget(notnull SCR_CampaignFaction faction, SCR_CampaignMilitaryBaseComponent target)
	{
		if (faction.GetPrimaryTarget() == target)
			return;

		int index = GetGame().GetFactionManager().GetFactionIndex(faction);
		RplId targetId = RplId.Invalid();

		if (target)
			targetId = Replication.FindId(target);

		Rpc(RPC_DoSetPrimaryTarget, index, targetId);
		RPC_DoSetPrimaryTarget(index, targetId)
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void RPC_DoSetPrimaryTarget(int factionIndex, int targetId)
	{
		SCR_CampaignFaction faction = SCR_CampaignFaction.Cast(GetGame().GetFactionManager().GetFactionByIndex(factionIndex));

		if (!faction)
			return;

		faction.SetPrimaryTarget(SCR_CampaignMilitaryBaseComponent.Cast(Replication.FindItem(targetId)));
	}
	
	//------------------------------------------------------------------------------------------------
	void BroadcastMHQFeedback(SCR_EMobileAssemblyStatus msgID, int playerID, int factionID)
	{
		Rpc(RpcDo_BroadcastMHQFeedback, msgID, playerID, factionID);
		
		if (RplSession.Mode() != RplMode.Dedicated)
			RpcDo_BroadcastMHQFeedback(msgID, playerID, factionID);	
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RpcDo_BroadcastMHQFeedback(SCR_EMobileAssemblyStatus msgID, int playerID, int factionID)
	{
		SCR_CampaignFeedbackComponent comp = SCR_CampaignFeedbackComponent.GetInstance();
		
		if (!comp)
			return;
		
		comp.MobileAssemblyFeedback(msgID, playerID, factionID)
	}

	//------------------------------------------------------------------------------------------------
	void RequestStart()
	{
		if (IsProxy())
			return;

		// Start the gamemode after OnWorldPostprocess so bases have time to init properly
		if (m_bWorldPostProcessDone && !m_bStarted)
			Start();
	}

	//------------------------------------------------------------------------------------------------
	protected void Start()
	{
		// Compose custom bases array from header
		SCR_MilitaryBaseSystem baseManager = SCR_MilitaryBaseSystem.GetInstance();

		if (!baseManager)
			return;

		float customHQSupplies = m_iHQStartingSupplies;
		bool whitelist = false;
		array<string> customBaseList = {};

		SCR_MissionHeaderCampaign header = SCR_MissionHeaderCampaign.Cast(GetGame().GetMissionHeader());

		if (header)
		{
			whitelist = header.m_bCustomBaseWhitelist;
			customHQSupplies = header.m_iStartingHQSupplies;

			foreach (SCR_CampaignCustomBase customBase : header.m_aCampaignCustomBaseList)
			{
				customBaseList.Insert(customBase.GetBaseName());
			}
		}

		array<SCR_CampaignMilitaryBaseComponent> candidatesForHQ = {};
		array<SCR_CampaignMilitaryBaseComponent> controlPoints = {};
		array<SCR_MilitaryBaseComponent> bases = {};
		baseManager.GetBases(bases);

		string baseName;
		SCR_CampaignMilitaryBaseComponent campaignBase;
		int listIndex;

		foreach (SCR_MilitaryBaseComponent base : bases)
		{
			campaignBase = SCR_CampaignMilitaryBaseComponent.Cast(base);

			if (!campaignBase)
				continue;

			// Ignore the base if it's disabled in mission header
			if (header)
			{
				baseName = campaignBase.GetOwner().GetName();
				listIndex = customBaseList.Find(baseName);

				if (listIndex != -1)
				{
					if (!whitelist)
						continue;
				}
				else if (whitelist)
				{
					continue;
				}

				if (whitelist && listIndex != -1)
					campaignBase.ApplyHeaderSettings(header.m_aCampaignCustomBaseList[listIndex]);
			}

			if (!campaignBase.DisableWhenUnusedAsHQ() || !campaignBase.CanBeHQ())
			{
				campaignBase.Initialize();
				m_BaseManager.AddTargetActiveBase();
			}

			if (campaignBase.CanBeHQ())
				candidatesForHQ.Insert(campaignBase);

			if (campaignBase.IsControlPoint())
				controlPoints.Insert(campaignBase);
		}

		m_BaseManager.UpdateBases();

		if (candidatesForHQ.Count() < 2)
		{
			Print("Not enough suitable starting locations found in current setup. Check 'Can Be HQ' attributes in SCR_CampaignMilitaryBaseComponent!", LogLevel.ERROR);
			return;
		}

		// Process HQ selection
		array<SCR_CampaignMilitaryBaseComponent> selectedHQs = {};
		m_BaseManager.SelectHQs(candidatesForHQ, controlPoints, selectedHQs);
		m_BaseManager.SetHQFactions(selectedHQs);

		foreach (SCR_CampaignMilitaryBaseComponent hq : selectedHQs)
		{
			hq.SetAsHQ(true);

			if (customHQSupplies == -1)
				hq.SetStartingSupplies(m_iHQStartingSupplies);
			else
				hq.SetStartingSupplies(customHQSupplies);

			if (!hq.IsInitialized())
			{
				hq.Initialize();
				m_BaseManager.AddTargetActiveBase();
			}
		}

		m_BaseManager.InitializeBases(selectedHQs, m_bRandomizeSupplies);

		if (m_iCallsignOffset == SCR_MilitaryBaseComponent.INVALID_BASE_CALLSIGN)
		{
			int basesCount = m_BaseManager.GetTargetActiveBasesCount();

			Math.Randomize(-1);
			m_iCallsignOffset = Math.RandomIntInclusive(0, Math.Ceil(basesCount * 0.5));
		}

		Replication.BumpMe();

		array<SCR_SpawnPoint> spawnpoints = SCR_SpawnPoint.GetSpawnPoints();

		foreach (SCR_SpawnPoint spawnpoint : spawnpoints)
		{
			DisableExtraSpawnpoint(spawnpoint);
		}

		SCR_SpawnPoint.Event_SpawnPointAdded.Insert(DisableExtraSpawnpoint);

		// Start periodical checks for winning faction
		GetGame().GetCallqueue().CallLater(CheckForWinner, DEFAULT_DELAY, true);

		SCR_CharacterRankComponent.s_OnRankChanged.Insert(OnRankChanged);
		SCR_AmbientVehicleSystem vehiclesManager = SCR_AmbientVehicleSystem.GetInstance();
		
		if (vehiclesManager)
			vehiclesManager.GetOnVehicleSpawned().Insert(OnAmbientVehicleSpawned);

		m_bStarted = true;
		Replication.BumpMe();
		m_BaseManager.OnAllBasesInitialized();
		OnStarted();
	}

	//------------------------------------------------------------------------------------------------
	void OnStarted()
	{
		SCR_RadioCoverageSystem coverageSystem = SCR_RadioCoverageSystem.GetInstance();
		
		if (coverageSystem)
			coverageSystem.TogglePeriodicUpdates(false);
		
		SCR_SpawnPoint.Event_SpawnPointFactionAssigned.Insert(OnSpawnPointFactionAssigned);

		if (m_OnStarted)
			m_OnStarted.Invoke();
	}

	//------------------------------------------------------------------------------------------------
	void OnCallsignOffsetChanged()
	{
		if (m_OnCallsignOffsetChanged)
			m_OnCallsignOffsetChanged.Invoke();
	}

	//------------------------------------------------------------------------------------------------
	protected void DisableExtraSpawnpoint(SCR_SpawnPoint spawnpoint)
	{
		if (spawnpoint.Type() != SCR_SpawnPoint)
			return;

		spawnpoint.SetFaction(null);
	}

	//------------------------------------------------------------------------------------------------
	//! Find out if any faction has won and it's time to end the match
	protected void CheckForWinner()
	{
		FactionManager factionManager = GetGame().GetFactionManager();
		array<Faction> factions = {};
		factionManager.GetFactionsList(factions);
		ChimeraWorld world = GetWorld();
		WorldTimestamp curTime = world.GetServerTimestamp();
		WorldTimestamp lowestVictoryTimestamp;
		WorldTimestamp blockPauseTimestamp;
		WorldTimestamp actualVictoryTimestamp;
		SCR_CampaignFaction winner;

		foreach (Faction faction : factions)
		{
			SCR_CampaignFaction fCast = SCR_CampaignFaction.Cast(faction);

			if (!fCast || !fCast.IsPlayable())
				continue;

			blockPauseTimestamp = fCast.GetPauseByBlockTimestamp();

			if (blockPauseTimestamp == 0)
				actualVictoryTimestamp = fCast.GetVictoryTimestamp();
			else
				actualVictoryTimestamp = curTime.PlusMilliseconds(
					fCast.GetVictoryTimestamp().DiffMilliseconds(fCast.GetPauseByBlockTimestamp())
				);

			if (actualVictoryTimestamp != 0)
			{
				if (!winner || actualVictoryTimestamp.Less(lowestVictoryTimestamp))
				{
					lowestVictoryTimestamp = actualVictoryTimestamp;
					winner = fCast;
				}
			}
		}

		if (winner)
		{
			if (lowestVictoryTimestamp.LessEqual(curTime))
			{
				GetGame().GetCallqueue().Remove(CheckForWinner);
				int winnerId = factionManager.GetFactionIndex(winner);
				RPC_DoEndMatch(winnerId);
				Rpc(RPC_DoEndMatch, winnerId);
				OnMatchSituationChanged();
			}
			else if (factionManager.GetFactionIndex(winner) != m_iWinningFactionId || winner.GetVictoryTimestamp() != m_fVictoryTimestamp || winner.GetPauseByBlockTimestamp() != m_fVictoryPauseTimestamp)
			{
				m_iWinningFactionId = factionManager.GetFactionIndex(winner);
				m_fVictoryTimestamp = winner.GetVictoryTimestamp();
				m_fVictoryPauseTimestamp = winner.GetPauseByBlockTimestamp();
				OnMatchSituationChanged();
				Replication.BumpMe();
			}
		}
		else if (m_iWinningFactionId != -1 || m_fVictoryTimestamp != 0)
		{
			m_iWinningFactionId = -1;
			m_fVictoryTimestamp = null;
			m_fVictoryPauseTimestamp = null;
			OnMatchSituationChanged();
			Replication.BumpMe();
		}
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void RPC_DoEndMatch(int winningFactionId)
	{
		m_bMatchOver = true;

		if (IsProxy())
			return;

		FactionManager fManager = GetGame().GetFactionManager();
		array<Faction> factions = {};
		fManager.GetFactionsList(factions);
		Faction winningFaction = fManager.GetFactionByIndex(winningFactionId);

		if (winningFaction)
		{
			foreach (Faction faction : factions)
			{
				SCR_CampaignFaction f = SCR_CampaignFaction.Cast(faction);

				if (!f)
					continue;

				if (winningFaction == f)
					f.SendHQMessage(SCR_ERadioMsg.VICTORY, param: winningFactionId);
				else
					f.SendHQMessage(SCR_ERadioMsg.DEFEAT, param: winningFactionId);
			}
		}

		// Match is over, save it so if "Continue" is selected following this the game is not loaded at an end screen
		GetGame().GetSaveManager().Save(ESaveType.AUTO);

		// For the server end the game, replicate to all clients.
		// listening components can react to this by e.g. showing end screen
		if (m_bTerminateScenario)
		{
			SCR_GameModeEndData endData = SCR_GameModeEndData.CreateSimple(EGameOverTypes.ENDREASON_SCORELIMIT, winnerFactionId: winningFactionId);
			EndGameMode(endData);
		}
	}
	//------------------------------------------------------------------------------------------------
	override void OnWorldPostProcess(World world)
	{
		super.OnWorldPostProcess(world);

		if (IsProxy() || !GetGame().InPlayMode())
			return;

		if (m_bAutostart)
			Start();

		m_bWorldPostProcessDone = true;
	}

	//------------------------------------------------------------------------------------------------
	FactionKey GetFactionKeyByEnum(SCR_ECampaignFaction faction)
	{
		switch (faction)
		{
			case SCR_ECampaignFaction.INDFOR:
			{
				return m_sINDFORFactionKey;
			};

			case SCR_ECampaignFaction.BLUFOR:
			{
				return m_sBLUFORFactionKey;
			};

			case SCR_ECampaignFaction.OPFOR:
			{
				return m_sOPFORFactionKey;
			};
		}

		return FactionKey.Empty;
	}

	//------------------------------------------------------------------------------------------------
	SCR_CampaignFaction GetFactionByEnum(SCR_ECampaignFaction faction)
	{
		FactionManager fManager = GetGame().GetFactionManager();

		if (!fManager)
			return null;

		return SCR_CampaignFaction.Cast(fManager.GetFactionByKey(GetFactionKeyByEnum(faction)));
	}

	//------------------------------------------------------------------------------------------------
	bool IsProxy()
	{
		return (m_RplComponent && m_RplComponent.IsProxy());
	}

	//------------------------------------------------------------------------------------------------
	//! Getter for "Rank required" parameter for spawning vehicles.
	// TRUE, if rank requirement is disabled
	bool CanRequestVehicleWithoutRank()
	{
		return m_bIgnoreMinimumVehicleRank;
	}

	//------------------------------------------------------------------------------------------------
	//! Handle the loaded struct; we want to apply it after gamemode has initialized
	void StoreLoadedData(notnull SCR_CampaignStruct struct)
	{
		m_LoadedData = struct;

		if (m_BaseManager.IsBasesInitDone())
			ApplyLoadedData();
		else
			m_BaseManager.GetOnAllBasesInitialized().Insert(ApplyLoadedData);
	}

	//------------------------------------------------------------------------------------------------
	protected void ApplyLoadedData()
	{
		m_BaseManager.GetOnAllBasesInitialized().Remove(ApplyLoadedData);

		if (!m_LoadedData)
			return;

		// Game was saved after match was over, don't load
		if (m_LoadedData.IsMatchOver())
			return;

		array<ref SCR_CampaignBaseStruct>basesStructs = m_LoadedData.GetBasesStructs();

		// No bases data available for load, something is wrong - terminate
		if (basesStructs.IsEmpty())
			return;

		m_bIsSessionLoadInProgress = true;
		m_BaseManager.LoadBasesStates(basesStructs);

		// We need to wait for all services to spawn before switching the progress bool to false so supplies are not deducted from bases
		GetGame().GetCallqueue().CallLater(EndSessionLoadProgress, DEFAULT_DELAY * 2);

		if (RplSession.Mode() != RplMode.Dedicated)
		{
			m_BaseManager.InitializeSupplyDepotIcons();
			m_BaseManager.HideUnusedBaseIcons();
		}

		m_BaseManager.RecalculateRadioCoverage(GetFactionByEnum(SCR_ECampaignFaction.BLUFOR));
		m_BaseManager.RecalculateRadioCoverage(GetFactionByEnum(SCR_ECampaignFaction.OPFOR));

		SCR_TimeAndWeatherHandlerComponent timeHandler = SCR_TimeAndWeatherHandlerComponent.GetInstance();

		// Weather has to be changed after init
		if (timeHandler)
		{
			GetGame().GetCallqueue().Remove(timeHandler.SetupDaytimeAndWeather);
			GetGame().GetCallqueue().CallLater(timeHandler.SetupDaytimeAndWeather, DEFAULT_DELAY, false, m_LoadedData.GetHours(), m_LoadedData.GetMinutes(), m_LoadedData.GetSeconds(), m_LoadedData.GetWeatherState(), true);
		}

		m_iCallsignOffset = m_LoadedData.GetCallsignOffset();
		Replication.BumpMe();

		LoadRemnantsStates(m_LoadedData.GetRemnantsStructs());
		LoadClientData(m_LoadedData.GetPlayersStructs());

		SCR_CampaignFaction factionBLUFOR = GetFactionByEnum(SCR_ECampaignFaction.BLUFOR);
		SCR_CampaignFaction factionOPFOR = GetFactionByEnum(SCR_ECampaignFaction.OPFOR);

		// Delayed spawns to avoid calling them during init
		if (factionBLUFOR && m_LoadedData.GetMHQLocationBLUFOR() != vector.Zero)
			GetGame().GetCallqueue().CallLater(SpawnMobileHQ, DEFAULT_DELAY, false, factionBLUFOR, m_LoadedData.GetMHQLocationBLUFOR(), m_LoadedData.GetMHQRotationBLUFOR());

		if (factionOPFOR && m_LoadedData.GetMHQLocationOPFOR() != vector.Zero)
			GetGame().GetCallqueue().CallLater(SpawnMobileHQ, DEFAULT_DELAY, false, factionOPFOR, m_LoadedData.GetMHQLocationOPFOR(), m_LoadedData.GetMHQRotationOPFOR());

		m_LoadedData = null;
	}

	//------------------------------------------------------------------------------------------------
	protected void EndSessionLoadProgress()
	{
		m_bIsSessionLoadInProgress = false;
	}

	//------------------------------------------------------------------------------------------------
	void StoreRemnantsStates(out notnull array<ref SCR_CampaignRemnantInfoStruct> outEntries)
	{
		SCR_AmbientPatrolSystem manager = SCR_AmbientPatrolSystem.GetInstance();

		if (!manager)
			return;

		array<int> remnantsInfo = {};

		for (int i = 0, count = manager.GetRemainingPatrolsInfo(remnantsInfo); i < count; i++)
		{
			SCR_CampaignRemnantInfoStruct struct = new SCR_CampaignRemnantInfoStruct();
			struct.SetID(remnantsInfo[i]);
			struct.SetMembersAlive(remnantsInfo[i + 1]);
			struct.SetRespawnTimer(remnantsInfo[i + 2]);
			outEntries.Insert(struct);
			i += 2;
		}
	}

	//------------------------------------------------------------------------------------------------
	void LoadRemnantsStates(notnull array<ref SCR_CampaignRemnantInfoStruct> entries)
	{
		SCR_AmbientPatrolSystem manager = SCR_AmbientPatrolSystem.GetInstance();
		
		if (!manager)
			return;
		
		array<SCR_AmbientPatrolSpawnPointComponent> patrols = {};
		manager.GetPatrols(patrols);
		ChimeraWorld world = GetWorld();
		WorldTimestamp curTime = world.GetServerTimestamp();

		foreach (SCR_AmbientPatrolSpawnPointComponent presence : patrols)
		{
			if (!presence)
				continue;

			foreach (SCR_CampaignRemnantInfoStruct info : entries)
			{
				if (info.GetID() == presence.GetID())
				{
					presence.SetMembersAlive(info.GetMembersAlive());
					presence.SetRespawnTimestamp(curTime.PlusMilliseconds(info.GetRespawnTimer()));
				}
			}
		}

		m_bRemnantsStateLoaded = true;
	}

	//------------------------------------------------------------------------------------------------
	void SetIsTutorial(bool isTutorial)
	{
		m_bIsTutorial = isTutorial;

		if (m_bIsTutorial)
			SCR_PopUpNotification.SetFilter(SCR_EPopupMsgFilter.TUTORIAL);
		else
			SCR_PopUpNotification.SetFilter(SCR_EPopupMsgFilter.ALL);
	}

	//------------------------------------------------------------------------------------------------
	protected void ApplyClientData(int playerId)
	{
		PlayerController pc = GetGame().GetPlayerManager().GetPlayerController(playerId);

		if (!pc)
			return;

		SCR_CampaignClientData clientData = GetClientData(playerId);

		if (!clientData)
			return;

		bool allowFactionLoad = true;

#ifdef ENABLE_DIAG
		if (SCR_RespawnComponent.Diag_IsCLISpawnEnabled())
			allowFactionLoad = false;
#endif

		// Automatically apply the client's previous faction
		int forcedFaction = clientData.GetFactionIndex();

		if (allowFactionLoad && forcedFaction != -1)
		{
			SCR_PlayerFactionAffiliationComponent fac = SCR_PlayerFactionAffiliationComponent.Cast(pc.FindComponent(SCR_PlayerFactionAffiliationComponent));

			if (fac)
			{
				Faction faction = GetGame().GetFactionManager().GetFactionByIndex(forcedFaction);
				fac.RequestFaction(faction);
			}
		}

		int xp;
		SCR_PlayerXPHandlerComponent handlerXP = SCR_PlayerXPHandlerComponent.Cast(pc.FindComponent(SCR_PlayerXPHandlerComponent));

		if (handlerXP)
			xp = handlerXP.GetPlayerXP();

		SCR_XPHandlerComponent comp = SCR_XPHandlerComponent.Cast(FindComponent(SCR_XPHandlerComponent));

		if (comp)
			comp.AwardXP(playerId, SCR_EXPRewards.UNDEFINED, 1, false, clientData.GetXP() - xp);

		SCR_FastTravelComponent fastTravel = SCR_FastTravelComponent.Cast(pc.FindComponent(SCR_FastTravelComponent));

		if (fastTravel)
			fastTravel.SetNextTransportTimestamp(clientData.GetNextFastTravelTimestamp());
	}

	//------------------------------------------------------------------------------------------------
	void LoadClientData(notnull array<ref SCR_CampaignPlayerStruct> data)
	{
		m_aRegisteredClients.Clear();

		foreach (SCR_CampaignPlayerStruct playerData : data)
		{
			SCR_CampaignClientData clientData = new SCR_CampaignClientData();

			clientData.SetID(playerData.GetID());
			clientData.SetXP(playerData.GetXP());
			clientData.SetFactionIndex(playerData.GetFactionIndex());

			m_aRegisteredClients.Insert(clientData);
		}
	}

	//------------------------------------------------------------------------------------------------
	void WriteAllClientsData()
	{
		array<int> pcList = {};

		for (int i = 0, playersCount = GetGame().GetPlayerManager().GetPlayers(pcList); i < playersCount; i++)
		{
			PlayerController pc = GetGame().GetPlayerManager().GetPlayerController(pcList[i]);

			if (!pc)
				continue;

			int ID = pc.GetPlayerId();
			WriteClientData(ID, pc: pc);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Save object with player's current data
	protected void WriteClientData(int playerID, bool disconnecting = false, PlayerController pc = null)
	{
		SCR_CampaignClientData clientData = GetClientData(playerID);

		if (!clientData)
			return;

		if (!pc)
			pc = GetGame().GetPlayerManager().GetPlayerController(playerID);

		if (!pc)
			return;

		SCR_PlayerXPHandlerComponent comp = SCR_PlayerXPHandlerComponent.Cast(pc.FindComponent(SCR_PlayerXPHandlerComponent));

		if (!comp)
			return;

		// Set data readable from PlayerController
		clientData.SetXP(comp.GetPlayerXP());
	}

	//------------------------------------------------------------------------------------------------
	void SpawnMobileHQ(notnull SCR_CampaignFaction faction, vector pos, vector rot)
	{
		if (faction.GetMobileAssembly())
			return;

		EntitySpawnParams params = EntitySpawnParams();
		GetWorldTransform(params.Transform);
		params.TransformMode = ETransformMode.WORLD;
		Math3D.AnglesToMatrix(rot, params.Transform);
		params.Transform[3] = pos;

		IEntity MHQ = GetGame().SpawnEntityPrefab(Resource.Load(faction.GetMobileHQPrefab()), null, params);

		if (!MHQ)
			return;

		SlotManagerComponent slotManager = SlotManagerComponent.Cast(MHQ.FindComponent(SlotManagerComponent));

		if (!slotManager)
			return;

		array<EntitySlotInfo> slots = {};
		slotManager.GetSlotInfos(slots);

		foreach (EntitySlotInfo slot : slots)
		{
			IEntity truckBed = slot.GetAttachedEntity();

			if (!truckBed)
				continue;

			SCR_CampaignMobileAssemblyComponent mobileAssemblyComponent = SCR_CampaignMobileAssemblyComponent.Cast(truckBed.FindComponent(SCR_CampaignMobileAssemblyComponent));

			if (mobileAssemblyComponent)
			{
				mobileAssemblyComponent.SetParentFactionID(GetGame().GetFactionManager().GetFactionIndex(faction));
				mobileAssemblyComponent.UpdateRadioCoverage();
				mobileAssemblyComponent.Deploy(SCR_EMobileAssemblyStatus.DEPLOYED);
				break;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	int GetClientsData(notnull out array<ref SCR_CampaignClientData> dataArray)
	{
		int count;

		foreach (SCR_CampaignClientData data : m_aRegisteredClients)
		{
			count++;
			dataArray.Insert(data);
		}

		return count;
	}

	//------------------------------------------------------------------------------------------------
	//! Get corresponding client data, create new object if not found
	SCR_CampaignClientData GetClientData(int playerId)
	{
		if (playerId == 0)
			return null;

		string playerIdentity = SCR_CampaignPlayerStruct.GetPlayerIdentity(playerId);

		if (playerIdentity == string.Empty)
			return null;

		SCR_CampaignClientData clientData;

		// Check if the client is reconnecting
		for (int i = 0, clientsCount = m_aRegisteredClients.Count(); i < clientsCount; i++)
		{
			if (m_aRegisteredClients[i].GetID() == playerIdentity)
			{
				clientData = m_aRegisteredClients[i];
				break;
			}
		}

		if (!clientData)
		{
			clientData = new SCR_CampaignClientData;
			clientData.SetID(playerIdentity);
			m_aRegisteredClients.Insert(clientData);
		}

		return clientData;
	}

	//------------------------------------------------------------------------------------------------
	protected void OnAmbientVehicleSpawned(SCR_AmbientVehicleSpawnPointComponent spawnpoint, Vehicle vehicle)
	{
		SCR_HelicopterDamageManagerComponent helicopterDamageManager = SCR_HelicopterDamageManagerComponent.Cast(vehicle.FindComponent(SCR_HelicopterDamageManagerComponent));

		// Ignore non-helicopter vehicles
		if (!helicopterDamageManager)
			return;

		array<HitZone> hitZones = {};
		helicopterDamageManager.GetAllHitZonesInHierarchy(hitZones);
		vector transform[3];
		transform[0] = vehicle.GetOrigin();
		transform[1] = vector.Forward;
		transform[2] = vector.Up;

		DamageManagerComponent damageManager;

		// Damage the engine and hull
		foreach (HitZone hitZone : hitZones)
		{
			if (!hitZone.IsInherited(SCR_EngineHitZone) && !hitZone.IsInherited(SCR_FlammableHitZone))
				continue;

			damageManager = DamageManagerComponent.Cast(hitZone.GetHitZoneContainer());
			
			SCR_DamageContext damageContext = new SCR_DamageContext(EDamageType.TRUE, hitZone.GetMaxHealth() * 0.75, transform, damageManager.GetOwner(), hitZone, Instigator.CreateInstigator(null), null, -1, -1);
			if (damageManager)
				helicopterDamageManager.HandleDamage(damageContext);
		}

		array<SCR_FuelManagerComponent> fuelManagers = {};
		array<BaseFuelNode> fuelNodes = {};
		SCR_FuelManagerComponent.GetAllFuelManagers(vehicle, fuelManagers);

		// Remove all fuel
		foreach (SCR_FuelManagerComponent fuelManager : fuelManagers)
		{
			fuelNodes.Clear();
			fuelManager.GetFuelNodesList(fuelNodes);

			foreach (BaseFuelNode fuelNode : fuelNodes)
			{
				fuelNode.SetFuel(0.0);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void OnRankChanged(SCR_ECharacterRank oldRank, SCR_ECharacterRank newRank, notnull IEntity owner, bool silent)
	{
		if (silent)
			return;

		int playerId = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(owner);
		SCR_CampaignFaction faction = SCR_CampaignFaction.Cast(SCR_FactionManager.SGetPlayerFaction(playerId));

		if (!faction)
			return;

		SCR_FactionManager factionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());

		if (!factionManager)
			return;

		SCR_RankIDCampaign rank = SCR_RankIDCampaign.Cast(factionManager.GetRankByID(newRank));

		if (!rank)
			return;

		SCR_ERadioMsg radio;

		if (newRank < oldRank && !rank.IsRankRenegade())
			radio = SCR_ERadioMsg.DEMOTION;
		else
			radio = rank.GetRadioMsg();

		faction.SendHQMessage(radio, calledID: playerId, public: false, param: newRank)
	}

	//------------------------------------------------------------------------------------------------
	override void OnPlayerRegistered(int playerId)
	{
		SCR_PlayerController playerController = SCR_PlayerController.Cast(GetGame().GetPlayerManager().GetPlayerController(playerId));

		if (playerController)
		{
			SCR_PlayerFactionAffiliationComponent playerFactionAff = SCR_PlayerFactionAffiliationComponent.Cast(playerController.FindComponent(SCR_PlayerFactionAffiliationComponent));

			if (playerFactionAff)
			{
				playerFactionAff.GetOnPlayerFactionResponseInvoker_O().Insert(OnPlayerFactionResponse_O);
				playerFactionAff.GetOnPlayerFactionResponseInvoker_S().Insert(OnPlayerFactionResponse_S);
			}
		}

		super.OnPlayerRegistered(playerId);

		if (!playerController)
			return;

		// Normally this is done in OnPlayerAuditSuccess, but in SP the callback is not triggered
		if (RplSession.Mode() == RplMode.None && !m_bIsTutorial)
			ApplyClientData(playerId);

		// See HandleOnFactionAssigned()
		if (SCR_PlayerController.GetLocalPlayerId() == 0)
			return;

		int key;

		for (int i = 0, count = m_mUnprocessedFactionAssignments.Count(); i < count; i++)
		{
			key = m_mUnprocessedFactionAssignments.GetKey(i);

			if (key == SCR_PlayerController.GetLocalPlayerId())
			{
				ProcessFactionAssignment(m_mUnprocessedFactionAssignments.Get(key));
				m_mUnprocessedFactionAssignments.Remove(key);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	override void OnPlayerAuditSuccess(int iPlayerID)
	{
		super.OnPlayerAuditSuccess(iPlayerID);

		// Apply data with a delay so client's game has time to initialize and register faction setting
		if (RplSession.Mode() != RplMode.None)
			GetGame().GetCallqueue().CallLater(ApplyClientData, MINIMUM_DELAY, false, iPlayerID);
	}

	//------------------------------------------------------------------------------------------------
	override void OnPlayerDisconnected(int playerId, KickCauseCode cause, int timeout)
	{
		super.OnPlayerDisconnected(playerId, cause, timeout);

		GetTaskManager().OnPlayerDisconnected(playerId);
		WriteClientData(playerId, true);
		m_BaseManager.OnPlayerDisconnected(playerId)
	}

	//------------------------------------------------------------------------------------------------
	override void OnPlayerSpawnFinalize_S(SCR_SpawnRequestComponent requestComponent, SCR_SpawnHandlerComponent handlerComponent, SCR_SpawnData data, IEntity entity)
	{
		super.OnPlayerSpawnFinalize_S(requestComponent, handlerComponent, data, entity);

		PlayerController pc = requestComponent.GetPlayerController();

		if (!pc)
			return;

		SCR_CampaignNetworkComponent campaignNetworkComponent = SCR_CampaignNetworkComponent.Cast(pc.FindComponent(SCR_CampaignNetworkComponent));

		if (campaignNetworkComponent)
			campaignNetworkComponent.OnPlayerAliveStateChanged(true);
	}

	//------------------------------------------------------------------------------------------------
	override void OnPlayerSpawnOnPoint_S(SCR_SpawnRequestComponent requestComponent, SCR_SpawnHandlerComponent handlerComponent, IEntity entity, SCR_SpawnPointSpawnData spawnPointData)
	{
		super.OnPlayerSpawnOnPoint_S(requestComponent, handlerComponent, entity, spawnPointData);

		// Award XP for the owner of the respawn radio (if applicable)
		SCR_XPHandlerComponent compXP = SCR_XPHandlerComponent.Cast(FindComponent(SCR_XPHandlerComponent));

		if (compXP)
		{
			SCR_SpawnPoint spawnpoint = spawnPointData.GetSpawnPoint();

			if (spawnpoint)
			{
				SCR_PlayerSpawnPoint playerSpawnpoint = SCR_PlayerSpawnPoint.Cast(spawnpoint);
				SCR_DeployableSpawnPoint radioSpawnpoint = SCR_DeployableSpawnPoint.Cast(spawnpoint);

				if (playerSpawnpoint)
				{
					compXP.AwardXP(playerSpawnpoint.GetPlayerID(), SCR_EXPRewards.SPAWN_PROVIDER);
				}
				else if (radioSpawnpoint)
				{
					SCR_BaseDeployableSpawnPointComponent comp = radioSpawnpoint.GetDeployableSpawnPointComponent();
				
					if (comp)
					{
						int playerId = comp.GetItemOwnerID();
						
						// Don't award XP if player respawns on their own spawnpoint
						if (playerId != GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(entity))
							compXP.AwardXP(playerId, SCR_EXPRewards.SPAWN_PROVIDER);
					}
				}
			}
		}

		// Location popup for player
		PlayerController playerController = GetGame().GetPlayerManager().GetPlayerController(requestComponent.GetPlayerId());

		if (playerController)
		{
			SCR_CampaignMilitaryBaseComponent spawnPointParentBase;
			IEntity parent = spawnPointData.GetSpawnPoint();
			
			//~ Check if spawn target is a base
			while (parent)
			{
				spawnPointParentBase = SCR_CampaignMilitaryBaseComponent.Cast(parent.FindComponent(SCR_CampaignMilitaryBaseComponent));
	
				if (spawnPointParentBase)
					break;
	
				parent = parent.GetParent();
			}
			
			//~ If spawned on base
			if (spawnPointParentBase)
			{
				SCR_CampaignNetworkComponent campaignNetworkComponent = SCR_CampaignNetworkComponent.Cast(playerController.FindComponent(SCR_CampaignNetworkComponent));

				if (campaignNetworkComponent)
					campaignNetworkComponent.RespawnLocationPopup(spawnPointParentBase.GetCallsign());
			}
		}
		
	}

	//------------------------------------------------------------------------------------------------
	void OnSpawnPointFactionAssigned(SCR_SpawnPoint spawnpoint)
	{
		IEntity owner = spawnpoint.GetParent();
		if (!owner)
			return;

		SCR_CampaignMilitaryBaseComponent parentBase = SCR_CampaignMilitaryBaseComponent.Cast(owner.FindComponent(SCR_CampaignMilitaryBaseComponent));
		if (parentBase)
			parentBase.OnSpawnPointFactionAssigned(spawnpoint.GetFactionKey());
	}

	//------------------------------------------------------------------------------------------------
	override void OnPlayerKilledEx(notnull SCR_InstigatorContextData instigatorContextData)
	{
		super.OnPlayerKilledEx(instigatorContextData);

		if (IsProxy())
			return;

		int playerId = instigatorContextData.GetVictimPlayerID();
		PlayerController pc = GetGame().GetPlayerManager().GetPlayerController(playerId);

		if (!pc)
			return;

		SCR_CampaignNetworkComponent campaignNetworkComponent = SCR_CampaignNetworkComponent.Cast(pc.FindComponent(SCR_CampaignNetworkComponent));
		
		if (campaignNetworkComponent)
		{
			campaignNetworkComponent.OnPlayerAliveStateChanged(false);
			campaignNetworkComponent.ResetSavedSupplies();
		}

		UpdateRespawnPenalty(playerId);

		if (instigatorContextData.HasAnyVictimKillerRelation(SCR_ECharacterDeathStatusRelations.SUICIDE))
			OnSuicide(playerId);
	}

	//------------------------------------------------------------------------------------------------
	//! Handles forgiving of post-suicide deploy timer penalties
	protected void UpdateRespawnPenalty(int playerId)
	{
		if (m_iSuicideRespawnDelay == 0 || m_iSuicideForgiveCooldown == 0)
			return;

		SCR_CampaignClientData clientData = GetClientData(playerId);

		if (!clientData)
			return;

		float respawnPenalty = clientData.GetRespawnPenalty();

		if (respawnPenalty == 0)
			return;

		float curTime = GetGame().GetWorld().GetWorldTime();
		float penaltyCooldownMs = (float)m_iSuicideForgiveCooldown * 1000;
		float timeSinceLastDeduction = curTime - clientData.GetLastPenaltyDeductionTimestamp();
		float penaltiesForgiven = Math.Floor(timeSinceLastDeduction / penaltyCooldownMs);

		if (penaltiesForgiven < 1)
			return;

		clientData.SetLastPenaltyDeductionTimestamp(curTime);
		float forgivenPenalty = (float)m_iSuicideRespawnDelay * penaltiesForgiven;
		clientData.SetRespawnPenalty(respawnPenalty - forgivenPenalty);

		array<Managed> timers = {};
		FindComponents(SCR_RespawnTimerComponent, timers);

		foreach (Managed timer : timers)
		{
			// Skip this specific type as it's handled separately for radio spawns
			if (timer.Type() == SCR_TimedSpawnPointComponent)
				continue;

			SCR_RespawnTimerComponent timerCast = SCR_RespawnTimerComponent.Cast(timer);

			if (!timerCast)
				continue;

			timerCast.SetRespawnTime(playerId, timerCast.GetRespawnTime() - respawnPenalty - forgivenPenalty);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void OnSuicide(int playerId)
	{
		// Don't issue penalties in WB so it doesn't interfere with debugging etc.
#ifdef WORKBENCH
		return;
#endif
#ifdef NO_SUICIDE_PENALTY
		return;
#endif
		if (m_bIsTutorial)
			return;

		if (m_iSuicideRespawnDelay == 0)
			return;
		
		PlayerController pc = GetGame().GetPlayerManager().GetPlayerController(playerId);

		// Do not process suicide if player was unconscious upon death
		if (pc)
		{
			SCR_CampaignFeedbackComponent comp = SCR_CampaignFeedbackComponent.Cast(pc.FindComponent(SCR_CampaignFeedbackComponent));
			
			if (comp && !comp.IsConscious())
				return;
		}

		SCR_CampaignClientData clientData = GetClientData(playerId);

		if (!clientData)
			return;

		float respawnPenalty = clientData.GetRespawnPenalty();
		float lastSuicideTimestamp = clientData.GetLastSuicideTimestamp();
		float curTime = GetGame().GetWorld().GetWorldTime();
		clientData.SetLastSuicideTimestamp(curTime);

		if (lastSuicideTimestamp == 0)
			return;

		float timeSinceLastSuicide = curTime - lastSuicideTimestamp;
		float penaltyCooldownMs = (float)m_iSuicidePenaltyCooldown * 1000;

		// Last suicide happened long enough time ago, don't issue a penalty
		if (timeSinceLastSuicide > penaltyCooldownMs)
			return;

		float addedPenalty = m_iSuicideRespawnDelay;

		clientData.SetLastPenaltyDeductionTimestamp(curTime);
		clientData.SetRespawnPenalty(respawnPenalty + addedPenalty);

		array<Managed> timers = {};
		FindComponents(SCR_RespawnTimerComponent, timers);

		foreach (Managed timer : timers)
		{
			// Skip this specific type as it's handled separately for radio spawns
			if (timer.Type() == SCR_TimedSpawnPointComponent)
				continue;

			SCR_RespawnTimerComponent timerCast = SCR_RespawnTimerComponent.Cast(timer);

			if (!timerCast)
				continue;

			timerCast.SetRespawnTime(playerId, timerCast.GetRespawnTime() + respawnPenalty + addedPenalty);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Disable spawning with custom loadout on bases with no armories
	/*!
		Authority:
			Override and implement logic for whether provided player can spawn.
			\param requestComponent The player request component instigating this spawn.
			\param handlerComponent The spawn handler component handling this spawn.
			\param data The request payload.
			\param[out] result Reason why respawn is disabled. Note that if returns true the reason will always be OK
			\return True when spawn is allowed, false otherwise. 
	*/
	override bool CanPlayerSpawn_S(SCR_SpawnRequestComponent requestComponent, SCR_SpawnHandlerComponent handlerComponent, SCR_SpawnData data, out SCR_ESpawnResult result = SCR_ESpawnResult.SPAWN_NOT_ALLOWED)
	{
		if (!super.CanPlayerSpawn_S(requestComponent, handlerComponent, data, result))
			return false;
		
		SCR_SpawnPointSpawnData spawnpointSpawnData = SCR_SpawnPointSpawnData.Cast(data);
		
		if (!spawnpointSpawnData)
			return true;

		SCR_PlayerLoadoutComponent loadoutComp = SCR_PlayerLoadoutComponent.Cast(requestComponent.GetPlayerController().FindComponent(SCR_PlayerLoadoutComponent));

		if (!loadoutComp)
			return true;
		
		SCR_PlayerArsenalLoadout loadout = SCR_PlayerArsenalLoadout.Cast(loadoutComp.GetLoadout());
		
		if (!loadout)
			return true;
		
		SCR_SpawnPoint spawnpoint = spawnpointSpawnData.GetSpawnPoint();
		
		if (!spawnpoint)
			return true;
		
		// Spawning on MHQ with custom loadouts is not allowed
		if (spawnpoint.FindComponent(SCR_CampaignMobileAssemblyStandaloneComponent))
		{
			result = SCR_ESpawnResult.NOT_ALLOWED_CUSTOM_LOADOUT;
			return false;
		}
		
		SCR_CampaignSpawnPointGroup spawnpointCampaign = SCR_CampaignSpawnPointGroup.Cast(spawnpoint);
		
		if (!spawnpointCampaign)
			return true;
		
		IEntity spawnpointParent = spawnpointCampaign.GetParent();
		
		if (!spawnpointParent)
			return true;
		
		SCR_CampaignMilitaryBaseComponent base = SCR_CampaignMilitaryBaseComponent.Cast(spawnpointParent.FindComponent(SCR_CampaignMilitaryBaseComponent));
		
		if (!base)
			return true;
		
		SCR_ServicePointComponent armory = base.GetServiceByType(SCR_EServicePointType.ARMORY);
		
		if (armory)
			return true;

		result = SCR_ESpawnResult.NOT_ALLOWED_NO_ARSENAL;
		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Award additional XP for enemies killed in friendly bases
	override void OnControllableDestroyedEx(notnull SCR_InstigatorContextData instigatorContextData)
	{
		super.OnControllableDestroyedEx(instigatorContextData);

		if (IsProxy())
			return;
		
		Instigator instigator = instigatorContextData.GetInstigator();
		
		// Ignore AI or NONE instigators
 		if (instigator.GetInstigatorType() != InstigatorType.INSTIGATOR_PLAYER)
			return;
		
		//~ Only handle cases were the player kills a character (No Suicide or friendly fire)
		if (!instigatorContextData.HasAnyVictimKillerRelation(SCR_ECharacterDeathStatusRelations.KILLED_BY_ENEMY_PLAYER))
			return;
		
		SCR_XPHandlerComponent compXP = SCR_XPHandlerComponent.Cast(FindComponent(SCR_XPHandlerComponent));
		if (!compXP)
			return;
		
		//~ Victim is not a character (Safty check)
		SCR_ChimeraCharacter victimCharacter = SCR_ChimeraCharacter.Cast(instigatorContextData.GetVictimEntity());
		if (!victimCharacter)
			return;
		
		vector victimPos = victimCharacter.GetOrigin();
		
		//~ Get nearest base
		SCR_CampaignMilitaryBaseComponent nearestBase = m_BaseManager.FindClosestBase(victimPos);
		if (!nearestBase)
			return;
		
		SCR_FactionManager factionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
		if (!factionManager)
			return;

		int killerId = instigator.GetInstigatorPlayerID();
		
		//~ Get killer faction to check if the closest base has the same faction as the killer
		Faction factionKiller = factionManager.GetPlayerFaction(killerId);
		if (!factionKiller)
			return;
		
		//this awards additional XP to base defenders, so if the instigator is not in his own base, there should be no reward
		if (nearestBase.GetFaction() != factionKiller)
			return;
		
		//~ Not in defending range
		if (vector.DistanceXZ(victimPos, nearestBase.GetOwner().GetOrigin()) > nearestBase.GetRadius())
			return;

		//~ Award defending XP
		compXP.AwardXP(killerId, SCR_EXPRewards.CUSTOM_1);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnPlayerFactionResponse_S(SCR_PlayerFactionAffiliationComponent component, int factionIndex, bool response)
	{
		if (!response)
			return;

		FactionManager factionManager = GetGame().GetFactionManager();

		if (!factionManager)
			return;

		SCR_Faction faction = SCR_Faction.Cast(factionManager.GetFactionByIndex(factionIndex));

		if (!faction)
			return;

		SCR_PlayerController playerController = SCR_PlayerController.Cast(component.GetPlayerController());

		if (!playerController)
			return;

		int playerID = playerController.GetPlayerId();

		// Save faction selected in client's data
		SCR_CampaignClientData clientData;
		clientData = GetClientData(playerID);

		if (clientData && faction)
			clientData.SetFactionIndex(factionManager.GetFactionIndex(faction));
	}

	//------------------------------------------------------------------------------------------------
	protected void OnPlayerFactionResponse_O(SCR_PlayerFactionAffiliationComponent component, int factionIndex, bool response)
	{
		if (!response)
			return;

		FactionManager factionManager = GetGame().GetFactionManager();

		if (!factionManager)
			return;

		SCR_Faction faction = SCR_Faction.Cast(factionManager.GetFactionByIndex(factionIndex));

		if (!faction)
			return;

		SCR_PlayerController playerController = SCR_PlayerController.Cast(component.GetPlayerController());

		if (!playerController)
			return;

		int playerID = playerController.GetPlayerId();

		// When a faction is being assigned to the client automatically by server, playerId might not yet be registered
		// In that case, this saves the connecting player's data and processes them later in local OnPlayerRegistered()
		if (SCR_PlayerController.GetLocalPlayerId() == playerID)
			ProcessFactionAssignment(faction);
		else
			m_mUnprocessedFactionAssignments.Set(playerID, faction);
	}

	//------------------------------------------------------------------------------------------------
	//! See HandleOnFactionAssigned
	protected void ProcessFactionAssignment(Faction assignedFaction)
	{
		m_BaseManager.SetLocalPlayerFaction(SCR_CampaignFaction.Cast(assignedFaction));

		if (m_OnFactionAssignedLocalPlayer)
			m_OnFactionAssignedLocalPlayer.Invoke(assignedFaction);

		// Delayed call so tasks are properly initialized
		if (IsProxy())
			GetGame().GetCallqueue().CallLater(m_BaseManager.UpdateTaskBases, DEFAULT_DELAY, false, assignedFaction);
	}

	//------------------------------------------------------------------------------------------------
	//! Called when an entity is spawned by Free Roam Building
	void OnEntityRequested(notnull IEntity spawnedEntity, IEntity user, SCR_Faction faction, SCR_MilitaryBaseLogicComponent service)
	{
		if (IsProxy())
			return;
		
		SCR_AIGroup aiGroup = SCR_AIGroup.Cast(spawnedEntity);
		if (aiGroup)
		{			
			SCR_CampaignMilitaryBaseManager militaryBaseManager = GetBaseManager();
			if (!militaryBaseManager)
				return;
			
			militaryBaseManager.OnDefenderGroupSpawned(service, aiGroup);
		}
		
		if (!spawnedEntity.IsInherited(Vehicle))
			return;

		// Vehicles requested in bases without fuel depot should have only a small amount of fuel
		array<SCR_FuelManagerComponent> fuelManagers = {};
		array<BaseFuelNode> fuelNodes = {};
		SCR_FuelManagerComponent.GetAllFuelManagers(spawnedEntity, fuelManagers);
		array<SCR_MilitaryBaseComponent> serviceBases = {};
		service.GetBases(serviceBases);
		bool fuelDepotNearby;

		foreach (SCR_MilitaryBaseComponent serviceBase : serviceBases)
		{
			if (serviceBase.GetServiceByType(SCR_EServicePointType.FUEL_DEPOT))
			{
				fuelDepotNearby = true;
				break;
			}
		}

		if (!fuelDepotNearby)
		{
			foreach (SCR_FuelManagerComponent fuelManager : fuelManagers)
			{
				fuelNodes.Clear();
				fuelManager.GetFuelNodesList(fuelNodes);

				foreach (BaseFuelNode fuelNode : fuelNodes)
				{
					fuelNode.SetFuel(fuelNode.GetMaxFuel() * 0.3);
				}
			}
		}

		PlayerManager playerManager = GetGame().GetPlayerManager();

		int playerId = playerManager.GetPlayerIdFromControlledEntity(user);

		if (playerId == 0)
			return;

		SCR_PlayerController playerController = SCR_PlayerController.Cast(playerManager.GetPlayerController(playerId));

		if (!playerController)
			return;

		SCR_CampaignNetworkComponent networkComp = SCR_CampaignNetworkComponent.Cast(playerController.FindComponent(SCR_CampaignNetworkComponent));

		if (!networkComp)
			return;

		ChimeraWorld world = spawnedEntity.GetWorld();
		networkComp.SetLastRequestTimestamp(world.GetServerTimestamp());

		BaseRadioComponent radioComponent = BaseRadioComponent.Cast(spawnedEntity.FindComponent(BaseRadioComponent));

		// Assign faction radio frequency
		if (radioComponent && faction)
		{
			BaseTransceiver transceiver = radioComponent.GetTransceiver(0);

			if (transceiver)
			{
				radioComponent.SetPower(false);
				transceiver.SetFrequency(faction.GetFactionRadioFrequency());
				radioComponent.SetEncryptionKey(faction.GetFactionRadioEncryptionKey());
			}
		}

		SlotManagerComponent slotManager = SlotManagerComponent.Cast(spawnedEntity.FindComponent(SlotManagerComponent));

		if (!slotManager)
			return;

		array<EntitySlotInfo> slots = {};
		slotManager.GetSlotInfos(slots);

		IEntity truckBed;
		SCR_CampaignSuppliesComponent suppliesComponent;
		SCR_CampaignMobileAssemblyComponent mobileAssemblyComponent;
		EventHandlerManagerComponent eventHandlerManager;

		// Handle Conflict-specific vehicles
		foreach (EntitySlotInfo slot : slots)
		{
			if (!slot)
				continue;

			truckBed = slot.GetAttachedEntity();

			if (!truckBed)
				continue;

			mobileAssemblyComponent = SCR_CampaignMobileAssemblyComponent.Cast(truckBed.FindComponent(SCR_CampaignMobileAssemblyComponent));

			// Mobile HQ
			if (mobileAssemblyComponent)
			{
				mobileAssemblyComponent.SetParentFactionID(GetGame().GetFactionManager().GetFactionIndex(faction));
				networkComp.SendVehicleSpawnHint(EHint.CONFLICT_MOBILE_HQ);
			}
		}
	}

#ifdef ENABLE_DIAG
	//------------------------------------------------------------------------------------------------
	override void EOnDiag(IEntity owner, float timeSlice)
	{
		super.EOnDiag(owner, timeSlice);

		// Cheat menu
		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_CAMPAIGN_RANK_UP))
		{
			DiagMenu.SetValue(SCR_DebugMenuID.DEBUGUI_CAMPAIGN_RANK_UP, 0);
			PlayerController pc = GetGame().GetPlayerController();

			if (pc)
			{
				SCR_PlayerXPHandlerComponent comp = SCR_PlayerXPHandlerComponent.Cast(pc.FindComponent(SCR_PlayerXPHandlerComponent));

				if (comp)
					comp.CheatRank();
			}
		}

		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_CAMPAIGN_RANK_DOWN))
		{
			DiagMenu.SetValue(SCR_DebugMenuID.DEBUGUI_CAMPAIGN_RANK_DOWN, 0);
			PlayerController pc = GetGame().GetPlayerController();

			if (pc)
			{
				SCR_PlayerXPHandlerComponent comp = SCR_PlayerXPHandlerComponent.Cast(pc.FindComponent(SCR_PlayerXPHandlerComponent));

				if (comp)
					comp.CheatRank(true);
			}
		}
	}
#endif

	//------------------------------------------------------------------------------------------------
	void SCR_GameModeCampaign(IEntitySource src, IEntity parent)
	{
		// Attributes check
		if (m_sBLUFORFactionKey == FactionKey.Empty)
			Print("SCR_GameModeCampaign: Empty BLUFOR faction key!", LogLevel.ERROR);

		if (m_sOPFORFactionKey == FactionKey.Empty)
			Print("SCR_GameModeCampaign: Empty OPFOR faction key!", LogLevel.ERROR);

		if (m_sINDFORFactionKey == FactionKey.Empty)
			Print("SCR_GameModeCampaign: Empty INDFOR faction key!", LogLevel.ERROR);

		if (!GetGame().InPlayMode())
			return;

		// Cheat menu
#ifdef ENABLE_DIAG
		DiagMenu.RegisterMenu(SCR_DebugMenuID.DEBUGUI_CAMPAIGN_MENU, "Conflict", "");
		DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_CAMPAIGN_INSTANT_BUILDING, "", "Instant composition spawning", "Conflict");
		DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_CAMPAIGN_RANK_UP, "", "Promotion", "Conflict");
		DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_CAMPAIGN_RANK_DOWN, "", "Demotion", "Conflict");
		SetFlags(EntityFlags.ACTIVE, false);
		ConnectToDiagSystem();
#endif

		// Parameters override from header
		SCR_MissionHeaderCampaign header = SCR_MissionHeaderCampaign.Cast(GetGame().GetMissionHeader());

		if (header)
		{
			m_bIgnoreMinimumVehicleRank = header.m_bIgnoreMinimumVehicleRank;

			int suppliesMax = header.m_iMaximumBaseSupplies;
			int suppliesMin = header.m_iMinimumBaseSupplies;
			int controlPointsLimit = header.m_iControlPointsCap;
			int victoryTimeout = header.m_fVictoryTimeout;

			if (suppliesMax != -1)
				m_iMaxStartingSupplies = suppliesMax;

			if (suppliesMin != -1)
				m_iMinStartingSupplies = suppliesMin;

			if (controlPointsLimit != -1)
				m_iControlPointsThreshold = controlPointsLimit;

			if (victoryTimeout != -1)
				m_fVictoryTimer = victoryTimeout;
		}
	}

	//------------------------------------------------------------------------------------------------
	void ~SCR_GameModeCampaign()
	{
		DisconnectFromDiagSystem();

		SCR_SpawnPoint.Event_SpawnPointFactionAssigned.Remove(OnSpawnPointFactionAssigned);

		SCR_AmbientVehicleSystem manager = SCR_AmbientVehicleSystem.GetInstance();

		if (manager)
			manager.GetOnVehicleSpawned().Remove(OnAmbientVehicleSpawned);
	}
}
