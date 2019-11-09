#include "include.h"
#include "bodypart.h"
#include "Chunk.h"

constexpr seconds NormalTickTime = 0.05;//0.05s
//constexpr miliseconds NormalTickTimeMs = 50;//50ms

constexpr fp
AirFriction = .98f,
AirFrictionMultiplier = (1 - AirFriction),
GroundFriction = .95f,
GroundFrictionMultiplier = (1 - GroundFriction),
FluidFriction = .9f,
FluidFrictionMultiplier = (1 - FluidFriction);
//const fp GravityForce30Fps = 9.8f / 465;//9.8m/s2 with 30 fps, 1 + 2 + 3 + ... 30 = 465
constexpr fp GravityForcePerTick = 0.08;//https://gaming.stackexchange.com/questions/178726/what-is-the-terminal-velocity-of-a-sheep
//constexpr fp SwimUpspeed = GravityForcePerTick + (0.5f * NormalTickTime * (1 - FluidFriction));//0.2


constexpr int pixelspermeter = 0x10;
constexpr fp pixelscale = 1.0 / pixelspermeter;

#pragma once
struct mob
{
	mob();
	virtual void Draw(GraphicsObject* graphics, mat4x4 transform, const vec3 camera, const vec3 direction);
	virtual void Update();
	//physics
	bool CollideHitbox(vec3 pos);
	void Physics();

	vec3 position, speed = vec3(), hitboxsize, hitboxmiddlepoint;
	bool falling = true, sinking = false, sneaking = false;
	//wether gravity is applied in physics.
	bool gravity;
	bodypart body;
	
};