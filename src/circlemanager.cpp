#include "circlemanager.h"

namespace WaitYourTurn
{
    void WaitYourTurn::CircleManager::StartCircling(CombatGroup *a_group)
    {
        CircleAll(a_group);
        WriteLocker locker(dataLock);
        auto& circleMembers = freeMemberMap.emplace(a_group->groupID, std::vector<FreeMember>()).first->second;
    }
    void CircleManager::StopCircling(CombatGroup *a_group)
    {
        FreeAll(a_group);
        WriteLocker locker(dataLock);
        freeMemberMap.erase(a_group->groupID);
    }
    void CircleManager::UpdateCircling(CombatGroup *a_group)
    {
        ReadLocker locker(dataLock);
        auto result = freeMemberMap.find(a_group->groupID);
        if (result == freeMemberMap.end())
        {
            locker.unlock();
            StartCircling(a_group);
            return;
        }
        locker.unlock();
        WriteLocker writeLocker(dataLock);
        float timeElapsed = GetSecondsSinceLastFrame();
        auto& circleMembers = result->second;
        for(size_t i = circleMembers.size(); i--;)
        {
            auto& member = circleMembers[i];
            member.timeRemaining -= timeElapsed;
            if (member.timeRemaining < 0.f)
            {
                member.Circle();
                circleMembers.erase(circleMembers.begin() + i);
            }
        }
        writeLocker.unlock();
        for(int n = circleMembers.size(); n < maxCircleMembers; n++)
        {
            FreeNewMember(a_group, circleMembers);
        }
    }
    void CircleManager::CircleAll(CombatGroup *a_group)
    {
        BSReadLockGuard lock(a_group->lock);
        auto& combatMembers = a_group->members;
        for(auto& combatMember : combatMembers)
        {
            auto* actor = combatMember.memberHandle.get().get();
            if (!actor) { continue; }
            PackageOverrideHook::AddOverride(actor->GetFormID());
        }
    }
    void CircleManager::FreeAll(CombatGroup *a_group)
    {
        BSReadLockGuard lock(a_group->lock);
        auto& combatMembers = a_group->members;
        for(auto& combatMember : combatMembers)
        {
            auto* actor = combatMember.memberHandle.get().get();
            if (!actor) { continue; }
            PackageOverrideHook::RemoveOverride(actor->GetFormID());
        }
    }
    void CircleManager::FreeNewMember(CombatGroup *a_group, std::vector<FreeMember> &circleMembers)
    {
        BSReadLockGuard lock(a_group->lock);
        auto& combatMembers = a_group->members;
        std::uniform_int_distribution<int> dist(0, a_group->members.size() - 1);
        auto index = dist(mt);
        auto formID = combatMembers[index].memberHandle.get()->GetFormID();

        WriteLocker locker(dataLock);
        circleMembers.emplace_back(FreeMember(formID, GetCircleDuration()));
    }
    float CircleManager::GetCircleDuration()
    {
        std::uniform_real_distribution<double> dist(0, 15.0);
        return dist(mt);
    }
    void CircleManager::FreeMember::Free()
    {
        PackageOverrideHook::RemoveOverride(formID); 
    }
    void CircleManager::FreeMember::Circle()
    {
        PackageOverrideHook::AddOverride(formID);
    }
}