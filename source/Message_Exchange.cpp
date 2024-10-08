#include <gm_turbostroi.h>
#include <Message_Exchange.h>

using namespace GarrysMod::Lua;

int SimThreadAffinityMask = 0;
unsigned int ThreadTickrate = 10;
double TargetTime = 0.0;
bool ThinkRunning = false;
bool ThinkStopped = true;

std::vector<TrainSystem> MetrostroiSystemsList;			// <BaseName,FileName>
std::vector<TrainSystem> LoadSystemList;				// <BaseName,LocalName>
boost::unordered_map<std::string, std::string> LoadedFilesCache;	// <path,content>

boost::lockfree::queue <shared_message, boost::lockfree::fixed_sized<true>, boost::lockfree::capacity<64>> printMessages;

SourceSDK::FactoryLoader ICvar_Loader("vstdlib");
ICvar* p_ICvar = nullptr;

/////////////////////////////////////////
//API FOR GMOD
/////////////////////////////////////////

LUA_FUNCTION(API_InitializeTrain)
{
	//Initialize LuaJIT for train
	lua_State* L = luaL_newstate();
	luaL_openlibs(L);
	luaopen_bit(L);
	lua_pushboolean(L, 1);
	lua_setglobal(L, "TURBOSTROI");
	lua_pushcfunction(L, shared_print);
	lua_setglobal(L, "print");
	lua_pushcfunction(L, ThreadRecvMessages);
	lua_setglobal(L, "RecvMessages");

	//Load neccessary files
	load(LUA, L, "metrostroi/sv_turbostroi_v2.lua", "LUA");
	load(LUA, L, "metrostroi/sh_failsim.lua", "LUA");

	//Load up all the systems
	for (TrainSystem system : MetrostroiSystemsList)
	{
		load(LUA, L, system.FileName.c_str(), "LUA");
	}

	//Initialize all the systems reported by the train
	for (TrainSystem system : LoadSystemList)
	{
		lua_getglobal(L, "LoadSystems");
		lua_pushstring(L, system.BaseName.c_str());
		lua_setfield(L, -2, system.FileName.c_str());
	}
	LoadSystemList.clear();

	//Initialize systems
	lua_getglobal(L, "Initialize");
	if (lua_pcall(L, 0, 0, 0)) {
		lua_pushcfunction(L, shared_print);
		lua_pushvalue(L, -2);
		lua_call(L, 1, 0);
		lua_pop(L, 1);
	}

	//New userdata
	thread_userdata* userdata = new thread_userdata();
	userdata->finished = 0;
	userdata->L = L;
	userdata->current_time = Plat_FloatTime();
	LUA->PushUserType(userdata, 1);
	LUA->SetField(1, "_sim_userdata");
	lua_pushlightuserdata(L, userdata);
	lua_setglobal(L, "_userdata");

	//Create thread for simulation
	boost::thread thread(ThreadSimulation, userdata);

	thread.detach();

	return 0;
}

LUA_FUNCTION(API_DeinitializeTrain)
{
	LUA->GetField(1, "_sim_userdata");
	thread_userdata* userdata = LUA->GetUserType<thread_userdata>(-1, 1);
	if (userdata) userdata->finished = 1;
	LUA->Pop();

	LUA->PushNil();
	LUA->SetField(1, "_sim_userdata");

	return 0;
}

LUA_FUNCTION(API_RegisterSystem)
{
	const char* name = LUA->GetString(1);
	const char* filename = LUA->GetString(2);
	if (!name || !filename)
		return 0;

	ConMsg("Metrostroi: Registering system %s [%s]\n", name, filename);
	MetrostroiSystemsList.emplace_back(name, filename);

	return 0;
}

LUA_FUNCTION(API_LoadSystem)
{
	const char* basename = LUA->GetString(1);
	const char* name = LUA->GetString(2);
	if (!basename || !name)
		return 0;

	LoadSystemList.emplace_back(name, basename);

	return 0;
}

