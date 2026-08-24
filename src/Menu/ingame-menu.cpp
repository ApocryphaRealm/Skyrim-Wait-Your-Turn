#include "ingame-menu.h"

#include "settings.h"

#include <algorithm>
#include <cstddef>

namespace WaitYourTurn
{
namespace Menu
{
namespace
{
// Kept alive for the lifetime of the process; the framework calls back into it whenever the
// Mod Control Panel opens or closes.
SKSEMenuFramework::Model::Event* g_menuEvent = nullptr;

// SKSE Menu Framework has no HelpMarker of its own, so reproduce the one the FLICK menu
// provided: a dimmed marker after the control, with the description in a tooltip.
void HelpMarker(const char* a_description)
{
    ImGuiMCP::SameLine();
    ImGuiMCP::TextDisabled("(?)");

    if (ImGuiMCP::IsItemHovered())
    {
        ImGuiMCP::SetTooltip("%s", a_description);
    }
}

// The bundled header reaches ImGui through the framework's exported cimgui entry points and
// calls them through raw function pointers. Versions 1 and 2 of the framework do not export
// those at all, so every widget below would call through null. Check for the ones this page
// uses before registering anything.
bool HasRequiredExports()
{
    constexpr const char* required[] = {
        "AddSectionItem",
        "igTextV",
        "igTextDisabled",
        "igTextWrapped",
        "igCollapsingHeader_TreeNodeFlags",
        "igCheckbox",
        "igSliderInt",
        "igSliderFloat",
        "igButton",
        "igSameLine",
        "igSeparator",
        "igSpacing",
        "igIndent",
        "igUnindent",
        "igIsItemHovered",
        "igSetTooltip"
    };

    for (const char* name : required)
    {
        if (!GetMenuFrameworkFunction<void*>(name))
        {
            SKSE::log::info("SKSE Menu Framework does not export \"{}\"", name);

            return false;
        }
    }

    return true;
}

void DrawCircling()
{
    auto& c = Settings::GetCircling();

    ImGuiMCP::Indent();
    ImGuiMCP::Checkbox("Legacy Circling##wyt", &c.bLegacyCircling);
    HelpMarker("Whether to use legacy packages for circling behavior (same behavior as the "
               "original papyrus mod). Using legacy packages REQUIRES WaitYourTurnRedux.esp "
               "to be present and enabled in the load order.");
    ImGuiMCP::Checkbox("Player Only##wyt", &c.bPlayerOnly);
    HelpMarker("If disabled, circling applies to ALL NPCs instead of just the player.");
    ImGuiMCP::Checkbox("Include Followers##wyt", &c.bIncludeFollowers);
    HelpMarker("Allows followers to benefit from the circling system alongside the player.");
    ImGuiMCP::Checkbox("Include Ranged / Magic##wyt", &c.bIncludeRangedMagic);
    HelpMarker("Whether ranged/magic NPCs are included in the circling system.");
    ImGuiMCP::Checkbox("Transform Protection##wyt", &c.bTransformProtection);
    HelpMarker("Stops attackers during werewolf / vampire lord transformation.");
    ImGuiMCP::Checkbox("Killmove Protection##wyt", &c.bKillmoveProtection);
    HelpMarker("Prevents killmoves on actors that are currently being circled.");

    int maxAttackers = static_cast<int>(c.iMaxAttackers);
    if (ImGuiMCP::SliderInt("Max Attackers##wyt", &maxAttackers, 1, 16, "%d"))
        c.iMaxAttackers = static_cast<size_t>(std::clamp(maxAttackers, 1, 16));
    HelpMarker("Maximum number of attackers allowed to engage a single target at once.");

    float minFollowUp = c.fMinFollowUpSeconds;
    float maxFollowUp = c.fMaxFollowUpSeconds;
    if (ImGuiMCP::SliderFloat("Min Follow-Up Seconds##wyt", &minFollowUp, 0.0f, 60.0f, "%.1f"))
        c.fMinFollowUpSeconds = std::clamp(minFollowUp, 0.0f, 60.0f);
    if (ImGuiMCP::SliderFloat("Max Follow-Up Seconds##wyt", &maxFollowUp, 0.0f, 60.0f, "%.1f"))
        c.fMaxFollowUpSeconds = std::clamp(maxFollowUp, 0.0f, 60.0f);
    HelpMarker("Random duration (seconds) an enemy stays in the active attacker role "
               "(not circling - free to pursue and attack its target) before being cycled "
               "back into the circling state. A random value is picked between Min and Max.");

    float minDefense = c.fMinDefenseSeconds;
    float maxDefense = c.fMaxDefenseSeconds;
    if (ImGuiMCP::SliderFloat("Min Defense Seconds##wyt", &minDefense, 0.0f, 60.0f, "%.1f"))
        c.fMinDefenseSeconds = std::clamp(minDefense, 0.0f, 60.0f);
    if (ImGuiMCP::SliderFloat("Max Defense Seconds##wyt", &maxDefense, 0.0f, 60.0f, "%.1f"))
        c.fMaxDefenseSeconds = std::clamp(maxDefense, 0.0f, 60.0f);
    HelpMarker("Random duration (seconds) an enemy "
               "is allowed to retaliate for any attacks it receives. This prevents the player from being able to spam attacks "
               "to an enemy when they are circling, preventing the player "
               "from piling up cheap shots. A random value is picked between Min and Max.");
    ImGuiMCP::Unindent();
}

void DrawProjectiles()
{
    auto& c = Settings::GetCircling();

    ImGuiMCP::Indent();
    ImGuiMCP::Checkbox("Slow Projectiles##wyt", &c.bProjectileSlowdown);
    HelpMarker("Slows projectiles when within a set distance of the player.");

    float radius = c.fProjectileSlowdownRadius;
    if (ImGuiMCP::SliderFloat("Slowdown Radius##wyt", &radius, 0.0f, 2048.0f, "%.0f"))
        c.fProjectileSlowdownRadius = (std::max)(radius, 0.0f);
    HelpMarker("Radius around the player within which projectiles are slowed.");

    float multiplier = c.fProjectileSlowdownMultiplier;
    if (ImGuiMCP::SliderFloat("Slowdown Multiplier##wyt", &multiplier, 0.0f, 1.0f, "%.2f"))
        c.fProjectileSlowdownMultiplier = std::clamp(multiplier, 0.0f, 1.0f);
    HelpMarker("Multiplier applied to projectile speed while inside the slowdown radius. "
               "0.0 = fully stopped, 1.0 = no slowdown.");
    ImGuiMCP::Unindent();
}

void DrawSystem()
{
    auto& c = Settings::GetCircling();

    ImGuiMCP::Checkbox("Debug Display##wyt", &c.bDebugDisplay);
    HelpMarker("Highlights attacking actors in white. Intended for debugging/development.");

    ImGuiMCP::Separator();

    if (ImGuiMCP::Button("Reset Defaults##wyt"))
    {
        Settings::RestoreDefaults();
    }
    HelpMarker("Restores all settings to their built-in defaults and saves them to the INI file.");

    ImGuiMCP::SameLine();

    if (ImGuiMCP::Button("Reload From INI##wyt"))
    {
        Settings::LoadSettings();
    }
    HelpMarker("Discards unsaved changes by reloading values from Settings.ini.");

    ImGuiMCP::SameLine();

    if (ImGuiMCP::Button("Save##wyt"))
    {
        Settings::SaveSettings();
    }
    HelpMarker("Writes the current settings to Settings.ini now, without waiting for the menu to close.");

    ImGuiMCP::Spacing();

    if (g_menuEvent)
    {
        ImGuiMCP::TextDisabled("Settings persist to Data/SKSE/Plugins/WaitYourTurn/Settings.ini when this menu closes.");
    }
    else
    {
        // Without the open/close hook there is nothing to save on the way out, so the Save
        // button is the only thing standing between the player and losing their changes.
        ImGuiMCP::TextWrapped("This build of SKSE Menu Framework does not report when the menu closes, "
                              "so press Save to keep your changes.");
    }
}
} // namespace

void __stdcall Render()
{
    if (ImGuiMCP::CollapsingHeader("Circling"))
        DrawCircling();
    if (ImGuiMCP::CollapsingHeader("Projectiles"))
        DrawProjectiles();
    if (ImGuiMCP::CollapsingHeader("System"))
        DrawSystem();
}

void __stdcall OnMenuEvent(SKSEMenuFramework::Model::EventType a_eventType)
{
    // These fire for the Mod Control Panel as a whole rather than for this page alone, which
    // is close enough to the per-tool OnOpen/OnClose the FLICK menu used: opening picks up
    // any hand-edit made to the INI, closing writes back whatever was changed here.
    switch (a_eventType)
    {
    case SKSEMenuFramework::Model::kOpenMenu:
        Settings::LoadSettings();
        break;
    case SKSEMenuFramework::Model::kCloseMenu:
        Settings::SaveSettings();
        break;
    default:
        break;
    }
}

void RegisterMenu()
{
    if (!SKSEMenuFramework::IsInstalled())
    {
        SKSE::log::info("SKSE Menu Framework is not installed; settings will be read from Settings.ini only");

        return;
    }

    if (!HasRequiredExports())
    {
        SKSE::log::error("The installed SKSE Menu Framework is older than this plugin's menu needs. "
                         "Update it to version 3 or newer to configure Wait Your Turn in game.");

        return;
    }

    SKSEMenuFramework::SetSection("Wait Your Turn");
    SKSEMenuFramework::AddSectionItem("Settings", Render);

    // Saving on close is a convenience rather than a requirement, so only wire it up if this
    // build of the framework offers it, and fall back to the Save button if it does not.
    if (GetMenuFrameworkFunction<void*>("RegisterEventPriority"))
    {
        g_menuEvent = SKSEMenuFramework::AddEvent(OnMenuEvent, 0.0f);
    }
    else
    {
        SKSE::log::info("SKSE Menu Framework does not export \"RegisterEventPriority\"; "
                        "settings will only be saved when the Save button is pressed");
    }

    SKSE::log::info("Registered the settings page with SKSE Menu Framework");
}
} // namespace Menu
} // namespace WaitYourTurn
