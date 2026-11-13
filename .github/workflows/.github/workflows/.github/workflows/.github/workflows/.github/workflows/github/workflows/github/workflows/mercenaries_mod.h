/* Mercenaries Recompiled mod API.
 *
 * A mod is a DLL in the mods folder beside mercenaries_recomp.exe that
 * exports mercenaries_mod_entry. The host loads every mods\*.dll at startup
 * in name order, calls the entry point once with its service table, and keeps
 * the returned descriptor for the life of the process: the MODS dialog in
 * the main and pause menus lists the descriptor's name and settings, and the
 * retail hooks below are dispatched to every mod that fills them in.
 *
 * Plain C ABI. Build the DLL for x64 with any C compiler; no other header or
 * library of the port is needed. See docs/mod-api.md, whose Text limits
 * section says how long each text shown in the MODS dialog may be. */
#ifndef MERCENARIES_MOD_H
#define MERCENARIES_MOD_H
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif

#define MERCENARIES_MOD_API_VERSION 2u
#define MERCENARIES_MOD_ENTRY_NAME "mercenaries_mod_entry"
#define MERCENARIES_MOD_MAX_SETTINGS 15u   /* settings one mod's MODS page lists */
#define MERCENARIES_MOD_SCRATCH_BYTES 0x3000u
#define MERCENARIES_MOD_TEXTURE_MAX_SIDE 256u
/* How a setting shows on its mod's MODS page, one entry per setting in
 * MercenariesMod.setting_controls. A host that does not know a control
 * shows the setting as a picker. */
#define MERCENARIES_MOD_CONTROL_PICKER 0u   /* the value's label between arrows, like GAME OPTIONS' INVERT Y */
#define MERCENARIES_MOD_CONTROL_SLIDER 1u   /* a slider with one stop per value, like AUDIO OPTIONS' MUSIC */
#define MERCENARIES_MOD_CONTROL_SLIDER_VALUE_BESIDE 2u   /* the slider, with the current value's label beside it */
#define MERCENARIES_MOD_CONTROL_SLIDER_VALUE_IN_LABEL 3u /* the slider, with the value after the setting's label: "SIZE: LARGE" */
/* The PDA shop's tabs, as MercenariesShopItem.category names them. */
#define MERCENARIES_SHOP_VEHICLES 0u      /* itemshop.ini's Category Vehicle */
#define MERCENARIES_SHOP_SUPPLIES 1u      /* Category Supply */
#define MERCENARIES_SHOP_AIR_STRIKES 2u   /* Category Weapon, which the shop calls AIR STRIKES */
#define MERCENARIES_SHOP_PRICE_MAX 9999999u
#define MERCENARIES_MOD_STRING_MAX 1023u  /* characters register_string keeps of one text */

/* One item of the PDA shop's catalogue: the fields of an itemshop.ini
 * section, as names. The host hashes every name the way the game does. */
typedef struct MercenariesShopItem {
    const char *template_name;          /* the support template an order hands over, "template_support_guidedmissile" */
    const char *display_name;           /* localization key of the item's name */
    const char *description;            /* key of the one-line caption under the name */
    const char *detailed_description;   /* key of the details page's text */
    const char *texture;                /* the shop's picture of it, "shop_surgicalstrike"; NULL shows the locked one */
    unsigned price;                     /* DefaultPrice, which the Mafia's mood then moves; calling the item in
                                         * charges its template_sam_* MonetaryCost, so give the same value */
    unsigned category;                  /* a MERCENARIES_SHOP_* tab */
} MercenariesShopItem;

typedef struct MercenariesModSetting {
    const char *label;            /* row label; uppercase fits the retail menu font */
    const char *const *values;    /* one label per value, stepped through with left and right */
    unsigned value_count;
    unsigned (*get)(void);        /* index of the current value */
    void (*set)(unsigned value);  /* applies the value at once where possible, and persists it */
} MercenariesModSetting;

