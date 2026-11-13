TOTAL WARFARE OVERHAUL
Version 1.2.0
For Mercenaries: Playground of Destruction (Xbox) - Recompiled 1.1.1
Mod Manager / Mod API 2 + optional MercFx (Native Effects) integration

WHAT THIS MOD PROVIDES
  - A Mod Manager MODS page (15 settings) for: smart bomb, surgical strike,
    carpet bomb, bunker buster (OFF/HEAVY/NUKE), cruise missile (OFF/HEAVY/
    THERMO/FOAB), tank buster, strategic nuke, C4 mega blast (1KM/2KM intent),
    weapon realism intent, mobility (parachute/jetpack intent), world freedom
    (no zones/expand/unify intent), vehicles (air/sea/all intent), shop profile,
    and debug logging.
  - Settings saved to mods\total_warfare.ini through the Mod Manager.
  - Optional Native Effects (MercFx) integration (v1.2.0, best-effort): when
    native_effects.dll is loaded and the MercFx API is available, the mod
    registers an effect and binds supported ammo templates. On impact it applies
    enhanced blasts (scaled damage/radius/force), camera shake/scorch, shockwaves,
    and slowdown. NUKE-level profiles (BUNKER BUSTER: NUKE, CRUISE: THERMO/FOAB,
    STRATEGIC NUKE: ON) also trigger a Meltdown visual via the game's explosion
    call when available.
  - Expanded coverage in v1.2.0: smart/surgical/carpet/cluster/fuel-air/artillery
    airstrikes, bunker buster, cruise/air-superiority, tank buster/guided missiles,
    strategic/meltdown, and best-effort C4 and explosive weapon impacts (grenades,
    RPG/LAW/ATGM/TOW/Hellfire, tank shells/cannon, mortars) when those templates
    produce MercFx impact events. First template bind wins if another FX mod owns it.
  - Best-effort localization string registration and extra shop-item hooks via
    Mod API (add_shop_item/register_string) where supported.

REQUIREMENTS / COMPATIBILITY
  - Mercenaries Recompiled 1.1.1, Mod Manager 2.x
  - For enhanced blast/shockwave/nuke FX: Native Effects plugin enabled
    (native_effects.dll / MercFx). Without it, the DLL still loads as a
    settings/profile/logging mod but cannot modify airstrike blasts.
  - Template binding is first-come: if Real Nuke, MOAB, or another MercFx mod
    already bound a template, this mod cannot take it over (check bridge log).
    You can run them together; specialized FX mods may provide richer visuals
    (custom particles/timelines/assets) for templates they own.

LIMITATIONS (IMPORTANT)
  - C4 detonator reach (1-2 km): not possible via this API. v1.2.0 can only
    enhance C4 blast on impact if MercFx reports a C4 impact event; actual
    detonation range/behavior needs runtime hooks or game-data mods.
  - Weapon realism (bullets, tracers/fire, shells, tank guns, missiles/hellfire
    flight/ballistics): only explosive impact blasts can be enhanced best-effort
    via MercFx. True ballistics/physics/tracers need runtime hooks or game-data.
  - Parachute / jetpack: cannot be added by this mod (needs movement/animation/
    runtime support or a dedicated mod).
  - Flyable F-15/other jets/planes and boats: needs vehicle assets, templates,
    controls, and runtime hooks; not provided by this DLL.
  - No restricted zones / remove barriers / expand or unify ACE 1-4 North+South:
    needs build-specific memory patches or data/script mods. NoRestrictedZone
    scaffolds in the provided mods are not working implementations.
  - Native "mushroom cloud" quality depends on available effects/assets: this
    mod fires Meltdown visual for nuke-level strikes when possible, but full
    custom mushroom/FOAB particle timelines and assets (as in Real Nuke/MOAB
    with their DATAxbox/mod.dsk) come from those dedicated FX mods.

INSTALL (MOD MANAGER MOD FOLDER)
  1. Build total_warfare.dll (see BUILD below) or place a prebuilt DLL at:
       mods\total_warfare.dll
     IMPORTANT: Mod Manager loads code mods from mods\*.dll. If total_warfare.dll
     is missing, the mod will not load and will not appear in the MODS menu.
     (Required files for build: total_warfare.c, mercenaries_mod.h, mercfx.h)
  2. Copy the "Total Warfare Overhaul" folder into your game's mods folder.
     The folder must contain:
       mods\total_warfare.dll   (compiled from total_warfare.c)
       mods\total_warfare.ini
       mods\total_warfare\      (this docs folder)
  3. Enable "Mod Manager" and "Total Warfare Overhaul" in the selector, then
     LAUNCH MODDED. For enhanced blast FX, also enable "Native Effects".
  4. Open MODS in-game to configure TOTAL WARFARE OVERHAUL.

FALLBACK PROFILE (OPTIONAL, UI ONLY)
  Copy total-warfare.profile from this docs folder to:
    <game>\mods\Mod Manager\profiles\total-warfare.profile
  This shows a profile-based settings page for mods\total_warfare.ini when that
  ini exists in the game folder. It does NOT provide DLL runtime hooks, MercFx
  integration, logging, or shop registration. Build/load total_warfare.dll for
  actual mod behavior.

BUILD (64-bit Windows DLL)
  Keep mercenaries_mod.h and mercfx.h in the same folder as total_warfare.c, then run one of:

    Visual Studio Developer Command Prompt:
      cl /nologo /W4 /O2 /LD /I . total_warfare.c /Fe:total_warfare.dll /link /MACHINE:X64

    MinGW-w64:
      x86_64-w64-mingw32-gcc -shared -O2 -Wall -Wextra -I. total_warfare.c -o total_warfare.dll

    clang-cl target:
      clang --target=x86_64-pc-windows-msvc -shared -O2 -I. total_warfare.c -o total_warfare.dll

  Or run BUILD_WINDOWS.cmd from an x64 developer environment. Copy the resulting
  total_warfare.dll to the mod's mods\ folder.

RECOMMENDED COMPANION MODS
  - Native Effects (required for MercFx blast/shockwave/slowmo/meltdown integration)
  - Real Nuke / MOAB (richer dedicated nuke/FOAB visuals, custom particles/assets)
  - Merchant of Menace Expanded (broader shop/ordnance/vehicle deliveries)
  - A working build-specific no-zone patch mod, if available (NoRestrictedZone scaffold is not enough)

TROUBLESHOOTING
  - Bridge log: "loaded total_warfare.dll: TOTAL WARFARE OVERHAUL 1.2.0, 15 settings"
  - Look for "MercFx attached..." and "enhanced blast on ..." / bind lines (DEBUG LOG).
  - If templates are "unavailable/owned", another FX mod bound them first.
  - If features have no effect, confirm the DLL is loaded, Native Effects is enabled,
    and note the limitations above for C4/physics/mobility/vehicles/map restrictions.

CREDITS
  BY GLYPH CLUSTER
  MercFx integration pattern informed by Svered's Native Effects / Real Nuke mod API usage.
  Built for Mercenaries Recompiled 1.1.1 Mod Manager (Mod API 2).
