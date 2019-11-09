#include "mob.h"
#pragma once
constexpr fp humanwidth = 0.6f; // player is 0.6 blocks wide
constexpr fp humanpixelsize = 1.8 /(8 + 12 + 12);
//https://minecraft.gamepedia.com/Player
//https://www.katsbits.com/smforum/index.php?topic=629.0
struct human : mob
{
	bodypart* head;
	bodypart* arm0;
	bodypart* arm1;
	bodypart* leg0;
	bodypart* leg1;
	bool visible = true;
	void Update();
	void Draw(GraphicsObject* graphics, mat4x4 transform, const vec3 camera, const vec3 direction);
	human(vec3 pos);
	fp traveleddistance;
};