LUA_FUNCTION(API_SendMessage)
{
	bool successful = true;
	LUA->CheckType(2, Type::Number);
	LUA->CheckType(3, Type::String);
	LUA->CheckType(4, Type::String);
	LUA->CheckType(5, Type::Number);
	LUA->CheckType(6, Type::Number);

	LUA->GetField(1, "_sim_userdata");
	thread_userdata* userdata = LUA->GetUserType<thread_userdata>(-1, 1);
	LUA->Pop();

	if (userdata) {
		thread_msg tmsg;
		tmsg.message = (int)LUA->GetNumber(2);
		strncpy(tmsg.system_name, LUA->GetString(3), 63);
		tmsg.system_name[63] = 0;
		strncpy(tmsg.name, LUA->GetString(4), 63);
		tmsg.name[63] = 0;
		tmsg.index = LUA->GetNumber(5);
		tmsg.value = LUA->GetNumber(6);
		if (!userdata->sim_to_thread.push(tmsg)) {
			successful = false;
		}
	}
	else {
		successful = false;
	}
	LUA->PushBool(successful);

	return 1;
}

LUA_FUNCTION(API_RecvMessage)
{
	LUA->GetField(1, "_sim_userdata");
	thread_userdata* userdata = LUA->GetUserType<thread_userdata>(-1, 1);
	LUA->Pop();

	if (userdata) {
		thread_msg tmsg;
		if (userdata->thread_to_sim.pop(tmsg)) {
			LUA->PushNumber(tmsg.message);
			LUA->PushString(tmsg.system_name);
			LUA->PushString(tmsg.name);
			LUA->PushNumber(tmsg.index);
			LUA->PushNumber(tmsg.value);
			return 5;
		}
	}
	return 0;
}

LUA_FUNCTION(API_RecvMessages)
{
	LUA->GetField(1, "_sim_userdata");
	thread_userdata* userdata = LUA->GetUserType<thread_userdata>(-1, 1);
	LUA->Pop();

	if (userdata) {
		LUA->CreateTable();
		for (size_t i = 0; i < userdata->thread_to_sim.read_available(); ++i) {
			userdata->thread_to_sim.consume_one([&](thread_msg tmsg) {
				LUA->PushNumber(i);
				LUA->CreateTable();
				LUA->PushNumber(1);		LUA->PushNumber(tmsg.message);			LUA->RawSet(-3);
				LUA->PushNumber(2);		LUA->PushString(tmsg.system_name);		LUA->RawSet(-3);
				LUA->PushNumber(3);		LUA->PushString(tmsg.name);				LUA->RawSet(-3);
				LUA->PushNumber(4);		LUA->PushNumber(tmsg.index);			LUA->RawSet(-3);
				LUA->PushNumber(5);		LUA->PushNumber(tmsg.value);			LUA->RawSet(-3);
				LUA->RawSet(-3);
			});
		}
		return 1;
	}
	return 0;
}

LUA_FUNCTION(API_ReadAvailable)
{
	LUA->GetField(1, "_sim_userdata");
	thread_userdata* userdata = LUA->GetUserType<thread_userdata>(-1, 1);
	LUA->Pop();
	LUA->PushNumber(userdata->thread_to_sim.read_available());
	return 1;
}

LUA_FUNCTION(API_SetSimulationFPS)
{
	LUA->CheckType(1, Type::Number);
	ThreadTickrate = 1000u / LUA->GetNumber(1);
	ConColorMsg(Color(0, 255, 0, 255), "Turbostroi: Changed to %u TPS (%d ms delay)\n", (unsigned int)(LUA->GetNumber(1) + 0.5), ThreadTickrate);
	return 0;
}

