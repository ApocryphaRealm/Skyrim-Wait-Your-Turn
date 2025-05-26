#pragma once
#include <shared_mutex>
#include <random>
#include "util.h"
#include "hook.h"
namespace WaitYourTurn
{
    class CircleManager
    {
        // who up circling they member
        struct FreeMember
        {
            float timeRemaining;
            FormID formID;
            FreeMember(FormID id, float time) : formID(id), timeRemaining(time) { Free(); }
            void Free();
            void Circle();
        };
        public:
        static void StartCircling(CombatGroup* a_group); 
        static void StopCircling(CombatGroup* a_group);
        static void UpdateCircling(CombatGroup* a_group); 
        private:
        using Lock = std::shared_mutex;
        using ReadLocker = std::shared_lock<Lock>;
        using WriteLocker = std::unique_lock<Lock>;
        static inline Lock dataLock;
        static inline std::random_device rd;
        static inline std::mt19937 mt{rd()};
        // static inline std::unordered_set<FormID> unlockActors;
        static inline std::unordered_map<uint32_t, std::vector<FreeMember>> freeMemberMap; 
        static inline std::unordered_set<FormID> freeActorIDs;
        static inline size_t maxCircleMembers = 1; 
        static void CircleAll(CombatGroup* a_group);
        static void FreeAll(CombatGroup* a_group);
        static void FreeNewMember(CombatGroup *a_group, std::vector<FreeMember>& freeMembers);
        static float GetCircleDuration();
    };
}