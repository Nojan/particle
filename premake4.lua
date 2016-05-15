-- A solution contains projects, and defines the available configurations
solution "Particle"
    configurations { "Debug", "Release" }
    location "build"
    libdirs { "externals/**" }
    includedirs { "externals/", "externals/**" }
    defines "GLM_FORCE_RADIANS"

    -- A project defines one build target
    project "Particle"
        kind "ConsoleApp"
        language "C++"
        location "build"
        files { "sources/**.hpp", "sources/**.cpp", "sources/**.h", "sources/**.c" }
        
    configuration "windows"
        defines { "WIN32", "NOMINMAX" }
        links { "glfw3", "glu32", "opengl32"}

    configuration "linux"
        buildoptions "-std=c++11"
        defines "__linux__"
        links { "glfw", "dl", "GLU", "GL" }
 
    configuration "Debug" 
        defines { "DEBUG" }
        flags { "Symbols", "EnableSSE2" }
        targetdir "debug"

    configuration "Release"
        defines { "NDEBUG" }
        flags { "Optimize", "Symbols", "EnableSSE2" }   
        targetdir "release"
