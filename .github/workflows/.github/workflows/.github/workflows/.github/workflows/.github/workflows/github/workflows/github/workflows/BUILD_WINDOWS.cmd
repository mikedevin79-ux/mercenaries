@echo off
setlocal
cd /d "%~dp0.."
echo Building Total Warfare Overhaul v1.2.0 (total_warfare.dll)...
echo Working folder: %cd%
echo Requires: total_warfare.c, mercenaries_mod.h, mercfx.h in this folder.
echo.

echo Trying MSVC (cl)...
where cl >nul 2>nul
if %errorlevel%==0 (
  cl /nologo /W4 /O2 /LD /I . total_warfare.c /Fe:total_warfare.dll /link /MACHINE:X64
  if %errorlevel%==0 goto :ok
  echo MSVC (cl) was found but the build failed. See errors above.
  echo Make sure you ran from "x64 Native Tools Command Prompt for VS".
  echo.
)

echo Trying MinGW-w64...
where x86_64-w64-mingw32-gcc >nul 2>nul
if %errorlevel%==0 (
  x86_64-w64-mingw32-gcc -shared -O2 -Wall -Wextra -I. total_warfare.c -o total_warfare.dll
  if %errorlevel%==0 goto :ok
  echo MinGW-w64 compiler was found but the build failed. See errors above.
  echo.
)

echo Trying clang...
where clang >nul 2>nul
if %errorlevel%==0 (
  clang --target=x86_64-pc-windows-msvc -shared -O2 -I. total_warfare.c -o total_warfare.dll
  if %errorlevel%==0 goto :ok
  echo clang was found but the build failed. See errors above.
  echo.
)

echo.
echo No supported 64-bit compiler built the DLL.
echo You need one of these installed:
echo   - Visual Studio Build Tools (C++ x64) ^> open "x64 Native Tools Command Prompt for VS"
echo   - MinGW-w64 (x86_64-w64-mingw32-gcc in PATH)
echo   - LLVM/Clang
echo.
echo Then run this script again, or run manually in that environment:
echo   cl /nologo /W4 /O2 /LD /I . total_warfare.c /Fe:total_warfare.dll /link /MACHINE:X64
echo.
pause
exit /b 1

:ok
echo.
echo Build succeeded: mods\total_warfare.dll
echo Enable "Native Effects" in the mod selector for MercFx-enhanced blasts.
pause
exit /b 0
