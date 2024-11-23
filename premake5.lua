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
        include "vendor/imgui.lua"
    group ""

    include "ParticleSystem/particle_system.lua"
