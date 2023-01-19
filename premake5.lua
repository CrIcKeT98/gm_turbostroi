PROJECT_GENERATOR_VERSION = 2

newoption({
	trigger = "gmcommon",
	description = "Sets the path to the garrysmod_common (https://github.com/danielga/garrysmod_common) directory",
	value = "set in this place path"
})

local gmcommon = assert(_OPTIONS.gmcommon or os.getenv("GARRYSMOD_COMMON"),
	"you didn't provide a path to your garrysmod_common (https://github.com/danielga/garrysmod_common) directory")
include(gmcommon .. "/generator.v2.lua")

includedirs { 	"source/include", 
				"external/LuaJIT/src", 
				"external/boost/",
				"external/garrysmod_common/include",
				"external/garrysmod_common/helpers_extended/include"
			}

CreateWorkspace({name = "turbostroi", abi_compatible = false, path = "projects/" .. os.target() .. "/" .. _ACTION})

local headerDir = "source/include"
files({
	headerDir .. "/*.h",
	headerDir .. "/*.hpp",
	headerDir .. "/*.hxx"
})

CreateProject({serverside = true, source_path = myBestProjectPath, manual_files = false})

filter({"system:windows", "architecture:x86"})
	libdirs({"external/LuaJIT/x86", "external/boost/bin/x86/lib"})
	links{"lua51", "luajit"}
	
filter({"system:windows", "architecture:x86_64"})
	libdirs({"external/LuaJIT/x64", "external/boost/bin/x64/lib"})
	links{"lua51", "luajit"}

filter({"system:linux", "architecture:x86"})
	libdirs({"external/LuaJIT/x86", "external/boost/bin/x86/lib"})
	links{"boost_atomic", "boost_chrono", "boost_thread", "luajit", "pthread", "dl"}

filter({"system:linux", "architecture:x86_64"})
	links{"boost_atomic", "boost_chrono", "boost_thread", "luajit", "pthread", "dl"}

IncludeSDKCommon()
IncludeSDKTier0()
IncludeSDKTier1()
