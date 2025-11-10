#include "circlemanager.h"

namespace WaitYourTurn
{
    using HEAD_TRACK = HighProcessData::HEAD_TRACK_TYPES::HEAD_TRACK_TYPE;
    void CircleManager::CircleMember::StopCircling()
    {
        PackageOverrideHook::RemoveOverride(formID); 
        auto id = formID;
        auto* tasks = SKSE::GetTaskInterface();
        if (!tasks) { return; }
        tasks->AddTask([id]() 
        {
            auto* actor = TESForm::LookupByID<Actor>(id);
            if (!actor || actor->IsDead() || actor->IsDisabled() || !actor->IsAIEnabled()) { return; }
            SKSE::log::info("{} stopped circling", actor->GetName());
            auto *high = actor->GetHighProcess();
            if (high)
            {
                high->pathLookAtTarget = ActorHandle();
            }
            actor->EvaluatePackage(true, false);

            auto *extraDataList = &actor->extraList;
            if (!extraDataList)
            {
                return;
            }

            auto *linkedRefs = extraDataList->GetByType<ExtraLinkedRef>();
            if (!linkedRefs)
            {
                return;
            }
            size_t removeIndex = 0;
            for (auto it = linkedRefs->linkedRefs.begin(); it != linkedRefs->linkedRefs.end(); it++)
            {
                auto &linkedRef = *it;
                if (linkedRef.keyword && linkedRef.keyword->GetFormID() == circleTargetKeyword->GetFormID())
                {
                    it = linkedRefs->linkedRefs.erase(it);
                    break;
                }
            }
        });
    }
    void CircleManager::CircleMember::StartCircling()
    {
        PackageOverrideHook::AddOverride(formID);
        auto id = formID;
        auto* tasks = SKSE::GetTaskInterface();
        if (!tasks) { return; }
        tasks->AddTask([id]() 
        {
            auto* actor = TESForm::LookupByID<Actor>(id);
            if (!actor || actor->IsDead()) { return; }
            SKSE::log::info("{} started circling", actor->GetName());
            actor->EvaluatePackage(true, false);
        });
    }
    CircleManager::CircleMember::~CircleMember()
    {
        StopCircling();
    }
    void CircleManager::UpdateTarget(FormID actorID)
    {
        ReadLocker readLocker(dataLock);
        auto result = circleGroupMap.find(actorID);
        if (result == circleGroupMap.end())
        {
            return;
        }
        auto& group = result->second;
        group.Update(GetSecondsSinceLastFrame());
    }
    void CircleManager::ChangeTarget(FormID targetID, FormID lastTargetID, FormID combatMemberID)
    {
        WriteLocker writeLocker(dataLock);
        auto result = circleGroupMap.find(targetID);
        CircleGroup* newCircleGroup;
        if (result == circleGroupMap.end())
        {
            result = circleGroupMap.emplace(targetID, CircleGroup()).first;
            result->second.circlerMap.emplace(combatMemberID, combatMemberID);
            // return;
        }
        newCircleGroup = &result->second;
        auto lastResult = circleGroupMap.find(lastTargetID);
        if (lastResult != circleGroupMap.end())
        {
            auto& oldCircleGroup = lastResult->second;
            if (oldCircleGroup.attackerMap.contains(combatMemberID)) 
            {
                newCircleGroup->attackerMap.insert(oldCircleGroup.attackerMap.extract(combatMemberID));
            }
            else if (oldCircleGroup.circlerMap.contains(combatMemberID))
            {
                newCircleGroup->circlerMap.insert(oldCircleGroup.circlerMap.extract(combatMemberID));
            }
        }
        if (result->second.attackerMap.contains(combatMemberID) || result->second.circlerMap.contains(combatMemberID)) { return; }
        result->second.circlerMap.emplace(combatMemberID, combatMemberID);
        SKSE::log::info("Changed {:X} from target {:X} to target {:X}", combatMemberID, lastTargetID, targetID);
    }
    void CircleManager::AddTarget(FormID targetID, FormID combatMemberID)
    {
        WriteLocker writeLocker(dataLock);
        auto result = circleGroupMap.find(targetID);
        CircleGroup* newCircleGroup;
        if (result == circleGroupMap.end())
        {
            result = circleGroupMap.emplace(targetID, CircleGroup()).first;
        }
        newCircleGroup = &result->second;
        if (result->second.attackerMap.contains(combatMemberID) || result->second.circlerMap.contains(combatMemberID)) { return; }
        SetupTarget(targetID, combatMemberID);
        result->second.circlerMap.emplace(combatMemberID, combatMemberID);
        SKSE::log::info("Added combatant {:X} to target {:X}", combatMemberID, targetID);
    }
    void CircleManager::SetupTarget(FormID targetID, FormID combatMemberID)
    {
        auto* actor = TESForm::LookupByID<Actor>(combatMemberID);
        auto* target = TESForm::LookupByID<Actor>(targetID);
        if (!target || !actor) { return; }

        auto *extraDataList = &actor->extraList;
        if (!extraDataList)
        {
            return;
        }

        auto *linkedRefs = extraDataList->GetByType<ExtraLinkedRef>();
        if (!linkedRefs)
        {
            linkedRefs = BSExtraData::Create<ExtraLinkedRef>();
            extraDataList->Add(linkedRefs);
        }
        for(auto& linkedRef : linkedRefs->linkedRefs)
        {
            if (linkedRef.keyword && linkedRef.keyword->GetFormID() == circleTargetKeyword->GetFormID())
            {
                linkedRef.refr = target;
                return;
            }
        }
        ExtraLinkedRef::LinkedRef linkedRef;
        linkedRef.keyword = circleTargetKeyword;
        linkedRef.refr = target;
        linkedRefs->linkedRefs.emplace_back(linkedRef); 
    }
    bool CircleManager::IsBeingCircled(Actor *a_target)
    {
        ReadLocker readLocker(dataLock);
        return circleGroupMap.contains(a_target->GetFormID());
    }
    bool CircleManager::CanCircle(Actor *a_target, Actor *a_combatant)
    {
        return !a_combatant->GetActorRuntimeData().boolBits.any(Actor::BOOL_BITS::kSearchingInCombat) 
        && (!a_combatant->IsPlayerRef()) 
        && (a_target->IsPlayerRef() 
        || ((!Settings::GetCircling().bPlayerOnly 
            || (a_combatant->IsPlayerTeammate() && Settings::GetCircling().bIncludeFollowers)) 
            && !(a_target->GetCurrentScene() 
                && a_target->GetCurrentScene()->isPlaying 
                && !a_target->GetCurrentScene()->flags.any(BGSScene::Flag::kInterruptible))))
        && !IsRangedOrMagic(a_combatant) 
        && IsHumanoid(a_combatant) 
        && !(a_combatant->GetCurrentScene() 
            && a_combatant->GetCurrentScene()->isPlaying 
            && !a_combatant->GetCurrentScene()->flags.any(BGSScene::Flag::kInterruptible))
        && !(a_target->GetCurrentScene() 
            && a_target->GetCurrentScene()->isPlaying 
            && !a_target->GetCurrentScene()->flags.any(BGSScene::Flag::kInterruptible));
    }
    void CircleManager::StopAllCircling(Actor *a_target)
    {
        if (!a_target)
        {
            return;
        }
        CircleManager::RemoveTarget(a_target->GetFormID());
        auto *combatGroup = a_target->GetCombatGroup();
        if (!combatGroup)
        {
            return;
        }
        RE::BSWriteLockGuard locker(combatGroup->lock);
        for (auto &target : combatGroup->targets)
        {
            auto *targetActor = target.targetHandle.get().get();
            if (!targetActor || !CircleManager::IsBeingCircled(targetActor))
            {
                continue;
            }
            CircleManager::RemoveCombatant(targetActor->GetFormID(), a_target->GetFormID());
        }
    }
    void CircleManager::RemoveTarget(FormID targetID)
    {
        WriteLocker writeLocker(dataLock);
        if (circleGroupMap.erase(targetID) > 0)
        {
            SKSE::log::info("Removed target {:X}", targetID);
        }
    }
    void CircleManager::RemoveCombatant(FormID targetID, FormID actorID)
    {
        WriteLocker writeLocker(dataLock);
        auto result = circleGroupMap.find(targetID);
        if (result == circleGroupMap.end())
        {
            SKSE::log::info("Target {:X} not found to remove combatant {:X}", targetID, actorID);
            return;
        }
        auto& group = result->second;
        WriteLocker groupLocker(group.lock);
        group.attackerMap.erase(actorID);
        group.circlerMap.erase(actorID);
    }
    void CircleManager::AllowCombatantDefense(FormID targetID, FormID actorID)
    {
        WriteLocker writeLocker(dataLock);
        auto result = circleGroupMap.find(targetID);
        if (result == circleGroupMap.end())
        {
            SKSE::log::info("Target {:X} not found to defend combatant {:X}", targetID, actorID);
            return;
        }
        auto& group = result->second;
        group.SetDefender(actorID);
    }
    void CircleManager::AllowAttackers(FormID targetID, bool state)
    {
        ReadLocker readLocker(dataLock);
        auto result = circleGroupMap.find(targetID);
        if (result == circleGroupMap.end())
        {
            SKSE::log::info("Target {:X} not found to lock attackers", targetID);
            return;
        }
        auto& group = result->second;
        group.AllowAttackers(state);
    }
    bool CircleManager::GetAllowAttackers(FormID targetID)
    {
        ReadLocker readLocker(dataLock);
        auto result = circleGroupMap.find(targetID);
        if (result == circleGroupMap.end())
        {
            SKSE::log::info("Target {:X} not found to get allowed attackers", targetID);
            return true;
        }
        auto& group = result->second; 
        return group.GetAllowAttackers(); 
    }
    void CircleManager::LoadCircleGroups()
    {
        auto* player = PlayerCharacter::GetSingleton();
        if (player && player->GetCombatGroup())
        {
            auto* combatGroup = player->GetCombatGroup();
            BSReadLockGuard locker(combatGroup->lock);
            for(auto& target : combatGroup->targets)
            {
                auto targetPtr = target.targetHandle.get();
                if (targetPtr && player->IsCombatTarget(targetPtr.get()) && CircleManager::CanCircle(player, targetPtr.get()))
                {
                    CircleManager::AddTarget(player->GetFormID(), targetPtr->GetFormID());
                }
            }
        }
        auto processLists = ProcessLists::GetSingleton();
        for(auto actorHandle : processLists->highActorHandles)
        {
            auto actorPtr = actorHandle.get();
            if (!actorPtr) { continue; }
            auto* combatController = actorPtr->GetActorRuntimeData().combatController;
            if (!combatController) { continue; }
            auto targetPtr = combatController->attackerHandle.get() ? 
            combatController->attackerHandle.get() : combatController->cachedAttacker ? 
            combatController->cachedAttacker : actorPtr->GetActorRuntimeData().currentCombatTarget.get();
            if (!targetPtr) 
            { 
                continue; 
            }
            if (CanCircle(targetPtr.get(), actorPtr.get()))
            {
                AddTarget(targetPtr->GetFormID(), actorPtr->GetFormID());
            }
        }
    }
    float CircleManager::GetAttackerDuration()
    {
        std::uniform_real_distribution<double> dist(Settings::GetCircling().fMinFollowUpSeconds, Settings::GetCircling().fMaxFollowUpSeconds);
        return dist(mt);
    }
    float CircleManager::GetDefenderDuration()
    {
        std::uniform_real_distribution<double> dist(Settings::GetCircling().fMinDefenseSeconds, Settings::GetCircling().fMaxDefenseSeconds);
        return dist(mt);
    }
    bool CircleManager::IsRangedOrMagic(Actor *a_actor)
    {
        if (Settings::GetCircling().bIncludeRangedMagic) { return false; }
        auto* process = a_actor->GetActorRuntimeData().currentProcess;
        if (!process) { return false; }

        auto* left_item = process->GetEquippedLeftHand();
        auto* right_item = process->GetEquippedRightHand();

        auto left_type = GetEquippedItemType(left_item);
        auto right_type = GetEquippedItemType(right_item);
        return IsMagicItemType(left_type) || IsMagicItemType(right_type) || IsRangedItemType(left_type) || IsRangedItemType(right_type);
    }
    void CircleManager::CircleGroup::SetAttacker(FormID formID)
    {
        WriteLocker locker(lock);
        auto iter = circlerMap.find(formID);
        if (iter == circlerMap.end())
        {
            return;
        }
        iter->second.StopCircling();
        iter->second.timeRemaining = GetAttackerDuration();
        SKSE::log::info("New attacker {:X} for {}", iter->second.formID, iter->second.timeRemaining);
        attackerMap.insert(circlerMap.extract(iter));
    }
    void CircleManager::CircleGroup::SetDefender(FormID formID)
    {
        WriteLocker locker(lock);
        auto iter = circlerMap.find(formID);
        if (iter == circlerMap.end())
        {
            return;
        }
        iter->second.StopCircling();
        iter->second.timeRemaining = GetDefenderDuration();
        SKSE::log::info("New defender {:X} for {}", iter->second.formID, iter->second.timeRemaining);
        attackerMap.insert(circlerMap.extract(iter));
    }
    void CircleManager::CircleGroup::UnsetAttacker(FormID formID)
    {
        WriteLocker locker(lock);
        auto iter = attackerMap.find(formID);
        if (iter == attackerMap.end())
        {
            return;
        }
        iter->second.StartCircling();
        SKSE::log::info("Stopped attacker {:X}", iter->second.formID);
        circlerMap.insert(attackerMap.extract(iter));
    }
    void CircleManager::CircleGroup::Update(float a_delta)
    {
        ReadLocker readLocker(lock);
        std::vector<FormID> idsToRemove;
        for(auto& kvp : attackerMap)
        {
            kvp.second.timeRemaining -= a_delta;
            if (kvp.second.timeRemaining <= 0.f)
            {
                idsToRemove.emplace_back(kvp.first);
            }
        }
        readLocker.unlock();
        for(auto id : idsToRemove)
        {
            UnsetAttacker(id);
        }
        readLocker.lock();
        size_t maxAttackers = Settings::GetCircling().iMaxAttackers;
        if (!allowAttackers || maxAttackers <= attackerMap.size()) { return; }
        std::vector<size_t> indices(circlerMap.size());
        for(auto& kvp : circlerMap)
        {
            indices.emplace_back(kvp.first);
        }
        std::shuffle(indices.begin(), indices.end(), mt);
        readLocker.unlock();
        for(int i = attackerMap.size(); i < maxAttackers && i < indices.size(); i++)
        {
            SetAttacker(indices.back());
            indices.pop_back();
        }
    }
    void CircleManager::CircleGroup::AllowAttackers(bool state)
    {
        WriteLocker locker(lock);
        allowAttackers = state;
        if (!allowAttackers)
        {
            std::vector<FormID> idsToRemove;
            for(auto& kvp : attackerMap)
            {
                idsToRemove.emplace_back(kvp.first);
            }
            locker.unlock();
            for(auto id : idsToRemove)
            {
                UnsetAttacker(id);
            }
        }
    }
}