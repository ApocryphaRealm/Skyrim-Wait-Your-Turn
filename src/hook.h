#pragma once 
#include <unordered_set>
#include <shared_mutex>
#include "circlemanager.h"
#include "util.h"
namespace WaitYourTurn
{
    //	j	Actor__CheckForCurrentAliasPackage_1405DB180+47	jmp     sub_140126A00
    //	j	sub_14066CB60+47	jmp     sub_14016D9D0 AE 1.6.1170

    class PackageOverrideHook
    {
        public:
        static void Load()
        {
            circlePackage = FormUtil::Parse::GetFormFromMod(0x800, "WaitYourTurnRedux.esp")->As<TESPackage>();
        }
        static void Install()
        {
            REL::Relocation<std::uintptr_t> target { REL::RelocationID(36404, 37398), REL::Relocate(0x47, 0x47) };
            auto& trampoline = SKSE::GetTrampoline();
            SKSE::AllocTrampoline(16); 
            _GetOverridePackage = trampoline.write_branch<5>(target.address(), GetOverridePackage);
        }
        static void ClearOverrides();
        static void RemoveOverride(RE::FormID actorId);
        static void AddOverride(RE::FormID actorId);
        static bool HasOverride(FormID actorId);
        private:
        static inline TESPackage* circlePackage; 
        static TESPackage* GetOverridePackage(ExtraDataList* a_extraData, Actor* a_actor); 
        static inline REL::Relocation<decltype(GetOverridePackage)> _GetOverridePackage; 

        using Lock = std::shared_mutex;
        using ReadLocker = std::shared_lock<Lock>;
        using WriteLocker = std::unique_lock<Lock>;
        static inline Lock dataLock;
        static inline std::unordered_set<FormID> overrideActors;
    };

    class CombatGroupHook
    {
        public:

        static void Install()
        {
            InstallUpdateHook();
            InstallMemberKilledHook();
            InstallMemberAttackedHook();
            // InstallStopCombatHook();
            // InstallSetCombatGroupHook();
            // InstallConstructorHooks();
        }
        private:
        //Down	p	NiTPrimitiveArray_CombatGroup____sub_1407A5960+7C	call    sub_14076A2F0
        //	p	sub_14083CA30+7C	call    sub_140803710
        static void InstallUpdateHook()
        {
            REL::Relocation<std::uintptr_t> target { REL::RelocationID(45569, 46869), REL::Relocate(0x7C, 0x7C) };
            auto& trampoline = SKSE::GetTrampoline();
            SKSE::AllocTrampoline(14);
            _Update = trampoline.write_call<5>(target.address(), Update);
        }
        //Up	p	Actor__KillImpl_140603B30+8C9	call    UpdateAllyKilled_14076C600
        static void InstallMemberKilledHook()
        {
            REL::Relocation<std::uintptr_t> target { REL::RelocationID(36872, 0), REL::Relocate(0x8C9, 0) };
            auto& trampoline = SKSE::GetTrampoline();
            SKSE::AllocTrampoline(14);
            _NotifyMemberKilled = trampoline.write_call<5>(target.address(), NotifyMemberKilled);
        }
        static void NotifyMemberKilled(CombatGroup *a_group, Actor *a_member, Actor *a_killer);
        static inline REL::Relocation<decltype(NotifyMemberKilled)> _NotifyMemberKilled;

