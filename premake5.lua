PROJECT_GENERATOR_VERSION = 2

include("external/garrysmod_common")

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
