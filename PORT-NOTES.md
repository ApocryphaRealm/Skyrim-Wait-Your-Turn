# Port notes — FLICK NG → SKSE Menu Framework

This is a clone of [Monitor221hz/Skyrim-Wait-Your-Turn](https://github.com/Monitor221hz/Skyrim-Wait-Your-Turn)
with its in-game settings menu moved from [FLICK NG](https://www.nexusmods.com/skyrimspecialedition/mods/181603)
to [SKSE Menu Framework 3](https://github.com/QTR-Modding/SKSE-Menu-Framework-3).

The upstream `README.md` and git history are untouched. The packaged, installable result
lives in `../Wait Your Turn Redux - SMF Settings`.

**This project is AGPL-3.0**, inherited from upstream. Distributing a built DLL obliges you
to make this modified source available under the same licence. See `LICENSE`.

## What changed

| File | Change |
| --- | --- |
| `src/SKSEMenuFramework.h` | Added. Vendored unmodified from SKSE Menu Framework 3.13. Self-contained — it reaches the framework through `GetProcAddress`, so nothing is linked and ImGui is not a build dependency. |
| `src/FUCK_API.h` | Deleted. The FLICK API is no longer used. |
| `src/Menu/ingame-menu.h`, `src/Menu/ingame-menu.cpp` | Rewritten against SKSE Menu Framework. Same three sections, same settings, same help text. |
| `src/plugin.cpp` | `Menu::RegisterMenu()` moved out of `SKSEPluginLoad` into the `kPostPostLoad` message. |
| `xmake.lua` | `imgui` dropped from `add_requires`/`add_packages`. |

Nothing outside the menu was touched — the circling system, hooks, events and
`src/settings.h` are exactly as upstream left them.

### Why registration moved

Upstream called `Menu::RegisterMenu()` directly from `SKSEPluginLoad`. FLICK's `Connect`
tolerated that, but SKSE Menu Framework is found with `GetModuleHandleW`, which only works
once its DLL is actually in the process. At `SKSEPluginLoad` time the load order may not
have reached it yet. `kPostPostLoad` fires after every plugin has finished loading, so the
lookup is reliable there.

### How the API maps across

FLICK's helpers line up almost one-to-one with `ImGuiMCP::`. The two that did not:

- **`FUCK::HelpMarker`** has no equivalent, so it is reimplemented locally as
  `SameLine` + `TextDisabled("(?)")` + `SetTooltip`.
- **`ITool::OnOpen` / `OnClose`** have no per-page equivalent. The nearest thing is
  `SKSEMenuFramework::AddEvent`, whose `kOpenMenu`/`kCloseMenu` fire for the Mod Control
  Panel as a whole. That is what the port uses, so load-on-open and save-on-close still
  happen — just for any opening of the panel, not only for this page. A `Save` button was
  added so the player never depends on that behaviour.

### Two things the port guards against

**An old framework would crash it.** The vendored header calls the framework's exported
cimgui functions (`igSliderFloat`, `igCheckbox`, …) through raw function pointers with no
null checks. Versions 1 and 2 do not export them — confirmed by dumping the exports of the
DLL bundled with the old SDK, which offers only `AddSectionItem`. `HasRequiredExports()`
probes all sixteen entry points this page uses and declines to register if any is absent,
logging which one. The mod itself keeps working; only the menu is skipped.

**`RegisterEventPriority` is treated as optional.** It is probed separately from the
mandatory widget exports, so a framework build that lacks it still gets a working page —
the System section then tells the player to use Save instead of relying on close-to-save.

## Building

Upstream builds with **xmake**, not CMake, and pulls CommonLibSSE-NG in as a git submodule.

```
git clone --recurse-submodules <this repo>
build.bat
```

`build.bat` points at a portable xmake unpacked under the session scratchpad and at the
Visual Studio 18 install on the machine it was written on — edit those two paths if yours
differ, or just put `xmake` on your `PATH` and run `xmake f -m releasedbg && xmake`.

Output lands in `build/windows/x64/releasedbg/WaitYourTurn.dll`. A full build from cold
takes roughly two minutes.

## Licence

GNU AGPL v3, as upstream. Original work by monitor221hz.
