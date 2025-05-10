@echo off

set "SCRIPT_DIR=%~dp0"
set "VCPKG_DIR=%SCRIPT_DIR%vcpkg"

if not exist "%VCPKG_DIR%" (
    rem Check if git is installed
    where git >nul 2>&1
    if errorlevel 1 (
        echo Git is not installed or not in PATH.
        echo Git is required to clone vcpkg as it's not installed.
        exit /b 1
    )
    echo vcpkg not found. Cloning vcpkg...
    git clone https://github.com/microsoft/vcpkg.git
)

if not exist "%VCPKG_DIR%\vcpkg.exe" (
    echo Bootstrapping vcpkg...
    call "%VCPKG_DIR%\bootstrap-vcpkg.bat"
)

rem Add vcpkg to the environment PATH
set "PATH=%PATH%;%VCPKG_DIR%"

echo Installing dependencies...
vcpkg install --triplet x64-windows
