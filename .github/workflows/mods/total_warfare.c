/* SPDX-License-Identifier: MIT */
/*
 * Total Warfare Overhaul
 * Mercenaries: Playground of Destruction (Xbox) - Recompiled 1.1.1
 * Mod Manager / Mod API 2 DLL mod, with optional MercFx (Native Effects) integration.
 *
 * Build (64-bit Windows DLL), with mercenaries_mod.h and mercfx.h beside this file:
 *   cl /nologo /W4 /O2 /LD total_warfare.c /I . /Fe:total_warfare.dll /link /MACHINE:X64
 *   x86_64-w64-mingw32-gcc -shared -O2 -Wall -Wextra -I. total_warfare.c -o total_warfare.dll
 *   clang --target=x86_64-pc-windows-msvc -shared -O2 -I. total_warfare.c -o total_warfare.dll
 */

#define WIN32_LEAN_AND_MEAN
#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <math.h>
#include <stdio.h>

#include "mercenaries_mod.h"
#include "mercfx.h"

#define MOD_NAME      "TOTAL WARFARE OVERHAUL"
#define MOD_VERSION   "1.2.0"
#define MOD_CREDITS   "BY GLYPH CLUSTER"
#define SETTINGS_FILE "mods\\total_warfare.ini"
#define SECTION       "TotalWarfare"
#define FX_EFFECT     "totalwarfare.fx"
#define FX_OWNER      "totalwarfare"
#define EXPLOSION_FN  0x000B6460u

enum {
    SET_MASTER = 0,
    SET_SMART_BOMB,
    SET_SURGICAL,
    SET_CARPET,
    SET_BUNKER_NUKE,
    SET_CRUISE_FOAB,
    SET_TANK_BUSTER,
    SET_STRATEGIC_NUKE,
    SET_C4_MEGA,
    SET_WEAPON_REALISM,
    SET_MOBILITY,
    SET_WORLD_FREEDOM,
    SET_VEHICLES,
    SET_SHOP_PROFILE,
    SET_DEBUG_LOG,
    SETTING_COUNT
};

enum {
    CAT_SMART = 0,
    CAT_SURGICAL,
    CAT_CARPET,
    CAT_BUNKER,
    CAT_CRUISE,
    CAT_TANK,
    CAT_STRATEGIC,
    CAT_C4,
    CAT_WEAPON,
    CAT_COUNT
};

typedef struct { const char *name; unsigned char category; } TriggerTemplate;

static const TriggerTemplate kTriggers[] = {
    /* Smart / precision bombs */
    {"template_amm_smartbomb",        CAT_SMART},
    {"template_amm_smartbomb0",       CAT_SMART},
    {"template_amm_smartbomb2",       CAT_SMART},
    {"template_amm_smartbomb3",       CAT_SMART},
    {"template_amm_hvysmartbomb",     CAT_SMART},
    {"template_amm_hvysmartbomb0",    CAT_SMART},
    /* Surgical strike */
    {"template_amm_surgicalstrike",   CAT_SURGICAL},
    {"template_amm_surgicalstrike0",  CAT_SURGICAL},
    /* Carpet / cluster / area bombardment */
    {"template_amm_carpetbomb",       CAT_CARPET},
    {"template_amm_carpetbomb0",      CAT_CARPET},
    {"template_amm_carpetbomblet",    CAT_CARPET},
    {"template_amm_clusterbomb",      CAT_CARPET},
    {"template_amm_clusterbomb0",     CAT_CARPET},
    {"template_amm_clusterbombBomb",  CAT_CARPET},
    {"template_amm_clusterbomblet",   CAT_CARPET},
    {"template_amm_clusterbomblet0",  CAT_CARPET},
    {"template_amm_fuelairbomb",      CAT_CARPET},
    {"template_amm_fuelairbomb0",     CAT_CARPET},
    {"template_amm_artillery",        CAT_CARPET},
    {"template_amm_artillery0",       CAT_CARPET},
    {"template_amm_artillery2",       CAT_CARPET},
    {"template_amm_artillery3",       CAT_CARPET},
    /* Bunker buster */
    {"template_amm_bunkerbuster",     CAT_BUNKER},
    {"template_amm_bunkerbuster0",    CAT_BUNKER},
    {"template_amm_bunkerbusterbomb", CAT_BUNKER},
    /* Cruise / heavy missile */
    {"template_amm_cruisemissile",    CAT_CRUISE},
    {"template_amm_cruisemissile0",   CAT_CRUISE},
    {"template_amm_airsuperiority",   CAT_CRUISE},
    {"template_amm_airsuperiority0",  CAT_CRUISE},
    /* Tank buster / guided anti-armor */
    {"template_amm_tankbuster",       CAT_TANK},
    {"template_amm_tankbuster0",      CAT_TANK},
    {"template_amm_guidedmissile",    CAT_TANK},
    {"template_amm_guidedmissile0",   CAT_TANK},
    /* Strategic / nuclear */
    {"template_amm_strategicmissile", CAT_STRATEGIC},
    {"template_amm_strategicmissile0",CAT_STRATEGIC},
    {"template_amm_meltdown",         CAT_STRATEGIC},
    /* C4 / placed explosives (best-effort if MercFx reports impacts) */
    {"template_amm_c4",               CAT_C4},
    {"template_amm_c4explosive",      CAT_C4},
    {"template_amm_c4_0",             CAT_C4},
    {"template_amm_satchel",          CAT_C4},
    /* Weapons / other explosives (best-effort) */
    {"template_amm_grenade",          CAT_WEAPON},
    {"template_amm_fraggrenade",      CAT_WEAPON},
    {"template_amm_rpg",              CAT_WEAPON},
    {"template_amm_rpg7",             CAT_WEAPON},
    {"template_amm_law",              CAT_WEAPON},
    {"template_amm_atgm",             CAT_WEAPON},
    {"template_amm_tow",              CAT_WEAPON},
    {"template_amm_hellfire",         CAT_WEAPON},
    {"template_amm_missile",          CAT_WEAPON},
    {"template_amm_tankround",        CAT_WEAPON},
    {"template_amm_tankshell",        CAT_WEAPON},
    {"template_amm_cannon",           CAT_WEAPON},
    {"template_amm_mortar",           CAT_WEAPON},
};
#define TRIGGER_COUNT (sizeof(kTriggers)/sizeof(kTriggers[0]))