typedef struct MercenariesModHost {
    unsigned api_version;         /* MERCENARIES_MOD_API_VERSION */
    unsigned size;                /* sizeof(MercenariesModHost) */
    uint8_t *guest_memory;        /* host address of guest address 0; read it inside hooks */
    uint32_t guest_limit;         /* first guest address past the guest memory */
    const char *directory;        /* executable directory with a trailing backslash */
    /* One line in the preview log under the given category. */
    void (*log)(const char *category, const char *format, ...);
    /* Ini values; file is relative to directory, for example "mods\\example.ini". */
    unsigned (*read_setting)(const char *file, const char *section, const char *key, unsigned fallback);
    void (*write_setting)(const char *file, const char *section, const char *key, unsigned value);
    /* True when the retail function at the guest address was recompiled. */
    int (*guest_function_exists)(uint32_t address);
    /* Calls a retail function with the retail thiscall or cdecl convention:
     * object lands in ecx, args are pushed right to left, eax is returned.
     * Valid only inside world_tick, radar_contacts, brush_end,
     * camera_update and brush_corrected. */
    uint32_t (*guest_call)(uint32_t address, uint32_t object, unsigned argc, const uint32_t *args);
    /* MERCENARIES_MOD_SCRATCH_BYTES of guest memory free for the running hook,
     * for out parameters and strings passed to guest_call; 0 outside hooks. */
    uint32_t (*guest_scratch)(void);
    /* Registers a texture for the retail 2D stream and returns the hash to
     * put in a record's texture word, or 0. pixels is 0xAARRGGBB row major,
     * straight alpha, at most MERCENARIES_MOD_TEXTURE_MAX_SIDE a side, and
     * must stay valid for the life of the process. Records reference it with
     * texel coordinates, like the retail sprites; the retail sprite helpers
     * accept the hash as well once the game has drawn one retail texture. */
    uint32_t (*register_texture)(unsigned width, unsigned height, const uint32_t *pixels);
    /* The camera aspect, height over width, that RedCamera SetPerspective,
     * 0x001F8F10, applies in this host when the game passes guest_aspect:
     * the player's aspect ratio option, or guest_aspect itself when the
     * option keeps the retail one. SetPerspective also multiplies the tangent
     * of half the field of view it is given by guest_aspect / camera_aspect,
     * so a mod that wants a horizontal field of view f passes
     * 2 * atan(tan(f / 2) * camera_aspect / guest_aspect). Read only when
     * size covers this field. */
    float (*camera_aspect)(float guest_aspect);
    /* The shape the player's aspect ratio option gives the screen, as
     * whole width and height: 4 by 3, 16 by 9, 16 by 10 or 21 by 9. The 640
     * by 480 canvas is stretched over that screen, so one canvas unit in x
     * is 4 * height / (3 * width) of a canvas unit in y. Callable at any
     * time on the game thread. Read only when size covers this field. */
    void (*presentation_aspect)(uint32_t *width, uint32_t *height);
    /* Adds an item to the PDA shop's catalogue after the game's own. The
     * game reads its catalogue as a campaign loads, before it restores the
     * save's stock, and the host appends every added item each time, so a
     * saved stock finds the item again and a save made with it loads
     * without it. The item is in the catalogue only: it reaches the shop's
     * lists once stocked, by the game's stock function 0x000CDA60 through
     * guest_call. Call it from mercenaries_mod_entry, since the first
     * catalogue may load before any world_tick. Returns 1 when kept; 0 for
     * a missing name, a category past MERCENARIES_SHOP_AIR_STRIKES, a price
     * past MERCENARIES_SHOP_PRICE_MAX, a template already added, or no room.
     * The game's catalogue holds 128 items; one the game already sells is
     * left out where it loads. Read only when size covers this field. */
    int (*add_shop_item)(const MercenariesShopItem *item);
    /* Answers the localization key with `text` wherever the game looks a
     * key up by its text lookup: shop names, captions and details, and the
     * messages that name them. The text is copied, single-byte characters
     * the game's fonts draw, at most MERCENARIES_MOD_STRING_MAX of them.
     * Returns 1 when kept; 0 for a missing or empty key or text, a text too
     * long, a key already registered, or no room. A key the game's language
     * file holds keeps the game's text. Read only when size covers this
     * field. */
    int (*register_string)(const char *key, const char *text);
} MercenariesModHost;