        //Up	p	sub_1404FD8F0+B3	call    sub_14076C500
        static void InstallMemberAttackedHook()
        {
            REL::Relocation<std::uintptr_t> target { REL::RelocationID(32473, 0), REL::Relocate(0xB3, 0) };
            auto& trampoline = SKSE::GetTrampoline();
            SKSE::AllocTrampoline(14);
            _NotifyMemberAttacked = trampoline.write_call<5>(target.address(), NotifyMemberAttacked);
        }
        static void NotifyMemberAttacked(CombatGroup* a_group, Actor* a_member, Actor* a_attacker);
        static inline REL::Relocation<decltype(NotifyMemberAttacked)> _NotifyMemberAttacked; 
        //	p	NiTPrimitiveArray_CombatGroup____sub_1407A7FD0+9F	call    sub_14076AE30
        //	p	sub_14083F220+9F	call    sub_140804760
        static void InstallStopCombatHook()
        {
            REL::Relocation<std::uintptr_t> target { REL::RelocationID(43495, 46897), REL::Relocate(0x9F, 0x9F) };
            auto& trampoline = SKSE::GetTrampoline();
            SKSE::AllocTrampoline(14);
            _StopCombat = trampoline.write_call<5>(target.address(), StopCombat);
        }
        static void InstallSetCombatGroupHook()
        {
            REL::Relocation<std::uintptr_t> actorVtbl { VTABLE_Actor[0] };
            _SetCombatGroup = actorVtbl.write_vfunc(0xD5, SetCombatGroup);
        }

        
        static void Update(CombatGroup* a_group); 
        static inline REL::Relocation<decltype(Update)> _Update; 

        static void StopCombat(CombatGroup* a_group);
        static inline REL::Relocation<decltype(StopCombat)> _StopCombat;

        static void SetCombatGroup(Actor* a_actor, CombatGroup* a_group); 
        static inline REL::Relocation<decltype(SetCombatGroup)> _SetCombatGroup;

        //Up	p	sub_14076A2F0+65	call    sub_140772CD0 43481



        using Lock = std::shared_mutex;
        using ReadLocker = std::shared_lock<Lock>;
        using WriteLocker = std::unique_lock<Lock>;
        static inline Lock dataLock;
    };

    class CombatControllerHook
    {
        public:
        /*
        Direction	Type	Address	Text
Up	p	sub_1404FD450+157	call    sub_1404FDFE0
Down	p	sub_1404FE300+151	call    sub_1404FDFE0
Down	o	.rdata:stru_141AECED8	IPtoStateMap <rva sub_1404FDFE0, -1>
Down	o	.pdata:00000001434F35E4	RUNTIME_FUNCTION <rva sub_1404FDFE0, rva algn_1404FE2F7, \
Direction	Type	Address	Text
	p	sub_140558B70+12F	call    sub_140559630
Down	p	sub_140559930+119	call    sub_140559630
Down	o	.pdata:0000000143649EA0	RUNTIME_FUNCTION <rva sub_140559630, rva algn_140559921, \*/
//33218     140516A30
        static void Install()
        {
            REL::Relocation<std::uintptr_t> target1 { REL::RelocationID(32471, 33218), REL::Relocate(0x157, 0x12F) };
            REL::Relocation<std::uintptr_t> target2 { REL::RelocationID(32489, 33236), REL::Relocate(0x151, 0x119) };
            //Down	p	fUnk_Attacked_1406285A0+658	call    sub_1404FD8F0
            REL::Relocation<std::uintptr_t> target3 { REL::RelocationID(37672, 0x0), REL::Relocate(0x658, 0x0) };

            auto& trampoline = SKSE::GetTrampoline();
            SKSE::AllocTrampoline(28);
            _SetTarget = trampoline.write_call<5>(target1.address(), SetTarget);
            _SetTarget2 = trampoline.write_call<5>(target2.address(), SetTarget2);
            // _AttackedBy = trampoline.write_call<5>(target3.address(), AttackedBy);
        }
        private:
        static void SetTarget(CombatController* a_controller, Actor* a_target);
        static void SetTarget2(CombatController* a_controller, Actor* a_target);
        static void AttackedBy(CombatController* a_controller, Actor* a_target);
        static inline REL::Relocation<decltype(SetTarget)> _SetTarget;
        static inline REL::Relocation<decltype(SetTarget2)> _SetTarget2;
        static inline REL::Relocation<decltype(AttackedBy)> _AttackedBy;
        
    };
}