static const MercenariesModHost *g_host;
static unsigned g_settings[SETTING_COUNT];
static unsigned g_ticks;
static unsigned g_announced;
static unsigned g_shop_attempted;

static const MercFxApi *g_fx;
static int g_fx_state; /* 0 try, 1 pending, 2 attached, 3 unavailable */
static unsigned g_fx_misses;
static unsigned g_fx_attempts;
static uint32_t g_template_hash[TRIGGER_COUNT];
static unsigned char g_bound[TRIGGER_COUNT];
static unsigned g_bound_count;
static double g_world_time;
static double g_next_rebind;
static int g_pending_meltdown;
static float g_pending_pos[3];
static uint32_t g_meltdown_hash;
static int g_have_explosion_fn;
static int g_checked_retail;
static int g_pending_log_fail;
static char g_pending_log_tpl[64];

static const char *const on_off[] = {"OFF", "ON"};
static const char *const bunker_values[] = {"OFF", "HEAVY", "NUKE"};
static const char *const cruise_values[] = {"OFF", "HEAVY", "THERMO", "FOAB"};
static const char *const c4_values[] = {"OFF", "MEGA", "MEGA 1KM", "MEGA 2KM"};
static const char *const mobility_values[] = {"OFF", "CHUTE", "JETPACK", "BOTH"};
static const char *const world_values[] = {"OFF", "NO ZONES", "EXPAND", "UNIFY"};
static const char *const vehicle_values[] = {"OFF", "AIR", "SEA", "ALL"};
static const char *const shop_values[] = {"OFF", "ON", "MAX"};

static const unsigned setting_defaults[SETTING_COUNT] = {
    1u, 1u, 1u, 1u, 2u, 3u, 1u, 1u, 3u, 1u, 3u, 3u, 3u, 2u, 0u
};

static unsigned clamp_value(unsigned value, unsigned count)
{
    return value < count ? value : (count ? count - 1u : 0u);
}

static void save_setting(const char *key, unsigned value)
{
    if (g_host) g_host->write_setting(SETTINGS_FILE, SECTION, key, value);
}

