#pragma once
#include "util.h"
#include "RE/TESPackageEvent.h"
#include "circlemanager.h"
namespace WaitYourTurn
{
    class PackageEventHandler: public BSTEventSink<TESPackageEvent>
    {
        private:
        static inline TESPackage* circlePackage;

        public: 
        static PackageEventHandler* GetSingleton()
        {
            static PackageEventHandler singleton; 
            return &singleton; 
        }
    
        static void Register()
        {
            circlePackage = FormUtil::Parse::GetFormFromMod(0x800, "WaitYourTurnRedux.esp")->As<TESPackage>();
            ScriptEventSourceHolder::GetSingleton()->AddEventSink<TESPackageEvent>(GetSingleton()); 
        }
    
        BSEventNotifyControl ProcessEvent(const TESPackageEvent* a_event,  BSTEventSource<TESPackageEvent>* a_eventSource) override; 
    };
    class CombatEventHandler: public BSTEventSink<TESCombatEvent>
    {
        private:

        public: 
        static CombatEventHandler* GetSingleton()
        {
            static CombatEventHandler singleton; 
            return &singleton; 
        }
    
        static void Register()
        {
            ScriptEventSourceHolder::GetSingleton()->AddEventSink<TESCombatEvent>(GetSingleton()); 
        }
    
        BSEventNotifyControl ProcessEvent(const TESCombatEvent* a_event,  BSTEventSource<TESCombatEvent>* a_eventSource) override; 
    };
}
