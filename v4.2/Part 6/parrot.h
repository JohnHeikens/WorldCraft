#include "mob.h"
#pragma once
//https://minecraft.gamepedia.com/Pig
struct parrot : mob
{
	bodypart* wing0;
	bodypart* wing1;
	bodypart* head0;
	bodypart* head1;
	bodypart* mouth0;
	bodypart* mouth1;
	bodypart* tailwing;
	bodypart* headwing;
	//fp rotationpitchspeed = 0;
	fp rotationyawspeed = 0;
	fp time;
	void Update();
	void Draw(GraphicsObject* graphics, mat4x4 transform, const vec3 camera, const vec3 direction);
	parrot(vec3 pos);
	color c;
};