static unsigned get_master(void) { return g_settings[SET_MASTER]; }
static void set_master(unsigned v) { g_settings[SET_MASTER] = clamp_value(v, 2u); save_setting("Master", g_settings[SET_MASTER]); }
static unsigned get_smart(void) { return g_settings[SET_SMART_BOMB]; }
static void set_smart(unsigned v) { g_settings[SET_SMART_BOMB] = clamp_value(v, 2u); save_setting("SmartBomb", g_settings[SET_SMART_BOMB]); }
static unsigned get_surgical(void) { return g_settings[SET_SURGICAL]; }
static void set_surgical(unsigned v) { g_settings[SET_SURGICAL] = clamp_value(v, 2u); save_setting("SurgicalStrike", g_settings[SET_SURGICAL]); }
static unsigned get_carpet(void) { return g_settings[SET_CARPET]; }
static void set_carpet(unsigned v) { g_settings[SET_CARPET] = clamp_value(v, 2u); save_setting("CarpetBomb", g_settings[SET_CARPET]); }
static unsigned get_bunker(void) { return g_settings[SET_BUNKER_NUKE]; }
static void set_bunker(unsigned v) { g_settings[SET_BUNKER_NUKE] = clamp_value(v, 3u); save_setting("BunkerBuster", g_settings[SET_BUNKER_NUKE]); }
static unsigned get_cruise(void) { return g_settings[SET_CRUISE_FOAB]; }
static void set_cruise(unsigned v) { g_settings[SET_CRUISE_FOAB] = clamp_value(v, 4u); save_setting("CruiseFOAB", g_settings[SET_CRUISE_FOAB]); }
static unsigned get_tankbuster(void) { return g_settings[SET_TANK_BUSTER]; }
static void set_tankbuster(unsigned v) { g_settings[SET_TANK_BUSTER] = clamp_value(v, 2u); save_setting("TankBuster", g_settings[SET_TANK_BUSTER]); }
static unsigned get_strategic(void) { return g_settings[SET_STRATEGIC_NUKE]; }
static void set_strategic(unsigned v) { g_settings[SET_STRATEGIC_NUKE] = clamp_value(v, 2u); save_setting("StrategicNuke", g_settings[SET_STRATEGIC_NUKE]); }
static unsigned get_c4(void) { return g_settings[SET_C4_MEGA]; }
static void set_c4(unsigned v) { g_settings[SET_C4_MEGA] = clamp_value(v, 4u); save_setting("C4Mega", g_settings[SET_C4_MEGA]); }
static unsigned get_realism(void) { return g_settings[SET_WEAPON_REALISM]; }
static void set_realism(unsigned v) { g_settings[SET_WEAPON_REALISM] = clamp_value(v, 2u); save_setting("WeaponRealism", g_settings[SET_WEAPON_REALISM]); }
static unsigned get_mobility(void) { return g_settings[SET_MOBILITY]; }
static void set_mobility(unsigned v) { g_settings[SET_MOBILITY] = clamp_value(v, 4u); save_setting("Mobility", g_settings[SET_MOBILITY]); }
static unsigned get_world(void) { return g_settings[SET_WORLD_FREEDOM]; }
static void set_world(unsigned v) { g_settings[SET_WORLD_FREEDOM] = clamp_value(v, 4u); save_setting("WorldFreedom", g_settings[SET_WORLD_FREEDOM]); }
static unsigned get_vehicles(void) { return g_settings[SET_VEHICLES]; }
static void set_vehicles(unsigned v) { g_settings[SET_VEHICLES] = clamp_value(v, 4u); save_setting("Vehicles", g_settings[SET_VEHICLES]); }
static unsigned get_shop(void) { return g_settings[SET_SHOP_PROFILE]; }
static void set_shop(unsigned v) { g_settings[SET_SHOP_PROFILE] = clamp_value(v, 3u); save_setting("ShopProfile", g_settings[SET_SHOP_PROFILE]); }
static unsigned get_debug(void) { return g_settings[SET_DEBUG_LOG]; }
static void set_debug(unsigned v) { g_settings[SET_DEBUG_LOG] = clamp_value(v, 2u); save_setting("DebugLog", g_settings[SET_DEBUG_LOG]); }

static const MercenariesModSetting settings[SETTING_COUNT] = {
    {"ENABLED", on_off, 2u, get_master, set_master},
    {"SMART BOMB", on_off, 2u, get_smart, set_smart},
    {"SURGICAL STRIKE", on_off, 2u, get_surgical, set_surgical},
    {"CARPET BOMB", on_off, 2u, get_carpet, set_carpet},
    {"BUNKER BUSTER", bunker_values, 3u, get_bunker, set_bunker},
    {"CRUISE MISSILE", cruise_values, 4u, get_cruise, set_cruise},
    {"TANK BUSTER", on_off, 2u, get_tankbuster, set_tankbuster},
    {"STRATEGIC NUKE", on_off, 2u, get_strategic, set_strategic},
    {"C4 MEGA BLAST", c4_values, 4u, get_c4, set_c4},
    {"WEAPON REALISM", on_off, 2u, get_realism, set_realism},
    {"MOBILITY", mobility_values, 4u, get_mobility, set_mobility},
    {"WORLD FREEDOM", world_values, 4u, get_world, set_world},
    {"VEHICLES", vehicle_values, 4u, get_vehicles, set_vehicles},
    {"SHOP PROFILE", shop_values, 3u, get_shop, set_shop},
    {"DEBUG LOG", on_off, 2u, get_debug, set_debug},
};

