#include "mob.h"
#pragma once
//https://minecraft.gamepedia.com/Pig
struct cod : mob
{
	bodypart* flipper0;
	bodypart* flipper1;
	bodypart* backflipper0;
	bodypart* backflipper1;
	fp rotationpitchspeed = 0;
	fp rotationyawspeed = 0;
	fp time;
	void Update();
	void Draw(GraphicsObject* graphics, mat4x4 transform, const vec3 camera, const vec3 direction);
	cod(vec3 pos);
};