#pragma once

#include <atomic>
#include <filesystem>
#include <format>
#include <string>

#include "SKSEMenuFramework.h"

namespace WaitYourTurn
{
namespace Menu
{
// Adds the Wait Your Turn page to SKSE Menu Framework's Mod Control Panel.
//
// Safe to call when the framework is missing or too old to drive: it writes the reason to
// the log and does nothing else, leaving the mod running on its INI settings alone.
//
// Must be called once every SKSE plugin has loaded (kPostPostLoad is a good moment),
// because the framework is looked up as an already-loaded module.
void RegisterMenu();

// The page renderer. SKSE Menu Framework calls this from the render thread.
void __stdcall Render();

// Open/close notifications for the Mod Control Panel, used to reload the INI when the panel
// opens and write it back when the panel closes.
void __stdcall OnMenuEvent(SKSEMenuFramework::Model::EventType a_eventType);
} // namespace Menu
} // namespace WaitYourTurn
