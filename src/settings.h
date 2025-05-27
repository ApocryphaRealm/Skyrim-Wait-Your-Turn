#pragma once 
#include <SimpleIni.h>
#include "nameof.hpp"
namespace WaitYourTurn
{
    class Settings  
    {
        private:
        struct Circling
        {
            bool bPlayerOnly = true;
            float fMinFollowUpSeconds = 8.f; 
            float fMaxFollowUpSeconds = 16.f;
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
            circleOptions.bPlayerOnly = ini.GetBoolValue(header, NAMEOF(circleOptions.bPlayerOnly).c_str(), true);
            circleOptions.iMaxAttackers = ini.GetLongValue(header, NAMEOF(circleOptions.iMaxAttackers).c_str(), 2);
            circleOptions.fMinFollowUpSeconds = (float)ini.GetDoubleValue(header, NAMEOF(circleOptions.fMinFollowUpSeconds).c_str(), 8.f);
            circleOptions.fMaxFollowUpSeconds = (float)ini.GetDoubleValue(header, NAMEOF(circleOptions.fMaxFollowUpSeconds).c_str(), 16.f);
        }

        [[nodiscard]] static const Circling& GetCircling() { return circleOptions; }
    };
}