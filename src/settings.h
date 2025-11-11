#pragma once 
#include "../include/SimpleIni.h"
#include "nameof.hpp"
namespace WaitYourTurn
{
    class Settings  
    {
        private:
        struct Circling
        {
            bool bLegacyCircling = false;
            bool bPlayerOnly = true;
            bool bIncludeFollowers = false;
            bool bIncludeRangedMagic = false;
            bool bTransformProtection = true;
            bool bKillmoveProtection = true;
            bool bProjectileSlowdown = true;
            float fProjectileSlowdownRadius = 192.0f;
            float fProjectileSlowdownMultiplier = 0.3f; 
            float fMinFollowUpSeconds = 8.f; 
            float fMaxFollowUpSeconds = 16.f;
            float fMinDefenseSeconds = 3.f;
            float fMaxDefenseSeconds = 6.f;
            size_t iMaxAttackers = 2; 
        };

        static inline Circling circleOptions;
        public:

        static void LoadSettings()
        {
            constexpr auto path = L"Data/SKSE/Plugins/WaitYourTurn/Settings.ini";
            CSimpleIniA ini; 
            ini.SetUnicode(); 
            if (ini.LoadFile(path) < SI_OK) { return; }
            auto header = "Circling";
            circleOptions.bLegacyCircling = ini.GetBoolValue(header, NAMEOF(circleOptions.bLegacyCircling).c_str(), false);
            circleOptions.bPlayerOnly = ini.GetBoolValue(header, NAMEOF(circleOptions.bPlayerOnly).c_str(), true);
            circleOptions.bTransformProtection = ini.GetBoolValue(header, NAMEOF(circleOptions.bTransformProtection).c_str(), true);
            circleOptions.bIncludeRangedMagic = ini.GetBoolValue(header, NAMEOF(circleOptions.bIncludeRangedMagic).c_str(), false);
            circleOptions.bKillmoveProtection = ini.GetBoolValue(header, NAMEOF(circleOptions.bKillmoveProtection).c_str(), true);
            circleOptions.bIncludeFollowers = ini.GetBoolValue(header, NAMEOF(circleOptions.bIncludeFollowers).c_str(), false);
            circleOptions.bProjectileSlowdown = ini.GetBoolValue(header, NAMEOF(circleOptions.bProjectileSlowdown).c_str(), true);
            circleOptions.iMaxAttackers = ini.GetLongValue(header, NAMEOF(circleOptions.iMaxAttackers).c_str(), 2);
            circleOptions.fProjectileSlowdownRadius = (float)ini.GetDoubleValue(header, NAMEOF(circleOptions.fProjectileSlowdownRadius).c_str(), 192.0);
            circleOptions.fProjectileSlowdownMultiplier = (float)ini.GetDoubleValue(header, NAMEOF(circleOptions.fProjectileSlowdownMultiplier).c_str(), 0.3);
            circleOptions.fMinFollowUpSeconds = (float)ini.GetDoubleValue(header, NAMEOF(circleOptions.fMinFollowUpSeconds).c_str(), 8.f);
            circleOptions.fMaxFollowUpSeconds = (float)ini.GetDoubleValue(header, NAMEOF(circleOptions.fMaxFollowUpSeconds).c_str(), 16.f);
            circleOptions.fMinDefenseSeconds = (float)ini.GetDoubleValue(header, NAMEOF(circleOptions.fMinDefenseSeconds).c_str(), 3.f);
            circleOptions.fMaxDefenseSeconds = (float)ini.GetDoubleValue(header, NAMEOF(circleOptions.fMaxDefenseSeconds).c_str(), 6.f);
        }

        [[nodiscard]] static const Circling& GetCircling() { return circleOptions; }
    };
}