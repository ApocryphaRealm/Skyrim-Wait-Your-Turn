#pragma once
#include <shared_mutex>
#include <random>
#include "util.h"
#include "hook.h"
#include "settings.h"
#include "packagecircle.h"
#include "flagcircle.h"
namespace WaitYourTurn
{
    class CircleManager
    {
        // who up circling they member
        struct CircleMember
        {
            float timeRemaining;
            FormID formID;
            CircleMember(FormID id, float time) : formID(id), timeRemaining(time) { circleHandler->StopCircling(formID); }
            CircleMember(FormID id) : formID(id), timeRemaining(0.f) { circleHandler->StartCircling(formID); }

            ~CircleMember();
        };
        struct CircleGroup
        {
            private:
            using Lock = std::shared_mutex;
            using ReadLocker = std::shared_lock<Lock>;
            using WriteLocker = std::unique_lock<Lock>;
            bool allowAttackers{ true };
            public:
            static inline Lock lock;
            
            std::unordered_map<FormID, CircleMember> circlerMap;
            std::unordered_map<FormID, CircleMember> attackerMap;
            void SetAttacker(FormID formID);
            void SetDefender(FormID formID);
            void UnsetAttacker(FormID formID);
            void Update(float a_delta);
            void AllowAttackers(bool state);
            bool GetAllowAttackers() { return allowAttackers; }
        };



        public:
        static void SetupCircleHandler()
        {
            circleHandler = Settings::GetCircling().bLegacyCircling ?
                std::unique_ptr<CircleHandler>(std::make_unique<PackageCircle>()) :
                std::unique_ptr<CircleHandler>(std::make_unique<FlagCircle>());
            circleHandler->Load();
        }
        static void UpdateTarget(FormID actorId);
        static void ChangeTarget(FormID targetID, FormID lastTargetID, FormID combatMemberID);
        static void AddTarget(FormID targetID, FormID combatMemberID);
        static void RemoveTarget(FormID targetID);
        static void RemoveCombatant(FormID targetID, FormID actorId);
        static void AllowCombatantDefense(FormID targetID, FormID actorID);
        static void AllowAttackers(FormID targetID, bool state); 
        static bool GetAllowAttackers(FormID targetID);
        static bool IsTargetCombatantPair(FormID targetID, FormID combatMemberID);

        static bool IsBeingCircled(Actor* a_target);
        static bool CanCircle(Actor* a_target, Actor* a_combatant);
        static void StopAllCircling(Actor* a_target);
        static void StopAllCircling(Actor* a_target, CombatGroup* a_group); 
        static void LoadCircleGroups();

        private:
        using Lock = std::shared_mutex;
        using ReadLocker = std::shared_lock<Lock>;
        using WriteLocker = std::unique_lock<Lock>;
        static inline Lock dataLock;
        static inline std::random_device rd;
        static inline std::mt19937 mt{rd()};

        static inline std::unique_ptr<CircleHandler> circleHandler = nullptr;
        // static inline std::unordered_set<FormID> unlockActors;
        static inline std::unordered_map<FormID, CircleGroup> circleGroupMap;
        static float GetAttackerDuration();
        static float GetDefenderDuration();
        static bool IsRangedOrMagic(Actor* a_actor);
        static inline bool IsHumanoid(Actor* a_actor)
        {
            auto bodyPartData = a_actor->GetRace() ? a_actor->GetRace()->bodyPartData : nullptr;
            return bodyPartData && bodyPartData->GetFormID() == 0x1d;
        }
        static inline bool IsRangedItemType(uint16_t type)
        {
            return (type == 7 || type == 12);
        }
        static inline bool IsMagicItemType(uint16_t type)
        {
            return (type == 8 || type == 9);
        }

        static uint16_t GetEquippedItemType(TESForm* a_form)
        {
            using func_t = decltype(GetEquippedItemType); 
            static REL::Relocation<func_t> func { REL::RelocationID(14125, 14220) }; 
            return func(a_form); 
        }
    };
}