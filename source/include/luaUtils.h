#pragma once
#include <GarrysMod/Lua/Interface.h>
#include "lua.hpp"
#include <Color.h>

//------------------------------------------------------------------------------
// Lua Utils
//------------------------------------------------------------------------------

void printType(int luaType, GarrysMod::Lua::ILuaBase* LUA, int stackPos = 1)
{
	switch (luaType)
	{
	case GarrysMod::Lua::Type::None:
		ConMsg("None");
		break;
	case GarrysMod::Lua::Type::Nil:
		ConMsg("Nil");
		break;
	case GarrysMod::Lua::Type::Bool:
		ConMsg("Bool,%s", LUA->GetBool(stackPos) ? "true" : "false");
		break;
	case GarrysMod::Lua::Type::LightUserData:
		ConMsg("LightUserData");
		break;
	case GarrysMod::Lua::Type::Number:
		ConMsg("Number,%lf", LUA->GetNumber(stackPos));
		break;
	case GarrysMod::Lua::Type::String:
		ConMsg("String,%s", LUA->GetString(stackPos));
		break;
	case GarrysMod::Lua::Type::Table:
		ConMsg("Table");
		break;
	case GarrysMod::Lua::Type::Function:
		ConMsg("Function");
		break;
	case GarrysMod::Lua::Type::UserData:
		ConMsg("Userdata");
		break;
	case GarrysMod::Lua::Type::Thread:
		ConMsg("Thread");
		break;
	case GarrysMod::Lua::Type::Entity:
		ConMsg("Entity");
		break;
	case GarrysMod::Lua::Type::Vector:
		ConMsg("Vector");
		break;
	case GarrysMod::Lua::Type::Angle:
		ConMsg("Angle");
		break;
	case GarrysMod::Lua::Type::PhysObj:
		ConMsg("PhysObj");
		break;
	case GarrysMod::Lua::Type::Save:
		ConMsg("Save");
		break;
	case GarrysMod::Lua::Type::Restore:
		ConMsg("Restore");
		break;
	case GarrysMod::Lua::Type::DamageInfo:
		ConMsg("DamageInfo");
		break;
	case GarrysMod::Lua::Type::EffectData:
		ConMsg("EffectData");
		break;
	case GarrysMod::Lua::Type::MoveData:
		ConMsg("MoveData");
		break;
	case GarrysMod::Lua::Type::RecipientFilter:
		ConMsg("RecipientFilter");
		break;
	case GarrysMod::Lua::Type::UserCmd:
		ConMsg("UserCmd");
		break;
	case GarrysMod::Lua::Type::ScriptedVehicle:
		ConMsg("ScriptedVehicle");
		break;
	case GarrysMod::Lua::Type::Material:
		ConMsg("Material");
		break;
	case GarrysMod::Lua::Type::Panel:
		ConMsg("Panel");
		break;
	case GarrysMod::Lua::Type::Particle:
		ConMsg("Particle");
		break;
	case GarrysMod::Lua::Type::ParticleEmitter:
		ConMsg("ParticleEmitter");
		break;
	case GarrysMod::Lua::Type::Texture:
		ConMsg("Texture");
		break;
	case GarrysMod::Lua::Type::UserMsg:
		ConMsg("UserMsg");
		break;
	case GarrysMod::Lua::Type::ConVar:
		ConMsg("ConVar");
		break;
	case GarrysMod::Lua::Type::IMesh:
		ConMsg("IMesh");
		break;
	case GarrysMod::Lua::Type::Matrix:
		ConMsg("Matrix");
		break;
	case GarrysMod::Lua::Type::Sound:
		ConMsg("Sound");
		break;
	case GarrysMod::Lua::Type::PixelVisHandle:
		ConMsg("PixelVisHandle");
		break;
	case GarrysMod::Lua::Type::DLight:
		ConMsg("DLight");
		break;
	case GarrysMod::Lua::Type::Video:
		ConMsg("Video");
		break;
	case GarrysMod::Lua::Type::File:
		ConMsg("File");
		break;
	case GarrysMod::Lua::Type::Locomotion:
		ConMsg("Locomotion");
		break;
	case GarrysMod::Lua::Type::Path:
		ConMsg("Path");
		break;
	case GarrysMod::Lua::Type::NavArea:
		ConMsg("NavArea");
		break;
	case GarrysMod::Lua::Type::SoundHandle:
		ConMsg("SoundHandle");
		break;
	case GarrysMod::Lua::Type::NavLadder:
		ConMsg("NavLadder");
		break;
	case GarrysMod::Lua::Type::ParticleSystem:
		ConMsg("ParticleSystem");
		break;
	case GarrysMod::Lua::Type::ProjectedTexture:
		ConMsg("ProjectedTexture");
		break;
	case GarrysMod::Lua::Type::PhysCollide:
		ConMsg("PhysCollide");
		break;
	case GarrysMod::Lua::Type::SurfaceInfo:
		ConMsg("SurfaceInfo");
		break;
	case GarrysMod::Lua::Type::Type_Count:
		ConMsg("Type_Count");
		break;
	default:
		ConMsg("%d", luaType);
		break;
	}
}

void printStack(GarrysMod::Lua::ILuaBase* LUA, int iStackFirst = 1, int iStackLast = 20)
{
	ConMsg("\nPrint stack from %d to %d:\n", iStackFirst, iStackLast);

	if (iStackFirst > iStackLast)
	{
		for (int i = iStackFirst; i <= iStackLast; i++)
		{
			ConMsg("%d = ", i);
			auto luaType = LUA->GetType(i);

			printType(luaType, LUA, i);

			ConMsg(",0x%p\n", LUA->GetPointer(i));
		}
	}
	else
	{
		for (int i = iStackLast; i >= iStackFirst; i--)
		{
			ConMsg("%d = ", i);
			auto luaType = LUA->GetType(i);

			printType(luaType, LUA, i);

			ConMsg(",0x%p\n", LUA->GetPointer(i));
		}
	}

}