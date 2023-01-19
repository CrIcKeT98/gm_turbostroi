#include <gm_turbostroi.h>
#include <Message_Exchange.h>

using namespace GarrysMod::Lua;

extern bool ThinkRunning;
extern bool ThinkStopped;
extern ICvar* p_ICvar;

GMOD_MODULE_OPEN()
{
	if (InitInterfaces())
		return 0;

	LUA->PushSpecial(GarrysMod::Lua::SPECIAL_GLOB);
	// Check metrostroi table
	LUA->GetField(-1, "Metrostroi");
	if (LUA->IsType(-1, Type::Nil)) {
		ConColorMsg(Color(255, 0, 0, 255), "Metrostroi: DLL failed to initialize. GM_Turbostroi can only be used on server\n");
		LUA->Pop(2);
		return 0;
	}
	LUA->Pop();

	InstallHooks(LUA);

	// Create functions table
	LUA->CreateTable();
	PushCFunc(API_InitializeTrain, "InitializeTrain");
	PushCFunc(API_DeinitializeTrain, "DeinitializeTrain");

	PushCFunc(API_RegisterSystem, "RegisterSystem");
	PushCFunc(API_LoadSystem, "LoadSystem");

	PushCFunc(API_SendMessage, "SendMessage");
	PushCFunc(API_RecvMessage, "RecvMessage");
	PushCFunc(API_RecvMessages, "RecvMessages");
	PushCFunc(API_ReadAvailable, "ReadAvailable");

	PushCFunc(API_SetSimulationFPS, "SetSimulationFPS");
	PushCFunc(API_SetMTAffinityMask, "SetMTAffinityMask");
	PushCFunc(API_SetSTAffinityMask, "SetSTAffinityMask");

	PushCFunc(API_StartRailNetwork, "StartRailNetwork");
	LUA->SetField(-2, "Turbostroi");
	LUA->Pop();

	ConMsg("Metrostroi: DLL initialized (built " __DATE__ ")\n");
	ConMsg("Metrostroi: Running with %i cores\n", boost::thread::hardware_concurrency());

	return 0;
}

GMOD_MODULE_CLOSE()
{
	ThinkRunning = false;
	while (!ThinkStopped)
		boost::this_thread::sleep_for(boost::chrono::milliseconds(2));

	if (p_ICvar != nullptr)
	{
		const ConCommand* pCommand = p_ICvar->FindCommand("turbostroi_clear_cache");
		if (pCommand) {
			p_ICvar->UnregisterConCommand(const_cast<ConCommand*>(pCommand));
		}
	}

	LUA->PushSpecial(GarrysMod::Lua::SPECIAL_GLOB);
	LUA->GetField(-1, "Metrostroi");
	if (LUA->IsType(-1, Type::Table)) {
		LUA->CreateTable();
		LUA->SetField(-2, "TurbostroiRegistered");
	}
	LUA->Pop();
	LUA->Pop();

	LUA->PushNil();
	LUA->SetField(GarrysMod::Lua::INDEX_GLOBAL, "Turbostroi");
	return 0;
}