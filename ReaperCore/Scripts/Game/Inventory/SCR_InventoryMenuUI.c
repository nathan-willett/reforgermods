
// --------------------------------------------------------------------------------------------------------------------
// ------           This script is part of REAPER CORE | an Arma Reforger SP/COOP Modification                   ------
// ------   You are not allowed to use this script or parts of it in your mod or pass it off as your own work.   ------
// ------                                                                                                        ------
// ------                         Written by REAPER 2024 - www.reaper-as.de                                      ------
// --------------------------------------------------------------------------------------------------------------------


modded class SCR_InventoryMenuUI : ChimeraMenuBase
{
	//------------------------------------------------------------------------------------------------
	
	protected ResourceName m_infoLayout = "{9A3073FB3DC602AC}UI/Inventory/REAPER_InventoryItemInfo.layout";
	
	//Var for Magazine filtering 
	protected bool m_filterMagazines = false;
	
	// Stuff for inspection 
	protected bool m_isWeaponInspection = false;
	
	//Stuff for Magazine repack
	protected SoundComponent m_Soundcomp;
	protected AudioHandle m_SoundHandler;
	protected string m_RepackMagsSound = "SOUND_REPACK_MAGS"; // REAPER_CharacterSounds.acp (Set on BaseCharacter SoundComp)
	
	//------------------------------------------------------------------------------------------------

	override void NavigationBarUpdate()
	{
		super.NavigationBarUpdate();
			
		if(!m_pNavigationBar) return;

		m_pNavigationBar.SetButtonEnabled( "ButtonRepackMags", true ); // Enable the repack ButtonID from SCR_NavigationBarConfig (config file)
	}
	
	//------------------------------------------------------------------------------------------------
	
	override void NavigationBarUpdateGamepad()
	{
		super.NavigationBarUpdateGamepad(); 
		
		if(!m_pNavigationBar) return;
		
		m_pNavigationBar.SetButtonEnabled( "ButtonRepackMags", true ); // Enable the repack ButtonID from SCR_NavigationBarConfig (config file)
	}
	
	//------------------------------------------------------------------------------------------------
	
	override void OnAction( SCR_InputButtonComponent comp, string action, SCR_InventoryStorageBaseUI pParentStorage = null, int traverseStorageIndex = -1 )
	{
		super.OnAction(comp, action, pParentStorage, traverseStorageIndex);
		
		if(action == "REAPER_RepackMags") { REAPER_RepackMagazines_start(); }	
	}

	//------------------------------------------------------------------------------------------------
		
	override protected void Action_Drop()
	{
		super.Action_Drop();
		UpdatePreview();
	}
	
	//------------------------------------------------------------------------------------------------
	
	override protected void Action_CloseInventory()
	{	
		REAPER_RepackMagazines_end(); //Stop Magazine repack!
		
		super.Action_CloseInventory();
	}	
	
	//------------------------------------------------------------------------------------------------
	
	protected void REAPER_RepackMagazines_start()
	{
		if(m_InventoryManager) {
			
			if(!m_CharController || m_CharController.GetLifeState() != ECharacterLifeState.ALIVE) {
				if(m_SoundHandler != AudioHandle.Invalid) AudioSystem.TerminateSound(m_SoundHandler);
				return;
			}
			
			Widget loading = m_widget.FindAnyWidget("LoadingOverlay");
			if(loading) loading.SetVisible(true);
			
			m_Soundcomp = SoundComponent.Cast(m_Player.FindComponent(SoundComponent));			
			if(m_Soundcomp) m_SoundHandler = m_Soundcomp.SoundEvent(m_RepackMagsSound);		
			
			GetGame().GetCallqueue().CallLater(REAPER_RepackMagazines_loop, 1000, false, true);		
		}
	}
	
	//------------------------------------------------------------------------------------------------
	
	protected void REAPER_RepackMagazines_loop(bool runManagerMethod)
	{
		if(!m_CharController || m_CharController.GetLifeState() != ECharacterLifeState.ALIVE) {
			REAPER_RepackMagazines_end();
			return;
		}
		
		if(!m_InventoryManager) return;
		
		if(runManagerMethod) m_InventoryManager.REAPER_RepackMagazines_C();
		
		if(m_InventoryManager.REAPER_GetRepackMagazinesComplete()) {
			REAPER_RepackMagazines_end();
			return;			
		} else {
			GetGame().GetCallqueue().CallLater(REAPER_RepackMagazines_loop, 1000, false, false);	
		}
	}
	
	//------------------------------------------------------------------------------------------------
		
	protected void REAPER_RepackMagazines_end() 
	{	
		GetGame().GetCallqueue().Remove(REAPER_RepackMagazines_loop);
		
		Widget loading = m_widget.FindAnyWidget("LoadingOverlay");
		if(loading) loading.SetVisible(false);
		if(m_SoundHandler != AudioHandle.Invalid) AudioSystem.TerminateSound(m_SoundHandler);
	}	
	

	//------------------------------------------------------------------------------------------------
	// REAPER - Updated InfoWidget with preview
	//------------------------------------------------------------------------------------------------
		
	override protected void SetFocusedSlotEffects()
	{
		if(!m_pFocusedSlotUI) return;
		
		//show info about the item
		InventoryItemComponent invItemComp = m_pFocusedSlotUI.GetInventoryItemComponent();
		if(!invItemComp) return;
		
		auto attribs = SCR_ItemAttributeCollection.Cast( invItemComp.GetAttributes() );
		if(!attribs) return;
		
		SCR_InventorySlotGearInspectionUI inspectSlot = SCR_InventorySlotGearInspectionUI.Cast(m_pFocusedSlotUI);

		UIInfo itemInfo = attribs.GetUIInfo();
		
		if(!itemInfo || inspectSlot) { // Do not show ItemInfo on inspection Slots!
			
			HideItemInfo();
			
		} else {

			IEntity item = invItemComp.GetOwner();
			SCR_InventoryUIInfo inventoryInfo = SCR_InventoryUIInfo.Cast(itemInfo);
			
			if (inventoryInfo) {
				ShowItemInfo(inventoryInfo.GetInventoryItemName(invItemComp), inventoryInfo.GetInventoryItemDescription(invItemComp), invItemComp.GetTotalWeight(), inventoryInfo, item);
			} else {
				ShowItemInfo(itemInfo.GetName(), itemInfo.GetDescription(), invItemComp.GetTotalWeight(), null, item);
			}
				
		}

		NavigationBarUpdate();
	}
	
	//------------------------------------------------------------------------------------------------	
	
	void ShowItemInfo( string sName = "", string sDescr = "", float sWeight = 0.0, SCR_InventoryUIInfo uiInfo = null, IEntity previewItem = null )
	{		
		if(!m_pItemInfo ) {
			Widget infoWidget = GetGame().GetWorkspace().CreateWidgets(m_infoLayout, m_widget);
			if(!infoWidget) return;

			infoWidget.AddHandler(new SCR_InventoryItemInfoUI());
			m_pItemInfo = SCR_InventoryItemInfoUI.Cast(infoWidget.FindHandler( SCR_InventoryItemInfoUI ) );
		}

		if(!m_pItemInfo) return;

		Widget w = WidgetManager.GetWidgetUnderCursor();
		if(!w) w = m_pFocusedSlotUI.GetButtonWidget();	
		
		m_pItemInfo.SetPreviewItem(previewItem);	
		m_pItemInfo.SetName( sName );
		m_pItemInfo.SetDescription( sDescr );	
		m_pItemInfo.SetWeight( sWeight );
		
		if (uiInfo && uiInfo.IsIconVisible())
			m_pItemInfo.SetIcon(uiInfo.GetIconPath(), uiInfo.GetIconColor());
		else
			m_pItemInfo.ShowIcon(false);
			
		//~ Add hints
		array<SCR_InventoryItemHintUIInfo> hintsInfo = {};
		if(uiInfo) uiInfo.GetItemHintArray(hintsInfo);
		
		//~ Arsenal supply cost hint if item is in an arsenal storage
		if(m_SupplyCostUIInfo) {
			SCR_ArsenalInventorySlotUI arsenalSlot = SCR_ArsenalInventorySlotUI.Cast(m_pFocusedSlotUI);
			if (arsenalSlot) {
				m_SupplyCostUIInfo.SetSupplyCost(arsenalSlot.GetItemSupplyCost());
				hintsInfo.InsertAt(m_SupplyCostUIInfo, 0);
			}
		}

		//~ If has hints show them
		if(!hintsInfo.IsEmpty()) m_pItemInfo.SetItemHints(m_pFocusedSlotUI.GetInventoryItemComponent(), hintsInfo, m_pFocusedSlotUI);
		
		// Move Info Widget to center (At playerRenderWidget! Because inventory is not symetric)
		float posX, posY;
		m_wPlayerRender.GetScreenPos(posX, posY);
	
		m_pItemInfo.Move( GetGame().GetWorkspace().DPIUnscale(posX), GetGame().GetWorkspace().DPIUnscale(270) );
		m_pItemInfo.Show( 0.6, w, m_bIsUsingGamepad );	
	}
	
	
	
	
	//------------------------------------------------------------------------------------------------
	// REAPER - Open Weapon Inspection from Item
	//------------------------------------------------------------------------------------------------
	
	// OnQueueProcessed called after loading is done! 
	// Modded, to make loadingOverlay available for actions!
	// Modded, to filter Magazines
	// Modded, to open weapon inspection
	
	override protected void OnQueueProcessed()  
	{
		m_bProcessInitQueue = false;
		m_wAttachmentPointsContainer.SetVisible(true);
		UpdateCharacterPreview();
		SetStorageSwitchMode(m_bIsUsingGamepad);
		if(m_LoadingOverlay) m_LoadingOverlay.GetRootWidget().SetVisible(false);
		//m_LoadingOverlay.GetRootWidget().RemoveFromHierarchy();  // Original BIS
		
		// REAPER: Open Inspection (for InvisibleArsenal) and set Magazine filter
		if(m_InventoryManager) {
			
			if(m_InventoryManager.REAPER_GetFilterMagazines()) {
				m_filterMagazines = true;
				 REAPER_FilterVicinityMagazines();
			} else {
				m_filterMagazines = false;
			}
			
			if(m_InventoryManager.REAPER_GetOpenWeaponInspectionMenu()) REAPER_OpenInspectWeapon();
		} 	
	}	
	
	//------------------------------------------------------------------------------------------------
			
	protected void REAPER_OpenInspectWeapon()
	{	
		if(m_InventoryManager) m_InventoryManager.REAPER_SetOpenWeaponInspectionMenu(false); // Reset to false
		
		//Get current weapon
		BaseWeaponComponent wComp = m_CharController.REAPER_GetCurrentWeapon();
		if(!wComp) return; 
			
		IEntity wEnt = wComp.GetOwner();			
					
		if(wEnt && m_pWeaponStorage) {				
	
			// Get ItemSlot
			array<SCR_InventorySlotUI> invSlots = {};
			m_pWeaponStorage.GetUISlots(invSlots);

			foreach (SCR_InventorySlotUI invSlot : invSlots) {
				
				InventoryItemComponent itemComp = invSlot.GetInventoryItemComponent();
				
				if(itemComp && wEnt == itemComp.GetOwner()) {
															
					m_pInspectedSlot = invSlot;			
					m_pSelectedSlotUI = invSlot;
					m_pFocusedSlotUI = invSlot;
					
					InspectItem(invSlot);
																							
					m_pPreviewManager.SetPreviewItem(m_wPlayerRender, wEnt, m_PlayerRenderAttributes, true);
					if(m_wPlayerRender && m_pGearInspectionPointUI) m_pGearInspectionPointUI.UpdatePreviewSlotWidgets(m_wPlayerRender);
					m_pPreviewManager.Update();
							
					break;
				}				
			}					
		}	
	}

	//------------------------------------------------------------------------------------------------
	// Cancel weapon inspection, if already open
	
	override void InspectItem(SCR_InventorySlotUI itemSlot)
	{
		if(!itemSlot && m_isWeaponInspection) {
			
			m_PlayerRenderAttributes.RotateItemCamera(Vector(0, 0, 0), Vector(0, 0, 0), Vector(0, 0, 0)); // reset rotation
			m_PlayerRenderAttributes.ResetDeltaRotation();
			m_PlayerRenderAttributes.ZoomCamera(0.5);
			
			m_isWeaponInspection = false;			
		}
		
		super.InspectItem(itemSlot);	
	}
		
	//------------------------------------------------------------------------------------------------
	// Set camera zoom for Weapon inspection
	
	override void InspectWeapon(SCR_WeaponAttachmentsStorageComponent weaponAttachmentStorage)
	{	
		IEntity wEnt = weaponAttachmentStorage.GetOwner();
		if(wEnt) {
	
			//Calculate camera Zoom				
			vector mins, maxs;
			wEnt.GetBounds(mins, maxs);
			float wDistance = vector.Distance(mins, maxs);
			
			BaseWeaponStatsManagerComponent statsMgr = BaseWeaponStatsManagerComponent.Cast(wEnt.FindComponent(BaseWeaponStatsManagerComponent)); 
			if(statsMgr) {
				float addLength;
				statsMgr.GetExtraObstructionLength(addLength);
				wDistance += addLength;
			} 
								
			float fovMin = wDistance * 30; 
			float fovMax = wDistance * 40;
			
			m_PlayerRenderAttributes.RotateItemCamera(Vector(0, 125, 0), Vector(0, 125, 0), Vector(0, 125, 0));
			m_PlayerRenderAttributes.ZoomCamera(1, fovMin, fovMax);
			
			m_isWeaponInspection = true;
			
			super.InspectWeapon(weaponAttachmentStorage);
					
		} else {
			m_isWeaponInspection = false;
		}
	

	}	

		
	//------------------------------------------------------------------------------------------------------------------------------
	// REAPER - Handle blocked slots
	//------------------------------------------------------------------------------------------------------------------------------
	
	bool REAPER_CanMoveItemToBlockedClothSlot()
	{		
		if(!m_pSelectedSlotUI && !m_pFocusedSlotUI) return false;	
					
		IEntity pItem;
		
		if(m_pSelectedSlotUI && m_pSelectedSlotUI.GetInventoryItemComponent()) {
			
			pItem = m_pSelectedSlotUI.GetInventoryItemComponent().GetOwner();
			
		} else {
			
			if(!pItem && m_pFocusedSlotUI && m_pFocusedSlotUI.GetInventoryItemComponent())
				pItem = m_pFocusedSlotUI.GetInventoryItemComponent().GetOwner();		
		}

		if(!pItem) return false;
								
		BaseLoadoutClothComponent clothComp = BaseLoadoutClothComponent.Cast(pItem.FindComponent(BaseLoadoutClothComponent));
		
		
		if(clothComp && m_StorageManager) {
			LoadoutAreaType AreaTypeItem = clothComp.GetAreaType();
			
			if(!AreaTypeItem) return false;
					
			array<typename> blockedSlots = {};
			m_StorageManager.GetBlockedSlots(blockedSlots);
						
			if (blockedSlots.Count() > 0 && blockedSlots.Contains(AreaTypeItem.Type())) {
				return false;
			} else {
				return true;
			}
		}
		return true;		
	
	}
	
	// Disable DragDrop if slot is blocked
	override void MoveItemToStorageSlot()
	{
		if(REAPER_CanMoveItemToBlockedClothSlot()) super.MoveItemToStorageSlot();		
	}
	
	// Disable PicUkp if slot is blocked
	override protected void Action_MoveBetween()
	{			
		if(REAPER_CanMoveItemToBlockedClothSlot()) super.Action_MoveBetween()		
	}	
	
	
	//----------------------------------------------------------------------------------
	// REAPER - Update the storageUI Slots for backpack items
	//----------------------------------------------------------------------------------
	
	override void HighlightAvailableStorages(SCR_InventorySlotUI itemSlot)
	{		
		if(!itemSlot) return;
		if(!m_pActiveHoveredStorageUI) return;
		
		InventoryItemComponent itemComp = itemSlot.GetInventoryItemComponent();
		if(!itemComp) return;
		
		IEntity itemEntity = itemComp.GetOwner();
		if(!itemEntity) return;
		
		if(itemEntity.GetParent()) {
			
			REAPER_BackpackComponent backpackComp = REAPER_BackpackComponent.Cast(itemEntity.GetParent().FindComponent(REAPER_BackpackComponent));
			
			if(backpackComp) {
				
				// REAPER -Spawn the item Localy to test the Sotrage Size
				IEntity testEntity = GetGame().SpawnEntityPrefabLocal(Resource.Load(itemEntity.GetPrefabData().GetPrefab().GetResourceName()));
				if(testEntity) {
					itemEntity = testEntity;
					itemComp = InventoryItemComponent.Cast(testEntity.FindComponent(InventoryItemComponent));
				}
				
				InventoryStorageSlot itemParentSlot = itemComp.GetParentSlot();
							
				BaseInventoryStorageComponent originStorage;
				if(itemParentSlot) originStorage = itemParentSlot.GetStorage();
					
				SCR_InventoryStorageManagerComponent invManagerTo = m_pActiveHoveredStorageUI.GetInventoryManager();	
				if(!invManagerTo) return;
				
				BaseInventoryStorageComponent contStorage;
				array<BaseInventoryStorageComponent> contStorageOwnedStorages = {};
				SCR_EquipmentStorageComponent equipmentStorage;
						
				foreach (SCR_InventoryStorageBaseUI storageBaseUI: m_aStorages)
				{	
					if (!storageBaseUI) continue;
					if (storageBaseUI.Type() == SCR_InventoryStorageLootUI) continue;
					
					contStorage = storageBaseUI.GetStorage();	
					if (!contStorage) continue;
					
					if (originStorage && contStorage == originStorage) continue;		
					if (IsStorageInsideLBS(originStorage, ClothNodeStorageComponent.Cast(contStorage))) continue;
					
					float itemWeight = itemComp.GetTotalWeight();
					float totalWeightWithInsertedItem;
					
					totalWeightWithInsertedItem = storageBaseUI.GetTotalRoundedUpWeight(contStorage);
					totalWeightWithInsertedItem += Math.Round(itemWeight * 100) * 0.01;
					
					storageBaseUI.UpdateTotalWeight(totalWeightWithInsertedItem);
					
					float totalOccupiedVolumeWithInsertedItem;
					totalOccupiedVolumeWithInsertedItem = storageBaseUI.GetOccupiedVolume(contStorage);
					totalOccupiedVolumeWithInsertedItem += itemComp.GetTotalVolume();	
		
					contStorageOwnedStorages.Clear();
					contStorage.GetOwnedStorages(contStorageOwnedStorages, 1, false);
					contStorageOwnedStorages.Insert(contStorage);
					
					bool shouldUpdateVolumePercentage = true;
					
					// Check to see if the itemEntity can fit into any equipment Storages so that volume is not updated in those cases.
					for (int i = 0, count = contStorageOwnedStorages.Count(); i < count; i++)
					{
						equipmentStorage = SCR_EquipmentStorageComponent.Cast(contStorageOwnedStorages.Get(i));
						if (!equipmentStorage) continue;
						
						bool canInsert = m_InventoryManager.CanInsertItemInStorage(itemEntity, equipmentStorage, -1); //split because of debug purposes
						bool canMove = m_InventoryManager.CanMoveItemToStorage(itemEntity, equipmentStorage, -1);
														
						if(canInsert || canMove) {
							shouldUpdateVolumePercentage = false;
							break;
						}
					}
					
					if(!m_InventoryManager.CanInsertItemInActualStorage(itemEntity, contStorage) && itemEntity.FindComponent(SCR_ResourceComponent))
						shouldUpdateVolumePercentage = false;
					
					if (shouldUpdateVolumePercentage)
						storageBaseUI.UpdateVolumePercentage(storageBaseUI.GetOccupiedVolumePercentage(contStorage, totalOccupiedVolumeWithInsertedItem), true);
					
					bool canInsert = m_InventoryManager.CanInsertItemInActualStorage(itemEntity, contStorage);
					
		
					SCR_UniversalInventoryStorageComponent uniContStorage = SCR_UniversalInventoryStorageComponent.Cast(contStorage);
					bool weightCheck = true;
					if(uniContStorage) weightCheck = uniContStorage.IsAdditionalWeightOk(itemComp.GetTotalWeight());
		
					EInvInsertFailReason reason;
					if (!contStorage.PerformDimensionValidation(itemEntity))
						reason |= EInvInsertFailReason.SIZE;
		
					if (!weightCheck)
						reason |= EInvInsertFailReason.WEIGHT;
		
					if (!contStorage.PerformVolumeValidation(itemEntity))
						reason |= EInvInsertFailReason.CAPACITY;
		
							
					if (canInsert)
						storageBaseUI.SetStorageAsHighlighted(true);
					else
						storageBaseUI.SetStorageAsHighlighted(false, reason);
				}
				
				if(testEntity) delete testEntity;
				
				return;			
			}
		}
				
		super.HighlightAvailableStorages(itemSlot);
	}

	//----------------------------------------------------------------------------------
	// REAPER - Set the backpack as storage to open 
	//----------------------------------------------------------------------------------
		
	override void ShowVicinity(bool compact = false)
	{
		if(!m_pVicinity) {
			Print("No vicnity component on character!", LogLevel.DEBUG);
			return;
		}

		if(m_wLootStorage) {
			m_wLootStorage.RemoveHandler( m_wLootStorage.FindHandler(SCR_InventoryStorageLootUI)); //remove the handler from the widget
			m_wLootStorage.RemoveFromHierarchy();
		}

		Widget parent = m_widget.FindAnyWidget( "StorageLootSlot" );
		m_wLootStorage = GetGame().GetWorkspace().CreateWidgets(BACKPACK_STORAGE_LAYOUT, parent);
		if(!m_wLootStorage) return;
		
		// Set Backpack 
		IEntity itemOfInterest = m_pVicinity.GetItemOfInterest();
		
		if(itemOfInterest) {
			
			REAPER_BackpackComponent backpackComp = REAPER_BackpackComponent.Cast(itemOfInterest.FindComponent(REAPER_BackpackComponent));
			
			if(backpackComp) {
				
				IEntity backpackOwner = backpackComp.GetOwner().GetParent();
				
				if(backpackOwner) {
									
					m_pVicinity.SetItemOfInterest(backpackOwner); // Set the Owner as ItemRoot!
					
					BaseInventoryStorageComponent pStorage = BaseInventoryStorageComponent.Cast(itemOfInterest.FindComponent(BaseInventoryStorageComponent));

					m_wLootStorage.AddHandler(new SCR_InventoryStorageLootUI(pStorage, null, this, 0, null, m_Player));
					
					m_pStorageLootUI = SCR_InventoryStorageBaseUI.Cast(m_wLootStorage.FindHandler(SCR_InventoryStorageLootUI));
						
					Widget closeButton = m_pStorageLootUI.GetRootWidget().FindAnyWidget("CloseStorageBtn");
					if(closeButton) {
						closeButton.SetVisible(false);
						closeButton.SetEnabled(false);
					}
																		
					return;		
				}		
			}		
		}		
			
		if (compact)
			m_wLootStorage.AddHandler( new SCR_InventoryStorageLootUI( null, null, this, 0, null, m_Player, 4, 6 ) );
		else
			m_wLootStorage.AddHandler( new SCR_InventoryStorageLootUI( null, null, this, 0, null, m_Player ) );
		m_pStorageLootUI = SCR_InventoryStorageBaseUI.Cast( m_wLootStorage.FindHandler( SCR_InventoryStorageLootUI ) );
	}	
	
	//----------------------------------------------------------------------------------
	// REAPER - Show compatible attachements from SCR_InventorySlotGearInspectionUI
	//----------------------------------------------------------------------------------
	
	void REAPER_ShowCompatibleAttachments(typename attachmentType, bool resetAllSlots = false)
	{	
		array<SCR_InventorySlotUI> slots = {};
		m_pStorageLootUI.GetSlots(slots);
		
		foreach (SCR_InventorySlotUI slot : slots)
		{
			if(slot) {
				
				if(resetAllSlots) {
					
					slot.SetEnabled(true);
					
				} else {
					
					InventoryItemComponent itemComp = slot.GetInventoryItemComponent();			
					if(itemComp) {
						
						ItemAttributeCollection itemAttributes = itemComp.GetAttributes();
						if(itemAttributes) {
												
							AttachmentAttributes itemAttribute = AttachmentAttributes.Cast(itemAttributes.FindAttribute(AttachmentAttributes));						
							if(itemAttribute && itemAttribute.GetAttachmentType()) {
								if(itemAttribute.GetAttachmentType().Type() == attachmentType) {
									slot.SetEnabled(true);
								} else {
									slot.SetEnabled(false);
								}		
							}				
						}
					}				
				}		
			}
		}
	}
	
	//----------------------------------------------------------------------------------
	// REAPER - Filter compatible Magazines
	//----------------------------------------------------------------------------------
	
	void REAPER_FilterVicinityMagazines()
	{	
		array<typename> playerMags = {}; 
		array<IEntity> playerWeapons = {};
		
		BaseWeaponManagerComponent weaponMgr = m_CharController.GetWeaponManagerComponent(); 
		if(weaponMgr) weaponMgr.GetWeaponsList(playerWeapons);
		
		foreach(IEntity weapon: playerWeapons) {
			MuzzleComponent muzzle = MuzzleComponent.Cast(weapon.FindComponent(MuzzleComponent)); 
			if(muzzle) playerMags.Insert(muzzle.GetMagazineWell().Type());
		}
		
		array<SCR_InventorySlotUI> slots = {};
		m_pStorageLootUI.GetSlots(slots);
		
		foreach (SCR_InventorySlotUI slot : slots)
		{
			if(slot) {
				
				InventoryItemComponent itemComp = slot.GetInventoryItemComponent();			
				if(itemComp) {
					
					MagazineComponent magComp = MagazineComponent.Cast(itemComp.GetOwner().FindComponent(MagazineComponent));
					if(magComp) {
					
						if(playerMags.Contains(magComp.GetMagazineWell().Type())) {
							slot.SetEnabled(true);
						} else {
							slot.SetEnabled(false);
						}
					}	
				}				
						
			}
		}
	}	
	
	//----------------------------------------------------------------------------------			

	override void ResetHighlightsOnAvailableStorages()
	{
		super.ResetHighlightsOnAvailableStorages(); 
		
		if(m_filterMagazines) REAPER_FilterVicinityMagazines();
	}
	
	//----------------------------------------------------------------------------------				
}







