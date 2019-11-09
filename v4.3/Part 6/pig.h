#include "mob.h"
#pragma once
//https://minecraft.gamepedia.com/Pig
struct pig : mob
{
	fp traveleddistance;
	bodypart* head;
	bodypart* leg00;
	bodypart* leg10;
	bodypart* leg01;
	bodypart* leg11;
	fp rotationspeed = 0;
	void Update();
	void Draw(GraphicsObject* graphics, mat4x4 transform, const vec3 camera, const vec3 direction);
	pig(vec3 pos);
};