static const char *const setting_descriptions[SETTING_COUNT] = {
    "Master switch for Total Warfare Overhaul profiles and logging.",
    "Smart bomb: boosts blast via Native Effects when available.",
    "Surgical strike: boosts blast via Native Effects when available.",
    "Carpet bomb: boosts bomblet blasts via Native Effects when available.",
    "Bunker buster: HEAVY/NUKE blast, shockwave and meltdown FX with Native Effects.",
    "Cruise missile: HEAVY/THERMO/FOAB blast and FX with Native Effects.",
    "Tank buster: boosts anti-armor blast via Native Effects when available.",
    "Strategic missile: nuclear blast, shockwave and meltdown FX with Native Effects.",
    "C4 profile: enhances blast on C4 impact if MercFx sees it; detonator range needs runtime hooks.",
    "Weapon realism: enhances explosive weapon impacts via MercFx when possible; ballistics/tracers need hooks/data.",
    "Parachute/jetpack intent. Needs runtime movement support/mod.",
    "No zones/expand/unify map intent. Needs runtime patch or data mod.",
    "Flyable jets/planes/boats intent. Needs runtime/asset/vehicle mod.",
    "Register mod strings and attempt extra shop/catalog hooks if possible.",
    "Writes status and heartbeat lines to the Mod Manager log.",
};

static const uint8_t setting_controls[SETTING_COUNT] = {
    MERCENARIES_MOD_CONTROL_PICKER, MERCENARIES_MOD_CONTROL_PICKER,
    MERCENARIES_MOD_CONTROL_PICKER, MERCENARIES_MOD_CONTROL_PICKER,
    MERCENARIES_MOD_CONTROL_SLIDER_VALUE_IN_LABEL, MERCENARIES_MOD_CONTROL_SLIDER_VALUE_IN_LABEL,
    MERCENARIES_MOD_CONTROL_PICKER, MERCENARIES_MOD_CONTROL_PICKER,
    MERCENARIES_MOD_CONTROL_SLIDER_VALUE_IN_LABEL, MERCENARIES_MOD_CONTROL_PICKER,
    MERCENARIES_MOD_CONTROL_SLIDER_VALUE_IN_LABEL, MERCENARIES_MOD_CONTROL_SLIDER_VALUE_IN_LABEL,
    MERCENARIES_MOD_CONTROL_SLIDER_VALUE_IN_LABEL, MERCENARIES_MOD_CONTROL_SLIDER_VALUE_IN_LABEL,
    MERCENARIES_MOD_CONTROL_PICKER,
};

static uint32_t fnv1a(const char *text)
{
    uint32_t hash = 0x811C9DC5u;
    while (*text) hash = (hash ^ ((unsigned char)*text++ | 0x20u)) * 16777619u;
    return hash;
}

static uint32_t fbits(float v) { uint32_t b; memcpy(&b, &v, sizeof(b)); return b; }

#define LOGF(...) do { if (g_host && g_host->log) g_host->log("totalwarfare", __VA_ARGS__); } while (0)

