#include "flagcircle.h"

namespace WaitYourTurn
{
    void FlagCircle::StartCircling(RE::FormID combatMemberID)
    {
        auto id = combatMemberID;
        auto *tasks = SKSE::GetTaskInterface();
        if (!tasks)
        {
            return;
        }
        tasks->AddTask([id]()
        {
            auto* actor = RE::TESForm::LookupByID<RE::Actor>(id);
            if (!actor) { return; }
            actor->GetActorRuntimeData().boolFlags.set(RE::Actor::BOOL_FLAGS::kAttackingDisabled); 
            actor->GetActorRuntimeData().boolFlags.set(RE::Actor::BOOL_FLAGS::kCastingDisabled); 
        });
    }
    void FlagCircle::StopCircling(RE::FormID combatMemberID)
    {
        auto id = combatMemberID;
        auto *tasks = SKSE::GetTaskInterface();
        if (!tasks)
        {
            return;
        }
        tasks->AddTask([id]()
        {
            auto* actor = RE::TESForm::LookupByID<RE::Actor>(id);
            if (!actor) { return; }
            actor->GetActorRuntimeData().boolFlags.reset(RE::Actor::BOOL_FLAGS::kAttackingDisabled); 
            actor->GetActorRuntimeData().boolFlags.reset(RE::Actor::BOOL_FLAGS::kCastingDisabled); 
        });
    }
}