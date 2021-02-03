project "ImGui"
	kind "StaticLib"
	language "C++"

	files
	{
		"src/imconfig.h",
		"src/imgui.h",
		"src/imgui.cpp",
		"src/imgui_draw.cpp",
		"src/imgui_internal.h",
		"src/imgui_widgets.cpp",
		"src/imstb_rectpack.h",
		"src/imstb_textedit.h",
		"src/imstb_truetype.h",
		"src/imgui_demo.cpp",
		"src/imgui_tables.cpp"
	}

	filter "system:windows"
		systemversion "latest"
		cppdialect "C++17"
		staticruntime "On"

	filter "system:linux"
		pic "On"
		systemversion "latest"
		cppdialect "C++17"
		staticruntime "On"

	filter "configurations:Debug"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		runtime "Release"
		optimize "on"