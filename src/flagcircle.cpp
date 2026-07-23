#include "flagcircle.h"
#include "settings.h"
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
            if (Settings::GetCircling().bDebugDisplay)
            {
                if (auto *mesh = actor->GetHeadPartObject(RE::BGSHeadPart::HeadPartType::kFace))
                {
                    mesh->TintScenegraph(RE::NiColorA(0.f, 0.f, 0.f, 0.f));
                }
            }
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

            if (Settings::GetCircling().bDebugDisplay)
            {
                if (auto *mesh = actor->GetHeadPartObject(RE::BGSHeadPart::HeadPartType::kFace))
                {
                    mesh->TintScenegraph(RE::NiColorA(0.f, 1.f, 0.f, 20.f));
                }
            }
        });
    }
}