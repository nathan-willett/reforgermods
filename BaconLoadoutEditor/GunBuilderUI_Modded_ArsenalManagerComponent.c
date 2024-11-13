modded class SCR_ArsenalManagerComponent {
	bool GunBuilderUI_CanSaveLoadout(int playerId, GameEntity characterEntity, FactionAffiliationComponent playerFactionAffiliation, SCR_ArsenalComponent arsenalComponent, bool sendNotificationOnFailed) {
		return CanSaveLoadout(playerId, characterEntity, playerFactionAffiliation, arsenalComponent, sendNotificationOnFailed);
	}
}