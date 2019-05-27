workspace "NRaster"
	configurations 	{ "Debug", "Release" }
	platforms 		{ "x64" }
	systemversion 	  "10.0.16299.0"

   	filter "configurations:Debug"
    	defines { "DEBUG" }
      	symbols "On"

   	filter "configurations:Release"
      	defines { "NDEBUG" }
      	optimize  "On"

	filter "platforms:x64"
    	architecture "x64"

project "NRaster"
	kind  		"ConsoleApp"
	language 	"C++"
	location 	"Temp/VSFiles"
	targetdir 	"Binaries/%{cfg.platform}/%{cfg.buildcfg}"
	files 		{ "Source/**.cpp","Source/**.h" }
	includedirs 
	{
		"Source/",
		"Dependencies/SDL2-2.0.9/include/"
	}

	libdirs {"Dependencies/SDL2-2.0.9/lib/x64/"}
	links   {"SDL2","SDL2main"}

   	-- filter "configurations:Debug"
   		
   


	