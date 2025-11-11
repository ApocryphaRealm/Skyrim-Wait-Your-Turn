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
    class CellAttachDetachEventHandler: public BSTEventSink<TESCellAttachDetachEvent>
    {
        private:

        public: 
        static CellAttachDetachEventHandler* GetSingleton()
        {
            static CellAttachDetachEventHandler singleton; 
            return &singleton; 
        }
    
        static void Register()
        {
            ScriptEventSourceHolder::GetSingleton()->AddEventSink<TESCellAttachDetachEvent>(GetSingleton()); 
        }
    
        BSEventNotifyControl ProcessEvent(const TESCellAttachDetachEvent* a_event,  BSTEventSource<TESCellAttachDetachEvent>* a_eventSource) override; 
    };
    class DeleteEventHandler : public BSTEventSink<TESFormDeleteEvent>
    {
        private:

        public: 
        static DeleteEventHandler* GetSingleton()
        {
            static DeleteEventHandler singleton; 
            return &singleton; 
        }
    
        static void Register()
        {
            ScriptEventSourceHolder::GetSingleton()->AddEventSink<TESFormDeleteEvent>(GetSingleton()); 
        }
    
        BSEventNotifyControl ProcessEvent(const TESFormDeleteEvent* a_event,  BSTEventSource<TESFormDeleteEvent>* a_eventSource) override; 
    };
    class DeathEventHandler : public BSTEventSink<TESDeathEvent>
    {
        private:

        public: 
        static DeathEventHandler* GetSingleton()
        {
            static DeathEventHandler singleton; 
            return &singleton; 
        }
    
        static void Register()
        {
            ScriptEventSourceHolder::GetSingleton()->AddEventSink<TESDeathEvent>(GetSingleton()); 
        }
    
        BSEventNotifyControl ProcessEvent(const TESDeathEvent* a_event,  BSTEventSource<TESDeathEvent>* a_eventSource) override; 
    };
    static void RegisterEventHandlers()
    {
        CombatEventHandler::Register();
        CellAttachDetachEventHandler::Register();
        DeleteEventHandler::Register();
        DeathEventHandler::Register();
    }
}