static void load_settings(void)
{
    if (!g_host) return;
    g_settings[SET_MASTER]         = clamp_value(g_host->read_setting(SETTINGS_FILE, SECTION, "Master", setting_defaults[SET_MASTER]), 2u);
    g_settings[SET_SMART_BOMB]     = clamp_value(g_host->read_setting(SETTINGS_FILE, SECTION, "SmartBomb", setting_defaults[SET_SMART_BOMB]), 2u);
    g_settings[SET_SURGICAL]       = clamp_value(g_host->read_setting(SETTINGS_FILE, SECTION, "SurgicalStrike", setting_defaults[SET_SURGICAL]), 2u);
    g_settings[SET_CARPET]         = clamp_value(g_host->read_setting(SETTINGS_FILE, SECTION, "CarpetBomb", setting_defaults[SET_CARPET]), 2u);
    g_settings[SET_BUNKER_NUKE]    = clamp_value(g_host->read_setting(SETTINGS_FILE, SECTION, "BunkerBuster", setting_defaults[SET_BUNKER_NUKE]), 3u);
    g_settings[SET_CRUISE_FOAB]    = clamp_value(g_host->read_setting(SETTINGS_FILE, SECTION, "CruiseFOAB", setting_defaults[SET_CRUISE_FOAB]), 4u);
    g_settings[SET_TANK_BUSTER]    = clamp_value(g_host->read_setting(SETTINGS_FILE, SECTION, "TankBuster", setting_defaults[SET_TANK_BUSTER]), 2u);
    g_settings[SET_STRATEGIC_NUKE] = clamp_value(g_host->read_setting(SETTINGS_FILE, SECTION, "StrategicNuke", setting_defaults[SET_STRATEGIC_NUKE]), 2u);
    g_settings[SET_C4_MEGA]        = clamp_value(g_host->read_setting(SETTINGS_FILE, SECTION, "C4Mega", setting_defaults[SET_C4_MEGA]), 4u);
    g_settings[SET_WEAPON_REALISM] = clamp_value(g_host->read_setting(SETTINGS_FILE, SECTION, "WeaponRealism", setting_defaults[SET_WEAPON_REALISM]), 2u);
    g_settings[SET_MOBILITY]       = clamp_value(g_host->read_setting(SETTINGS_FILE, SECTION, "Mobility", setting_defaults[SET_MOBILITY]), 4u);
    g_settings[SET_WORLD_FREEDOM]  = clamp_value(g_host->read_setting(SETTINGS_FILE, SECTION, "WorldFreedom", setting_defaults[SET_WORLD_FREEDOM]), 4u);
    g_settings[SET_VEHICLES]       = clamp_value(g_host->read_setting(SETTINGS_FILE, SECTION, "Vehicles", setting_defaults[SET_VEHICLES]), 4u);
    g_settings[SET_SHOP_PROFILE]   = clamp_value(g_host->read_setting(SETTINGS_FILE, SECTION, "ShopProfile", setting_defaults[SET_SHOP_PROFILE]), 3u);
    g_settings[SET_DEBUG_LOG]      = clamp_value(g_host->read_setting(SETTINGS_FILE, SECTION, "DebugLog", setting_defaults[SET_DEBUG_LOG]), 2u);
}

static unsigned category_enabled(unsigned cat)
{
    if (!g_settings[SET_MASTER]) return 0;
    switch (cat) {
        case CAT_SMART: return g_settings[SET_SMART_BOMB];
        case CAT_SURGICAL: return g_settings[SET_SURGICAL];
        case CAT_CARPET: return g_settings[SET_CARPET];
        case CAT_BUNKER: return g_settings[SET_BUNKER_NUKE] != 0u;
        case CAT_CRUISE: return g_settings[SET_CRUISE_FOAB] != 0u;
        case CAT_TANK: return g_settings[SET_TANK_BUSTER];
        case CAT_STRATEGIC: return g_settings[SET_STRATEGIC_NUKE];
        case CAT_C4: return g_settings[SET_C4_MEGA] != 0u;
        case CAT_WEAPON: return g_settings[SET_WEAPON_REALISM];
        default: return 0;
    }
}

static MercFxDecision __cdecl fx_impact(const MercFxImpact *event, void *userdata);

static void fx_bind_pass(int announce)
{
    unsigned i, ok = 0;
    if (!g_fx || !g_fx->bind_template) return;
    for (i = 0; i < (unsigned)TRIGGER_COUNT; ++i) {
        int want = (int)category_enabled(kTriggers[i].category);
        unsigned char b = g_fx->bind_template(kTriggers[i].name, FX_EFFECT, want) ? 1u : 0u;
        g_template_hash[i] = fnv1a(kTriggers[i].name);
        if (b) ++ok;
        if ((b != g_bound[i] || announce) && g_settings[SET_DEBUG_LOG]) {
            LOGF("bind %s: %s", kTriggers[i].name, b ? "BOUND" : "unavailable/owned");
        }
        g_bound[i] = b;
    }
    g_bound_count = ok;
    if (announce) LOGF("MercFx bind: %u of %u templates bound", g_bound_count, (unsigned)TRIGGER_COUNT);
}

