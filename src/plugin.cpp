#include "log.h"
#include "hook.h"
#include "event.h"
#include "circlemanager.h"
using namespace WaitYourTurn;
void OnDataLoaded()
{
   
}

void MessageHandler(SKSE::MessagingInterface::Message* a_msg)
{
	switch (a_msg->type) {
	case SKSE::MessagingInterface::kDataLoaded:
		Settings::LoadSettings();
        PackageOverrideHook::Load();
		PackageEventHandler::Register();
		CombatEventHandler::Register();
		break;
	case SKSE::MessagingInterface::kPostLoad:
		break;
	case SKSE::MessagingInterface::kPreLoadGame:
		break;
	case SKSE::MessagingInterface::kPostLoadGame:
		// PackageOverrideHook::ClearOverrides();
		CircleManager::LoadCircleGroups();
        break;
	case SKSE::MessagingInterface::kNewGame:
		break;
	}
}

SKSEPluginLoad(const SKSE::LoadInterface *skse) {
    SKSE::Init(skse);
	SetupLog();

	PackageOverrideHook::Install();
	CombatGroupHook::Install();
	CombatControllerHook::Install();
	CombatRangeHook::Install();
	RaceTransformHook::Install();
    auto messaging = SKSE::GetMessagingInterface();
	if (!messaging->RegisterListener("SKSE", MessageHandler)) {
		return false;
	}

	
    return true;
}