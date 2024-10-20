project "glfw"
    location "bin-int/project-files"
    kind "StaticLib"
    language "C"
    
    targetdir("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
    objdir("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

    files
    {
        "glfw/include/GLFW/glfw3.h",
        "glfw/include/GLFW/glfw3native.h",
        "glfw/src/**.c",
        "glfw/src/**.h"
    }

    filter "system:windows"
        systemversion "latest"
        staticruntime "On"

        defines 
        { 
            "_GLFW_WIN32",
            "_CRT_SECURE_NO_WARNINGS"
        }

    filter "system:linux"
        pic "On"
        systemversion "latest"
        staticruntime "On"

        defines
		{
			"_GLFW_X11"
		}

    filter "configurations:Debug"
        runtime "Debug"
        symbols "on"

    filter "configurations:Release"
        runtime "Release"
        optimize "on"