static void try_attach_fx(void)
{
    HMODULE mod;
    MercFxGetApi get_api;
    if (!g_host || g_fx_state == 2 || g_fx_state == 3) return;

    mod = GetModuleHandleW(L"native_effects.dll");
    if (!mod) {
        if (++g_fx_misses >= 600u) {
            g_fx_state = 3;
            LOGF("native_effects.dll not loaded; enhanced blast FX unavailable (use Native Effects)");
        }
        return;
    }
    get_api = (MercFxGetApi)(void *)GetProcAddress(mod, "MercFx_GetApi");
    if (!get_api) { g_fx_state = 3; LOGF("native_effects.dll missing MercFx_GetApi"); return; }
    g_fx = get_api(MERCFX_API_VERSION);
    if (!g_fx || g_fx->version != MERCFX_API_VERSION) {
        if (++g_fx_attempts >= 3u) { g_fx_state = 3; LOGF("MercFx API version mismatch"); }
        return;
    }
    if (g_fx->size < offsetof(MercFxApi, add_ground_scorch) + sizeof(g_fx->add_ground_scorch) ||
        !g_fx->register_effect || !g_fx->bind_template || !g_fx->apply_blast_configured ||
        !g_fx->emit_shockwave || !g_fx->request_slowdown) {
        g_fx_state = 3; LOGF("MercFx API incomplete; enhanced blast FX unavailable"); return;
    }
    if (!g_fx->register_effect(FX_EFFECT, fx_impact, NULL)) {
        if (++g_fx_attempts >= 3u) { g_fx_state = 3; LOGF("MercFx effect registration failed"); }
        return;
    }
    g_fx_state = 2;
    fx_bind_pass(1);
    LOGF("MercFx attached: enhanced airstrike blasts enabled where templates are bound");
}

static MercFxDecision __cdecl fx_impact(const MercFxImpact *event, void *userdata)
{
    MercFxImpact *ev;
    MercFxBlastOptions opt;
    MercFxShockwave sw;
    unsigned i, cat = 0xFFu;
    float dmg_m = 1.0f, rad_m = 1.0f, force_m = 1.0f;
    int big = 0, do_meltdown = 0;

    (void)userdata;
    if (!g_settings[SET_MASTER] || !event) return MERCFX_UNHANDLED;

    for (i = 0; i < (unsigned)TRIGGER_COUNT; ++i) {
        if (event->template_hash == g_template_hash[i]) { cat = kTriggers[i].category; break; }
    }
    if (cat == 0xFFu || !category_enabled(cat)) return MERCFX_UNHANDLED;

    ev = (MercFxImpact *)event;
    switch (cat) {
        case CAT_SMART:
            dmg_m = 1.8f; rad_m = 1.6f; force_m = 1.4f; break;
        case CAT_SURGICAL:
            dmg_m = 2.2f; rad_m = 1.3f; force_m = 1.5f; break;
        case CAT_CARPET:
            dmg_m = 1.7f; rad_m = 2.0f; force_m = 1.6f; break;
        case CAT_BUNKER:
            if (g_settings[SET_BUNKER_NUKE] == 2u) { dmg_m = 8.0f; rad_m = 5.0f; force_m = 6.0f; big = 1; do_meltdown = 1; }
            else { dmg_m = 3.0f; rad_m = 2.2f; force_m = 2.5f; big = 1; }
            break;
        case CAT_CRUISE:
            if (g_settings[SET_CRUISE_FOAB] == 3u) { dmg_m = 7.0f; rad_m = 4.5f; force_m = 5.5f; big = 1; do_meltdown = 1; }
            else if (g_settings[SET_CRUISE_FOAB] == 2u) { dmg_m = 5.0f; rad_m = 3.5f; force_m = 4.0f; big = 1; }
            else { dmg_m = 2.5f; rad_m = 2.0f; force_m = 2.0f; big = 1; }
            break;
        case CAT_TANK:
            dmg_m = 2.5f; rad_m = 1.4f; force_m = 1.8f; break;
        case CAT_STRATEGIC:
            dmg_m = 10.0f; rad_m = 6.0f; force_m = 7.0f; big = 1; do_meltdown = 1; break;
        case CAT_C4:
            /* Best-effort blast enhancement on C4 impact events (detonator range is engine-side). */
            if (g_settings[SET_C4_MEGA] == 3u) { dmg_m = 5.0f; rad_m = 4.0f; force_m = 4.0f; big = 1; }
            else if (g_settings[SET_C4_MEGA] == 2u) { dmg_m = 4.0f; rad_m = 3.0f; force_m = 3.0f; big = 1; }
            else { dmg_m = 3.0f; rad_m = 2.0f; force_m = 2.5f; }
            break;
        case CAT_WEAPON:
            /* Best-effort enhancement for explosive weapon impacts (rockets/shells/grenades/mortars). */
            dmg_m = 1.6f; rad_m = 1.5f; force_m = 1.4f; break;
        default:
            return MERCFX_UNHANDLED;
    }

    ev->splash_damage *= dmg_m;
    ev->splash_radius *= rad_m;
    ev->splash_force *= force_m;
    if (ev->splash_radius > 800.0f) ev->splash_radius = 800.0f;

    opt.size = sizeof(opt);
    opt.damage_type = big ? MERCFX_BLAST_BUNKERBUSTER : MERCFX_BLAST_FROM_PROJECTILE;
    opt.shake_duration = big ? 3.0f : 0.8f;
    opt.shake_magnitude = big ? 3.0f : 0.8f;
    opt.scorch_radius = big ? 40.0f : 20.0f;
    opt.suppress_default_sound = 0u;

    if (!g_fx->apply_blast_configured(event, &opt)) {
        g_pending_log_fail = 1;
        return MERCFX_UNHANDLED;
    }

    if (big) {
        sw.size = sizeof(sw);
        sw.flags = MERCFX_SHOCKWAVE_REFRACTION | MERCFX_SHOCKWAVE_RINGS;
        sw.radius = (rad_m >= 5.0f) ? 200.0f : 120.0f;
        sw.duration = (rad_m >= 5.0f) ? 4.0f : 2.0f;
        g_fx->emit_shockwave(event->position, &sw);
        g_fx->request_slowdown(FX_OWNER, (rad_m >= 5.0f) ? 0.45f : 0.6f, (rad_m >= 5.0f) ? 1.5f : 0.8f);
    }

    if (do_meltdown) {
        memcpy(g_pending_pos, event->position, sizeof(g_pending_pos));
        g_pending_meltdown = 1;
    }

    _snprintf(g_pending_log_tpl, sizeof(g_pending_log_tpl) - 1, "%s", kTriggers[i].name);
    g_pending_log_tpl[sizeof(g_pending_log_tpl) - 1] = 0;
    return MERCFX_REMOVE;
}

