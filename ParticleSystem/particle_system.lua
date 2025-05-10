project "ParticleSystem"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++20"

    targetdir("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
    objdir("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

    files
    {
        "**.h",
        "**.cpp",
        "**.hpp",
        "**.vert", 
        "**.frag",
        "**.comp"
    }

    includedirs
    {
        "%{IncludeDir.glfw}",
        "%{IncludeDir.VulkanSDK}",
        "%{IncludeDir.glm}",
        "%{IncludeDir.imgui}",
        "%{IncludeDir.stb}",
        "%{IncludeDir.tinyobjloader}"
    }

    libdirs
    {
        "%{LibraryDir.VulkanSDK}"
    }

    links
    {
        "glfw",
        Library[os.target()]["Vulkan"],
        "imgui"
    }

    defines
    {
        "GLFW_INCLUDE_VULKAN",
        "GLFW_INCLUDE_NONE",

        "GLM_FORCE_RADIANS",
        "GLM_FORCE_DEPTH_ZERO_TO_ONE",
        "GLM_ENABLE_EXPERIMENTAL"
    }

    -- TODO: only the files existing at the time of executing this script will be considered
    --       any new addition will be ignored, so you have to run again generate_project script

    -- prebuild command to automatically compile .vert/.frag/.comp files for Windows
    filter { "files:**.vert or **.frag or **.comp", "system:windows" }
        -- A message to display while this build step is running (optional)
        buildmessage 'Compiling %{file.name}'

        -- One or more commands to run (required)
        buildcommands 
        {
            '"%{wks.location}/vendor/VulkanSDK/glslc.exe" "%{file.relpath}" -o "%{cfg.targetdir}/shaders/%{file.name}.spv"',
            '"%{wks.location}/vendor/VulkanSDK/glslc.exe" "%{file.relpath}" -o "%{file.directory}/%{file.name}.spv"'
        }

        -- One or more outputs resulting from the build (required)
        buildoutputs
        { 
            '%{cfg.targetdir}/shaders/%{file.name}.spv',
            '%{file.directory}/%{file.name}.spv'
        }

    -- prebuild command to automatically compile .vert/.frag/.comp files for Linux
    filter { "files:**.vert or **.frag or **.comp", "system:linux" }
        -- A message to display while this build step is running (optional)
        buildmessage 'Compiling %{file.name}'

        -- One or more commands to run (required)
        buildcommands 
        {
            '"%{wks.location}/vendor/VulkanSDK/glslc" "%{file.relpath}" -o "%{cfg.targetdir}/shaders/%{file.name}.spv"',
            '"%{wks.location}/vendor/VulkanSDK/glslc" "%{file.relpath}" -o "%{file.directory}/%{file.name}.spv"'
        }

        -- One or more outputs resulting from the build (required)
        buildoutputs
        { 
            '%{cfg.targetdir}/shaders/%{file.name}.spv',
            '%{file.directory}/%{file.name}.spv'
        }

    filter "system:windows"
        staticruntime "On"
        systemversion "latest"

        defines
        {
            "PLATFORM_WINDOWS"
        }

    filter "system:linux"
        prebuildcommands
        {
            '[ -d "%{cfg.targetdir}/shaders" ] || mkdir -p "%{cfg.targetdir}/shaders"'
        }

        defines
        {
            "PLATFORM_LINUX"
        }

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
