#include "event.h"
#include "hook.h"
namespace WaitYourTurn
{
    using Control = BSEventNotifyControl;
    using HEAD_TRACK = HighProcessData::HEAD_TRACK_TYPES::HEAD_TRACK_TYPE;
    using EventType = TESPackageEvent::Type;
    BSEventNotifyControl PackageEventHandler::ProcessEvent(const TESPackageEvent *a_event, BSTEventSource<TESPackageEvent> *a_eventSource)
    {
        if (a_event->target && circlePackage && a_event->packageFormID == circlePackage->GetFormID())
        {
            auto* actor = a_event->target->As<Actor>();
            if (!actor) { return Control::kContinue; }
            
            auto* target = actor->GetActorRuntimeData().currentCombatTarget.get().get();
            if (!target) { return Control::kContinue; }

            auto* process = actor->GetActorRuntimeData().currentProcess;
            if (!process) { return Control::kContinue; }

            auto* high = process->high;
            if (!high) { return Control::kContinue; }

            switch(a_event->eventType)
            {
                case EventType::kPackageStart:
                    SKSE::log::info("Overriding headtracking for {}", actor->GetName());
                    high->pathLookAtTarget = actor->GetActorRuntimeData().currentCombatTarget;
                    // high->SetHeadtrackTarget(HEAD_TRACK::kCombat, target);
                    break;
                case EventType::kPackageChange:
                    SKSE::log::info("Clearing headtracking for {}", actor->GetName());
                    high->pathLookAtTarget = ActorHandle();
                    break;
                case EventType::kPackageEnd:
                    SKSE::log::info("Clearing headtracking for {}", actor->GetName());
                    high->pathLookAtTarget = ActorHandle();
                    break;
            }

        }
        return BSEventNotifyControl::kContinue;
    }
    BSEventNotifyControl CombatEventHandler::ProcessEvent(const TESCombatEvent *a_event, BSTEventSource<TESCombatEvent> *a_eventSource)
    {
        auto actor = a_event->actor; 
        auto targetActor = a_event->targetActor;
        if (a_event->newState == ACTOR_COMBAT_STATE::kNone || a_event->newState == ACTOR_COMBAT_STATE::kSearching)
        {
            if (actor)
            {
                CircleManager::RemoveTarget(actor->GetFormID());
            }
            if (targetActor)
            {
                CircleManager::RemoveTarget(targetActor->GetFormID());
            }
            auto* player = PlayerCharacter::GetSingleton();
            
            if (player && !player->GetCombatGroup())
            {
                CircleManager::RemoveTarget(player->GetFormID());
            }
        }
        if (a_event->newState == ACTOR_COMBAT_STATE::kCombat)
        {
            if (actor && targetActor && actor && CircleManager::CanCircle(targetActor->As<Actor>(), actor->As<Actor>()))
            {
                CircleManager::AddTarget(targetActor->GetFormID(), actor->GetFormID());
            }
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
        }
        return Control::kContinue;
    }
    BSEventNotifyControl CellAttachDetachEventHandler::ProcessEvent(const TESCellAttachDetachEvent *a_event, BSTEventSource<TESCellAttachDetachEvent> *a_eventSource)
    {
        if (a_event->attached || !a_event->reference || a_event->reference->GetFormType() != FormType::ActorCharacter) { return BSEventNotifyControl::kContinue; }
        SKSE::log::info("Stopping circling for detached actor {}", a_event->reference->GetName());
        CircleManager::StopAllCircling(a_event->reference->As<Actor>());
        return BSEventNotifyControl::kContinue;
    }
    BSEventNotifyControl DeleteEventHandler::ProcessEvent(const TESFormDeleteEvent *a_event, BSTEventSource<TESFormDeleteEvent> *a_eventSource)
    {
        CircleManager::RemoveTarget(a_event->formID);
        return BSEventNotifyControl::kContinue;
    }
    BSEventNotifyControl DeathEventHandler::ProcessEvent(const TESDeathEvent *a_event, BSTEventSource<TESDeathEvent> *a_eventSource)
    {
        if (!a_event->actorDying) { return BSEventNotifyControl::kContinue; }
        auto* actor = a_event->actorDying->As<Actor>();
        if (!actor) { return BSEventNotifyControl::kContinue; }
        CircleManager::StopAllCircling(actor);
        return BSEventNotifyControl::kContinue;
    }
}