LUA_FUNCTION(API_SetMTAffinityMask)
{
	LUA->CheckType(1, Type::Number);
	int MTAffinityMask = LUA->GetNumber(1);

#if defined(_WIN32)
	ConColorMsg(Color(0, 255, 0, 255), "Turbostroi: Main Thread Running on CPU%i\n", GetCurrentProcessorNumber());

	if (!SetThreadAffinityMask(GetCurrentThread(), static_cast<DWORD_PTR>(MTAffinityMask)))
		ConColorMsg(Color(255, 0, 0, 255), "Turbostroi: SetMTAffinityMask failed!\n");
	else
		ConColorMsg(Color(0, 255, 0, 255), "Turbostroi: Changed to CPU%i\n", GetCurrentProcessorNumber());
#elif defined(POSIX)
	cpu_set_t cpuSet;
	CPU_ZERO(&cpuSet);

	for (int i = 0; i < 32; i++)
		if (MTAffinityMask & (1 << i))
			CPU_SET(i, &cpuSet);

	if (sched_setaffinity(getpid(), sizeof(cpuSet), &cpuSet))
		ConColorMsg(Color(255, 0, 0, 255), "Turbostroi: SetMTAffinityMask failed on train thread! (0x%04X)\n", errno);
	else
		ConColorMsg(Color(0, 255, 0, 255), "Turbostroi: Changed to CPU%i\n", sched_getcpu());
#endif

	return 0;
}

LUA_FUNCTION(API_SetSTAffinityMask)
{
	LUA->CheckType(1, Type::Number);
	SimThreadAffinityMask = LUA->GetNumber(1);
	ConColorMsg(Color(0, 255, 0, 255), "Turbostroi: Assign Train Threads affinity to %i\n", SimThreadAffinityMask);
	return 0;
}

LUA_FUNCTION(API_StartRailNetwork)
{
	return 0;
}

LUA_FUNCTION(Think_handler) {
	TargetTime = Plat_FloatTime();
	shared_message msg;

	if (printMessages.pop(msg))
	{
		ConColorMsg(Color(255, 0, 255, 255), msg.message);
	}

	return 0;
}


/////////////////////////////////////////
//LUAJIT FUNCS
/////////////////////////////////////////

int InitInterfaces()
{
	p_ICvar = ICvar_Loader.GetInterface<ICvar>(CVAR_INTERFACE_VERSION);
	if (p_ICvar == nullptr)
	{
		ConColorMsg(Color(255, 0, 0, 255), "Metrostroi: DLL failed to initialize. ICvar interface not initialized.\n");
		return 2;
	}

	const ConCommand* pCommand = new ConCommand("turbostroi_clear_cache", ClearLoadCache, "Clear cache for reload systems", FCVAR_NOTIFY);
	p_ICvar->RegisterConCommand(const_cast<ConCommand*>(pCommand));

	return 0;
}

void InstallHooks(ILuaBase* LUA) {
	ConColorMsg(Color(0, 255, 0, 255), "Turbostroi: Installing hooks!\n");
	LUA->PushSpecial(GarrysMod::Lua::SPECIAL_GLOB);
	LUA->GetField(-1, "hook");
	LUA->GetField(-1, "Add");
	LUA->PushString("Think");
	LUA->PushString("Turbostroi_TargetTimeSync");
	LUA->PushCFunction(Think_handler);
	LUA->Call(3, 0);
	LUA->Pop(2);
}

