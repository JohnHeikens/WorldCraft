#include "GlobalFunctions.h"
#pragma once

//https://minecraft.gamepedia.com/Gameplay#Game_modes
enum gamemode : byte
{
	survival, creative, adventure, spectator, gamemodecount
};
const std::wstring gamemodename[]
{
	L"survival",
	L"creative",
	L"adventure",
	L"spectator"
};
const bool hasgravity[gamemodecount]
{
	true,//survival
	false,//creative
	true,//adventure
	false,//spectator
};