static void register_mod_strings(void)
{
    if (!g_host) return;
    if (g_host->size < offsetof(MercenariesModHost, register_string) + sizeof(g_host->register_string)) return;
    if (!g_host->register_string) return;
    g_host->register_string("totalwarfare.name", "TOTAL WARFARE OVERHAUL");
    g_host->register_string("totalwarfare.tagline", "Enhanced ordnance, mobility, and sandbox profiles");
}

static int try_add_shop_item(const char *tpl, const char *name, const char *desc, const char *detail,
                             const char *tex, unsigned price, unsigned category)
{
    MercenariesShopItem item;
    if (!g_host || !g_host->add_shop_item) return 0;
    if (g_host->size < offsetof(MercenariesModHost, add_shop_item) + sizeof(g_host->add_shop_item)) return 0;
    item.template_name = tpl; item.display_name = name; item.description = desc;
    item.detailed_description = detail; item.texture = tex; item.price = price; item.category = category;
    return g_host->add_shop_item(&item);
}

static void attempt_shop_registration(void)
{
    int added = 0;
    if (!g_host || g_shop_attempted) return;
    if (!g_settings[SET_MASTER] || g_settings[SET_SHOP_PROFILE] == 0u) { g_shop_attempted = 1u; return; }
    if (g_settings[SET_BUNKER_NUKE] == 2u)
        added += try_add_shop_item("template_support_p_bunkerbuster_nuke", "totalwarfare.name",
                                   "Nuclear bunker buster profile", "Nuclear bunker buster profile",
                                   "shop_bunkerbuster", 500000u, MERCENARIES_SHOP_AIR_STRIKES);
    if (g_settings[SET_CRUISE_FOAB] >= 2u)
        added += try_add_shop_item("template_support_p_cruise_foab", "totalwarfare.name",
                                   "Thermobaric/FOAB cruise profile", "Thermobaric/FOAB cruise profile",
                                   "shop_cruisemissile", 350000u, MERCENARIES_SHOP_AIR_STRIKES);
    if (g_settings[SET_STRATEGIC_NUKE])
        added += try_add_shop_item("template_support_p_strategic_nuke", "totalwarfare.name",
                                   "Strategic nuclear missile profile", "Strategic nuclear missile profile",
                                   "shop_strategicmissile", 999999u, MERCENARIES_SHOP_AIR_STRIKES);
    g_shop_attempted = 1u;
    if (g_settings[SET_DEBUG_LOG]) LOGF("shop extras added: %d (duplicates/unavailable are normal)", added);
}

