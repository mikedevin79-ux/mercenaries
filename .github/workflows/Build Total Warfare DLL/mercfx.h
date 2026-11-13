#ifndef MERCFX_H
#define MERCFX_H
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
#define MERCFX_API_VERSION 1u
#define MERCFX_PROBE_NAME "mercfx_probe"

typedef struct MercFxImpact {
    uint32_t size, projectile, effect_hash, hit_actor, early_detonation;
    float position[3], normal[3];
    uint32_t template_hash, original_effect_hash;
    float splash_damage, splash_radius, splash_force;
    uint32_t projectile_damage_type; /* Raw RsDamage::DAMAGE_TYPE, not the blast enum below. */
} MercFxImpact;

/* RsExplosion::ExplosionDamageType is a separate enum from RsDamage::DAMAGE_TYPE.
   STANDARD=0 becomes DAMAGE_TYPE_EXPLOSION, not DAMAGE_TYPE_BASH. */
typedef enum MercFxBlastDamageType {
    MERCFX_BLAST_STANDARD = 0,
    MERCFX_BLAST_STUN = 1,
    MERCFX_BLAST_C4 = 2,
    MERCFX_BLAST_BUNKERBUSTER = 3,
    /* Translate the projectile's EXPLOSION/STUN/EXPLOSION_C4/BUNKERBUSTER type.
       Other raw types are unsupported by RsExplosion and reject without damage. */
    MERCFX_BLAST_FROM_PROJECTILE = 4
} MercFxBlastDamageType;

typedef struct MercFxBlastOptions {
    uint32_t size;
    MercFxBlastDamageType damage_type;
    /* Raw engine duration (Xbox countdown runs at half dt), not wall seconds.
       Magnitude is an engine scalar; retail Bunker Buster uses 0.5s and 1.0.
       Either zero disables shake. Framework bounds: 0-30s, 0-10 strength. */
    float shake_duration, shake_magnitude;
    float scorch_radius; /* Terrain only; 0 disables, otherwise at most 40. */
    uint32_t suppress_default_sound; /* Optional tail: 1 silences the built-in blast cue. */
} MercFxBlastOptions;
#define MERCFX_SHOCKWAVE_REFRACTION 1u
#define MERCFX_SHOCKWAVE_RINGS 2u
typedef struct MercFxShockwave {
    uint32_t size, flags;
    float radius, duration; /* Refraction only: (0,200] and (0,5] seconds. */
    /* Bright rings retain the retail ten-ring geometry and ~0.43s lifetime.
       Return bits report which visuals were requested successfully, not rendered. */
} MercFxShockwave;

typedef struct MercFxParticle {
    uint32_t size, texture_hash;
    int32_t blend_mode;
    uint32_t colors[3]; /* 0xAARRGGBB; retail full alpha is 0x80. */
    float widths[3], heights[3], time_b;
    float y_acceleration, friction, lifetime, lifetime_random, random_scale;
    float random_acceleration, wind, max_distance;
    uint32_t flags; /* Lit=1, fixed orientation=2, animated=4. No clustering. */
} MercFxParticle;
typedef void (__cdecl *MercFxSimulationHandler)(float delta_seconds, void *);
typedef void (__cdecl *MercFxResetHandler)(void *);
typedef enum MercFxDecision {
    MERCFX_UNHANDLED = 0,
    MERCFX_REMOVE = 1,
    MERCFX_KEEP = 2
} MercFxDecision;
typedef MercFxDecision (__cdecl *MercFxImpactHandler)(const MercFxImpact *, void *);
typedef struct MercFxStats {
    uint64_t intercepted, handled, bursts, rejected;
} MercFxStats;
typedef struct MercFxRuntimeStats {
    uint32_t size, active_particle_types;
    uint64_t emitted, dropped, blasts, resets, simulation_steps;
} MercFxRuntimeStats;
typedef struct MercFxApi {
    uint32_t size, version;
    /* Registration is owner-thread only, outside impact dispatch. Names are unique
       case-insensitive ASCII identifiers. Providers remain loaded until shutdown. */
    int (__cdecl *register_effect)(const char *, MercFxImpactHandler, void *);
    /* Only valid synchronously inside an impact handler. Visual only: no damage. */
    int (__cdecl *emit_probe_burst)(const float position[3]);
    int (__cdecl *get_stats)(MercFxStats *);
    /* Append-only v1 capabilities. Check size before accessing these fields. */
    uint32_t (__cdecl *register_particle)(const MercFxParticle *);
    int (__cdecl *prepare_particle)(uint32_t handle);
    int (__cdecl *emit_particle)(uint32_t handle, const float position[3], const float velocity[3], float spin, float alpha);
    int (__cdecl *apply_blast)(const MercFxImpact *); /* At most once per impact. */
    int (__cdecl *register_simulation)(MercFxSimulationHandler, MercFxResetHandler, void *);
    int (__cdecl *bind_template)(const char *template_name, const char *effect_name, int enabled);
    int (__cdecl *set_template_enabled)(const char *template_name, int enabled);
    int (__cdecl *unregister_effect)(const char *effect_name);
    int (__cdecl *unregister_simulation)(MercFxSimulationHandler, void *);
    int (__cdecl *get_runtime_stats)(MercFxRuntimeStats *);
    /* Same once-per-impact budget as apply_blast, which retains STANDARD behavior.
       Invalid/unsupported types return 0 before any blast or shake is applied. */
    int (__cdecl *apply_blast_typed)(const MercFxImpact *, MercFxBlastDamageType);
    /* All blast variants share the same once-per-impact commitment. Invalid
       options reject before damage; unavailable optional visuals do not cancel it. */
    int (__cdecl *apply_blast_configured)(const MercFxImpact *, const MercFxBlastOptions *);
    int (__cdecl *add_ground_scorch)(const float position[3], float radius);
    uint32_t (__cdecl *emit_shockwave)(const float position[3], const MercFxShockwave *);
    /* Owner-keyed temporary simulation slowdown: rate (0,1], duration (0,30]
       in simulation seconds. Same-owner requests extend; another owner rejects.
       Never speeds up an already slower clock. External SetRate calls relinquish
       ownership, even if setting the same value. No wall-clock/pause countdown. */
    int (__cdecl *request_slowdown)(const char *owner, float rate, float duration);
    int (__cdecl *cancel_slowdown)(const char *owner);
    /* Terrain-only height, not buildings/water. Native callback only; returns
       0 without changing *height if terrain is unavailable or X/Z out of bounds. */
    int (__cdecl *sample_terrain_height)(float x, float z, float *height);
    /* Copied cue name (e.g. a_strike.bb_explosion), not a filename/bank name.
       Register outside dispatch. Native updates prepare resident wavebanks using
       nonblocking references; unknown/nonpositional/chained cues reject for now. */
    uint32_t (__cdecl *register_sound)(const char *cue_name);
    int (__cdecl *sound_ready)(uint32_t sound); /* Native callbacks only. */
    int (__cdecl *play_sound)(uint32_t sound, const float position[3]); /* One shot, no delayed retry. */
} MercFxApi;
/* Obtain with GetProcAddress on the already loaded native_effects.dll. No implicit
   LoadLibrary: the framework must have initialized first. */
typedef const MercFxApi *(__cdecl *MercFxGetApi)(uint32_t version);
#ifdef __cplusplus
}
#endif
#endif
