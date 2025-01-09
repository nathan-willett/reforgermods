//------------------------------------------------------------------------------------------------
class SCR_PlacementCallback : ScriptedInventoryOperationCallback
{
	RplId m_PlaceableId;
	RplId m_TargetId;
	int m_iNodeId;
	bool m_bIsBeingAttachedToEntity;
	
	//------------------------------------------------------------------------------------------------
	protected override void OnComplete()
	{
		SCR_ItemPlacementComponent placementComponent = SCR_ItemPlacementComponent.Cast(GetGame().GetPlayerController().FindComponent(SCR_ItemPlacementComponent));
		if (!placementComponent)
			return;

		placementComponent.AskPlaceItem(m_PlaceableId, m_TargetId, m_iNodeId, m_bIsBeingAttachedToEntity);

		super.OnComplete();
	}
}