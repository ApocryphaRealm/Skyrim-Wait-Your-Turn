#pragma once

#include "../include/SimpleIni.h"
#include "FUCK_API.h"

namespace WaitYourTurn
{
namespace Menu
{
void RegisterMenu();

struct WYTTool : FUCK::ITool
{
    const char* PluginName() const override { return "WaitYourTurn"; }
    const char* Name() const override { return "Wait Your Turn"; }
    const char* Group() const override { return "Wait Your Turn"; }
    void        Draw() override;
    void        OnOpen() override;
    void        OnClose() override;

private:
    void DrawCircling();
    void DrawProjectiles();
    void DrawSystem();
};

inline WYTTool g_wytTool;
} // namespace Menu
} // namespace WaitYourTurn
