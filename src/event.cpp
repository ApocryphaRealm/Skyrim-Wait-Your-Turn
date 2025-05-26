#include "event.h"
namespace WaitYourTurn
{
    using Control = BSEventNotifyControl;
    using HEAD_TRACK = HighProcessData::HEAD_TRACK_TYPES::HEAD_TRACK_TYPE;
    using EventType = TESPackageEvent::Type;
    BSEventNotifyControl PackageEventHandler::ProcessEvent(const TESPackageEvent *a_event, BSTEventSource<TESPackageEvent> *a_eventSource)
    {
        if (a_event->target && a_event->packageFormID == circlePackage->GetFormID())
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
                    high->SetHeadtrackTarget(HEAD_TRACK::kCombat, target);
                    break;
                case EventType::kPackageChange:
                    high->ClearHeadtrackTarget(HEAD_TRACK::kCombat, true);
                    break;
                case EventType::kPackageEnd:
                    high->ClearHeadtrackTarget(HEAD_TRACK::kCombat, true);
                    break;
            }

        }
        return BSEventNotifyControl::kContinue;
    }
}