void load(GarrysMod::Lua::ILuaBase* LUA, lua_State* L, const char* filename, const char* path, const char* variable, const char* defpath, bool json) {
	//Load up "sv_turbostroi.lua" in the new JIT environment
	const char* file_data = NULL;
	auto cache_item = LoadedFilesCache.find(filename);
	if (cache_item != LoadedFilesCache.end()) {
		file_data = cache_item->second.c_str();
	}
	else {
		LUA->PushSpecial(GarrysMod::Lua::SPECIAL_GLOB);
		LUA->GetField(-1, "file");
		LUA->GetField(-1, "Read");
		LUA->PushString(filename);
		LUA->PushString(path);
		LUA->Call(2, 1); //file.Read(...)
		if (LUA->GetString(-1)) {
			file_data = LUA->GetString(-1);
			LoadedFilesCache.emplace(filename, file_data);
		}
		LUA->Pop(); //returned result
		LUA->Pop(); //file
		LUA->Pop(); //GLOB
	}

	if (file_data) {
		if (!variable) {
			if (luaL_loadbuffer(L, file_data, strlen(file_data), filename) || lua_pcall(L, 0, LUA_MULTRET, 0)) {
				ConColorMsg(Color(255, 0, 127, 255), lua_tostring(L, -1));
				lua_pop(L, 1);
			}
		}
		else {
			if (!json) {
				lua_pushstring(L, file_data);
				lua_setglobal(L, variable);
			}
			else {
				LUA->PushSpecial(GarrysMod::Lua::SPECIAL_GLOB);
				LUA->GetField(-1, "table");
				LUA->GetField(-1, "ToString");
				LUA->Remove(-2);
				LUA->GetField(-2, "table");
				LUA->GetField(-1, "Sanitise");
				LUA->Remove(-2);
				LUA->GetField(-3, "util");
				LUA->GetField(-1, "JSONToTable");
				LUA->Remove(-2);
				LUA->PushString(file_data);
				LUA->Call(1, 1);
				LUA->Call(1, 1);
				LUA->PushString(variable);
				LUA->PushBool(false);
				LUA->Call(3, 1);

				if (luaL_loadbuffer(L, LUA->GetString(-1), strlen(LUA->GetString(-1)), filename) || lua_pcall(L, 0, LUA_MULTRET, 0)) {
					ConColorMsg(Color(255, 0, 127, 255), lua_tostring(L, -1));
					lua_pop(L, 1);
				}

				LUA->Pop(); //returned result
				LUA->Pop(); //GLOB
			}
		}
	}
	else {
		if (!defpath)
			ConColorMsg(Color(255, 0, 127, 255), "Turbostroi: File not found! ('%s' in '%s' path)\n", filename, path);
		else
		{
			ConColorMsg(Color(255, 0, 127, 255), "Turbostroi: File not found! ('%s' in '%s' path). Trying to load default file in '%s' path\n", filename, path, defpath);
			load(LUA, L, filename, defpath, variable, NULL, json);
		}
	}
}

int shared_print(lua_State* L) {
	int n = lua_gettop(L);
	char buffer[512];
	char* buf = buffer;
	buffer[0] = 0;

	lua_getglobal(L, "tostring");
	for (int i = 1; i <= n; i++) {
		const char* str;
		lua_pushvalue(L, -1);
		lua_pushvalue(L, i);
		lua_call(L, 1, 1);
		str = lua_tostring(L, -1);

		if (strlen(str) + strlen(buffer) < 512) {
			strcpy(buf, str);
			buf = buf + strlen(buf);
			buf[0] = '\t';
			buf = buf + 1;
			buf[0] = 0;
		}
		else if (i == 1 && buffer[0] == 0) {
			strcpy(buf, "[!] Message length limit reached!");
			buf = buf + strlen(buf);
			buf[0] = '\t';
			buf = buf + 1;
			buf[0] = 0;
		}

		lua_pop(L, 1);
	}

	buffer[strlen(buffer) - 1] = '\n';

	shared_message msg;
	char tempbuffer[512] = { 0 };
	strncat(tempbuffer, buffer, (512 - 1) - strlen(buffer));
	strcpy(msg.message, tempbuffer);

	printMessages.push(msg);

	return 0;
}

int ThreadRecvMessages(lua_State* state) {
	lua_getglobal(state, "_userdata");
	thread_userdata* userdata = (thread_userdata*)lua_touserdata(state, -1);
	lua_pop(state, 1);

	if (userdata) {
		size_t total = userdata->sim_to_thread.read_available();
		lua_createtable(state, total, 0);
		for (size_t i = 0; i < total; ++i) {
			userdata->sim_to_thread.consume_one([&](thread_msg tmsg) {
				lua_createtable(state, 0, 5);
				lua_pushnumber(state, tmsg.message);     lua_rawseti(state, -2, 1);
				lua_pushstring(state, tmsg.system_name); lua_rawseti(state, -2, 2);
				lua_pushstring(state, tmsg.name);        lua_rawseti(state, -2, 3);
				lua_pushnumber(state, tmsg.index);       lua_rawseti(state, -2, 4);
				lua_pushnumber(state, tmsg.value);       lua_rawseti(state, -2, 5);
				lua_rawseti(state, -2, i);
			});
		}
		return 1;
	}
	return 0;
}

