@echo off
setlocal

if not exist build mkdir build

set "GCC_BIN="

where gcc >nul 2>nul
if %errorlevel%==0 set "GCC_BIN=gcc"

if not "%GCC_BIN%"=="" goto use_gcc

where x86_64-w64-mingw32-gcc >nul 2>nul
if %errorlevel%==0 set "GCC_BIN=x86_64-w64-mingw32-gcc"

if not "%GCC_BIN%"=="" goto use_gcc

if exist "C:\msys64\ucrt64\bin\gcc.exe" set "GCC_BIN=C:\msys64\ucrt64\bin\gcc.exe"
if exist "C:\msys64\mingw64\bin\gcc.exe" set "GCC_BIN=C:\msys64\mingw64\bin\gcc.exe"

if not "%GCC_BIN%"=="" goto use_gcc

if "%WATCOM%"=="" goto no_compiler

echo Pouzivam Open Watcom 32-bit Windows target...
wcl386 -q -bt=nt -ox -i=src -fe=build\hexwin.exe ^
  src\main_win.c src\game.c src\render.c src\assets.c src\platform_win.c src\config.c ^
  user32.lib gdi32.lib

if errorlevel 1 (
    echo Windows build selhal.
    exit /b 1
)

echo Hotovo: build\hexwin.exe
exit /b 0

:use_gcc
echo Pouzivam GCC: %GCC_BIN%

echo %GCC_BIN% | find "\" >nul
if not errorlevel 1 (
    for %%I in ("%GCC_BIN%") do set "GCC_DIR=%%~dpI"
    if not "%GCC_DIR%"=="" set "PATH=%GCC_DIR%;%PATH%"
)

"%GCC_BIN%" -std=c89 -O2 -Isrc -o build\hexwin.exe ^
  src\main_win.c src\game.c src\render.c src\assets.c src\platform_win.c src\config.c ^
  -lgdi32 -luser32 -lm

if errorlevel 1 (
    echo Windows build selhal.
    exit /b 1
)

echo Hotovo: build\hexwin.exe
exit /b 0

:no_compiler
echo Neni dostupny ani GCC ani Open Watcom.
echo Pro MSYS2 nainstaluj balicek mingw-w64-ucrt-x86_64-gcc a pouzij UCRT64 shell.
echo Alternativne pridej C:\msys64\ucrt64\bin do PATH.
exit /b 1
