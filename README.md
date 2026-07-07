# Hexen-like Mode 13h Prototype

Tento projekt je kostra old-school fantasy FPS ve stylu Hexenu. Renderuje se interne v rozliseni 320x200 s 256barvovou paletou. DOS build bezi primo v VGA Mode 13h, Windows build zachovava ostre pixely pomoci nearest-neighbor upscale a letterboxu do pomeru 4:3.

## Aktualni stav

- Jeden sdileny software raycaster v C
- DOS backend pro VGA Mode 13h
- Windows backend pres Win32 a GDI
- Failsafe textura pro chybejici assety
- Placeholder scena s kombinaci builtin a chybejicich textur

## Ovládání

- W/S: dopredu/dozadu
- A/D: otaceni
- Q/E: krok do stran
- F: otevreni dveri
- Space: strelba
- Tab: dalsi vlastnena zbran
- Esc: konec

Ve Windows funguje i mouse-look a volitelne leva mys jako fire. Oboji lze menit v [assets/config.txt](assets/config.txt).

Minimapa je nyni lokalni kruhovy vyrez kolem hrace s fog-of-war a lze ji prepinat klavesou M.

Na DOS backendu jsou primarne podporovane klavesy W, A, S, D, Q, E a Esc.

Mapovani klaves i FPS limit lze menit v [assets/config.txt](assets/config.txt).

## Asset pipeline

Renderer umi nacitat jednoduche paletove textury jako soubory RAW o velikosti 64x64 bajtu. Kazdy bajt je primo index do 256barvove palety.

- Soubor: `assets/wall01.raw`
- Rozmer: 64x64
- Velikost: 4096 bajtu
- Format: 1 bajt = 1 index do palety

Kdyz soubor chybi nebo ma spatnou velikost, engine automaticky pouzije fallback texturu. V demu je fallback zamerne viditelny na casti mapy, aby bylo jasne, ze mechanismus funguje.

Mapa se nacita z assets/level.txt podle skutecneho rozmeru souboru. Kazdy radek je jedna rada mapy, sirka mapy se urci podle nejdelsiho radku a kratsi radky se doplni prazdnymi poli. Sprity, pickupy, nepratele, zamcene dvere a trigger body jsou kodovane primo v mape, takze muzes menit layout bez rekompilace.

Aktualni symboly v mape:

- . = prazdne pole
- 1..4 = steny s texturou 1..4
- D = posuvne dvere
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

Manifest textur je v assets/textures.txt. Kdyz doplnis vlastni RAW soubory, engine se je pokusi nacist; kdyz chybne nebo chybi, zustane funkcni diky fallback texturam.

Skript triggeru a targetu je v assets/logic.txt. Muzes definovat vazby typu `T1 -> A1`, kde jeden trigger spusti vice actuatoru najednou, napriklad otevreni dveri, spawn nepratele nebo prechod na jinou mapu.

Obsah enginu je ted rozdeleny do datovych registru:

- assets/tiles.txt pro znaceni mapy a chovani tile symbolu
- assets/entities.txt pro nepratele, pickupy, AI typy a dekorace
- assets/weapons.txt pro statistiky zbrani a jejich ammo type
- assets/game.txt pro startovni nastaveni hry
- assets/textures.txt pro vazbu texture id -> soubor
- assets/logic.txt pro trigger/actuator script dane mapy

To znamena, ze nove mapy, nepratele, pickupy, zamcene dvere, AI chovani, textury, ammo typy a parametry zbrani uz nemusis pridavat pres dalsi `if` vetve v kodu. Pridavas je primarne zmenou datovych souboru.

Stejne tak runtime config je v [assets/config.txt](assets/config.txt): muzes tam menit `fps_cap`, `bind`, `mouse_look`, `mouse_fire` a `mouse_sensitivity` bez rekompilace.

## Doporucene nastroje

Minimalni sestava:

1. Open Watcom 2 pro DOS build: <https://github.com/open-watcom/open-watcom-v2/releases>
1. DOSBox-X pro spousteni DOS verze: <https://dosbox-x.com/>

Volitelne pro pohodlnejsi Windows build:

1. MSYS2 UCRT64 MinGW-w64 GCC
2. Visual Studio Build Tools

## Build

### DOS build pres Open Watcom

1. Nainstaluj Open Watcom 2.
2. Otevri shell s nastavenym prostredim Watcomu.
3. Spust `build_dos.bat`.
4. Vznikne `build\hexdos.exe`.

### Windows build

`build_win.bat` se pokusi pouzit nejdriv GCC z PATH, potom bezne MSYS2 instalace `C:\msys64\ucrt64\bin\gcc.exe` a `C:\msys64\mingw64\bin\gcc.exe`, a nakonec `wcl386` z Open Watcomu.

Doporucena varianta pro Windows build:

1. Nainstaluj MSYS2.
2. V UCRT64 shellu doinstaluj balicek `mingw-w64-ucrt-x86_64-gcc`.
3. Spust `build_win.bat` bud z UCRT64 shellu, nebo z PowerShellu/cmd s `C:\msys64\ucrt64\bin` v PATH.

Po uspesnem buildu vznikne `build\hexwin.exe`.

## Struktura

- `src/` sdileny engine a platformni backandy
- `assets/` budoucni textury
- `tools/level_editor.html` samostatny editor map bez zavislosti
- `build_dos.bat` DOS build
- `build_win.bat` Windows build

## Level Editor

V [tools/level_editor.html](tools/level_editor.html) je jednoduchy editor pro mapy a logic.txt ve stejnem textovem formatu, ktery engine nacita.

- umi otevrit a ulozit `level.txt`
- umi otevrit a ulozit `logic.txt`
- umi menit rozmery mapy
- umi malovat vsechny podporovane symboly
- umi pridavat, upravovat a mazat triggery i actuatory
- umi exportovat text mapy i logiky nebo je rovnou ulozit do souboru
- umi kontrolovat, jestli trigger souradnice opravdu miri na `T` marker v mape

Ve Windows staci soubor otevrit v Edge nebo Chrome. Pokud prohlizec podporuje File System Access API, umi editor ukladat primo zpet do souboru.

## Dalsi logicke kroky

1. Doplnit format pro sprite assety.
2. Dodelat nacitani mapy ze souboru.
3. Pridat billboard sprity, dvere a jednoduche interakce.
4. Zavedeni HUD a fantasy UI vrstvy.
