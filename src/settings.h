#pragma once 
#include "../include/SimpleIni.h"
#include "nameof.hpp"
namespace WaitYourTurn
{
    class Settings  
    {
        public:
        struct Circling
        {
            bool bLegacyCircling = false;
            bool bPlayerOnly = true;
            bool bIncludeFollowers = false;
            bool bIncludeRangedMagic = false;
            bool bTransformProtection = true;
            bool bKillmoveProtection = true;
            bool bProjectileSlowdown = true;
            bool bDebugDisplay = false;
            float fProjectileSlowdownRadius = 192.0f;
            float fProjectileSlowdownMultiplier = 0.3f; 
            float fMinFollowUpSeconds = 8.f; 
            float fMaxFollowUpSeconds = 16.f;
            float fMinDefenseSeconds = 3.f;
            float fMaxDefenseSeconds = 6.f;
            size_t iMaxAttackers = 2; 
        };

        private:
        static inline Circling circleOptions;
        static constexpr auto settingsPath = L"Data/SKSE/Plugins/WaitYourTurn/Settings.ini";
        public:

        static void LoadSettings()
        {
            CSimpleIniA ini; 
            ini.SetUnicode(); 
            if (ini.LoadFile(settingsPath) < SI_OK) { return; }
            auto header = "Circling";
            circleOptions.bLegacyCircling = ini.GetBoolValue(header, NAMEOF(circleOptions.bLegacyCircling).c_str(), false);
            circleOptions.bPlayerOnly = ini.GetBoolValue(header, NAMEOF(circleOptions.bPlayerOnly).c_str(), true);
            circleOptions.bTransformProtection = ini.GetBoolValue(header, NAMEOF(circleOptions.bTransformProtection).c_str(), true);
            circleOptions.bIncludeRangedMagic = ini.GetBoolValue(header, NAMEOF(circleOptions.bIncludeRangedMagic).c_str(), false);
            circleOptions.bKillmoveProtection = ini.GetBoolValue(header, NAMEOF(circleOptions.bKillmoveProtection).c_str(), true);
            circleOptions.bIncludeFollowers = ini.GetBoolValue(header, NAMEOF(circleOptions.bIncludeFollowers).c_str(), false);
            circleOptions.bProjectileSlowdown = ini.GetBoolValue(header, NAMEOF(circleOptions.bProjectileSlowdown).c_str(), true);
            circleOptions.bDebugDisplay = ini.GetBoolValue(header, NAMEOF(circleOptions.bDebugDisplay).c_str(), false);
            circleOptions.iMaxAttackers = static_cast<size_t>(ini.GetLongValue(header, NAMEOF(circleOptions.iMaxAttackers).c_str(), 2));
            circleOptions.fProjectileSlowdownRadius = (float)ini.GetDoubleValue(header, NAMEOF(circleOptions.fProjectileSlowdownRadius).c_str(), 192.0);
            circleOptions.fProjectileSlowdownMultiplier = (float)ini.GetDoubleValue(header, NAMEOF(circleOptions.fProjectileSlowdownMultiplier).c_str(), 0.3);
            circleOptions.fMinFollowUpSeconds = (float)ini.GetDoubleValue(header, NAMEOF(circleOptions.fMinFollowUpSeconds).c_str(), 8.f);
            circleOptions.fMaxFollowUpSeconds = (float)ini.GetDoubleValue(header, NAMEOF(circleOptions.fMaxFollowUpSeconds).c_str(), 16.f);
            circleOptions.fMinDefenseSeconds = (float)ini.GetDoubleValue(header, NAMEOF(circleOptions.fMinDefenseSeconds).c_str(), 3.f);
            circleOptions.fMaxDefenseSeconds = (float)ini.GetDoubleValue(header, NAMEOF(circleOptions.fMaxDefenseSeconds).c_str(), 6.f);
        }

        static void SaveSettings()
        {
            CSimpleIniA ini;
            ini.SetUnicode();
            // Load existing file first so comments and structure are preserved.
            ini.LoadFile(settingsPath);
            auto header = "Circling";
            ini.SetBoolValue(header, NAMEOF(circleOptions.bLegacyCircling).c_str(), circleOptions.bLegacyCircling);
            ini.SetBoolValue(header, NAMEOF(circleOptions.bPlayerOnly).c_str(), circleOptions.bPlayerOnly);
            ini.SetBoolValue(header, NAMEOF(circleOptions.bTransformProtection).c_str(), circleOptions.bTransformProtection);
            ini.SetBoolValue(header, NAMEOF(circleOptions.bIncludeRangedMagic).c_str(), circleOptions.bIncludeRangedMagic);
            ini.SetBoolValue(header, NAMEOF(circleOptions.bKillmoveProtection).c_str(), circleOptions.bKillmoveProtection);
            ini.SetBoolValue(header, NAMEOF(circleOptions.bIncludeFollowers).c_str(), circleOptions.bIncludeFollowers);
            ini.SetBoolValue(header, NAMEOF(circleOptions.bProjectileSlowdown).c_str(), circleOptions.bProjectileSlowdown);
            ini.SetBoolValue(header, NAMEOF(circleOptions.bDebugDisplay).c_str(), circleOptions.bDebugDisplay);
            ini.SetLongValue(header, NAMEOF(circleOptions.iMaxAttackers).c_str(), static_cast<long>(circleOptions.iMaxAttackers));
            ini.SetDoubleValue(header, NAMEOF(circleOptions.fProjectileSlowdownRadius).c_str(), circleOptions.fProjectileSlowdownRadius);
            ini.SetDoubleValue(header, NAMEOF(circleOptions.fProjectileSlowdownMultiplier).c_str(), circleOptions.fProjectileSlowdownMultiplier);
            ini.SetDoubleValue(header, NAMEOF(circleOptions.fMinFollowUpSeconds).c_str(), circleOptions.fMinFollowUpSeconds);
            ini.SetDoubleValue(header, NAMEOF(circleOptions.fMaxFollowUpSeconds).c_str(), circleOptions.fMaxFollowUpSeconds);
            ini.SetDoubleValue(header, NAMEOF(circleOptions.fMinDefenseSeconds).c_str(), circleOptions.fMinDefenseSeconds);
            ini.SetDoubleValue(header, NAMEOF(circleOptions.fMaxDefenseSeconds).c_str(), circleOptions.fMaxDefenseSeconds);
            ini.SaveFile(settingsPath);
        }

        static void RestoreDefaults()
        {
            circleOptions = Circling{};
            SaveSettings();
        }

        [[nodiscard]] static Circling& GetCircling() { return circleOptions; }
    };
}
