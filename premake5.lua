PROJECT_GENERATOR_VERSION = 3

local workspace_name = "turbostroi"
local project_serverside = true

newoption({
	trigger = "gmcommon",
	description = "Sets the path to the garrysmod_common (https://github.com/danielga/garrysmod_common) directory",
	value = "../garrysmod_common"
})

local gmcommon = assert(_OPTIONS.gmcommon or os.getenv("GARRYSMOD_COMMON"),
	"you didn't provide a path to your garrysmod_common (https://github.com/danielga/garrysmod_common) directory")
include(gmcommon .. "/generator.v2.lua")

sysincludedirs { "source/include", 
				"external/LuaJIT/src", 
				"external/boost/libs"
				}
				
--Fix include order
removesysincludedirs 	{ "external/garrysmod_common/include", "external/garrysmod_common/helpers/include" }
sysincludedirs 			{ "external/garrysmod_common/include", "external/garrysmod_common/helpers/include" }


CreateWorkspace({name = "gm_turbostroi", abi_compatible = false, path = "projects/" .. os.target() .. "/" .. _ACTION})

CreateProject({serverside = true, source_path = "source", manual_files = false})

filter({"system:windows", "architecture:x86"})
	libdirs("external/LuaJIT/x86")
	
filter({"system:windows", "architecture:x86_64"})
	libdirs("external/LuaJIT/x64")

filter("system:windows or macosx")
	links("lua51", "luajit")

filter({"system:linux", "architecture:x86"})
	libdirs("external/LuaJIT/linux32")
	links("lua51", "luajit")

filter({"system:linux", "architecture:x86_64"})
	links("lua51", "luajit")

filter({})

IncludeSDKCommon()
IncludeSDKTier0()
IncludeSDKTier1()
