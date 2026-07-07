# Assets

Do teto slozky muzes doplnovat vlastni 64x64 paletove RAW textury.

Aktualne engine pri startu zkousi nacist:

- wall01.raw
- wall02.raw
- wall03.raw
- wall04.raw
- sprite_barrel.raw
- sprite_enemy.raw
- door01.raw
- pickup_health.raw
- sprite_knight.raw
- pickup_ammo.raw
- pickup_key.raw
- trigger_button.raw
- exit_marker.raw
- pickup_weapon.raw
- projectile.raw

Kazdy soubor musi mit presne 4096 bajtu.
Pokud chybi nebo nesedi velikost, engine automaticky prepne na fallback texturu.

Mapa se nyni nacita ze souboru level.txt jako mrizka 16x16 znaku.

- . = prazdne pole
- 1..4 = steny s texturou 1..4
- D = dvere
- L = zamcene dvere na zluty klic
- P = start hrace
- B = dekoracni sprite
- E = lehci nepritel
- K = tezsi rytir
- H = health pickup
- M = ammo pickup
- Y = zluty klic
- W = pickup dalsi zbrane
- T = trigger marker nebo tlacitko

Manifest textur je v textures.txt ve formatu:

- cislo_textury jmeno_souboru.raw

Tile registry je v tiles.txt:

- symbol kind texture_id key_mask
- priklad: `L DOOR 7 1`

Entity registry je v entities.txt:

- symbol kind texture_id health touch_damage speed_milli pickup_health pickup_ammo pickup_ammo_type_id pickup_key_mask pickup_weapon_id ai_mode alert_range_milli attack_range_milli attack_cooldown_ms
- ai_mode muze byt `NONE`, `MELEE` nebo `RANGED`
- priklad: `M PICKUP 10 1 0 0 0 6 2 0 0 NONE 0 0 0`

Weapon registry je v weapons.txt:

- id name ammo_type_id ammo_cost damage range_milli hit_cone_milli flash_ms start_ammo projectile_speed_milli projectile_texture_id
- projectile_speed_milli 0 znamena hitscan, kladna hodnota znamena skutecny projektil
- priklad: `2 crossbow 2 2 3 9500 300 110 8 6200 15`

Pri hre lze zbrane prepinat klavesou Tab. Weapon pickup muze rovnou pridelit novou zbran i munici bez zmeny kodu, jen pres entities.txt a symbol v mape.

Globalni start config je v game.txt:

- start_health 100
- start_weapon 1

Runtime config je v config.txt:

- `fps_cap 144`
- `mouse_look 1`
- `mouse_fire 1`
- `mouse_sensitivity 3`
- `bind forward W`
- `bind fire SPACE`
- podporovane klavesy: `W A S D Q E F SPACE TAB ESC UP DOWN LEFT RIGHT`

Mouse-look je zatim implementovany pro Windows backend. DOS backend dal funguje pres klavesnici.

Skript triggeru a actuatoru je v logic.txt. Zakladni format:

- TRIGGER T1 USE 1 2 A1 0
- TRIGGER T2 ENTER 12 13 A2 1
- ACTUATOR A1 DOOR 7 2 2400
- ACTUATOR A1 SPAWN 13 2 E
- ACTUATOR A1 AMMO 2 8
- ACTUATOR A1 GIVE_WEAPON 2
- ACTUATOR A2 MAP assets/level2.txt assets/logic2.txt

Trigger `USE` se aktivuje tlacitkem F v dosahu dane pozice. Trigger `ENTER` se aktivuje vstupem do dane bunky. Jedno `A1` muze mit vice actuatoru a vsechny se spusti najednou.
