#pragma once

#include <spdlog/sinks/basic_file_sink.h>

#include "settings.h"

inline void SetupLog() {
    auto logsFolder = SKSE::log::log_directory();
    if (!logsFolder) SKSE::stl::report_and_fail("SKSE log_directory not provided, logs disabled.");
    auto pluginName = SKSE::PluginDeclaration::GetSingleton()->GetName();
    auto logFilePath = *logsFolder / std::format("{}.log", pluginName);
    auto fileLoggerPtr = std::make_shared<spdlog::sinks::basic_file_sink_mt>(logFilePath.string(), true);
    auto loggerPtr = std::make_shared<spdlog::logger>("log", std::move(fileLoggerPtr));
    spdlog::set_default_logger(std::move(loggerPtr));
    // Starts wide open so everything before the INI is read (hook installation, plugin load)
    // is captured. ApplyLogLevel() narrows it to whatever uLogLevel asks for once
    // Settings::LoadSettings() has run at kDataLoaded.
    spdlog::set_level(spdlog::level::trace);
    spdlog::flush_on(spdlog::level::trace);
}

// Applies the [Debug] uLogLevel from Settings.ini to the logger. Call after
// Settings::LoadSettings(), and again whenever the value changes (the settings page does).
// Also writes a one-line header naming the active level and what it drops, so a log sent in
// with a bug report is self-describing even when the INI that produced it cannot be checked.
inline void ApplyLogLevel() {
    constexpr const char* names[] = { "trace", "debug", "info", "warning", "error", "critical", "off" };

    auto raw = WaitYourTurn::Settings::GetDebug().uLogLevel;
    if (raw > static_cast<uint32_t>(spdlog::level::off)) raw = 0;

    auto level = static_cast<spdlog::level::level_enum>(raw);
    spdlog::set_level(level);
    spdlog::flush_on(level);

    if (level == spdlog::level::off) return;

    spdlog::default_logger()->log(level, "Log level {} ({}). Set uLogLevel=0 under [Debug] in "
                                         "Data/SKSE/Plugins/WaitYourTurn/Settings.ini to capture everything.",
                                  raw, names[raw]);
}
