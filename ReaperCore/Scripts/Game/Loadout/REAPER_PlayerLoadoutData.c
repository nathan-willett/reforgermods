

// --------------------------------------------------------------------------------------------------------------------
// ------           This script is part of REAPER CORE | an Arma Reforger SP/COOP Modification                   ------
// ------   You are not allowed to use this script or parts of it in your mod or pass it off as your own work.   ------
// ------                                                                                                        ------
// ------                         Written by REAPER 2024 - www.reaper-as.de                                      ------
// --------------------------------------------------------------------------------------------------------------------

// This file modifies SCR_PlayerLoadoutData to store additional information about weapon attachments and character prefabs.
// 
// It defines two new classes:
// 	-	SCR_WeaponAttachmentLoadoutData (stores the attachment slot index and prefab name).
// 	-	SCR_WeaponAttachmentMuzzleLoadoutData (stores muzzle attachment data).
// 
// The Extract and Inject methods handle serialization and deserialization of these new properties, ensuring the correct 
// loadout data is saved and loaded.
// 
// This file is crucial for managing stored loadouts, as it saves clothing, weapons, and attachments for a character.

// Modded to store WeaponAttachments and Character into SCR_PlayerLoadoutData 

class SCR_WeaponAttachmentLoadoutData
{
	int SlotIdx;
	ResourceName AttachmentPrefab;
}

class SCR_WeaponAttachmentMuzzleLoadoutData
{
	int SlotIdx;
	ResourceName AttachmentMuzzlePrefab;
}

