; Fallback Mod Manager profile for Total Warfare Overhaul.
; A Mod API 2 DLL mod normally appears in MODS from mods\total_warfare.dll.
; If the DLL is not built/loaded, copy this file to:
;   <game>\mods\Mod Manager\profiles\total-warfare.profile
; to expose a settings page for mods\total_warfare.ini (UI only, no DLL hooks).

[Mod]
Title=TOTAL WARFARE OVERHAUL
Credits=BY GLYPH CLUSTER
Description=Overhaul profiles for airstrikes, explosives, weapons, mobility, vehicles, and world freedom.
Ini=mods\total_warfare.ini
Reload=restart

[Row1]
Kind=toggle
Label=ENABLED
Description=Master switch for Total Warfare Overhaul profiles and logging.
Section=TotalWarfare
Key=Master
Default=1

[Row2]
Kind=toggle
Label=SMART BOMB
Description=Smart bomb profile: enhanced precision blast behavior intent.
Section=TotalWarfare
Key=SmartBomb
Default=1

[Row3]
Kind=toggle
Label=SURGICAL STRIKE
Description=Surgical strike profile: tighter high-impact bombing behavior intent.
Section=TotalWarfare
Key=SurgicalStrike
Default=1

[Row4]
Kind=toggle
Label=CARPET BOMB
Description=Carpet bomb profile: dense multi-bomb bombardment behavior intent.
Section=TotalWarfare
Key=CarpetBomb
Default=1

[Row5]
Kind=number
Label=BUNKER BUSTER
Description=0 OFF, 1 HEAVY, 2 NUKE with mushroom-style FX intent.
Section=TotalWarfare
Key=BunkerBuster
Min=0
Max=2
Step=1
Default=2

[Row6]
Kind=number
Label=CRUISE MISSILE
Description=0 OFF, 1 HEAVY, 2 THERMO, 3 FOAB devastation intent.
Section=TotalWarfare
Key=CruiseFOAB
Min=0
Max=3
Step=1
Default=3

[Row7]
Kind=toggle
Label=TANK BUSTER
Description=Tank buster profile: realistic anti-armor missile behavior intent.
Section=TotalWarfare
Key=TankBuster
Default=1

[Row8]
Kind=toggle
Label=STRATEGIC NUKE
Description=Strategic missile profile: nuclear detonation and mushroom intent.
Section=TotalWarfare
Key=StrategicNuke
Default=1

[Row9]
Kind=number
Label=C4 MEGA BLAST
Description=0 OFF, 1 MEGA, 2 MEGA 1KM, 3 MEGA 2KM detonator reach intent.
Section=TotalWarfare
Key=C4Mega
Min=0
Max=3
Step=1
Default=3

[Row10]
Kind=number
Label=WEAPON REALISM
Description=0 OFF, 1 ON. Realistic bullets, fire, shells, and missiles intent.
Section=TotalWarfare
Key=WeaponRealism
Min=0
Max=1
Step=1
Default=1

[Row11]
Kind=number
Label=MOBILITY
Description=0 OFF, 1 CHUTE, 2 JETPACK, 3 BOTH when runtime support exists.
Section=TotalWarfare
Key=Mobility
Min=0
Max=3
Step=1
Default=3

[Row12]
Kind=number
Label=WORLD FREEDOM
Description=0 OFF, 1 NO ZONES, 2 EXPAND, 3 UNIFY ACE/North/South intent.
Section=TotalWarfare
Key=WorldFreedom
Min=0
Max=3
Step=1
Default=3

[Row13]
Kind=number
Label=VEHICLES
Description=0 OFF, 1 AIR, 2 SEA, 3 ALL when runtime support is present.
Section=TotalWarfare
Key=Vehicles
Min=0
Max=3
Step=1
Default=3

[Row14]
Kind=number
Label=SHOP PROFILE
Description=0 OFF, 1 ON, 2 MAX. DLL attempts extra shop/string hooks if loaded.
Section=TotalWarfare
Key=ShopProfile
Min=0
Max=2
Step=1
Default=2

[Row15]
Kind=toggle
Label=DEBUG LOG
Description=Writes status and heartbeat lines to the Mod Manager log.
Section=TotalWarfare
Key=DebugLog
Default=0
