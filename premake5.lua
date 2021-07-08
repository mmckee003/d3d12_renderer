workspace "d3d12_renderer"
	architecture "x64"
	startproject "d3d12_renderer"

	configurations { "Debug", "Release", "Dist" }

outputdir = "%{cfg.buildcfg}/%{cfg.system}/%{cfg.architecture}"

-- Include directories relative to root folder --
IncludeDir = {}
--IncludeDir["GLFW"] = "Rogue/vendor/GLFW/include"

-- Includes GLFW premake5.lua
--include "Rogue/vendor/GLFW"

project "d3d12_renderer"
	kind "WindowedApp"
	language "C++"

	targetdir ("build/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files {
		"src/**.h",
		"src/**.cpp",
		"src/core/**.h",
		"src/core/**.cpp"
	}

	includedirs {
		"src"
	}
	
	links {

	}

	filter "system:windows"
		cppdialect "c++17"
		staticruntime "On"
		systemversion "latest"

		defines {
			"PLATFORM_WINDOWS"
		}

	filter "configurations:Debug"
			defines {
				"RENDERER_DEBUG",
				"RENDERER_ENABLE_ASSERTS"
			}
			symbols "On"

	filter "configurations:Release"
			defines "RENDERER_RELEASE"
			optimize "On"

	filter "configurations:Dist"
			defines "RENDERER_DIST"
			optimize "On"