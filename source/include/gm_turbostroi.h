#pragma once

#if defined(_MSC_VER)
#define TURBOSTROI_EXPORT __declspec(dllexport)
#define TURBOSTROI_IMPORT __declspec(dllimport)
#include <Windows.h>
#elif defined(__GNUC__)
//  GCC
#define TURBOSTROI_EXPORT __attribute__((visibility("default")))
#define TURBOSTROI_IMPORT
#else
//  do nothing and hope for the best?
#define TURBOSTROI_EXPORT
#define TURBOSTROI_IMPORT
#pragma warning Unknown dynamic link import/export semantics.
#endif

//STL
#include <vector>
#include <string>
#include <string_view>

//Boost libs
#include <boost/thread.hpp>
#include <boost/chrono.hpp>
#include <boost/atomic.hpp>
#include <boost/lockfree/queue.hpp>
#include <boost/lockfree/spsc_queue.hpp>
#include <boost/unordered_map.hpp>

//Gmod interface
#include <lua.hpp>
#include <GarrysMod/FactoryLoader.hpp>
#include <GarrysMod/Lua/Interface.h>
#include <convar.h>
#include <Color.h>

#define PushCFunc(_function,_name) LUA->PushCFunction(_function); LUA->SetField(-2, _name);

//struct LocalSystem
//{
//	LocalSystem(std::string sysLocalName, TrainSystem *sysBase)
//	{
//		LocalName = sysLocalName;
//		BaseSystem = sysBase;
//	}
//
//	std::string LocalName;
//	TrainSystem *BaseSystem;
//};

struct TrainSystem {
	TrainSystem(std::string sysName, std::string sysFileName)
	{
		BaseName = sysName;
		FileName = sysFileName;
	}

	std::string BaseName;

	// Can be local name of loaded system
	std::string FileName;
};

struct shared_message {
	char message[512];
};

typedef struct {
	int message;
	char system_name[64];
	char name[64];
	double index;
	double value;
} thread_msg;

struct thread_userdata {
	lua_State* L = NULL;
	double current_time = 0.0;
	int finished = 0;

	boost::lockfree::spsc_queue<thread_msg> thread_to_sim, sim_to_thread;
	
	thread_userdata() : thread_to_sim(1024), sim_to_thread(1024) //256
	{
	}
};