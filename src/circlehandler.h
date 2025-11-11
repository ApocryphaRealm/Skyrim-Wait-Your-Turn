#pragma once
namespace WaitYourTurn
{
    class CircleHandler
    {
        public:
        virtual void Load() = 0;
        virtual void SetupCircling(RE::FormID targetID, RE::FormID combatMemberID) = 0;
        virtual void StartCircling(RE::FormID combatMemberID) = 0;
        virtual void StopCircling(RE::FormID combatMemberID) = 0;
    };
}