modded class SCR_PlayerLoadoutData
{
	ref array<ref SCR_WeaponAttachmentLoadoutData> Attachments = {};
	ref array<ref SCR_WeaponAttachmentMuzzleLoadoutData> AttachmentMuzzles = {};
	ResourceName characterPrefab;
	
	// ----------------------------------------------------------------------------------------------------------------
	
	override static bool Extract(SCR_PlayerLoadoutData instance, ScriptCtx ctx, SSnapSerializerBase snapshot)
	{
		//Attachments
		int attachmentCount = instance.Attachments.Count();
		snapshot.SerializeInt(attachmentCount);
		
		for (int i = 0; i < attachmentCount; ++i)
		{
			snapshot.SerializeInt(instance.Attachments[i].SlotIdx);
			
			string resourceName = instance.Attachments[i].AttachmentPrefab;
			snapshot.SerializeString(resourceName);
		}

		//Muzzles
		int muzzleCount = instance.AttachmentMuzzles.Count();
		snapshot.SerializeInt(muzzleCount);
		
		for (int i = 0; i < muzzleCount; ++i)
		{
			snapshot.SerializeInt(instance.AttachmentMuzzles[i].SlotIdx);
			
			string resourceName = instance.AttachmentMuzzles[i].AttachmentMuzzlePrefab;
			snapshot.SerializeString(resourceName);
		}
				
		//Cloth
		int clothingCount = instance.Clothings.Count();
		snapshot.SerializeInt(clothingCount);
		
		for (int i = 0; i < clothingCount; ++i)
		{
			snapshot.SerializeInt(instance.Clothings[i].SlotIdx);
			
			string resourceName = instance.Clothings[i].ClothingPrefab;
			snapshot.SerializeString(resourceName);
		}
		
		//Weapons
		int weaponCount = instance.Weapons.Count();
		snapshot.SerializeInt(weaponCount);
		
		for (int i = 0; i < weaponCount; ++i)
		{
			snapshot.SerializeInt(instance.Weapons[i].SlotIdx);
			
			string resourceName = instance.Weapons[i].WeaponPrefab;
			snapshot.SerializeString(resourceName);
		}

		snapshot.SerializeFloat(instance.LoadoutCost);
		snapshot.SerializeInt(instance.FactionIndex);
		snapshot.SerializeString(instance.characterPrefab);
		
		return true;
	   	//return super.Extract(instance, ctx, snapshot);
	}
	
	// ----------------------------------------------------------------------------------------------------------------
	
	override static bool Inject(SSnapSerializerBase snapshot, ScriptCtx ctx, SCR_PlayerLoadoutData instance)
	{	
		//Attachments
		int attachmentCount;
		snapshot.SerializeInt(attachmentCount);
		instance.Attachments.Clear();
		
		for (int i = 0; i < attachmentCount; ++i)
		{
			SCR_WeaponAttachmentLoadoutData attachmentData();
			
			snapshot.SerializeInt(attachmentData.SlotIdx);
			
			string resourceName;
			snapshot.SerializeString(resourceName);
			attachmentData.AttachmentPrefab = resourceName;
			
			instance.Attachments.Insert(attachmentData);
		}
		
		//Muzzles
		int muzzleCount;
		snapshot.SerializeInt(muzzleCount);
		instance.AttachmentMuzzles.Clear();
		
		for (int i = 0; i < muzzleCount; ++i)
		{
			SCR_WeaponAttachmentMuzzleLoadoutData attachmentData();
			
			snapshot.SerializeInt(attachmentData.SlotIdx);
			
			string resourceName;
			snapshot.SerializeString(resourceName);
			attachmentData.AttachmentMuzzlePrefab = resourceName;			
			instance.AttachmentMuzzles.Insert(attachmentData);
		}
		
		//Cloth
		int clothingCount;
		snapshot.SerializeInt(clothingCount);
		instance.Clothings.Clear();
		
		for (int i = 0; i < clothingCount; ++i)
		{
			SCR_ClothingLoadoutData clothingData();
			
			snapshot.SerializeInt(clothingData.SlotIdx);
			
			string resourceName;
			snapshot.SerializeString(resourceName);
			clothingData.ClothingPrefab = resourceName;
			
			instance.Clothings.Insert(clothingData);
		}
		
		//Weapons
		int weaponCount;
		snapshot.SerializeInt(weaponCount);
		instance.Weapons.Clear();
		
		for (int i = 0; i < weaponCount; ++i)
		{
			SCR_WeaponLoadoutData weaponData();
			
			snapshot.SerializeInt(weaponData.SlotIdx);
			
			string resourceName;
			snapshot.SerializeString(resourceName);
			weaponData.WeaponPrefab = resourceName;
			
			instance.Weapons.Insert(weaponData);
		}

		snapshot.SerializeFloat(instance.LoadoutCost);
		snapshot.SerializeInt(instance.FactionIndex);
		snapshot.SerializeString(instance.characterPrefab);
		
        	return true;
	
		//return super.Inject(snapshot, ctx, instance);
	}
	
	// ----------------------------------------------------------------------------------------------------------------
	
	override static void Encode(SSnapSerializerBase snapshot, ScriptCtx ctx, ScriptBitSerializer packet)
	{	
		// Attachments
		int attachmentCount;
		snapshot.SerializeBytes(attachmentCount, 4);
		packet.Serialize(attachmentCount, 32);
		
		for (int i = 0; i < attachmentCount; ++i)
		{
			snapshot.EncodeInt(packet);
			snapshot.EncodeString(packet);
		}
		
		// Muzzles
		int muzzleCount;
		snapshot.SerializeBytes(muzzleCount, 4);
		packet.Serialize(muzzleCount, 32);
		
		for (int i = 0; i < muzzleCount; ++i)
		{
			snapshot.EncodeInt(packet);
			snapshot.EncodeString(packet);
		}
	
		//Cloth
		int clothingCount;
		snapshot.SerializeBytes(clothingCount, 4);
		packet.Serialize(clothingCount, 32);
		
		for (int i = 0; i < clothingCount; ++i)
		{
			snapshot.EncodeInt(packet);
			snapshot.EncodeString(packet);
		}
		
		//Weapons
		int weaponCount;
		snapshot.SerializeBytes(weaponCount, 4);
		packet.Serialize(weaponCount, 32);
		
		for (int i = 0; i < weaponCount; ++i)
		{
			snapshot.EncodeInt(packet);
			snapshot.EncodeString(packet);
		}

		snapshot.EncodeFloat(packet);
		snapshot.EncodeInt(packet);
		snapshot.EncodeString(packet);
				
		//super.Encode(snapshot, ctx, packet);
	}
 	
	// ----------------------------------------------------------------------------------------------------------------
	
	override static bool Decode(ScriptBitSerializer packet, ScriptCtx ctx, SSnapSerializerBase snapshot)
	{		
		//Attachments
		int attachmentCount;
		packet.Serialize(attachmentCount, 32);
		snapshot.SerializeBytes(attachmentCount, 4);
		
		for (int i = 0; i < attachmentCount; ++i)
		{
			snapshot.DecodeInt(packet);
			snapshot.DecodeString(packet);
		}

		//Muzzles
		int muzzleCount;
		packet.Serialize(muzzleCount, 32);
		snapshot.SerializeBytes(muzzleCount, 4);
		
		for (int i = 0; i < muzzleCount; ++i)
		{
			snapshot.DecodeInt(packet);
			snapshot.DecodeString(packet);
		}
				
		//Cloth
		int clothingCount;
		packet.Serialize(clothingCount, 32);
		snapshot.SerializeBytes(clothingCount, 4);
		
		for (int i = 0; i < clothingCount; ++i)
		{
			snapshot.DecodeInt(packet);
			snapshot.DecodeString(packet);
		}
		
		//Weapons
		int weaponCount;
		packet.Serialize(weaponCount, 32);
		snapshot.SerializeBytes(weaponCount, 4);
		
		for (int i = 0; i < weaponCount; ++i)
		{
			snapshot.DecodeInt(packet);
			snapshot.DecodeString(packet);
		}
		
		snapshot.DecodeFloat(packet);
		snapshot.DecodeInt(packet);
		snapshot.DecodeString(packet);
		
       	return true;

		//return super.Decode(packet, ctx, snapshot);
	}		
}