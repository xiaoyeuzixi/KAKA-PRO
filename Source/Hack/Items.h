#pragma once
#include <DMALibrary/Memory/Memory.h>
#include <Common/Data.h>
#include <Common/Entitys.h>
#include <Utils/Utils.h>
#include <Utils/Throttler.h>
#include <Hack/GNames.h>
#include <Hack/Decrypt.h>

class Items
{
public:
    static std::unordered_map<int, int> GetWeaponAttachedItems(VMMDLL_SCATTER_HANDLE hScatter)
    {
        struct FWeaponAttachmentItem
        {
            uint64_t Item;
            uint64_t WeaponAttachmentData;
            int ItemID;
            EntityInfo EntityInfo;
        };

        uint64_t CurrentWeapon = GameData.LocalPlayerInfo.CurrentWeapon;
        std::vector<FWeaponAttachmentItem> WeaponAttachmentItems;
        std::unordered_map<int, int> WeaponAttachmentItemID;

        //手持过滤
        if (!GameData.Config.Item.HandheldWeaponFilter)
        {

        }

        //头甲过滤
        if (!GameData.Config.Item.FilterHelmets)
        {

        }
        // 处理背包物品数据
        {
            uint64_t InventoryFacade = Decrypt::Xe(mem.Read<uint64_t>(GameData.AcknowledgedPawn + GameData.Offset["InventoryFacade"]));
            uint64_t Inventory = mem.Read<uint64_t>(InventoryFacade + GameData.Offset["Inventory"]);
            auto InventoryItems = mem.Read<TArray<uint64_t>>(Inventory + GameData.Offset["InventoryItems"]);
            struct InventoryItemInfo
            {
                uint64_t Item;
                uint64_t ItemTable;
                int ItemID;
                int Count;
            };

            std::unordered_map<uint64_t, InventoryItemInfo> CacheInventoryItemInfos;
            std::unordered_map<uint64_t, InventoryItemInfo> InventoryItemInfos;

            for (auto& Item : InventoryItems.GetVector())
            {
                InventoryItemInfo ItemInfo;
                ItemInfo.Item = Item;
                CacheInventoryItemInfos[Item] = ItemInfo;
            }

            for (auto& Item : CacheInventoryItemInfos)
            {
                mem.AddScatterRead(hScatter, Item.first + GameData.Offset["ItemTable"], (uint64_t*)&Item.second.ItemTable);
                mem.AddScatterRead(hScatter, Item.first + GameData.Offset["InventoryItemTagItemCount"], (uint64_t*)&Item.second.Count);
            }
            mem.ExecuteReadScatter(hScatter);
            for (auto& Item : CacheInventoryItemInfos)
            {
                mem.AddScatterRead(hScatter, Item.second.ItemTable + GameData.Offset["ItemID"], (int*)&Item.second.ItemID);
            }
            mem.ExecuteReadScatter(hScatter);

            //背包物品上限过滤
            if (GameData.Config.Item.ItemLimit)
            {
                for (auto& Item : CacheInventoryItemInfos)
                {
                    if (Item.second.ItemID == 0) continue;

                    EntityInfo EntityInfo = Data::GetGNameListsByIDItem(Item.second.ItemID);

                    // 绷带 
                    if (EntityInfo.Name == "Item_Heal_Bandage_C" && Item.second.Count >= GameData.Config.ItemFiltering.Bandage)
                    {
                        WeaponAttachmentItemID[Item.second.ItemID] = Item.second.ItemID;
                    }

                    //急救包
                    if (EntityInfo.Name == "Item_Heal_FirstAid_C" && Item.second.Count >= GameData.Config.ItemFiltering.FirstAidKit)
                    {
                        WeaponAttachmentItemID[Item.second.ItemID] = Item.second.ItemID;
                    }

                    //医疗箱
                    if (EntityInfo.Name == "Item_Heal_MedKit_C" && Item.second.Count >= GameData.Config.ItemFiltering.MedicalKit)
                    {
                        WeaponAttachmentItemID[Item.second.ItemID] = Item.second.ItemID;
                    }

                    //止痛药
                    if (EntityInfo.Name == "Item_Boost_PainKiller_C" && Item.second.Count >= GameData.Config.ItemFiltering.Painkiller)
                    {
                        WeaponAttachmentItemID[Item.second.ItemID] = Item.second.ItemID;
                    }

                    //能量饮料
                    if (EntityInfo.Name == "Item_Boost_EnergyDrink_C" && Item.second.Count >= GameData.Config.ItemFiltering.EnergyDrink)
                    {
                        WeaponAttachmentItemID[Item.second.ItemID] = Item.second.ItemID;
                    }

                    //肾上腺素
                    if (EntityInfo.Name == "Item_Boost_AdrenalineSyringe_C" && Item.second.Count >= GameData.Config.ItemFiltering.Adrenaline)
                    {
                        WeaponAttachmentItemID[Item.second.ItemID] = Item.second.ItemID;
                    }

                    //C4
                    if (EntityInfo.Name == "Item_Weapon_C4_C" && Item.second.Count >= GameData.Config.ItemFiltering.C4)
                    {
                        WeaponAttachmentItemID[Item.second.ItemID] = Item.second.ItemID;
                    }

                    //手榴弹
                    if (EntityInfo.Name == "Item_Weapon_Grenade_C" && Item.second.Count >= GameData.Config.ItemFiltering.Grenade)
                    {
                        WeaponAttachmentItemID[Item.second.ItemID] = Item.second.ItemID;
                    }

                    //烟雾弹
                    if (EntityInfo.Name == "Item_Weapon_SmokeBomb_C" && Item.second.Count >= GameData.Config.ItemFiltering.SmokeGrenade)
                    {
                        WeaponAttachmentItemID[Item.second.ItemID] = Item.second.ItemID;
                    }

                    //闪光弹
                    if (EntityInfo.Name == "Item_Weapon_FlashBang_C" && Item.second.Count >= GameData.Config.ItemFiltering.Flashbang)
                    {
                        WeaponAttachmentItemID[Item.second.ItemID] = Item.second.ItemID;
                    }

                    //燃烧瓶
                    if (EntityInfo.Name == "Item_Weapon_Molotov_C" && Item.second.Count >= GameData.Config.ItemFiltering.Molotov)
                    {
                        WeaponAttachmentItemID[Item.second.ItemID] = Item.second.ItemID;
                    }

                    //篮圈手雷
                    if (EntityInfo.Name == "Item_Weapon_BluezoneGrenade_C" && Item.second.Count >= GameData.Config.ItemFiltering.BlueZoneGrenade)
                    {
                        WeaponAttachmentItemID[Item.second.ItemID] = Item.second.ItemID;
                    }

                    //粘性炸弹
                    if (EntityInfo.Name == "Item_Weapon_StickyGrenade_C" && Item.second.Count >= GameData.Config.ItemFiltering.StickyBomb)
                    {
                        WeaponAttachmentItemID[Item.second.ItemID] = Item.second.ItemID;
                    }

                }
            }
        }
        //配件过滤
        if (GameData.Config.Item.AccessoriesFilter)
        {
            if (!Utils::ValidPtr(CurrentWeapon))
            {
                TArray<uint64_t> AttachedItems = mem.Read<TArray<uint64_t>>(CurrentWeapon + GameData.Offset["AttachedItems"]);
                for (const auto& Item : AttachedItems.GetVector())
                {
                    if (Utils::ValidPtr(Item)) continue;
                    FWeaponAttachmentItem WeaponAttachmentItem;
                    WeaponAttachmentItem.Item = Item;
                    WeaponAttachmentItems.emplace_back(WeaponAttachmentItem);
                }

                if (WeaponAttachmentItems.size() > 0)
                {
                    for (auto& Item : WeaponAttachmentItems)
                    {
                        mem.AddScatterRead(hScatter, Item.Item + GameData.Offset["WeaponAttachmentData"], &Item.WeaponAttachmentData);
                    }

                    mem.ExecuteReadScatter(hScatter);

                    for (auto& Item : WeaponAttachmentItems)
                    {
                        mem.AddScatterRead(hScatter, Item.WeaponAttachmentData + GameData.Offset["ItemID"], &Item.ItemID);
                    }

                    mem.ExecuteReadScatter(hScatter);

                    std::vector<int> NeedGetNameIDs;
                    for (auto& Item : WeaponAttachmentItems)
                    {
                        WeaponAttachmentItemID[Item.ItemID] = Item.ItemID;

                    }
                }
            }

        }


        return WeaponAttachmentItemID;
    }