static void world_tick(uint32_t delta_bits)
{
    float delta;
    memcpy(&delta, &delta_bits, sizeof(delta));
    if (!(delta > 0.0f) || delta > 10.0f) delta = 0.0f;
    if (!g_host || !g_settings[SET_MASTER]) return;

    g_world_time += delta;
    g_ticks++;

    if (g_fx_state != 2 && g_fx_state != 3) try_attach_fx();

    if (g_fx_state == 2 && g_bound_count < (unsigned)TRIGGER_COUNT) {
        if (g_next_rebind == 0.0) g_next_rebind = g_world_time + 2.0;
        else if (g_world_time >= g_next_rebind) { g_next_rebind = g_world_time + 2.0; fx_bind_pass(0); }
    }

    if (!g_shop_attempted) { register_mod_strings(); attempt_shop_registration(); }

    if (g_pending_log_fail) { g_pending_log_fail = 0; LOGF("MercFx blast rejected; vanilla explosion used"); }
    if (g_pending_log_tpl[0]) { LOGF("enhanced blast on %s", g_pending_log_tpl); g_pending_log_tpl[0] = 0; }

    if (g_pending_meltdown) {
        g_pending_meltdown = 0;
        if (!g_checked_retail) {
            g_have_explosion_fn = g_host->guest_function_exists(EXPLOSION_FN) ? 1 : 0;
            g_meltdown_hash = fnv1a("Meltdown");
            g_checked_retail = 1;
        }
        if (g_have_explosion_fn) {
            uint32_t xp[13];
            memcpy(xp, g_pending_pos, 12);
            xp[3] = fbits(500.0f); xp[4] = fbits(0.0f); xp[5] = 0; xp[6] = 0;
            xp[7] = g_meltdown_hash; xp[8] = 0; xp[9] = 0; xp[10] = 0; xp[11] = 0; xp[12] = 0;
            g_host->guest_call(EXPLOSION_FN, 0, 13, xp);
            LOGF("Meltdown visual fired");
        }
    }

    if (!g_announced && g_ticks > 300u) {
        g_announced = 1u;
        LOGF("Total Warfare Overhaul %s active", MOD_VERSION);
        if (g_fx_state == 2) LOGF("Native Effects (MercFx): attached, %u/%u templates bound", g_bound_count, (unsigned)TRIGGER_COUNT);
        else if (g_fx_state == 3) LOGF("Native Effects (MercFx): unavailable; load Native Effects for enhanced blasts/FX");
        if (g_settings[SET_WORLD_FREEDOM] || g_settings[SET_VEHICLES] || g_settings[SET_MOBILITY] || g_settings[SET_C4_MEGA] || g_settings[SET_WEAPON_REALISM])
            LOGF("note: map/vehicles/mobility/C4 range/weapon physics need runtime or game-data mods");
    }

    if (g_settings[SET_DEBUG_LOG] && (g_ticks % 3600u) == 0u) LOGF("heartbeat ticks=%u fx=%d", g_ticks, g_fx_state);
}

static int key_pressed(unsigned virtual_key) { (void)virtual_key; return 0; }

static void shutdown_hook(void)
{
    if (g_fx && g_fx->cancel_slowdown) g_fx->cancel_slowdown(FX_OWNER);
}

static const MercenariesMod descriptor = {
    MERCENARIES_MOD_API_VERSION,
    sizeof(MercenariesMod),
    MOD_NAME,
    MOD_VERSION,
    settings,
    SETTING_COUNT,
    world_tick,
    NULL,
    NULL,
    key_pressed,
    shutdown_hook,
    MOD_CREDITS,
    "Airstrike/ordnance/explosive overhaul with optional Native Effects (MercFx) enhanced blasts, shockwaves and nuke visuals. Detonator range, ballistics, mobility, vehicles and map freedom need runtime/data mods.",
    setting_descriptions,
    setting_controls,
    NULL,
    NULL,
    setting_defaults,
};

__declspec(dllexport) const MercenariesMod *mercenaries_mod_entry(const MercenariesModHost *table)
{
    if (!table || table->api_version != MERCENARIES_MOD_API_VERSION || table->size < sizeof(*table)) return NULL;
    g_host = table;
    g_ticks = 0u; g_announced = 0u; g_shop_attempted = 0u;
    g_fx = NULL; g_fx_state = 0; g_fx_misses = 0; g_fx_attempts = 0;
    g_bound_count = 0; g_world_time = 0.0; g_next_rebind = 0.0;
    g_pending_meltdown = 0; g_checked_retail = 0; g_have_explosion_fn = 0;
    g_pending_log_fail = 0; g_pending_log_tpl[0] = 0;
    memset(g_bound, 0, sizeof(g_bound));
    memset(g_template_hash, 0, sizeof(g_template_hash));
    load_settings();
    LOGF("loaded %s %s (Native Effects integration: best-effort)", MOD_NAME, MOD_VERSION);
    try_attach_fx();
    return &descriptor;
}
