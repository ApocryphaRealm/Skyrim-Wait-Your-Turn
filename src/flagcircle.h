#pragma once 
#include "circlehandler.h"
namespace WaitYourTurn
{
    class FlagCircle : public CircleHandler
    {
        public:
        void Load() override {}
        void SetupCircling(RE::FormID targetID, RE::FormID combatMemberID) override {}
        void StartCircling(RE::FormID combatMemberID) override;
        void StopCircling(RE::FormID combatMemberID) override;
    };
}