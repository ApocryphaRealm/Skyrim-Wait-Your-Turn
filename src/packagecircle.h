#pragma once
#include "circlehandler.h"
namespace WaitYourTurn
{
    class PackageCircle : public CircleHandler
    {
        public:
        void Load(); 
        void SetupCircling(RE::FormID targetID, RE::FormID combatMemberID) override;
        void StartCircling(RE::FormID combatMemberID) override;
        void StopCircling(RE::FormID combatMemberID) override;
        private:
        RE::BGSKeyword* circleTargetKeyword = nullptr; 
    }; 
}