@echo off
setlocal

if "%WATCOM%"=="" (
    echo Open Watcom environment neni nastaveny.
    echo Spust nejdriv watcom shell nebo nastav WATCOM a PATH.
    exit /b 1
)

if not exist build mkdir build

wcl -q -bt=dos -ml -0 -ox -fm=build\hexdos.map -fe=build\hexdos.exe -i=src ^
  src\main_dos.c src\game.c src\render.c src\assets.c src\platform_dos.c src\config.c

if errorlevel 1 (
    echo DOS build selhal.
    exit /b 1
)

echo Hotovo: build\hexdos.exe
exit /b 0
