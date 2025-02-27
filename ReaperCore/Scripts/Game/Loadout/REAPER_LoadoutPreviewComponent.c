// --------------------------------------------------------------------------------------------------------------------
// ------           This script is part of REAPER CORE | an Arma Reforger SP/COOP Modification                   ------
// ------   You are not allowed to use this script or parts of it in your mod or pass it off as your own work.   ------
// ------                                                                                                        ------
// ------                         Written by REAPER 2024 - www.reaper-as.de                                      ------
// --------------------------------------------------------------------------------------------------------------------



//--------------------------------------------------------------------------------------------------------
// Modded, set the old character body as the new one! Also: Show all Weapon attachments
//--------------------------------------------------------------------------------------------------------

modded class SCR_LoadoutPreviewComponent : ScriptedWidgetComponent
{
	
	//-------------------------------------------------------------------------------------------------------------------
		
	override IEntity SetPreviewedLoadout(notnull SCR_BasePlayerLoadout loadout, PreviewRenderAttributes attributes = null)
	{
		if (!m_bReloadLoadout) return null;
		
		ChimeraWorld world = GetGame().GetWorld();
		m_PreviewManager = world.GetItemPreviewManager();

		if(!m_PreviewManager) {
			Resource res = Resource.Load(m_sPreviewManager);
			if(res.IsValid()) GetGame().SpawnEntityPrefabLocal(res, world);
			
			m_PreviewManager = world.GetItemPreviewManager();
			if(!m_PreviewManager) return null;
		}
				
		ResourceName resName = loadout.GetLoadoutResource();
		
		if (SCR_PlayerArsenalLoadout.Cast(loadout))
		{					
			IEntity previewedEntity = m_PreviewManager.ResolvePreviewEntityForPrefab(resName);
			if (!previewedEntity) return previewedEntity;
			
			SCR_ArsenalManagerComponent arsenalManager;
			if (!SCR_ArsenalManagerComponent.GetArsenalManager(arsenalManager)) return previewedEntity;
			
			SCR_PlayerLoadoutData loadoutData = arsenalManager.m_LocalPlayerLoadoutData;
			if (!loadoutData) return previewedEntity;
			
			//Load Character from Loadout! To get the same Identity 
			ResourceName loadourChar = loadoutData.characterPrefab; 
			if(!loadourChar.IsEmpty()) {
						
				IEntity previewedEntityNew = m_PreviewManager.ResolvePreviewEntityForPrefab(loadourChar);
								
				if(!previewedEntityNew) {
					return previewedEntity;	
				} else {
					previewedEntity = previewedEntityNew;
				}
			}		
					
			DeleteChildrens(previewedEntity, false);		
			
			// Equip the cloth first, to get the Weapon Offset!
			EquipedLoadoutStorageComponent loadoutStorage = EquipedLoadoutStorageComponent.Cast(previewedEntity.FindComponent(EquipedLoadoutStorageComponent));
			if (loadoutStorage)
			{		
				for (int i = 0; i < loadoutData.Clothings.Count(); ++i)
				{
					InventoryStorageSlot slot = loadoutStorage.GetSlot(loadoutData.Clothings[i].SlotIdx);
					if (!slot) continue;
					
					Resource resource = Resource.Load(loadoutData.Clothings[i].ClothingPrefab);
					if (!resource) continue;
					
					IEntity cloth = GetGame().SpawnEntityPrefabLocal(resource, previewedEntity.GetWorld());
					if (!cloth) continue;
					
					slot.AttachEntity(cloth);
				}
			}			
			
			// Spawn Weapons to Slot
			EquipedWeaponStorageComponent weaponStorage = EquipedWeaponStorageComponent.Cast(previewedEntity.FindComponent(EquipedWeaponStorageComponent));
			IEntity previewWeaponEntity;
			if (weaponStorage)
			{
				for (int i = 0; i < loadoutData.Weapons.Count(); ++i)
				{
					InventoryStorageSlot slot = weaponStorage.GetSlot(loadoutData.Weapons[i].SlotIdx);
					if(!slot) continue;
								
					Resource resource = Resource.Load(loadoutData.Weapons[i].WeaponPrefab);
					if (!resource) continue;					
								
					previewWeaponEntity = GetGame().SpawnEntityPrefabLocal(resource, previewedEntity.GetWorld());
					if(!previewWeaponEntity) continue;
													
					slot.AttachEntity(previewWeaponEntity);					
				}
				
				
				//If primary slot (index 0) add Attachments from Loadout
				InventoryStorageSlot primSlot = weaponStorage.GetSlot(0);
				
				if(primSlot && primSlot.GetAttachedEntity()) { 
					BaseWeaponComponent pwComp = BaseWeaponComponent.Cast(primSlot.GetAttachedEntity().FindComponent(BaseWeaponComponent));
					if(pwComp) {	
						
						array<GenericComponent> attachmentSlots = {};
						pwComp.FindComponents(AttachmentSlotComponent, attachmentSlots);			
								
						for(int i = 0; i < loadoutData.Attachments.Count(); ++i) {
							
							int slotIndex = loadoutData.Attachments[i].SlotIdx;
							
							AttachmentSlotComponent aSlot = AttachmentSlotComponent.Cast(attachmentSlots[slotIndex]);
							
							if(aSlot) {
								
								Resource attachRes = Resource.Load(loadoutData.Attachments[i].AttachmentPrefab);
								if (!attachRes) continue;
													
								IEntity attachEnt = GetGame().SpawnEntityPrefabLocal(attachRes, previewedEntity.GetWorld());
								if(!attachEnt) continue;	
								
								aSlot.SetAttachment(attachEnt);						
							
							}									
						}
						
						
						//Fix for suppressors in Reforger 1.2.1
						BaseMuzzleComponent muzzleComp = BaseMuzzleComponent.Cast(pwComp.FindComponent(BaseMuzzleComponent));
						if(muzzleComp) {
							
							array<GenericComponent> attachmentMuzzles = {};
							muzzleComp.FindComponents(AttachmentSlotComponent, attachmentMuzzles);
							
							if(attachmentMuzzles.Count() > 0) {
								for (int i = 0; i < attachmentMuzzles.Count(); ++i) {
																																					
									int slotIndex = loadoutData.AttachmentMuzzles[i].SlotIdx;
													
									AttachmentSlotComponent aSlot = AttachmentSlotComponent.Cast(attachmentMuzzles[slotIndex]);
									
									if(aSlot) {
										
										Resource attachRes = Resource.Load(loadoutData.AttachmentMuzzles[i].AttachmentMuzzlePrefab);
										if (!attachRes) continue;	
															
										IEntity attachEnt = GetGame().SpawnEntityPrefabLocal(attachRes, previewedEntity.GetWorld());
										if(!attachEnt) continue;									
										
										aSlot.SetAttachment(attachEnt);								
									}									
									
								}						
							}
						}						
						
						
					}
												
				}					
			}
			
			// Select Weapon
			BaseWeaponManagerComponent weaponManager = BaseWeaponManagerComponent.Cast(previewedEntity.FindComponent(BaseWeaponManagerComponent));
			if (weaponManager)
			{
				int weaponDefaultIndex = weaponManager.GetDefaultWeaponIndex();
				if (weaponDefaultIndex > -1)
				{
					array<WeaponSlotComponent> outSlots = {};
					weaponManager.GetWeaponsSlots(outSlots);
					foreach (WeaponSlotComponent weaponSlot: outSlots)
					{
						if (weaponSlot.GetWeaponSlotIndex() == weaponDefaultIndex)
						{
							weaponManager.SelectWeapon(weaponSlot);
							break;
						}
					}
				}
			}			
				
			m_PreviewManager.SetPreviewItem(m_wPreview, previewedEntity, attributes, true);
			return previewedEntity;
		}
		else
		{
			m_PreviewManager.SetPreviewItemFromPrefab(m_wPreview, resName, attributes);
			return m_PreviewManager.ResolvePreviewEntityForPrefab(resName);
		}
	}

		
}