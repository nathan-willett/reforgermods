[EntityEditorProps(category: "GameScripted/Components", description: "")]
class SCR_PlaceableInventoryItemComponentClass : SCR_BaseInventoryItemComponentClass
{
}

void ScriptInvokerItemPlacedMethod(ChimeraCharacter placingCharacter, SCR_PlaceableInventoryItemComponent placedItemIIC);
typedef func ScriptInvokerItemPlacedMethod;
typedef ScriptInvokerBase<ScriptInvokerItemPlacedMethod> ScriptInvokerItemPlaced;

class SCR_PlaceableInventoryItemComponent : SCR_BaseInventoryItemComponent
{
	protected vector m_vMat[4];
	protected bool m_bUseTransform = false;
	protected RplId m_ParentRplId = RplId.Invalid();
	protected RplId m_PlacingCharacterRplId = RplId.Invalid();
	protected int m_iParentNodeId;
	protected IEntity m_Parent;
	protected IEntity m_RootParent;

	protected static ref ScriptInvokerItemPlaced s_OnPlacementDoneInvoker;

	//------------------------------------------------------------------------------------------------
	//!
	//! \return
	RplId GetParentRplId()
	{
		return m_ParentRplId;
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \return
	int GetParentNodeId()
	{
		return m_iParentNodeId;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	static ScriptInvokerItemPlaced GetOnPlacementDoneInvoker()
	{
		if (!s_OnPlacementDoneInvoker)
			s_OnPlacementDoneInvoker = new ScriptInvokerItemPlaced();

		return s_OnPlacementDoneInvoker;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] user that placed this item
	// To be overridden, called when placement is done in SCR_ItemPlacementComponent
	protected void PlacementDone(notnull ChimeraCharacter user)
	{
		if (s_OnPlacementDoneInvoker)
			s_OnPlacementDoneInvoker.Invoke(user, this);
	}

	//------------------------------------------------------------------------------------------------
	//!
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void RPC_DoPlaceItem(RplId placingCharacterRplId)
	{
		IEntity item = GetOwner();
		InventoryItemComponent itemComponent = InventoryItemComponent.Cast(item.FindComponent(InventoryItemComponent));
		if (!itemComponent)
			return;

		itemComponent.EnablePhysics();
		itemComponent.ActivateOwner(true);

		m_ParentRplId = -1;
		m_iParentNodeId = -1;

		PlayPlacedSound(m_vMat[1], m_vMat[3]);

		RplComponent characterRplComp = RplComponent.Cast(Replication.FindItem(placingCharacterRplId));
		if (!characterRplComp)
			return;

		ChimeraCharacter placingCharacter = ChimeraCharacter.Cast(characterRplComp.GetEntity());
		if (!placingCharacter)
			return;

		PlacementDone(placingCharacter);
	}

	//------------------------------------------------------------------------------------------------
	//!
	void PlaceItem()
	{
		Rpc(RPC_DoPlaceItem, m_PlacingCharacterRplId);
		RPC_DoPlaceItem(m_PlacingCharacterRplId);
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] entity
	//! \return
	protected bool ValidateEntity(notnull IEntity entity)
	{
		Physics physics = entity.GetPhysics();
		if (physics && (physics.IsDynamic() || physics.IsKinematic()))
			return false;

		// F. e. Slotted vehicle parts are physically static, but their main parent (vehicle) is not, we need to check that
		IEntity mainEntity = entity.GetRootParent();
		if (mainEntity && mainEntity != entity)
		{
			physics = mainEntity.GetPhysics();
			if (physics && physics.IsDynamic())
				return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] e
	//! \return
	protected bool FilterCallback(notnull IEntity e)
	{
		Physics physics = e.GetPhysics();
		if (physics)
		{
			if (physics.GetInteractionLayer() & EPhysicsLayerDefs.Water)
				return true;
		}

		return ValidateEntity(e);
	}

	//------------------------------------------------------------------------------------------------
	//! Authority method used to change the position at which item will placed when removed from the inventory
	void SetPlacementPosition(vector right, vector up, vector forward, vector position, RplId characterRplId)
	{
		m_vMat[0] = right;
		m_vMat[1] = up;
		m_vMat[2] = forward;
		m_vMat[3] = position;
		m_bUseTransform = true;
		m_PlacingCharacterRplId = characterRplId;
	}

	//------------------------------------------------------------------------------------------------
	override bool OverridePlacementTransform(IEntity caller, out vector computedTransform[4])
	{
		ActivateOwner(true);

		// Enable physics to receive contact events
		Physics physics = GetOwner().GetPhysics();
		if (physics)
			EnablePhysics();

		if (m_bUseTransform)
		{
			m_bUseTransform = false;
			computedTransform = m_vMat;

			// Entity was purposefully "placed" somewhere so we assume it should stay there (e.g. mines and flags to mark them)
			SCR_GarbageSystem garbageSystem = SCR_GarbageSystem.GetByEntityWorld(GetOwner());
			if (garbageSystem)
				garbageSystem.Withdraw(GetOwner());

			return true;
		}

		m_ParentRplId = -1;
		m_iParentNodeId = -1;
		m_PlacingCharacterRplId = RplId.Invalid();

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] up
	//! \param[in] position
	protected void PlayPlacedSound(vector up, vector position)
	{
		SCR_SoundDataComponent soundData;
		SoundComponent soundComp = SoundComponent.Cast(GetOwner().FindComponent(SoundComponent));
		if (!soundComp)
		{
			soundData = SCR_SoundDataComponent.Cast(GetOwner().FindComponent(SCR_SoundDataComponent));
			if (!soundData)
				return;
		}

		TraceParam param = new TraceParam();
		param.Start = position;
		param.End = param.Start - up;
		param.Flags = TraceFlags.WORLD | TraceFlags.ENTS;
		param.Exclude = GetOwner();
		param.LayerMask = EPhysicsLayerPresets.Projectile;
		GetOwner().GetWorld().TraceMove(param, null);

		GameMaterial material = param.SurfaceProps;
		if (!material)
			return;

		if (soundComp)
		{
			soundComp.SetSignalValueStr(SCR_AudioSource.SURFACE_SIGNAL_NAME, material.GetSoundInfo().GetSignalValue());
			soundComp.SoundEvent(SCR_SoundEvent.SOUND_PLACE_OBJECT);
		}
		else if (soundData)
		{
			SCR_SoundManagerEntity soundManager = GetGame().GetSoundManagerEntity();
			if (!soundManager)
				return;

			SCR_AudioSource soundSrc = soundManager.CreateAudioSource(GetOwner(), SCR_SoundEvent.SOUND_PLACE_OBJECT);
			if (!soundSrc)
				return;

			soundSrc.SetSignalValue(SCR_AudioSource.SURFACE_SIGNAL_NAME, material.GetSoundInfo().GetSignalValue());
			vector mat[4];	//due to the fact that item might still be in players inventory we need to override sound position
			mat[0] = up.Perpend();
			mat[1] = up;
			mat[2] = mat[0] * up;
			mat[3] = position;
			soundManager.PlayAudioSource(soundSrc, mat);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Same as PlaceItem but with params that allow attaching the object to new parent entity
	void PlaceItemWithParentChange(RplId newParentRplId, int nodeId = -1)
	{
		Rpc(RPC_DoPlaceItemWithParentChange, newParentRplId, nodeId, m_PlacingCharacterRplId);
		RPC_DoPlaceItemWithParentChange(newParentRplId, nodeId, m_PlacingCharacterRplId);
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] newParentRplId
	//! \param[in] nodeId
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void RPC_DoPlaceItemWithParentChange(RplId newParentRplId, int nodeId, RplId placingCharacterRplId)
	{
		InventoryItemComponent itemComponent = InventoryItemComponent.Cast(GetOwner().FindComponent(InventoryItemComponent));
		if (!itemComponent)
			return;

		itemComponent.EnablePhysics();
		itemComponent.ActivateOwner(true);

		if (newParentRplId.IsValid())
		{
			m_ParentRplId = newParentRplId;
			m_iParentNodeId = nodeId;
			if (itemComponent.IsLocked())
				itemComponent.m_OnLockedStateChangedInvoker.Insert(AttachToNewParentWhenUnlocked);
			else
				AttachToNewParent();
		}
		else
		{
			m_ParentRplId = -1;
			m_iParentNodeId = -1;
		}

		PlayPlacedSound(m_vMat[1], m_vMat[3]);

		RplComponent characterRplComp = RplComponent.Cast(Replication.FindItem(placingCharacterRplId));
		if (!characterRplComp)
			return;

		ChimeraCharacter placingCharacter = ChimeraCharacter.Cast(characterRplComp.GetEntity());
		if (!placingCharacter)
			return;

		PlacementDone(placingCharacter);
	}

	//------------------------------------------------------------------------------------------------
	//! Method for updating parent when proxy streams in placeable item
	//! \param[in] parentId replication id of a entity to which this item should be attached
	//! \param[in] nodeId id of a node to which this item will be attached to
	void SetNewParent(RplId parentId = RplId.Invalid(), int nodeId = -1)
	{
		if (m_ParentRplId == parentId)
			return;

		m_ParentRplId = parentId;
		m_iParentNodeId = nodeId;

		AttachToNewParent();
	}

	//------------------------------------------------------------------------------------------------
	//! Method that will try to find new parent entity and make this entity a child of it
	//! \param[in] nowLocked
	protected void AttachToNewParentWhenUnlocked(bool nowLocked)
	{
		if (nowLocked)
			return;

		InventoryItemComponent itemComponent = InventoryItemComponent.Cast(GetOwner().FindComponent(InventoryItemComponent));
		if (itemComponent)
			itemComponent.m_OnLockedStateChangedInvoker.Remove(AttachToNewParentWhenUnlocked);

		AttachToNewParent();
	}

	//------------------------------------------------------------------------------------------------
	//! Method that will try to find new parent entity and make this entity a child of it
	protected void AttachToNewParent()
	{
		if (!m_ParentRplId.IsValid())
			return;

		IEntity item = GetOwner();
		if (!item || item.IsDeleted())
			return;

		RplComponent newParentRplComp = RplComponent.Cast(Replication.FindItem(m_ParentRplId));
		IEntity newParentEntity;
		if (newParentRplComp)
			m_Parent = newParentRplComp.GetEntity();	//cache it as depending how this item will be transfered later we may not have an easy access to it

		if (!m_Parent)
			m_Parent = IEntity.Cast(Replication.FindItem(m_ParentRplId));

		if (!m_Parent)
			return;

		item.GetWorldTransform(m_vMat);

		RplComponent rplComp = RplComponent.Cast(item.FindComponent(RplComponent));
		if (rplComp && newParentRplComp && rplComp.IsOwner())
		{
			RplNode explosiveCharge = rplComp.GetNode();
			RplNode newParent = newParentRplComp.GetNode();

			explosiveCharge.SetParent(newParent);
		}

		m_Parent.AddChild(item, m_iParentNodeId);
		item.SetWorldTransform(m_vMat);
		item.Update();

		HitZoneContainerComponent parentDamageManager = HitZoneContainerComponent.Cast(m_Parent.FindComponent(HitZoneContainerComponent));
		if (parentDamageManager)
		{
			SCR_HitZone hitZone = SCR_HitZone.Cast(parentDamageManager.GetDefaultHitZone());
			if (hitZone)
				hitZone.GetOnDamageStateChanged().Insert(OnParentDamageStateChanged);
		}

		InventoryItemComponent iic = InventoryItemComponent.Cast(item.FindComponent(InventoryItemComponent));
		if (iic)
			iic.m_OnParentSlotChangedInvoker.Insert(StartWatchingParentSlots);

		iic = InventoryItemComponent.Cast(m_Parent.FindComponent(InventoryItemComponent));
		if (iic)
			iic.m_OnParentSlotChangedInvoker.Insert(DetachFromParent);

		if (m_Parent == m_Parent.GetRootParent())
			return;

		m_RootParent = m_Parent.GetRootParent();	//cache it as when this item will be transfered we may not be able to tell which one it was
		iic = InventoryItemComponent.Cast(m_RootParent.FindComponent(InventoryItemComponent));
		if (iic)
			iic.m_OnParentSlotChangedInvoker.Insert(DetachFromParent);
	}

	//------------------------------------------------------------------------------------------------
	//! Method that is meant to be added to the parent default hit zone in order to detect when it is destroyed
	//! \param[in] hitZone
	protected void OnParentDamageStateChanged(SCR_HitZone hitZone)
	{
		if (hitZone.GetDamageState() == EDamageState.DESTROYED)
			DetachFromParent();
	}

	//------------------------------------------------------------------------------------------------
	//! Removes this entity from its parent hierarchy and places it on the ground
	protected void DetachFromParent()
	{
		if (!m_Parent)
			return;

		IEntity owner = GetOwner();
		if (!owner || owner.IsDeleted())
			return;

		m_ParentRplId = -1;
		m_iParentNodeId = -1;
		m_Parent.RemoveChild(owner, true);
		array<IEntity> excludeArray = {m_Parent};
		if (m_RootParent)
			excludeArray.Insert(m_RootParent);

		SCR_EntityHelper.SnapToGround(owner, excludeArray, onlyStatic: true);
		owner.SetAngles({0, owner.GetAngles()[1], 0});
		owner.Update();

		HitZoneContainerComponent parentDamageManager = HitZoneContainerComponent.Cast(m_Parent.FindComponent(HitZoneContainerComponent));
		if (parentDamageManager)
		{
			SCR_HitZone hitZone = SCR_HitZone.Cast(parentDamageManager.GetDefaultHitZone());
			if (hitZone)
				hitZone.GetOnDamageStateChanged().Remove(OnParentDamageStateChanged);
		}

		InventoryItemComponent iic = InventoryItemComponent.Cast(owner.FindComponent(InventoryItemComponent));
		if (iic)
			iic.m_OnParentSlotChangedInvoker.Remove(StopWatchingParentSlots);

		iic = InventoryItemComponent.Cast(m_Parent.FindComponent(InventoryItemComponent));
		if (iic)
			iic.m_OnParentSlotChangedInvoker.Remove(DetachFromParent);

		m_Parent = null;
		if (!m_RootParent)
			return;

		iic = InventoryItemComponent.Cast(m_RootParent.FindComponent(InventoryItemComponent));
		if (iic)
			iic.m_OnParentSlotChangedInvoker.Remove(DetachFromParent);

		m_RootParent = null;
	}

	//------------------------------------------------------------------------------------------------
	//! Callback method triggered when item will finish its transfer to new parent when it is being attached to it
	protected void StartWatchingParentSlots()
	{
		InventoryItemComponent iic = InventoryItemComponent.Cast(GetOwner().FindComponent(InventoryItemComponent));
		if (!iic)
			return;

		iic.m_OnParentSlotChangedInvoker.Remove(StartWatchingParentSlots);
		iic.m_OnParentSlotChangedInvoker.Insert(StopWatchingParentSlots);
	}

	//------------------------------------------------------------------------------------------------
	//! Callback method when item is being transfered (f.e. picked up) after it was attached to some object
	protected void StopWatchingParentSlots()
	{
		if (!GetOwner())
			return;

		InventoryItemComponent iic = InventoryItemComponent.Cast(GetOwner().FindComponent(InventoryItemComponent));
		if (iic)
			iic.m_OnParentSlotChangedInvoker.Remove(StopWatchingParentSlots);

		if (m_Parent)
		{
			HitZoneContainerComponent parentDamageManager = HitZoneContainerComponent.Cast(m_Parent.FindComponent(HitZoneContainerComponent));
			if (parentDamageManager)
			{
				SCR_HitZone hitZone = SCR_HitZone.Cast(parentDamageManager.GetDefaultHitZone());
				if (hitZone)
					hitZone.GetOnDamageStateChanged().Remove(OnParentDamageStateChanged);
			}

			iic = InventoryItemComponent.Cast(m_Parent.FindComponent(InventoryItemComponent));
			if (iic)
				iic.m_OnParentSlotChangedInvoker.Remove(DetachFromParent);

			m_Parent = null;
		}

		if (!m_RootParent)
			return;

		iic = InventoryItemComponent.Cast(m_RootParent.FindComponent(InventoryItemComponent));
		if (iic)
			iic.m_OnParentSlotChangedInvoker.Remove(DetachFromParent);

		m_RootParent = null;
	}

	//------------------------------------------------------------------------------------------------
	override void OnDelete(IEntity owner)
	{
		StopWatchingParentSlots();
	}
}
