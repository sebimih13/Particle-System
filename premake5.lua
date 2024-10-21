----------------------------------------------------
---------------------- Global ----------------------
----------------------------------------------------

VULKAN_SDK_SYSTEM_VARIABLE = "VULKAN_SDK"

WINDOWS_VULKAN_SDK_INSTALL_URL = "https://sdk.lunarg.com/sdk/download/latest/windows/vulkan_sdk.exe"
WINDOWS_VULKAN_SDK_OUTPUT_FILE = "VulkanSDK-Installer.exe"

LINUX_VULKAN_SDK_INSTALL_URL = "https://sdk.lunarg.com/sdk/download/latest/linux/vulkan_sdk.tar.xz"
LINUX_VULKAN_SDK_OUTPUT_FILE = "VulkanSDK-Installer.tar.xz"

function progress(total, current)
    local ratio = current / total;
    ratio = math.min(math.max(ratio, 0), 1);
    local percent = math.floor(ratio * 100);

    io.write("\27[F\27[KDownload progress (" .. percent .. "%/100%)\n")
    io.flush()
end

if _ACTION then
    local vulkanSDKPath = os.getenv(VULKAN_SDK_SYSTEM_VARIABLE)
    print(vulkanSDKPath)

    if vulkanSDKPath then
        print("Vulkan SDK is already installed")
    else
        print("Vulkan SDK is not installed")

        if os.target() == "windows" then
            print("Download installer for Windows")
            print()
            local result_str, response_code = http.download(WINDOWS_VULKAN_SDK_INSTALL_URL, WINDOWS_VULKAN_SDK_OUTPUT_FILE, { progress = progress})
            print(result_str)
            print()

            print("Run Vulkan SDK Installer")
            local command = WINDOWS_VULKAN_SDK_OUTPUT_FILE .. " --root C:\\VulkanSDK --accept-licenses --default-answer --confirm-command install com.lunarg.vulkan.core com.lunarg.vulkan.vma"
            print(command)
            os.execute(command)

            -- set the new vulkanSDKPath
            vulkanSDKPath = "C:\\VulkanSDK"

            print("Remove Vulkan SDK Installer")
            os.remove(WINDOWS_VULKAN_SDK_OUTPUT_FILE)            

        elseif os.target() == "linux" then
            -- print("Download installer for Linux")
            -- print()
            -- local result_str, response_code = http.download(LINUX_VULKAN_SDK_INSTALL_URL, LINUX_VULKAN_SDK_OUTPUT_FILE, { progress = progress})
            -- print(result_str)
            -- print()

            -- TODO: implement
            -- print("Run Vulkan SDK Installer")
            -- local command = LINUX_VULKAN_SDK_OUTPUT_FILE .. " --root C:\\VulkanSDK --accept-licenses --default-answer --confirm-command install com.lunarg.vulkan.core com.lunarg.vulkan.vma"
            -- print(command)
            -- os.execute(command)

            -- TODO: set the new vulkanSDKPath
            -- vulkanSDKPath = "TODO"

            -- print("Remove Vulkan SDK Installer")
            -- os.remove(LINUX_VULKAN_SDK_OUTPUT_FILE)
        end
    end

    -- create VulkanSDK vendor folder
    if os.target() == "windows" then
        print("Copy 'include' folder")
        local copyIncludeFolderCommand = string.format("xcopy \"%s\\Include\" \"%s\\vendor\\VulkanSDK\\include\" /E /I /Y", vulkanSDKPath, os.getcwd())
        os.execute(copyIncludeFolderCommand)
        
        print("Copy 'lib' folder")
        local copyLibFolderCommand = string.format("xcopy \"%s\\Lib\" \"%s\\vendor\\VulkanSDK\\lib\" /E /I /Y", vulkanSDKPath, os.getcwd())
        os.execute(copyLibFolderCommand)

        print("Copy 'glslc.exe'")
        local copyGLSLCCommand = string.format("xcopy \"%s\\Bin\\glslc.exe\" \"%s\\vendor\\VulkanSDK\" /E /I /Y", vulkanSDKPath, os.getcwd())
        os.execute(copyGLSLCCommand)

    elseif os.target() == "linux" then
        -- print("Copy 'include' folder")
        -- TODO

        --print("Copy 'lib' folder")
        -- TODO

        -- print("Copy 'glslc.exe'")
        -- TODO
    end
end

----------------------------------------------------
-------------- Workspace and projects --------------
----------------------------------------------------

include "dependencies.lua"

workspace 'ParticleSystem'
    architecture "x64"
    startproject "ParticleSystem"

    configurations
    {
        "Debug",
        "Release"
    }

    -- Ouput directories for bin and intermediate files
    outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

    group "Dependencies"
        include "vendor/glfw.lua"
    group ""

    include "ParticleSystem/particle_system.lua"
