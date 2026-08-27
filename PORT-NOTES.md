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
| `build.bat`, `find-msvc.bat` | Added. Discover Visual Studio with `vswhere` and take xmake from `PATH`, so no machine-specific path is committed. |

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

## 1.0.2: null-safety audit (preventative, no known crash in this mod)

Standing rule now (`CLAUDE.md` rule 14, in the sibling Skyrim mods project): every mod gets
audited for unchecked null-dereference lookups, not just the one where the pattern actually
crashed (Dragon's Eye Minimap). This mod's own settings load through `CSimpleIniA`
(SimpleIni), which returns default values directly rather than a pointer, so the specific
`GetSetting(...)->data` crash class doesn't apply here. Three real unchecked derefs were
found and fixed in `src/hook.cpp` instead: `CombatGroupHook::Destroy` dereferenced
`a_controller->combatGroup` and `a_controller->attackerHandle.get()` unconditionally;
`CombatGroupHook::Update` dereferenced `a_group->lock` without checking `a_group`, unlike
every sibling hook in the same file which already checked their own `a_group` parameter;
`CombatGroupHook::StopCombat` (currently dead code, not wired up in `Install()`) had the
same missing check, fixed for consistency. No known crash was ever reported for this mod
specifically - this pass is preventative.

## 1.0.3: `uLogLevel` surfaced to the INI and the settings page

Standing rule (`CLAUDE.md` rule 28, in the sibling Skyrim mods project): every mod ships with
the most comprehensive log level as its default, in both the shipped INI and the compiled-in
default, and the two must agree exactly - so a bug report arrives with a log detailed enough
to diagnose from without asking the reporter to reproduce anything.

This mod already logged at trace, but hardcoded in `SetupLog()`: there was no way to turn it
down, and nothing in the INI named the level at all. A new `[Debug]` section in
`ini/Settings.ini` now carries `uLogLevel` (0 = trace ... 6 = off, indexing spdlog's own level
table), read through the same `CSimpleIniA`/`NAMEOF` pattern as every other setting in
`settings.h`, with the struct's own initializer - `uint32_t uLogLevel = 0` - as the compiled
default matching the shipped INI.

`SetupLog()` still opens at trace, deliberately: the INI is not read until `kDataLoaded`, so
everything before that (plugin load, hook installation) is captured whatever `uLogLevel` turns
out to say. `ApplyLogLevel()` narrows it from there, and is called again whenever the value can
change - the settings page's Log Level combo, Reload From INI, Reset Defaults, and the Mod
Control Panel's own open event. It also writes a one-line header naming the active level, at
that level, so a log file sent in with a report is self-describing.

`log.h`'s two functions are `inline` now, because the header is included from two translation
units (`plugin.cpp` and `Menu/ingame-menu.cpp`) rather than one.

## Building

Upstream builds with **xmake**, not CMake, and pulls CommonLibSSE-NG in as a git submodule.

You need three things:

- **Visual Studio** with the *Desktop development with C++* workload. Community is fine.
- **xmake** from <https://xmake.io>. The portable zip from its GitHub releases works and
  needs no installer or admin rights; put the folder on your `PATH`.
- **git**, on your `PATH`. xmake shells out to it, and without it xmake misdetects the
  host as MSYS and fails while trying to fetch git as a host package.

```
git clone --recurse-submodules <this repo>
build.bat
```

The `--recurse-submodules` matters: CommonLibSSE-NG is a submodule, and the build cannot
find it otherwise.

Nothing in the build scripts is specific to one machine. `find-msvc.bat` locates Visual
Studio with `vswhere` and puts the CMake and Ninja that ship with it on `PATH`, and
`build.bat` takes xmake from `PATH`, or from an `XMAKE` environment variable pointing at
the executable if you keep it somewhere unusual.

Output lands in `build/windows/x64/releasedbg/WaitYourTurn.dll`. A full build from cold
takes roughly two minutes.

## Licence

GNU AGPL v3, as upstream. Original work by monitor221hz.
