----------------------------------------------------
------------------ include + libs ------------------
----------------------------------------------------

IncludeDir = {}
IncludeDir["VulkanSDK"] = "%{wks.location}/vendor/VulkanSDK/include"
IncludeDir["glfw"] = "%{wks.location}/vendor/glfw/include"
IncludeDir["glm"] = "%{wks.location}/vendor/glm"
IncludeDir["imgui"] = "%{wks.location}/vendor/imgui"
IncludeDir["stb"] = "%{wks.location}/vendor/stb"
IncludeDir["tinyobjloader"] = "%{wks.location}/vendor/tinyobjloader"
IncludeDir["json"] = "%{wks.location}/vendor/json/include"

LibraryDir = {}
LibraryDir["VulkanSDK"] = "%{wks.location}/vendor/VulkanSDK/lib"

Library = {
    windows = {
        Vulkan = "vulkan-1.lib"
    },
    linux = {
        Vulkan = "vulkan"
    }
}
