----------------------------------------------------
------------------ include + libs ------------------
----------------------------------------------------

IncludeDir = {}
IncludeDir["VulkanSDK"] = "%{wks.location}/vendor/VulkanSDK/include"
IncludeDir["glfw"] = "%{wks.location}/vendor/glfw/include"
IncludeDir["glm"] = "%{wks.location}/vendor/glm"

LibraryDir = {}
LibraryDir["VulkanSDK"] = "%{wks.location}/vendor/VulkanSDK/lib"

Library = {}
Library["Vulkan"] = "vulkan-1.lib"