    static void Update()
    {
        auto hScatter = mem.CreateScatterHandle();
        std::unordered_map<uint64_t, ItemInfo> CacheItems;
        Throttler Throttlered;
        while (true)
        {
            if (GameData.Scene != Scene::Gaming)
            {
                CacheItems.clear();
                Data::SetItems({});
                Sleep(GameData.ThreadSleep);
                continue;
            }

            if (!GameData.Config.Item.Enable)
            {
                CacheItems.clear();
                Data::SetItems({});
                Sleep(200);
                continue;
            }

            std::unordered_map<int, int> WeaponAttachmentItemID = GetWeaponAttachedItems(hScatter);

            Throttlered.executeTask("UpdateItemsData", std::chrono::milliseconds(2000), [&hScatter, &CacheItems, &WeaponAttachmentItemID] {
                //Timer time("Update");

                std::unordered_map<uint64_t, DroppedItemInfo> CacheDroppedItems = Data::GetCacheDroppedItems();
                std::unordered_map<uint64_t, DroppedItemGroupInfo> CacheDroppedItemGroups = Data::GetCacheDroppedItemGroups();

                //物品组
                {
                    for (auto& Item : CacheDroppedItemGroups)
                    {
                        mem.AddScatterRead(hScatter, Item.second.Entity + GameData.Offset["DroppedItemGroup"], (uint64_t*)&Item.second.pDroppedItemGroup);
                        mem.AddScatterRead(hScatter, Item.second.Entity + GameData.Offset["DroppedItemGroup"] + 0x8, (int*)&Item.second.Count);
                    }

                    mem.ExecuteReadScatter(hScatter);

                    for (auto& Item : CacheDroppedItemGroups)
                    {
                        if (Item.second.Count <= 0 || Item.second.Count > 100)
                        {
                            continue;
                        }

                        for (int i = 0; i < Item.second.Count; i++)
                        {
                            DroppedItemGroupItemInfo DroppedItemGroupItem;
                            DroppedItemGroupItem.pItemGroupComponent = Item.second.pDroppedItemGroup + i * 0x10;
                            Item.second.Items.push_back(DroppedItemGroupItem);
                        }
                    }

                    mem.ExecuteReadScatter(hScatter);

                    for (auto& Item : CacheDroppedItemGroups)
                    {
                        for (auto& DroppedItemGroupItem : Item.second.Items)
                        {
                            mem.AddScatterRead(hScatter, DroppedItemGroupItem.pItemGroupComponent, (uint64_t*)&DroppedItemGroupItem.ItemGroupComponent);
                        }
                    }

                    mem.ExecuteReadScatter(hScatter);

                    for (auto& Item : CacheDroppedItemGroups)
                    {
                        for (auto& DroppedItemGroupItem : Item.second.Items)
                        {
                            mem.AddScatterRead(hScatter, DroppedItemGroupItem.ItemGroupComponent + GameData.Offset["DroppedItemGroupUItem"], (uint64_t*)&DroppedItemGroupItem.ItemGroupUItem);
                        }
                    }

                    mem.ExecuteReadScatter(hScatter);

                    for (auto& Item : CacheDroppedItemGroups)
                    {
                        for (auto& DroppedItemGroupItem : Item.second.Items)
                        {
                            mem.AddScatterRead(hScatter, DroppedItemGroupItem.ItemGroupUItem + GameData.Offset["ItemTable"], (uint64_t*)&DroppedItemGroupItem.ItemTable);
                        }
                    }

                    mem.ExecuteReadScatter(hScatter);

                    for (auto& Item : CacheDroppedItemGroups)
                    {
                        for (auto& DroppedItemGroupItem : Item.second.Items)
                        {
                            mem.AddScatterRead(hScatter, DroppedItemGroupItem.ItemTable + GameData.Offset["ItemID"], (int*)&DroppedItemGroupItem.ItemID);
                        }
                    }

                    mem.ExecuteReadScatter(hScatter);
                }

                //物品列
                {
                    for (auto& Item : CacheDroppedItems)
                    {
                        mem.AddScatterRead(hScatter, Item.second.Entity + GameData.Offset["DroppedItem"], (uint64_t*)&Item.second.pDroppedItem);
                        mem.AddScatterRead(hScatter, Item.second.Entity + GameData.Offset["RootComponent"], (uint64_t*)&Item.second.RootComponent);
                    }

                    mem.ExecuteReadScatter(hScatter);

                    for (auto& Item : CacheDroppedItems)
                    {
                        Item.second.RootComponent = Decrypt::Xe(Item.second.RootComponent);
                        Item.second.pDroppedItem = Decrypt::Xe(Item.second.pDroppedItem);
                        mem.AddScatterRead(hScatter, Item.second.pDroppedItem + GameData.Offset["ItemTable"], (uint64_t*)&Item.second.ItemTable);
                    }

                    mem.ExecuteReadScatter(hScatter);

                    for (auto& Item : CacheDroppedItems)
                    {
                        mem.AddScatterRead(hScatter, Item.second.ItemTable + GameData.Offset["ItemID"], (int*)&Item.second.ID);
                    }

                    mem.ExecuteReadScatter(hScatter);
                }

                std::unordered_map<uint64_t, ItemInfo> AddCacheItems;

                for (auto& Item : CacheDroppedItemGroups)
                {
                    for (auto& DroppedItemGroupItem : Item.second.Items)
                    {
                        if (WeaponAttachmentItemID.count(DroppedItemGroupItem.ItemID) > 0)
                        {
                            continue;
                        }

                        if (DroppedItemGroupItem.ItemID == 0) continue;

                        ItemInfo ItemInfo;
                        ItemInfo.Entity = DroppedItemGroupItem.pItemGroupComponent;
                        ItemInfo.ID = DroppedItemGroupItem.ItemID;
                        ItemInfo.RootComponent = DroppedItemGroupItem.ItemGroupComponent;
                        ItemInfo.ItemTable = DroppedItemGroupItem.ItemTable;

                        AddCacheItems[ItemInfo.Entity] = ItemInfo;
                    }
                }

                for (auto& Item : CacheDroppedItems)
                {
                    if (WeaponAttachmentItemID.count(Item.second.ID) > 0)
                    {
                        continue;
                    }

                    if (Item.second.ID == 0) continue;

                    ItemInfo ItemInfo;
                    ItemInfo.Entity = Item.second.Entity;
                    ItemInfo.ID = Item.second.ID;
                    ItemInfo.RootComponent = Item.second.RootComponent;
                    ItemInfo.ItemTable = Item.second.ItemTable;

                    AddCacheItems[ItemInfo.Entity] = ItemInfo;
                }

                CacheItems.clear();
                std::vector<int> NeedGetNameIDs;
                for (auto& Item : AddCacheItems)
                {
                    EntityInfo EntityInfo = Data::GetGNameListsByIDItem(Item.second.ID);

                    //printf(U8("EntityInfo [%s]  [%s]  [%d]\n"), EntityInfo.DisplayName.c_str(), EntityInfo.Name.c_str(), Item.second.ID);

                    if (EntityInfo.ID == 0)
                    {
                        NeedGetNameIDs.push_back(Item.second.ID);
                    }
                    else {
                        if (EntityInfo.Type != EntityType::Item) continue;

                        Item.second.Type = EntityInfo.Type;
                        Item.second.DisplayName = EntityInfo.DisplayName;
                        Item.second.Name = EntityInfo.Name;
                        Item.second.ItemType = EntityInfo.WeaponType;

                        CacheItems[Item.second.Entity] = Item.second;
                    }
                }

                if (NeedGetNameIDs.size() > 0) {
                    GNames::ReadGNames(NeedGetNameIDs);
                }

                //std::cout << time.get() << std::endl;
                });

            for (auto& Item : CacheItems)
            {
                if (WeaponAttachmentItemID.count(Item.second.ID) > 0)
                {
                    Item.second.bHidden = true;
                    continue;
                }
                mem.AddScatterRead(hScatter, Item.second.RootComponent + GameData.Offset["ComponentLocation"], (FVector*)&Item.second.Location);
            }

            mem.ExecuteReadScatter(hScatter);

            for (auto& Item : CacheItems)
            {
                Item.second.ScreenLocation = VectorHelper::WorldToScreen(Item.second.Location);
                Item.second.Distance = GameData.Camera.Location.Distance(Item.second.Location) / 100.0f;
            }

            Data::SetItems(CacheItems);

            Sleep(200);
        }
        mem.CloseScatterHandle(hScatter);
    }
};