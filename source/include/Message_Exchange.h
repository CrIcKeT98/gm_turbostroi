#ifndef MESSAGE_EXCHANGE_H
#define MESSAGE_EXCHANGE_H

#include <GarrysMod/Lua/Interface.h>

/////////////////////////////////////////
//API FOR GMOD
/////////////////////////////////////////

LUA_FUNCTION_DECLARE(API_InitializeTrain);
LUA_FUNCTION_DECLARE(API_DeinitializeTrain);

LUA_FUNCTION_DECLARE(API_RegisterSystem);
LUA_FUNCTION_DECLARE(API_LoadSystem);

LUA_FUNCTION_DECLARE(API_SendMessage);
LUA_FUNCTION_DECLARE(API_RecvMessage);
LUA_FUNCTION_DECLARE(API_RecvMessages);
LUA_FUNCTION_DECLARE(API_ReadAvailable);

LUA_FUNCTION_DECLARE(API_SetSimulationFPS);
LUA_FUNCTION_DECLARE(API_SetMTAffinityMask);
LUA_FUNCTION_DECLARE(API_SetSTAffinityMask);
LUA_FUNCTION_DECLARE(API_StartRailNetwork);

LUA_FUNCTION_DECLARE(Think_handler);

/////////////////////////////////////////
//LUAJIT FUNCS
/////////////////////////////////////////

int InitInterfaces();
void InstallHooks(GarrysMod::Lua::ILuaBase* LUA);
void load(GarrysMod::Lua::ILuaBase* LUA, lua_State* L, const char* filename, const char* path, const char* variable = NULL, const char* defpath = NULL, bool json = false);
int shared_print(lua_State* L);
int ThreadRecvMessages(lua_State* state);
void ThreadSimulation(thread_userdata* userdata);
void ClearLoadCache(const CCommand& command);

#endif
