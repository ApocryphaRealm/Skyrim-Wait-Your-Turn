#include "ingame-menu.h"

#include "settings.h"

#include <algorithm>
#include <cstddef>

namespace WaitYourTurn
{
namespace Menu
{
void WYTTool::Draw()
{
    if (FUCK::CollapsingHeader("Circling"))
        DrawCircling();
    if (FUCK::CollapsingHeader("Projectiles"))
        DrawProjectiles();
    if (FUCK::CollapsingHeader("System"))
        DrawSystem();
}

void WYTTool::DrawCircling()
{
    auto& c = Settings::GetCircling();

    FUCK::Indent();
    FUCK::Checkbox("Legacy Circling##wyt", &c.bLegacyCircling);
    FUCK::HelpMarker("Whether to use legacy packages for circling behavior (same behavior as the "
                     "original papyrus mod). Using legacy packages REQUIRES WaitYourTurnRedux.esp "
                     "to be present and enabled in the load order.");
    FUCK::Checkbox("Player Only##wyt", &c.bPlayerOnly);
    FUCK::HelpMarker("If disabled, circling applies to ALL NPCs instead of just the player.");
    FUCK::Checkbox("Include Followers##wyt", &c.bIncludeFollowers);
    FUCK::HelpMarker("Allows followers to benefit from the circling system alongside the player.");
    FUCK::Checkbox("Include Ranged / Magic##wyt", &c.bIncludeRangedMagic);
    FUCK::HelpMarker("Whether ranged/magic NPCs are included in the circling system.");
    FUCK::Checkbox("Transform Protection##wyt", &c.bTransformProtection);
    FUCK::HelpMarker("Stops attackers during werewolf / vampire lord transformation.");
    FUCK::Checkbox("Killmove Protection##wyt", &c.bKillmoveProtection);
    FUCK::HelpMarker("Prevents killmoves on actors that are currently being circled.");

    int maxAttackers = static_cast<int>(c.iMaxAttackers);
    if (FUCK::SliderInt("Max Attackers##wyt", &maxAttackers, 1, 16, "%d"))
        c.iMaxAttackers = static_cast<size_t>(std::clamp(maxAttackers, 1, 16));
    FUCK::HelpMarker("Maximum number of attackers allowed to engage a single target at once.");

    float minFollowUp = c.fMinFollowUpSeconds;
    float maxFollowUp = c.fMaxFollowUpSeconds;
    if (FUCK::SliderFloat("Min Follow-Up Seconds##wyt", &minFollowUp, 0.0f, 60.0f, "%.1f"))
        c.fMinFollowUpSeconds = std::clamp(minFollowUp, 0.0f, 60.0f);
    if (FUCK::SliderFloat("Max Follow-Up Seconds##wyt", &maxFollowUp, 0.0f, 60.0f, "%.1f"))
        c.fMaxFollowUpSeconds = std::clamp(maxFollowUp, 0.0f, 60.0f);
    FUCK::HelpMarker("Random duration (seconds) an enemy stays in the active attacker role "
                     "(not circling - free to pursue and attack its target) before being cycled "
                     "back into the circling state. A random value is picked between Min and Max.");

    float minDefense = c.fMinDefenseSeconds;
    float maxDefense = c.fMaxDefenseSeconds;
    if (FUCK::SliderFloat("Min Defense Seconds##wyt", &minDefense, 0.0f, 60.0f, "%.1f"))
        c.fMinDefenseSeconds = std::clamp(minDefense, 0.0f, 60.0f);
    if (FUCK::SliderFloat("Max Defense Seconds##wyt", &maxDefense, 0.0f, 60.0f, "%.1f"))
        c.fMaxDefenseSeconds = std::clamp(maxDefense, 0.0f, 60.0f);
    FUCK::HelpMarker("Random duration (seconds) an enemy "
                     "is allowed to retaliate for any attacks it receives. This prevents the player from being able to spam attacks "
                     "to an enemy when they are circling, preventing the player "
                     "from piling up cheap shots. A random value is picked between Min and Max.");
    FUCK::Unindent();
}

void WYTTool::DrawProjectiles()
{
    auto& c = Settings::GetCircling();

    FUCK::Indent();
    FUCK::Checkbox("Slow Projectiles##wyt", &c.bProjectileSlowdown);
    FUCK::HelpMarker("Slows projectiles when within a set distance of the player.");

    float radius = c.fProjectileSlowdownRadius;
    if (FUCK::SliderFloat("Slowdown Radius##wyt", &radius, 0.0f, 2048.0f, "%.0f"))
        c.fProjectileSlowdownRadius = (std::max)(radius, 0.0f);
    FUCK::HelpMarker("Radius around the player within which projectiles are slowed.");

    float multiplier = c.fProjectileSlowdownMultiplier;
    if (FUCK::SliderFloat("Slowdown Multiplier##wyt", &multiplier, 0.0f, 1.0f, "%.2f"))
        c.fProjectileSlowdownMultiplier = std::clamp(multiplier, 0.0f, 1.0f);
    FUCK::HelpMarker("Multiplier applied to projectile speed while inside the slowdown radius. "
                     "0.0 = fully stopped, 1.0 = no slowdown.");
    FUCK::Unindent();
}

void WYTTool::DrawSystem()
{
    auto& c = Settings::GetCircling();

    FUCK::Checkbox("Debug Display##wyt", &c.bDebugDisplay);
    FUCK::HelpMarker("Highlights attacking actors in white. Intended for debugging/development.");

    FUCK::Separator();

    if (FUCK::Button("Reset Defaults##wyt"))
    {
        Settings::RestoreDefaults();
    }
    FUCK::HelpMarker("Restores all settings to their built-in defaults and saves them to the INI file.");

    FUCK::SameLine();

    if (FUCK::Button("Reload From INI##wyt"))
    {
        Settings::LoadSettings();
    }
    FUCK::HelpMarker("Discards unsaved changes by reloading values from Settings.ini.");

    FUCK::TextDisabled("Settings persist to Data/SKSE/Plugins/WaitYourTurn/Settings.ini when this menu closes.");
}

void WYTTool::OnOpen()
{
    Settings::LoadSettings();
}

void WYTTool::OnClose()
{
    Settings::SaveSettings();
}

void RegisterMenu()
{
    if (FUCK::Connect("WaitYourTurn", 1))
    {
        FUCK::RegisterTool(&g_wytTool);
    }
}
} // namespace Menu
} // namespace WaitYourTurn
