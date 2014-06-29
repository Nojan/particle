-- A solution contains projects, and defines the available configurations
solution "Particle"
    configurations { "Debug", "Release" }
    location "build"
    libdirs { "externals/**" }
    includedirs { "externals/", "externals/**" }
    defines "GLM_FORCE_RADIANS"
    defines "GLEW_STATIC"

    -- A project defines one build target
    project "Particle"
        kind "ConsoleApp"
        language "C++"
        location "build"
        files { "sources/**.hpp", "sources/**.cpp" }
        
    configuration "windows"
        defines "WIN32"
        links { "glfw3", "glew32s", "glu32", "opengl32"}

    configuration "linux"
        buildoptions "-std=c++11"
        links { "glfw", "GLEW", "GLU", "GL" }
 
    configuration "Debug" 
        defines { "DEBUG" }
        flags { "Symbols", "EnableSSE2" }
        targetdir "debug"

    configuration "Release"
        defines { "NDEBUG" }
        flags { "Optimize", "Symbols", "EnableSSE2" }   
        targetdir "release"
