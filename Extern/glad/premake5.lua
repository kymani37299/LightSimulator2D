project "GLAD"
	kind "StaticLib"
	language "C"

	files
	{
		"include/glad.h",
		"src/glad.c",
		"include/KHR/khrplatform.h"
	}

	includedirs
	{
		"include"
	}