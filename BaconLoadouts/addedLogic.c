bool LoadValidPlayerLoadout(int playerId, string factionKey, int slotId) {
    Bacon_GunBuilder_PlayerLoadout playerLoadout;
    if (!GetLoadout(factionKey, slotId, playerLoadout)) {
        Print("Loadout not found for slot", LogLevel.ERROR);
        return false;
    }

    // Separate valid and invalid items
    string[] validClothes = FilterValidItems(playerLoadout.metadata_clothes);
    string[] validWeapons = FilterValidItems(playerLoadout.metadata_weapons);

    foreach (string item in validClothes) {
        if (!LoadItem(item)) {
            Print(string.Format("Failed to load clothing item: %1", item), LogLevel.WARNING);
        }
    }
    
    foreach (string item in validWeapons) {
        if (!LoadItem(item)) {
            Print(string.Format("Failed to load weapon item: %1", item), LogLevel.WARNING);
        }
    }
    
    return true;
}

string[] FilterValidItems(string metadata) {
    array<string> items = metadata.Split("\n");
    array<string> validItems = {};
    
    foreach (string item in items) {
        if (IsItemValid(item)) {
            validItems.Insert(item);
        }
    }
    return validItems;
}

bool IsItemValid(string item) {
    // Implement logic to check if the item exists or if the mod is loaded
    return FileIO.FileExists(string.Format("$mod:/%1", item));
}

bool LoadItem(string itemName) {
    // Logic to load item into player's inventory
    return true;
}
