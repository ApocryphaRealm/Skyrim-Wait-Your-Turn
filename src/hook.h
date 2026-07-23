#pragma once 
#include <unordered_set>
#include <shared_mutex>
#include "circlemanager.h"
#include "util.h"
#include <safetyhook.hpp>
namespace WaitYourTurn
{
    //	j	Actor__CheckForCurrentAliasPackage_1405DB180+47	jmp     sub_140126A00
    //	j	sub_14066CB60+47	jmp     sub_14016D9D0 AE 1.6.1170

    class PackageOverrideHook
    {
        public:
        static void Load()
        {
            auto* form = FormUtil::Parse::GetFormFromMod(0x800, "WaitYourTurnRedux.esp");
            if (form) {
                circlePackage = form->As<TESPackage>();
            }
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
    class CombatRangeHook
    {
        public:

        static void Install()
        {
/*Direction	Type	Address	Text
Up	p	sub_1406D20A0+61	call    TESObjectREFR__sub_1407BF1B0 npcs only
Up	p	sub_140782AA0+193	call    TESObjectREFR__sub_1407BF1B0
Down	p	sub_140826F60+1E3	call    TESObjectREFR__sub_1407BF1B0 x

Direction	Type	Address	Text
Up	p	sub_1407663A0+126	call    sub_140856200
Up	p	sub_14081ECC0+13F	call    sub_140856200
	p	sub_1408BE880+213	call    sub_140856200
*/  
            REL::Relocation<std::uintptr_t> target { REL::RelocationID(40259, 41261), REL::Relocate(0x61, 0x126) };
            auto& trampoline = SKSE::GetTrampoline();
            SKSE::AllocTrampoline(14);
            _IsInAttackRange = trampoline.write_call<5>(target.address(), IsInAttackRange);
        }
        private:
        static bool IsInAttackRange(Actor *a_attacker, Actor *a_target, float a_extend);
        static inline REL::Relocation<decltype(IsInAttackRange)> _IsInAttackRange; 
    };
    class CombatGroupHook
    {
        public:
        static void Install()
        {
            InstallUpdateHook();
            // InstallRemoveMemberHook();
            InstallDestructorHook();
            // InstallMemberKilledHook();
            InstallMemberAttackedHook();
            InstallMergeGroupsHook();
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
        //Up	p	sub_1406972D0+96A	call    sub_140806180
        static void InstallMemberKilledHook()
        {
            REL::Relocation<std::uintptr_t> target { REL::RelocationID(36872, 37896), REL::Relocate(0x8C9, 0x96A) };
            auto& trampoline = SKSE::GetTrampoline();
            SKSE::AllocTrampoline(14);
            _NotifyMemberKilled = trampoline.write_call<5>(target.address(), NotifyMemberKilled);
        }
        static void NotifyMemberKilled(CombatGroup *a_group, Actor *a_member, Actor *a_killer);
        static inline REL::Relocation<decltype(NotifyMemberKilled)> _NotifyMemberKilled;

        //Up	p	sub_1404FD8F0+B3	call    sub_14076C500
        //Up	p	sub_140558FC0+98	call    sub_140806090
        static void InstallMemberAttackedHook()
        {
            REL::Relocation<std::uintptr_t> target { REL::RelocationID(32473, 33220), REL::Relocate(0xB3, 0x98) };
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

        static void InstallRemoveMemberHook() // deprecated
        {
            REL::Relocation<std::uintptr_t> target3 { REL::RelocationID(43559, 44785), REL::Relocate(0x274, 0x0) };
            auto& trampoline = SKSE::GetTrampoline();
            SKSE::AllocTrampoline(16);

            _RemoveMember = trampoline.write_call<5>(target3.address(), RemoveMember);
        }

        static void InstallDestructorHook()
        {
            REL::Relocation<std::uintptr_t> target { REL::RelocationID(32468, 33215)};
            _Destroy = safetyhook::create_inline(target.address(), Destroy); 
        }
        static void Destroy(RE::CombatController* a_controller);
        static inline SafetyHookInline _Destroy;

        static void InstallMergeGroupsHook()
        {
            //Down	p	NiTPrimitiveArray_CombatGroup____sub_1407A5FA0+65	call    sub_14076B160 SE
            //	sub_14083D2C0+65 AE
            REL::Relocation<std::uintptr_t> target { REL::RelocationID(45574, 46874), REL::Relocate(0x65, 0x65) };
            auto& trampoline = SKSE::GetTrampoline();
            SKSE::AllocTrampoline(14);
            _MergeGroups = trampoline.write_call<5>(target.address(), MergeGroups);
        }
        
        static void Update(CombatGroup* a_group); 
        static inline REL::Relocation<decltype(Update)> _Update; 

        static void StopCombat(CombatGroup* a_group);
        static inline REL::Relocation<decltype(StopCombat)> _StopCombat;

        static void SetCombatGroup(Actor* a_actor, CombatGroup* a_group); 
        static inline REL::Relocation<decltype(SetCombatGroup)> _SetCombatGroup;

        //Up	p	sub_14076A2F0+65	call    sub_140772CD0 43481

        static void RemoveMember(CombatGroup* a_group, Actor* a_actor);
        static void RemoveMember2(CombatGroup* a_group, Actor* a_actor);
        static void RemoveMember3(CombatGroup* a_group, Actor* a_actor);
        

        static bool MergeGroups(CombatGroup* a_group, CombatGroup* a_other);

        static inline REL::Relocation<decltype(RemoveMember)> _RemoveMember;
        static inline REL::Relocation<decltype(RemoveMember2)> _RemoveMember2; 
        static inline REL::Relocation<decltype(RemoveMember3)> _RemoveMember3; 

        static inline REL::Relocation<decltype(MergeGroups)> _MergeGroups;


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

    class RaceTransformHook
    {
        public:
        static void Install()
        {
            REL::Relocation<std::uintptr_t> WerewolfEffectVtbl { VTABLE_WerewolfEffect[0] };
            REL::Relocation<std::uintptr_t> VampireLordEffectVtbl { VTABLE_VampireLordEffect[0] };

            _StartWerewolf = WerewolfEffectVtbl.write_vfunc(0x14, StartWerewolf);
            _FinishWerewolf = WerewolfEffectVtbl.write_vfunc(0x15, FinishWerewolf);

            _StartVampire = VampireLordEffectVtbl.write_vfunc(0x14, StartVampire);
            _FinishVampire = VampireLordEffectVtbl.write_vfunc(0x15, FinishVampire);
        }

        static void StartWerewolf(WerewolfEffect* a_effect);
        static void FinishWerewolf(WerewolfEffect* a_effect);
        static void StartVampire(VampireLordEffect* a_effect);
        static void FinishVampire(VampireLordEffect* a_effect);

        static inline REL::Relocation<decltype(StartWerewolf)> _StartWerewolf; 
        static inline REL::Relocation<decltype(FinishWerewolf)> _FinishWerewolf; 
        static inline REL::Relocation<decltype(StartVampire)> _StartVampire; 
        static inline REL::Relocation<decltype(FinishVampire)> _FinishVampire;
    };
    class KillmoveHook
    {
        public:
        static void Install()
        {
            REL::Relocation<std::uintptr_t> target { REL::RelocationID(37659, 38613), REL::Relocate(0x44, 0x44) }; 
            auto& trampoline = SKSE::GetTrampoline(); 
            SKSE::AllocTrampoline(14); 
            _StartKillmove = trampoline.write_call<5>(target.address(), StartKillmove); 
        }
        private:
        static bool StartKillmove(Actor* a_attacker, Actor* a_victim); 
        static inline REL::Relocation<decltype(StartKillmove)> _StartKillmove; 
    };

    class ProjectileHook
    {
        public:
        static void Install()
        {
            REL::Relocation<std::uintptr_t> target { VTABLE_ArrowProjectile[0] }; 
            _UpdateImpl = target.write_vfunc(0xAB, UpdateImpl);
        }
        private:
        static void UpdateImpl(Projectile* a_projectile, float a_delta);
        static inline REL::Relocation<decltype(UpdateImpl)> _UpdateImpl;                                                                                                                     

    };
    //	p	Character__sub_1406280D0+44	call    ActorProcess__StartDeferredKill_1406820E0

    class DisableHook
    {
        public:
        static void Install()
        {
            REL::Relocation<std::uintptr_t> actorVtbl { VTABLE_Actor[0] };
            _Disable = actorVtbl.write_vfunc(0x89, Disable);
        }
        private:
        static void Disable(Actor* a_actor);
        static inline REL::Relocation<decltype(Disable)> _Disable;
    };
    class EquipMeleeHook
    {
        public:
        static void Install()
        {
            REL::Relocation<std::uintptr_t> actorVtbl { VTABLE_CombatInventoryItemMelee[0] };
            _EquipItem = actorVtbl.write_vfunc(0x11, EquipItem);
        }
        private:
        static void EquipItem(CombatInventoryItem* a_item, CombatController* a_controller);
        static inline REL::Relocation<decltype(EquipItem)> _EquipItem; 
    };
    class EquipRangedHook
    {
        public:
        static void Install()
        {
            REL::Relocation<std::uintptr_t> actorVtbl { VTABLE_CombatInventoryItemRanged[0] };
            _EquipItem = actorVtbl.write_vfunc(0x11, EquipItem);
        }
        private:
        static void EquipItem(CombatInventoryItem* a_item, CombatController* a_controller);
        static inline REL::Relocation<decltype(EquipItem)> _EquipItem; 
    };
    class EquipMagicHook
    {
        public:
        static void Install()
        {
            REL::Relocation<std::uintptr_t> actorVtbl { VTABLE_CombatInventoryItemMagic[0] };
            _EquipItem = actorVtbl.write_vfunc(0x11, EquipItem);
        }
        private:
        static void EquipItem(CombatInventoryItem* a_item, CombatController* a_controller);
        static inline REL::Relocation<decltype(EquipItem)> _EquipItem; 
    };
    class EquipStaffHook
    {
        public:
        static void Install()
        {
            REL::Relocation<std::uintptr_t> actorVtbl { VTABLE_CombatInventoryItemStaff[0] };
            _EquipItem = actorVtbl.write_vfunc(0x11, EquipItem);
        }
        private:
        static void EquipItem(CombatInventoryItem* a_item, CombatController* a_controller);
        static inline REL::Relocation<decltype(EquipItem)> _EquipItem; 
    };
}