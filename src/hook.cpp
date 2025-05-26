#include "hook.h"

namespace WaitYourTurn
{
    void PackageOverrideHook::RemoveOverride(RE::FormID actorId)
    {
        WriteLocker locker(dataLock);
        overrideActors.erase(actorId);
    }

    void PackageOverrideHook::AddOverride(RE::FormID actorId)
    {
        WriteLocker locker(dataLock);
        overrideActors.emplace(actorId);
    }

    TESPackage *PackageOverrideHook::GetOverridePackage(ExtraDataList *a_extraData, Actor *a_actor)
    {
        ReadLocker locker(dataLock);
        if (a_actor && a_actor->GetCombatGroup() && overrideActors.contains(a_actor->GetFormID()))
        {
            SKSE::log::info("Overriding {} : {:X}", a_actor->GetName(), a_actor->GetFormID());
            return circlePackage;
        }
        return _GetOverridePackage(a_extraData, a_actor);
    }
    void CombatGroupHook::Update(CombatGroup *a_group)
    {
        CircleManager::UpdateCircling(a_group);
        BSReadLockGuard readLock(a_group->lock);
        auto& members = a_group->members;
        for(auto& member : members)
        {
            auto* actor = member.memberHandle.get().get();
            if (!actor) { continue; }
            actor->EvaluatePackage();
            SKSE::log::info("Member {} : {:X}", actor->GetName(), actor->GetFormID());
        }
    }
    void CombatGroupHook::StopCombat(CombatGroup *a_group)
    {
        CircleManager::StopCircling(a_group);
    }
    CombatGroup *CombatGroupHook::Constructor(CombatGroup *a_group, uint32_t a_ind)
    {
        auto* result = _Constructor(a_group, a_ind);
        CircleManager::StartCircling(a_group);
        return result;
    }
    CombatGroup *CombatGroupHook::LoadGameConstructor(CombatGroup *a_group, uint32_t a_ind)
    {
        auto* result = _LoadGameConstructor(a_group, a_ind);
        CircleManager::StartCircling(a_group);
        return result;
    }
}