class SCR_WeaponLoadoutData
{
	int SlotIdx;
	ResourceName WeaponPrefab;
}

class SCR_ClothingLoadoutData
{
	int SlotIdx;
	ResourceName ClothingPrefab;
}

class SCR_PlayerLoadoutData
{
    ref array<ref SCR_ClothingLoadoutData> Clothings = {};
	ref array<ref SCR_WeaponLoadoutData> Weapons = {};
	float LoadoutCost;
	int FactionIndex;

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] instance
	//! \param[in] ctx
	//! \param[in] snapshot
	//! \return
    static bool Extract(SCR_PlayerLoadoutData instance, ScriptCtx ctx, SSnapSerializerBase snapshot)
    {
        // Fill a snapshot with values from an instance.
		
		int clothingCount = instance.Clothings.Count();
		snapshot.SerializeInt(clothingCount);
		
		for (int i = 0; i < clothingCount; ++i)
		{
			snapshot.SerializeInt(instance.Clothings[i].SlotIdx);
			
			string resourceName = instance.Clothings[i].ClothingPrefab;
			snapshot.SerializeString(resourceName);
		}
		
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

        return true;
    }
 
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] snapshot
	//! \param[in] ctx
	//! \param[in] instance
	//! \return
    static bool Inject(SSnapSerializerBase snapshot, ScriptCtx ctx, SCR_PlayerLoadoutData instance)
    {
        // Fill an instance with values from snapshot.
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
		
        return true;
    }
 
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] snapshot
	//! \param[in] ctx
	//! \param[in] packet
	//! \return
    static void Encode(SSnapSerializerBase snapshot, ScriptCtx ctx, ScriptBitSerializer packet)
    {
		int clothingCount;
		snapshot.SerializeBytes(clothingCount, 4);
		packet.Serialize(clothingCount, 32);
		
		for (int i = 0; i < clothingCount; ++i)
		{
			snapshot.EncodeInt(packet);
			snapshot.EncodeString(packet);
		}
		
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
    }
 
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] packet
	//! \param[in] ctx
	//! \param[in] snapshot
	//! \return
    static bool Decode(ScriptBitSerializer packet, ScriptCtx ctx, SSnapSerializerBase snapshot)
    {
		int clothingCount;
		packet.Serialize(clothingCount, 32);
		snapshot.SerializeBytes(clothingCount, 4);
		
		for (int i = 0; i < clothingCount; ++i)
		{
			snapshot.DecodeInt(packet);
			snapshot.DecodeString(packet);
		}
		
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

        return true;
    }
 
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] lhs
	//! \param[in] rhs
	//! \param[in] ctx
	//! \return
	static bool SnapCompare(SSnapSerializerBase lhs, SSnapSerializerBase rhs, ScriptCtx ctx)
    {
		Print("Cannot use SCR_PlayerLoadoutData as a property", LogLevel.ERROR);
        return true;
    }

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] instance
	//! \param[in] snapshot
	//! \param[in] ctx
	//! \return
    static bool PropCompare(SCR_PlayerLoadoutData instance, SSnapSerializerBase snapshot, ScriptCtx ctx)
    {
        Print("Cannot use SCR_PlayerLoadoutData as a property", LogLevel.ERROR);
        return true;
    }
}
