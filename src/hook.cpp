#include "hook.h"

namespace WaitYourTurn
{
    void PackageOverrideHook::ClearOverrides()
    {
        WriteLocker locker(dataLock);
        overrideActors.clear();
    }

    void PackageOverrideHook::RemoveOverride(RE::FormID actorId)
    {
        WriteLocker locker(dataLock);
        SKSE::log::info("Removed package override for {:X}", actorId);
        overrideActors.erase(actorId);
    }

    void PackageOverrideHook::AddOverride(RE::FormID actorId)
    {
        WriteLocker locker(dataLock);
        SKSE::log::info("Added package override for {:X}", actorId);
        overrideActors.emplace(actorId);
    }

    bool PackageOverrideHook::HasOverride(FormID actorId)
    {
        ReadLocker locker(dataLock);
        return overrideActors.contains(actorId);
    }

    TESPackage *PackageOverrideHook::GetOverridePackage(ExtraDataList *a_extraData, Actor *a_actor)
    {
        ReadLocker locker(dataLock);
        if (a_actor && overrideActors.contains(a_actor->GetFormID()))
        {
            SKSE::log::info("Returned package override for {:X}", a_actor->GetFormID());
            return circlePackage;
        }
        return _GetOverridePackage(a_extraData, a_actor);
    }
    void CombatGroupHook::NotifyMemberKilled(CombatGroup *a_group, Actor *a_member, Actor *a_killer)
    {
        _NotifyMemberKilled(a_group, a_killer, a_member);
        SKSE::log::info("Member {} ({:X}) killed by {} ({:X})", a_member->GetName(), a_member->GetFormID(), a_killer->GetName(), a_killer->GetFormID());
        if (CircleManager::IsBeingCircled(a_killer))
        {
            CircleManager::RemoveCombatant(a_killer->GetFormID(), a_member->GetFormID());
        }
        if (!a_group) { return; }
        BSReadLockGuard readLock(a_group->lock);
        auto& targets = a_group->targets;
        for(auto& target : targets)
        {
            auto* targetActor = target.targetHandle.get().get();
            if (!targetActor || !CircleManager::IsBeingCircled(targetActor)) { continue; }
            CircleManager::RemoveCombatant(targetActor->GetFormID(), a_member->GetFormID());
        }
    }
    void CombatGroupHook::NotifyMemberAttacked(CombatGroup *a_group, Actor *a_member, Actor *a_attacker)
    {
        _NotifyMemberAttacked(a_group, a_member, a_attacker);
        if (PackageOverrideHook::HasOverride(a_member->GetFormID()))
        {
            a_member->NotifyAnimationGraph("blockStart");
        }
    }
    void CombatGroupHook::Update(CombatGroup *a_group)
    {
        _Update(a_group);
        BSReadLockGuard readLock(a_group->lock);
        auto& members = a_group->members;
        for(auto& member : members)
        {
            auto* actor = member.memberHandle.get().get();
            if (!actor) { continue; }
            CircleManager::UpdateTarget(actor->GetFormID());

            // auto* combatController = actor->GetActorRuntimeData().combatController;
            // if (!combatController) { continue; }

            // auto cachedTarget = combatController->cachedTarget;
            // auto handledTarget = combatController->targetHandle.get();
            // auto defaultTarget = actor->GetActorRuntimeData().currentCombatTarget.get();
            // auto target = handledTarget ? handledTarget : cachedTarget ? cachedTarget : defaultTarget;
            // if (!target) { continue; }

            // auto* targetActor = target.get();
            // if (!targetActor) { continue; }
            // // SKSE::log::info("Target {} : Combatant {}", targetActor->GetName(), actor->GetName());
            // if (CircleManager::CanCircle(targetActor, actor))
            // {
            //     CircleManager::AddTarget(targetActor->GetFormID(), actor->GetFormID());
            // }


            
            // actor->EvaluatePackage();
            // SKSE::log::info("Member {} : {:X}", actor->GetName(), actor->GetFormID());
        }

    }
    void CombatGroupHook::StopCombat(CombatGroup *a_group)
    {
        BSReadLockGuard readLock(a_group->lock);
        auto& members = a_group->members;
        for(auto& member : members)
        {
            auto* actor = member.memberHandle.get().get();
            if (!actor) { continue; }
            CircleManager::RemoveTarget(actor->GetFormID());
            // actor->EvaluatePackage();
            // SKSE::log::info("Member {} : {:X}", actor->GetName(), actor->GetFormID());
        }
        // CircleManager::StopCircling(a_group);
    }
    void CombatGroupHook::SetCombatGroup(Actor *a_actor, CombatGroup *a_group)
    {
        if (a_actor)
        {
            if (!a_group)
            {
                CircleManager::RemoveTarget(a_actor->GetFormID());
            }
            else
            {
                SKSE::log::info("Scanning new combat group for {}", a_actor->GetName());
                BSReadLockGuard readLock(a_group->lock);
                auto& targets = a_group->targets; 
                for(auto& target : targets)
                {
                    auto targetPtr = target.targetHandle.get();
                    if (!targetPtr) { continue; }
                    auto* targetActor = targetPtr.get();
                    if (!targetActor) { continue; }
        
                    if (targetActor->IsCombatTarget(a_actor) && CircleManager::CanCircle(targetActor, a_actor))
                    {
                        CircleManager::AddTarget(targetActor->GetFormID(), a_actor->GetFormID());
                    }
        
                    if (a_actor->IsCombatTarget(targetActor) && CircleManager::CanCircle(a_actor, targetActor))
                    {
                        CircleManager::AddTarget(a_actor->GetFormID(), targetActor->GetFormID());
                    }
                }
            }
            
        }
        return _SetCombatGroup(a_actor, a_group);
    }
    void CombatControllerHook::SetTarget(CombatController *a_controller, Actor *a_target)
    {
        auto combatant = a_controller->attackerHandle.get() ? a_controller->attackerHandle.get() : a_controller->cachedAttacker;
        auto oldTarget = a_controller->targetHandle.get() ? a_controller->targetHandle.get() : a_controller->cachedTarget;
        _SetTarget(a_controller, a_target);

        auto newTarget = a_controller->targetHandle.get() ? a_controller->targetHandle.get() : a_controller->cachedTarget;

        if (combatant && newTarget)
        {
            if (!CircleManager::CanCircle(newTarget.get(), combatant.get())) { return; }
            if (oldTarget && newTarget->GetFormID() != oldTarget->GetFormID())
            {
                SKSE::log::info("Set target combatant {}", combatant->GetName());
                CircleManager::ChangeTarget(newTarget->GetFormID(), oldTarget->GetFormID(), combatant->GetFormID());
                return;
            }
            SKSE::log::info("Add target combatant {}", combatant->GetName());
            CircleManager::AddTarget(newTarget->GetFormID(), combatant->GetFormID());
        }
        // if (a_controller->cachedAttacker && a_controller->cachedTarget)
        // {
        //     CircleManager::UpdateTargetChange(a_controller)
        // }
    }
    void CombatControllerHook::SetTarget2(CombatController *a_controller, Actor *a_target)
    {
        auto combatant = a_controller->attackerHandle.get() ? a_controller->attackerHandle.get() : a_controller->cachedAttacker;
        auto oldTarget = a_controller->targetHandle.get() ? a_controller->targetHandle.get() : a_controller->cachedTarget;
        _SetTarget2(a_controller, a_target);

        auto newTarget = a_controller->targetHandle.get() ? a_controller->targetHandle.get() : a_controller->cachedTarget;

        if (combatant && newTarget)
        {
            if (!CircleManager::CanCircle(newTarget.get(), combatant.get())) { return; }
            if (oldTarget && newTarget->GetFormID() != oldTarget->GetFormID())
            {
                SKSE::log::info("Set target combatant {}", combatant->GetName());
                CircleManager::ChangeTarget(newTarget->GetFormID(), oldTarget->GetFormID(), combatant->GetFormID());
                return;
            }
            SKSE::log::info("Add target combatant {}", combatant->GetName());
            CircleManager::AddTarget(newTarget->GetFormID(), combatant->GetFormID());
        }
    }
    void CombatControllerHook::AttackedBy(CombatController *a_controller, Actor *a_target)
    {
        _AttackedBy(a_controller, a_target);
        if (a_target && CircleManager::IsBeingCircled(a_target))
        {
            a_target->NotifyAnimationGraph("blockStart");
        }
    }
}