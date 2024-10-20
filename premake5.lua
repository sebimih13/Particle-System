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
    
    if vulkan_sdk then
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

            print("Remove Vulkan SDK Installer")
            os.remove(WINDOWS_VULKAN_SDK_OUTPUT_FILE)
        elseif os.target() == "linux" then
            print("Download installer for Linux")
            print()
            local result_str, response_code = http.download(LINUX_VULKAN_SDK_INSTALL_URL, LINUX_VULKAN_SDK_OUTPUT_FILE, { progress = progress})
            print(result_str)
            print()
        end
    end
end

----------------------------------------------------
-------------- Workspace and projects --------------
----------------------------------------------------

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

    -- TODO: move to 'dependencies.lua'
    -- Include directories relative to root folder (solution directory)
    IncludeDir = {}
    IncludeDir["VulkanSDK"] = "vendor/VulkanSDK/include"
    IncludeDir["glfw"] = "vendor/glfw/include"
    IncludeDir["glm"] = "vendor/glm"



    project "ParticleSystem"
        location "ParticleSystem"
        kind "ConsoleApp"
        language "C++"
        cppdialect "C++17"

        targetdir("bin/" .. outputdir .. "/%{prj.name}")
        objdir("bin-int/" .. outputdir .. "/%{prj.name}")

        files
        {
            "%{prj.name}/**.h",
            "%{prj.name}/**.cpp",
            "%{prj.name}/**.hpp",
            "%{prj.name}/**.vert",
            "%{prj.name}/**.frag"
        }

        includedirs
        {
            "%{IncludeDir.glfw}",
            "%{IncludeDir.VulkanSDK}",
            "%{IncludeDir.glm}"
        }

        libdirs
        {
            "vendor/VulkanSDK/lib"
        }

        links
        {
            "glfw",
            "vulkan-1.lib",
        }

        -- TODO: only the files existing at the time of executing this script will be considered
        --       any new addition will be ignored, so you have to run again this script

        -- prebuild command to automatically compile .vert and .frag files
        filter "files:**.vert or **.frag"
            -- A message to display while this build step is running (optional)
            buildmessage 'Compiling %{file.name}'

            -- One or more commands to run (required)
            buildcommands 
            {
                '"shaders/glslc.exe" "%{file.relpath}" -o "%{cfg.targetdir}/shaders/%{file.name}.spv"', -- TODO : compiled shaders for bin directory
                '"shaders/glslc.exe" "%{file.relpath}" -o "%{file.directory}/%{file.name}.spv"'
            }

            -- One or more outputs resulting from the build (required)
            buildoutputs
            { 
                '%{cfg.targetdir}/shaders/%{file.name}.spv',    -- TODO : compiled shaders for bin directory
                '%{file.directory}/%{file.name}.spv'
            }

        filter "system:windows"
            staticruntime "On"
            systemversion "latest"

        filter "configurations:Debug"
            symbols "On"

            defines
            {
                "DEBUG"
            }

        filter "configurations:Release"
            optimize "On"

            defines
            {
                "NDEBUG"
            }
