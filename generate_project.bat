@echo off

set WINDOWS_VULKAN_SDK_INSTALL_URL=https://sdk.lunarg.com/sdk/download/latest/windows/vulkan_sdk.exe
set WINDOWS_VULKAN_SDK_OUTPUT_FILE=VulkanSDK-Installer.exe
set VULKAN_SDK_DEFAULT=C:\VulkanSDK

setlocal

:: BatchGotAdmin
:-------------------------------------
REM  --> Check for permissions
>nul 2>&1 "%SYSTEMROOT%\system32\cacls.exe" "%SYSTEMROOT%\system32\config\system"

REM --> If error flag set, we do not have admin.
if '%errorlevel%' NEQ '0' (
    echo Requesting administrative privileges...
    goto UACPrompt
) else ( goto gotAdmin )

:UACPrompt
    echo Set UAC = CreateObject^("Shell.Application"^) > "%temp%\getadmin.vbs"
    set params = %*:"=""
    echo UAC.ShellExecute "cmd.exe", "/c %~s0 %params%", "", "runas", 1 >> "%temp%\getadmin.vbs"

    "%temp%\getadmin.vbs"
    del "%temp%\getadmin.vbs"
    exit /B

:gotAdmin
    pushd "%CD%"
    CD /D "%~dp0"
:--------------------------------------

echo Checking for Vulkan SDK
if defined VULKAN_SDK (
    echo Vulkan SDK is already installed - %VULKAN_SDK%
) else (
    echo Vulkan SDK is not installed

    echo Download installer for Windows
    curl -o "%WINDOWS_VULKAN_SDK_OUTPUT_FILE%" "%WINDOWS_VULKAN_SDK_INSTALL_URL%"

    echo Run Vulkan SDK Installer
    "%WINDOWS_VULKAN_SDK_OUTPUT_FILE%" --root %VULKAN_SDK_DEFAULT% --accept-licenses --default-answer --confirm-command install com.lunarg.vulkan.core com.lunarg.vulkan.vma

    echo Set VULKAN_SDK
    set VULKAN_SDK=%VULKAN_SDK_DEFAULT%

    echo Remove Vulkan SDK Installer
    del "%WINDOWS_VULKAN_SDK_OUTPUT_FILE%"
)

if not defined VULKAN_SDK (
    echo Rerun the script
    pause
    exit
)

echo Checking for VulkanSDK vendor folder
if exist vendor\VulkanSDK (
    echo Found VulkanSDK vendor folder
) else (
    echo Create VulkanSDK vendor folder
    mkdir vendor\VulkanSDK /p

    echo Copy 'include' folder
    xcopy %VULKAN_SDK%\Include\ %cd%\vendor\VulkanSDK\include /E /I /Y

    echo Copy 'lib' folder
    xcopy %VULKAN_SDK%\Lib\ %cd%\\vendor\VulkanSDK\lib /E /I /Y

    echo Copy 'glslc.exe'
    xcopy %VULKAN_SDK%\Bin\glslc.exe %cd%\vendor\VulkanSDK /E /I /Y
)

REM run premake
call vendor\premake5\premake5.exe vs2022

endlocal
pause
exit