typedef struct MercenariesMod {
    unsigned api_version;         /* MERCENARIES_MOD_API_VERSION */
    unsigned size;                /* sizeof(MercenariesMod) */
    const char *name;             /* title of the mod's MODS page */
    const char *version;
    const MercenariesModSetting *settings;
    unsigned setting_count;       /* at most MERCENARIES_MOD_MAX_SETTINGS */
    /* Hooks; leave any of them NULL. All run on the retail game thread. */
    void (*world_tick)(uint32_t delta_bits);      /* every retail world update, as a float in bits */
    void (*radar_contacts)(uint32_t radar);       /* RsHudRadar draw, before its contract, deck, flag and contact markers */
    /* A retail 2D brush finished painting. The retail stream holds 20-byte
     * vertices (x, y, color, u, v) and 28-byte primitive records; a mod may
     * append to both and must then advance the ends it was given. pda is 1
     * for the seven RsDataPodDisplay brushes, which the host aspect-corrects
     * after this call. */
    void (*brush_end)(uint32_t brush, int pda, uint32_t vertex_base, uint32_t vertex_first,
                      uint32_t *vertex_end, uint32_t primitive_base, uint32_t primitive_first,
                      uint32_t *primitive_end);
    /* Host window key down without auto-repeat; return 1 to consume the key. */
    int (*key_pressed)(unsigned virtual_key);
    void (*shutdown)(void);
    /* Credits, shown after the version under the mod's title in the MODS
     * dialog. Name every author and the work the mod builds on, in
     * uppercase, on one short line: "BY DIOCULOS" or "BY LIS, FROM NOOT'S
     * PS2 MOD". Read only when size covers this field, so older builds of a
     * mod still load. */
    const char *credits;
    /* What the mod does, in a sentence or two. The MODS dialog shows it
     * under the mod list, wrapped, while the mod is selected. Optional, and
     * read only when size covers this field. */
    const char *description;
    /* One explanation per setting, in the order of settings; any entry may
     * be NULL. The MODS dialog shows the selected setting's under its rows.
     * Optional, and read only when size covers this field. */
    const char *const *setting_descriptions;
    /* One MERCENARIES_MOD_CONTROL_* per setting, in the order of settings,
     * for ordered choices such as sizes and distances; NULL shows every
     * setting as a picker. A slider needs two values or more. A plain slider
     * shows no value label, so explain its stops in the setting's
     * description; the other slider controls show the current one beside
     * the slider or after the setting's label. Optional, and read only when
     * size covers this field. */
    const uint8_t *setting_controls;
    /* Each retail camera update, after the camera manager placed the camera
     * and before the matrix reaches RedCamera. camera is the camera manager,
     * matrix the guest address of the 64-byte matrix it renders from, with
     * the right, up, back and position rows 16 bytes apart, and delta_bits
     * the update's time step as a float in bits. A mod may move the position
     * row. Not called while the recomp's free camera owns the view. Optional,
     * and read only when size covers this field. */
    void (*camera_update)(uint32_t camera, uint32_t matrix, uint32_t delta_bits);
    /* The same brush as brush_end, once every mod's brush_end ran and the
     * host corrected the brush's aspect: coordinates and record clips are
     * final canvas units. The ranges are those brush_end ended with. A mod
     * may rewrite the brush's vertices and records in place but never
     * append, so the ends are values. Optional, and read only when size
     * covers this field. */
    void (*brush_corrected)(uint32_t brush, int pda, uint32_t vertex_base, uint32_t vertex_first,
                            uint32_t vertex_end, uint32_t primitive_base, uint32_t primitive_first,
                            uint32_t primitive_end);
    /* One default value per setting, in the order of settings: the index X
     * on the mod's MODS page sets the setting back to. An entry at or past
     * its setting's value_count leaves that setting alone. NULL, or a build
     * without this field, has the Mod Manager keep the values it first saw
     * and set those back instead. Optional, and read only when size covers
     * this field. */
    const unsigned *setting_defaults;
} MercenariesMod;

/* Exported by every mod: returns its descriptor, or NULL to stay unloaded.
 * The descriptor and every string and setting it points to must stay valid
 * for the life of the process. */
typedef const MercenariesMod *(*MercenariesModEntry)(const MercenariesModHost *host);

#ifdef __cplusplus
}
#endif
#endif