void ThreadSimulation(thread_userdata* userdata) {
	if (!userdata)
		return;

	if (SimThreadAffinityMask)
	{

#if defined(_WIN32)
		if (!SetThreadAffinityMask(GetCurrentThread(), static_cast<DWORD_PTR>(SimThreadAffinityMask))) {
			ConColorMsg(Color(255, 0, 0, 255), "Turbostroi: SetSTAffinityMask failed on train thread!\n");
		}
#elif defined(POSIX)
		cpu_set_t cpuSet;
		CPU_ZERO(&cpuSet);

		for (int i = 0; i < 32; i++)
			if (SimThreadAffinityMask & (1 << i))
				CPU_SET(i, &cpuSet);

		if (sched_setaffinity(getpid(), sizeof(cpuSet), &cpuSet))
			ConColorMsg(Color(255, 0, 255, 255), "Turbostroi: SetSTAffinityMask failed on train thread! (0x%04X)\n", errno);
#endif
	}

	lua_State* L = userdata->L;

	while (userdata && !userdata->finished) {
		lua_settop(L, 0);

		lua_pushnumber(L, TargetTime);
		lua_setglobal(L, "CurrentTime");

		lua_getglobal(L, "Think");
		if (userdata->current_time < TargetTime)
		{
			//Simulate one step
			userdata->current_time = TargetTime;
			lua_pushboolean(L, false);
		}
		else
			//Execute function Think(skipped)
			lua_pushboolean(L, true);

		if (lua_pcall(L, 1, 0, 0)) {
			std::string err = lua_tostring(L, -1);
			err += "\n";
			shared_message msg;
			strcpy(msg.message, err.c_str());
			printMessages.push(msg);
		}

		boost::this_thread::sleep_for(boost::chrono::milliseconds(ThreadTickrate));
	}

	//Release resources
	ConColorMsg(Color(255, 0, 255, 255), "[!] Terminating train thread\n");
	lua_close(L);
	delete userdata; //check this
}

extern "C" TURBOSTROI_EXPORT bool ThreadSendMessage(void* p, int message, const char* system_name, const char* name, double index, double value) {
	bool successful = false;

	thread_userdata* userdata = (thread_userdata*)p;

	if (userdata) {
		thread_msg tmsg;
		tmsg.message = message;
		strncpy(tmsg.system_name, system_name, 63);
		tmsg.system_name[63] = 0;
		strncpy(tmsg.name, name, 63);
		tmsg.name[63] = 0;
		tmsg.index = index;
		tmsg.value = value;
		if (userdata->thread_to_sim.push(tmsg)) {
			successful = true;
		}
	}
	return successful;
}
extern "C" TURBOSTROI_EXPORT int ThreadReadAvailable(void* p) {
	thread_userdata* userdata = (thread_userdata*)p;
	return userdata->sim_to_thread.read_available();
}

extern "C" TURBOSTROI_EXPORT thread_msg ThreadRecvMessage(void* p) {
	thread_userdata* userdata = (thread_userdata*)p;
	thread_msg tmsg;

	if (userdata) {
		userdata->sim_to_thread.pop(tmsg);
	}

	return tmsg;
}

void ClearLoadCache(const CCommand& command) {
	if (!strcmp(command.Arg(1), "1"))
	{
		ConMsg("Files in cache:\n");
		if (LoadedFilesCache.size() == 0)
		{
			ConMsg("\tNo files in cache.\n");
		}
		else
		{
			for (const auto& [key, value] : LoadedFilesCache) {
				ConMsg("\t%s\n", key.c_str());
			}
		}
	}
	LoadedFilesCache.clear();
	ConColorMsg(Color(0, 255, 0, 255), "Turbostroi: Cache cleared!\n");
}
