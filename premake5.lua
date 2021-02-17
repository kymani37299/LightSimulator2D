workspace "LightSimulator2D"
	architecture "x86"

	configurations
	{
		"Debug",
		"Release"
	}

	targetdir ("out/bin/%{prj.name}/%{cfg.longname}")
	objdir ("out/obj/%{prj.name}/%{cfg.longname}")

include "Extern/glad"
include "Extern/imgui"

project "LightSimulator2D"
	location "LightSimulator2D"
	kind "ConsoleApp"
	language "C++"

	files
	{
		"%{prj.name}/src/**.vert",
		"%{prj.name}/src/**.frag",
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp",
		"%{prj.name}/src/**.cs"
	}

	includedirs
	{
		"%{prj.name}/src",
		"Extern/glad/include",
		"Extern/glfw/include",
		"Extern/glm/include",
		"Extern/stb/include",
		"Extern/imgui/src"
	}

	libdirs
	{
		"Extern/glfw/lib"
	}

	links
	{
		"glfw3.lib",
		"opengl32.lib",
		"user32.lib",
		"GLAD",
		"ImGui"
	}

	postbuildcommands 
	{
		"{COPY} src/shaders %{cfg.targetdir}/src/shaders"
	}

	filter { "configurations:Debug" }
		linkoptions { "-IGNORE:4098" }
		symbols "On"
		defines
		{
			"DEBUG",
			"_DEBUG",
			"_CONSOLE"
		}

	filter { "configurations:Release" }
		optimize "On"