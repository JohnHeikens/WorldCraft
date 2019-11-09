#include "mob.h"
#pragma once
constexpr fp humanwidth = 0.6f; // player is 0.6 blocks wide
constexpr fp humanpixelsize = 1.8 /(8 + 12 + 12);
constexpr fp humanlegheight = 12 * humanpixelsize;
constexpr fp humanstandheight = 1.8, sneakheight = 1.5;//1.65 on bedrock, but from javas perspective
constexpr fp maxhumanhealth = 20;
//https://minecraft.gamepedia.com/Player
//https://www.katsbits.com/smforum/index.php?topic=629.0
struct human : public mob
{
	bodypart* head;
	bodypart* arm0;
	bodypart* arm1;
	bodypart* leg0;
	bodypart* leg1;
	fp currentheight = humanstandheight;

	fp Pitch = 0, Yaw = 0;
	fp traveleddistance;
	vec3* lastspawnpoint;

	bool visible = true;
	void Update();
	void TakeDamage(cfp damage);
	virtual hitbox CalculateHitBox(vec3 pos) const override;
	void RecalculateBodyPartTransforms() override;
	void Draw(GraphicsObject* graphics, mat4x4 transform, const vec3 camera, const vec3 direction);
	human(vec3 pos);
	void Death();
};