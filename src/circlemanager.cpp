#include "circlemanager.h"

namespace WaitYourTurn
{
    using HEAD_TRACK = HighProcessData::HEAD_TRACK_TYPES::HEAD_TRACK_TYPE;
    CircleManager::CircleMember::~CircleMember()
    {
        circleHandler->StopCircling(formID);
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
        //SKSE::log::info("Changed {:X} from target {:X} to target {:X}", combatMemberID, lastTargetID, targetID);
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
        circleHandler->SetupCircling(combatMemberID, targetID);
        result->second.circlerMap.emplace(combatMemberID, combatMemberID);
        //SKSE::log::info("Added combatant {:X} to target {:X}", combatMemberID, targetID);
    }
    bool CircleManager::IsBeingCircled(Actor *a_target)
    {
        if (!a_target) { return false; }
        ReadLocker readLocker(dataLock);
        return circleGroupMap.contains(a_target->GetFormID());
    }
    bool CircleManager::CanCircle(Actor *a_target, Actor *a_combatant)
    {
        if (!a_target || !a_combatant) { return false; }
        return !a_combatant->GetActorRuntimeData().boolBits.any(Actor::BOOL_BITS::kSearchingInCombat)
        && (!a_combatant->GetActorRuntimeData().combatController ||
                (!a_combatant->GetActorRuntimeData().combatController->IsFleeing()
                && !a_combatant->GetActorRuntimeData().combatController->ignoringCombat))
        && (!a_combatant->IsPlayerRef()) 
        && (a_target->IsPlayerRef() 
        || (!Settings::GetCircling().bPlayerOnly 
            || (a_combatant->IsPlayerTeammate() && Settings::GetCircling().bIncludeFollowers)))
        && !IsRangedOrMagic(a_combatant) 
        && IsHumanoid(a_combatant); 
        // && !(a_combatant->GetCurrentScene() 
        //     && a_combatant->GetCurrentScene()->isPlaying 
        //     && !a_combatant->GetCurrentScene()->flags.any(BGSScene::Flag::kInterruptible))
        // && !(a_target->GetCurrentScene() 
        //     && a_target->GetCurrentScene()->isPlaying 
        //     && !a_target->GetCurrentScene()->flags.any(BGSScene::Flag::kInterruptible));
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
    void CircleManager::StopAllCircling(Actor *a_target, CombatGroup *a_group)
    {
        if (!a_target)
        {
            return;
        }
        CircleManager::RemoveTarget(a_target->GetFormID());
        if (!a_group)
        {
            return;
        }
        RE::BSWriteLockGuard locker(a_group->lock);
        for (auto &target : a_group->targets)
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
            //SKSE::log::info("Removed target {:X}", targetID);
        }
    }
    void CircleManager::RemoveCombatant(FormID targetID, FormID actorID)
    {
        WriteLocker writeLocker(dataLock);
        auto result = circleGroupMap.find(targetID);
        if (result == circleGroupMap.end())
        {
            //SKSE::log::info("Target {:X} not found to remove combatant {:X}", targetID, actorID);
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
            //SKSE::log::info("Target {:X} not found to defend combatant {:X}", targetID, actorID);
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
            //SKSE::log::info("Target {:X} not found to lock attackers", targetID);
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
            //SKSE::log::info("Target {:X} not found to get allowed attackers", targetID);
            return true;
        }
        auto& group = result->second; 
        return group.GetAllowAttackers(); 
    }
    bool CircleManager::IsTargetCombatantPair(FormID targetID, FormID combatMemberID)
    {
        ReadLocker readLocker(dataLock);
        auto result = circleGroupMap.find(targetID);
        if (result == circleGroupMap.end())
        {
            return false;
        }
        auto& group = result->second;
        ReadLocker groupLocker(group.lock);
        return group.circlerMap.contains(combatMemberID) || group.attackerMap.contains(combatMemberID);
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
        if (!processLists) { return; }
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
        if (!a_actor || Settings::GetCircling().bIncludeRangedMagic) { return false; }
        auto* process = a_actor->GetActorRuntimeData().currentProcess;
        if (!process) { return false; }

        auto* left_item = process->GetEquippedLeftHand();
        auto* right_item = process->GetEquippedRightHand();

        uint16_t left_type  = left_item  ? GetEquippedItemType(left_item)  : 0;
        uint16_t right_type = right_item ? GetEquippedItemType(right_item) : 0;
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
        circleHandler->StopCircling(iter->second.formID);
        iter->second.timeRemaining = GetAttackerDuration();
       //SKSE::log::info("New attacker {:X} for {}", iter->second.formID, iter->second.timeRemaining);
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
        circleHandler->StopCircling(iter->second.formID);
        iter->second.timeRemaining = GetDefenderDuration();
        //SKSE::log::info("New defender {:X} for {}", iter->second.formID, iter->second.timeRemaining);
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
        circleHandler->StartCircling(iter->second.formID);
        //SKSE::log::info("Stopped attacker {:X}", iter->second.formID);
        circlerMap.insert(attackerMap.extract(iter));
    }
    void CircleManager::CircleGroup::Update(float a_delta)
    {
        // Phase 1: decrement attacker timers under a write lock (we are mutating
        // timeRemaining, so a read lock would be a data race). Collect expired IDs.
        std::vector<FormID> idsToRemove;
        {
            WriteLocker writeLocker(lock);
            for(auto& kvp : attackerMap)
            {
                kvp.second.timeRemaining -= a_delta;
                if (kvp.second.timeRemaining <= 0.f)
                {
                    idsToRemove.emplace_back(kvp.first);
                }
            }
        }

        // Phase 2: promote expired attackers back to circlers. UnsetAttacker takes
        // its own write lock, so we must not hold ours when calling it.
        for(auto id : idsToRemove)
        {
            UnsetAttacker(id);
        }

        // Phase 3: snapshot the circler FormIDs and the promotion gate under a read
        // lock. We cannot call SetAttacker while holding ours (it takes its own
        // write lock and std::shared_mutex is not recursive).
        std::vector<FormID> indices;
        bool                shouldPromote   = false;
        size_t              maxAttackers    = 0;
        size_t              currentAttackers = 0;
        {
            ReadLocker readLocker(lock);
            maxAttackers     = Settings::GetCircling().iMaxAttackers;
            currentAttackers = attackerMap.size();
            if (allowAttackers && maxAttackers > currentAttackers && !circlerMap.empty())
            {
                indices.reserve(circlerMap.size());
                for(const auto& kvp : circlerMap)
                {
                    indices.emplace_back(kvp.first);
                }
                shouldPromote = true;
            }
        }

        if (!shouldPromote) { return; }

        std::shuffle(indices.begin(), indices.end(), mt);

        // Phase 4: promote circlers to attackers to fill remaining slots. SetAttacker
        // re-validates that the FormID is still a circler, so it is safe even if a
        // concurrent thread removed it between our snapshot and the call.
        size_t needed = maxAttackers - currentAttackers;
        for(size_t i = 0; i < needed && i < indices.size(); i++)
        {
            SetAttacker(indices[i]);
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