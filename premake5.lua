workspace "baremetal"
    configurations { "Release", "Debug" }
    startproject "test"

    architecture "x64"
    language "C++"
    cppdialect "C++latest"
    warnings "Extra"

    flags {
        "MultiProcessorCompile"
    }

    -- buildoptions { "-fsanitize=address" }
    -- linkoptions { "-fsanitize=address" }
    -- debugformat "C7"

    filter "toolset:gcc or clang"
        buildoptions { "-Wno-comment", "-Wno-missing-field-initializers" }

    filter "configurations:Release"
        defines { "NDEBUG" }
        optimize "On"
        symbols "Off"

    filter "configurations:Debug"
        symbols "On"
        optimize "Off"
        runtime "Debug"
        defines { "DEBUG", "_DEBUG" }

project "utility"
    kind "Utility"
    location "source/utility"

    targetdir "bin/%{cfg.buildcfg}"

    files { "source/utility/**.cpp", "source/utility/**.h" }
    includedirs { "source/" }

    targetdir "output/utility/bin/%{cfg.buildcfg}"
    objdir "output/utility/obj/%{cfg.buildcfg}"

project "baremetal"
    kind "StaticLib"
    location "source/baremetal"

    targetdir "bin/%{cfg.buildcfg}"

    files { "source/baremetal/**.cpp", "source/baremetal/**.h" }
    includedirs { "source/" }
    links { "utility" }

    targetdir "output/baremetal/bin/%{cfg.buildcfg}"
    objdir "output/baremetal/obj/%{cfg.buildcfg}"


project "test"
    kind "ConsoleApp"
    location "test"

    targetdir "bin/%{cfg.buildcfg}"
    files { "source/test/**.cpp" }
    includedirs { "source/" }
    links { "baremetal", "utility" }

    targetdir "output/test/bin/%{cfg.buildcfg}"
    objdir "output/test/obj/%{cfg.buildcfg}"