#include "packagecircle.h"
#include "hook.h"
#include "event.h"
namespace WaitYourTurn
{
    void PackageCircle::Load()
    {
        PackageOverrideHook::Load();
        circleTargetKeyword = FormUtil::Parse::GetFormFromMod(0x802, "WaitYourTurnRedux.esp")->As<BGSKeyword>(); 
        PackageEventHandler::Register();
    }
    void PackageCircle::SetupCircling(RE::FormID targetID, RE::FormID combatMemberID)
    {
        auto *actor = TESForm::LookupByID<Actor>(combatMemberID);
        auto *target = TESForm::LookupByID<Actor>(targetID);
        if (!target || !actor)
        {
            return;
        }

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
        for (auto &linkedRef : linkedRefs->linkedRefs)
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
    void PackageCircle::StartCircling(RE::FormID combatMemberID)
    {
        PackageOverrideHook::AddOverride(combatMemberID);
        auto id = combatMemberID;
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
    void PackageCircle::StopCircling(RE::FormID combatMemberID)
    {
        PackageOverrideHook::RemoveOverride(combatMemberID); 
        auto id = combatMemberID;
        auto* tasks = SKSE::GetTaskInterface();
        if (!tasks) { return; }
        auto* keyword = circleTargetKeyword;
        tasks->AddTask([id, keyword]() 
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
                if (linkedRef.keyword && linkedRef.keyword->GetFormID() == keyword->GetFormID())
                {
                    it = linkedRefs->linkedRefs.erase(it);
                    break;
                }
            }
        });
    }
}