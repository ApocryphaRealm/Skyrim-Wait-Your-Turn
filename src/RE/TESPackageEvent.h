#pragma once 
namespace RE
{
    struct TESPackageEvent
    {
        public:
        enum Type : uint32_t
        {
            kPackageStart = 0,  // package form ID is for the new package
            kPackageEnd = 1,    // package form ID is for the old package
            kPackageChange = 2, // package form ID is for the old package
        };
        //
        RE::TESObjectREFR *target; // 00
        uint32_t packageFormID;    // 08
        Type eventType;            // 0C
    };
    static_assert(sizeof(TESPackageEvent) == 0x10);
}
