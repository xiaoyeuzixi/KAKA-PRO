#pragma once
#include <DMALibrary/Memory/Memory.h>
#include <Common/Data.h>
#include <Common/Entitys.h>
#include <Utils/Utils.h>
#include <Utils/Throttler.h>
#include <Hack/GNames.h>
#include <Utils/FNVHash.h>
#include <algorithm>

class Actors
{
public:
    static void Update()
    {
        auto hScatter = mem.CreateScatterHandle();
        Throttler Throttlered;

        while (true)
        {
            if (GameData.Scene != Scene::Gaming)
            {
                Sleep(GameData.ThreadSleep);
                continue;
            }

            Throttlered.executeTask("UpdateCacheEntitys", std::chrono::milliseconds(2000), [] {
                Data::SetCacheEntitys({});

                });



            std::vector<ActorEntityInfo> Entitys;
            TArray<uint64_t> ActorArray = mem.Read<TArray<uint64_t>>(GameData.ActorArray);
            const auto ActorList = ActorArray.GetVector();
            Entitys.reserve(ActorList.size());


            for (auto Actor : ActorList)
            {
                ActorEntityInfo Entity;
                Entity.Index = 0;
                Entity.Entity = Actor;
                Entity.DecodeID = 0;
                Entity.ID = 0;
                Entitys.emplace_back(Entity);
            }

            std::unordered_map<uint64_t, ActorEntityInfo> CacheEntitys = Data::GetCacheEntitys();
            std::unordered_map<uint64_t, Player> CachePlayers;
            std::unordered_map<uint64_t, VehicleInfo> CacheVehicles;
            std::unordered_map<uint64_t, DroppedItemInfo> CacheDroppedItems;
            std::unordered_map<uint64_t, DroppedItemGroupInfo> CacheDroppedItemGroups;
            std::unordered_map<uint64_t, ProjectInfo> CacheProjects;
            std::unordered_map<uint64_t, PackageInfo> CachePackages;
            std::vector<int> NeedGetNameIDs;
            const auto ReserveCount = ActorList.size();
            CachePlayers.reserve(ReserveCount);
            CacheVehicles.reserve(ReserveCount);
            CacheDroppedItems.reserve(ReserveCount);
            CacheDroppedItemGroups.reserve(ReserveCount);
            CacheProjects.reserve(ReserveCount);
            CachePackages.reserve(ReserveCount);
            NeedGetNameIDs.reserve(ReserveCount);



            for (ActorEntityInfo& Entity : Entitys)
            {
                const auto cacheIt = CacheEntitys.find(Entity.Entity);
                if (cacheIt != CacheEntitys.end())
                {
                    const auto& CachedEntity = cacheIt->second;
                    if (CachedEntity.DecodeID > 0 && CachedEntity.DecodeID < 1000000 && CachedEntity.EntityInfo.Type != EntityType::Player && CachedEntity.EntityInfo.Type != EntityType::AI)
                    {
                        Entity = CachedEntity;
                        continue;
                    }
                }

                mem.AddScatterRead(hScatter, Entity.Entity + GameData.Offset["ObjID"], (int*)&Entity.ID);
            }
            mem.ExecuteReadScatter(hScatter);

            for (ActorEntityInfo& Entity : Entitys)
            {
                if (!Entity.DecodeID) {
                    Entity.DecodeID = Decrypt::CIndex(Entity.ID);
                }

                EntityInfo EntityInfo = Data::GetGNameListsByIDItem(Entity.DecodeID);

                ActorEntityInfo NewActorEntityInfo;
                NewActorEntityInfo.Entity = Entity.Entity;
                NewActorEntityInfo.ID = Entity.ID;
                NewActorEntityInfo.DecodeID = Entity.DecodeID;
                NewActorEntityInfo.EntityInfo = EntityInfo;

                CacheEntitys[Entity.Entity] = NewActorEntityInfo;

                if (EntityInfo.ID == 0) {
                    NeedGetNameIDs.push_back(Entity.DecodeID);
                }
                else {

                    if (EntityInfo.Type == EntityType::Player || EntityInfo.Type == EntityType::AI || EntityInfo.Name.find("Zombie") != std::string::npos) {
                        Player Player;
                        Player.Type = EntityInfo.Type;
                        Player.Entity = Entity.Entity;
                        Player.ObjID = Entity.DecodeID;

                        CachePlayers[Player.Entity] = Player;
                    }

                    if (EntityInfo.Type == EntityType::Project) {
                        ProjectInfo Project;
                        Project.Type = EntityInfo.Type;
                        Project.Entity = Entity.Entity;
                        Project.ID = Entity.DecodeID;
                        Project.Name = EntityInfo.DisplayName;
                        Project.EntityName = EntityInfo.Name;

                        CacheProjects[Project.Entity] = Project;
                    }
                    if (EntityInfo.Type == EntityType::Unknown)
                    {
                        if (EntityInfo.Name.find("Panzer") != std::string::npos ||
                            EntityInfo.Name.find("Faust") != std::string::npos ||
                            EntityInfo.Name.find("Rocket") != std::string::npos)
                        {
                            ProjectInfo Project;
                            Project.Type = EntityType::Project;
                            Project.Entity = Entity.Entity;
                            Project.ID = Entity.DecodeID;
                            Project.Name = EntityInfo.DisplayName;
                            Project.EntityName = EntityInfo.Name;
                            CacheProjects[Project.Entity] = Project;
                        }
                    }
                    ////��ȡδ֪ʵ����Ϣ
                    //if (EntityInfo.Type == EntityType::Unknown &&
                    //    EntityInfo.Name.find("BP_") == 0) {
                    //    Utils::Log(1, "DLC-CAR ClassName: %s",
                    //        EntityInfo.Name.c_str());
                    //}

                    if (EntityInfo.Type == EntityType::Vehicle) {
                        VehicleInfo Vehicle;
                        Vehicle.Type = EntityInfo.Type;
                        Vehicle.Entity = Entity.Entity;
                        Vehicle.ObjID = Entity.DecodeID;
                        Vehicle.Name = EntityInfo.DisplayName;

                        CacheVehicles[Vehicle.Entity] = Vehicle;
                    }

                    if (EntityInfo.Type == EntityType::DroppedItem) {
                        DroppedItemInfo DroppedItem;
                        DroppedItem.Type = EntityInfo.Type;
                        DroppedItem.Entity = Entity.Entity;
                        DroppedItem.ID = Entity.DecodeID;

                        CacheDroppedItems[DroppedItem.Entity] = DroppedItem;
                    }

                    if (EntityInfo.Type == EntityType::DroppedItemGroup) {
                        DroppedItemGroupInfo DroppedItemGroup;
                        DroppedItemGroup.Type = EntityInfo.Type;
                        DroppedItemGroup.Entity = Entity.Entity;
                        DroppedItemGroup.ID = Entity.DecodeID;

                        CacheDroppedItemGroups[DroppedItemGroup.Entity] = DroppedItemGroup;
                    }

                    if (EntityInfo.Type == EntityType::DeadBox || EntityInfo.Type == EntityType::AirDrop)
                    {
                        PackageInfo Package;
                        Package.Type = EntityInfo.Type;
                        Package.Entity = Entity.Entity;
                        Package.ID = Entity.DecodeID;

                        CachePackages[Package.Entity] = Package;
                    }
                }
            }

            Data::SetCachePlayers(std::move(CachePlayers));
            Data::SetCacheVehicles(std::move(CacheVehicles));
            Data::SetCacheEntitys(std::move(CacheEntitys));
            Data::SetCacheDroppedItems(std::move(CacheDroppedItems));
            Data::SetCacheDroppedItemGroups(std::move(CacheDroppedItemGroups));
            Data::SetCacheProjects(std::move(CacheProjects));
            Data::SetCachePackages(std::move(CachePackages));

            if (NeedGetNameIDs.size() > 0) {
                GNames::ReadGNames(NeedGetNameIDs);
            }

            Sleep(30);
        }
        mem.CloseScatterHandle(hScatter);
    }
};
