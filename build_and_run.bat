@echo off
setlocal
cd /d "%~dp0"

echo Building Path2Plate...

REM Prefer the UCRT64 compiler installed by MSYS2 in the user's Downloads folder.
set "MINGW_BIN=%USERPROFILE%\Downloads\ucrt64\bin"
if not exist "%MINGW_BIN%\g++.exe" set "MINGW_BIN=C:\msys64\ucrt64\bin"

if exist "%MINGW_BIN%\g++.exe" (
    set "PATH=%MINGW_BIN%;%PATH%"
    set "CXX=%MINGW_BIN%\g++.exe"
) else (
    where g++ >nul 2>nul
    if errorlevel 1 (
        echo ERROR: g++ was not found.
        echo Install the MSYS2 UCRT64 compiler or add its bin folder to PATH.
        pause
        exit /b 1
    )
    set "CXX=g++"
)

REM C++14 avoids the Windows RPC 'byte' vs std::byte ambiguity seen with C++17.
"%CXX%" -std=c++14 Path2Plate.cpp -o Path2Plate.exe -lws2_32
if errorlevel 1 (
    echo.
    echo Build failed. Please send a screenshot of the errors above.
    pause
    exit /b 1
)

echo.
echo Build successful. Starting Path2Plate...
Path2Plate.exe
pause
