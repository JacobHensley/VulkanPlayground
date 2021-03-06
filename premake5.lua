VK_SDK_PATH = os.getenv("VK_SDK_PATH")

workspace "VulkanPlayground"
	architecture "x64"
	startproject "VulkanPlayground"

	configurations
	{
		"Debug",
		"Release"
	}

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

IncludeDir = {}
IncludeDir["VulkanSDK"]         = VK_SDK_PATH .. "/include"
IncludeDir["GLFW"]              = "VulkanPlayground/vendor/GLFW/include"
IncludeDir["glm"]               = "VulkanPlayground/vendor/glm"
IncludeDir["spdlog"]            = "VulkanPlayground/vendor/spdlog/include"
IncludeDir["VMA"]               = "VulkanPlayground/vendor/VMA/include"
IncludeDir["SPIRVCross"]        = "VulkanPlayground/vendor/SPIRV-Cross"
IncludeDir["imgui"]             = "VulkanPlayground/vendor/imgui"
IncludeDir["stb_image"]         = "VulkanPlayground/vendor/stb"

include "VulkanPlayground/vendor/GLFW"
include "VulkanPlayground/vendor/SPIRV-Cross"
include "VulkanPlayground/vendor/imgui"

project "VulkanPlayground"
	location "VulkanPlayground"
	kind "ConsoleApp"
	language "C++"
	staticruntime "on"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin/intermediates/" .. outputdir .. "/%{prj.name}")	

	pchheader "pch.h"
	pchsource "VulkanPlayground/src/pch.cpp"

	files
	{
		"%{prj.name}/src/**.cpp",
		"%{prj.name}/src/**.h",
		-- STB
		"%{prj.name}/vendor/stb/**.cpp",
		"%{prj.name}/vendor/stb/**.h",
		-- TinyGltf
		"%{prj.name}/vendor/tinygltf/**.cpp",
		"%{prj.name}/vendor/tinygltf/**.hpp",
		"%{prj.name}/vendor/tinygltf/**.h",
	}

	includedirs
	{
		"%{prj.name}/src",
		"%{prj.name}/vendor",
		"%{IncludeDir.VulkanSDK}",
		"%{IncludeDir.GLFW}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.spdlog}",
		"%{IncludeDir.VMA}",
		"%{IncludeDir.SPIRVCross}",
		"%{IncludeDir.imgui}",
		"%{IncludeDir.stb_image}",
	}

	links 
	{ 
		"GLFW",
		"SPIRV-Cross",
		"imgui",
		VK_SDK_PATH .. "/Lib/vulkan-1.lib",
		VK_SDK_PATH .. "/Lib/shaderc_shared.lib",
	}

	filter "system:windows"
		cppdialect "C++17"
		systemversion "latest"

	filter "configurations:Debug"
		runtime "Debug"
		symbols "On"

	defines 
	{
		"ENABLE_ASSERTS"
	}

	filter "configurations:Release"
		runtime "Release"
		optimize "On"