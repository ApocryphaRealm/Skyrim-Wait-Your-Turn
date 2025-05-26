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
            circle_package = FormUtil::Parse::GetFormFromMod(0x800, "WaitYourTurnRedux.esp")->As<TESPackage>();
        }
        static void Install()
        {
            REL::Relocation<std::uintptr_t> target { REL::RelocationID(36404, 37398), REL::Relocate(0x47, 0x47) };
            auto& trampoline = SKSE::GetTrampoline();
            SKSE::AllocTrampoline(16); 
            _GetOverridePackage = trampoline.write_branch<5>(target.address(), GetOverridePackage);
        }
        static void RemoveOverride(RE::FormID actorId);
        static void AddOverride(RE::FormID actorId);
        private:
        static inline TESPackage* circle_package; 
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
            InstallStopCombatHook();
            // InstallConstructorHooks();
        }
        //Down	p	NiTPrimitiveArray_CombatGroup____sub_1407A5960+7C	call    sub_14076A2F0
        static void InstallUpdateHook()
        {
            REL::Relocation<std::uintptr_t> target { REL::RelocationID(45569, 0), REL::Relocate(0x7C, 0x0) };
            auto& trampoline = SKSE::GetTrampoline();
            SKSE::AllocTrampoline(14);
            _Update = trampoline.write_call<5>(target.address(), Update);
        }
        //	p	NiTPrimitiveArray_CombatGroup____sub_1407A7FD0+9F	call    sub_14076AE30
        static void InstallStopCombatHook()
        {
            REL::Relocation<std::uintptr_t> target { REL::RelocationID(45597, 0), REL::Relocate(0x9F, 0x0) };
            auto& trampoline = SKSE::GetTrampoline();
            SKSE::AllocTrampoline(14);
            _StopCombat = trampoline.write_call<5>(target.address(), StopCombat);
        }
        /*
        Direction	Type	Address	Text
Down	p	NiTPrimitiveArray_CombatGroup____sub_1407A79B0+154	call    sub_140769DF0
Down	p	NiTPrimitiveArray_CombatGroup____sub_1407A80E0+89	call    sub_140769DF0
Down	o	.rdata:stru_141B832F8	IPtoStateMap <rva sub_140769DF0, -1>
Down	o	.pdata:00000001435132E8	RUNTIME_FUNCTION <rva sub_140769DF0, rva algn_14076A027, \*/
        static void InstallConstructorHooks()
        {
            REL::Relocation<std::uintptr_t> target { REL::RelocationID(45595, 0), REL::Relocate(0x154, 0x0) };
            REL::Relocation<std::uintptr_t> target2 { REL::RelocationID(45599, 0), REL::Relocate(0x89, 0x0) };
            auto& trampoline = SKSE::GetTrampoline();
            SKSE::AllocTrampoline(28);
            _Constructor = trampoline.write_call<5>(target2.address(), Constructor);
            _LoadGameConstructor = trampoline.write_call<5>(target.address(), LoadGameConstructor);
        }

        private:
        static void Update(CombatGroup* a_group); 
        static inline REL::Relocation<decltype(Update)> _Update; 

        static void StopCombat(CombatGroup* a_group);
        static inline REL::Relocation<decltype(StopCombat)> _StopCombat;

        static CombatGroup* Constructor(CombatGroup* a_group, uint32_t a_ind);
        static CombatGroup* LoadGameConstructor(CombatGroup* a_group, uint32_t a_ind);
        static inline REL::Relocation<decltype(Constructor)> _Constructor;
        static inline REL::Relocation<decltype(LoadGameConstructor)> _LoadGameConstructor; 

        using Lock = std::shared_mutex;
        using ReadLocker = std::shared_lock<Lock>;
        using WriteLocker = std::unique_lock<Lock>;
        static inline Lock dataLock;
    };
}