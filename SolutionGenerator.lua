workspace "NRaster"
	configurations 	{ "Debug", "Release" }
	platforms 		{ "x64" }
	systemversion 	  "10.0.16299.0"

   	filter "configurations:Debug"
    	defines { "DEBUG" }
      	symbols "On"
      	inlining("auto")

   	filter "configurations:Release"
      	defines { "NDEBUG" }
      	optimize  "On"
      	inlining("auto")

	filter "platforms:x64"
    	architecture "x64"

project "NRaster"
	kind  		"ConsoleApp"
	language 	"C++"
	vectorextensions "avx"
	vectorextensions "SSE4.1"
	location 	"Temp/VSFiles"
	targetdir 	"Binaries/%{cfg.platform}/%{cfg.buildcfg}"
	files 		{ "Source/**.cpp","Source/**.h" }
	files 		{"Dependencies/tinythreads/source/*.cpp"}
	includedirs 
	{
		"Source/",
		"Dependencies/SDL2-2.0.9/include/",
		"Dependencies/hlslpp/include/",
		"Dependencies/glm/glm/",
		"Dependencies/tinyobj/",
		"Dependencies/tinythreads/source"
	}
	defines {"NOMINMAX"}
	libdirs {"Dependencies/SDL2-2.0.9/lib/x64/"}
	links   {"SDL2","SDL2main"}
   		
   


	