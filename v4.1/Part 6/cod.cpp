#include "cod.h"
color codcolor = color(210, 149, 20);
void cod::Update()
{
	fp cospitch = cos(body.rotations[0].angle);
	fp sinpitch = sin(body.rotations[0].angle);
	fp cosyaw = cos(body.rotations[1].angle);
	fp sinyaw = sin(body.rotations[1].angle);
	vec3 direction = vec3(cospitch * cosyaw, cospitch * sinyaw, sinpitch);
	block b = GetBlock(position + direction);
	speed += 0.005 * direction;
	fp mult = b == Water ? 0.01 : 0.05;
	rotationpitchspeed += (RANDFP - 0.5) * mult;
	rotationpitchspeed *= 0.99;
	rotationyawspeed += (RANDFP - 0.5) * mult;
	rotationyawspeed *= 0.99;
	body.rotations[0].angle += rotationpitchspeed;//pitch
	body.rotations[0].angle *= 0.99;
	body.rotations[1].angle += rotationyawspeed;//yaw

	time += 2;
	//flipper rotation
	flipper0->rotations[0].angle = sin(time);
	flipper0->changed = true;
	flipper1->rotations[0].angle = -sin(time);
	flipper1->changed = true;
	backflipper0->rotations[0].angle = sin(time * 0.5);
	backflipper0->changed = true;
	backflipper1->rotations[0].angle = sin(time * 0.5);
	backflipper1->changed = true;


	if (this->sinking)
	{
		speed.z += GravityForcePerTick * FluidFriction;
	}
}
void cod::Draw(GraphicsObject* graphics, mat4x4 transform, const vec3 camera, const vec3 direction)
{
	body.Draw(graphics, transform, &codcolor, camera, direction);
}


cod::cod(vec3 pos)
{//y+ = front
	gravity = true;
	position = pos;
	hitboxsize = vec3(0.5, 0.5, 0.3);
	hitboxmiddlepoint = hitboxsize * 0.5;
	const vec3 bodysize = vec3(0.3, 0.5, 0.3);
	body = bodypart(NULL, pos, bodysize, bodysize * 0.5, { rotation(zup::right), rotation() });
	const vec3 sideflippersize = vec3(0.1, 0.05, 0.25);
	const vec3 sideflipperjoint = vec3(0, sideflippersize.y * .5, sideflippersize.z * 0.5);
	const vec3 sideflipperoffset = vec3(bodysize.x * .5, bodysize.y * 0.2, 0);
	flipper0 = new bodypart(&body, vec3(-sideflipperoffset.x, sideflipperoffset.y, sideflipperoffset.z), sideflippersize, vec3(sideflippersize.x, sideflipperjoint.y, sideflipperjoint.z), { rotation() });
	flipper1 = new bodypart(&body, vec3(sideflipperoffset.x, sideflipperoffset.y, sideflipperoffset.z), sideflippersize, vec3(0, sideflipperjoint.y, sideflipperjoint.z), { rotation() });
	const vec3 backflipper0size = vec3(0.05, 0.1, 0.1);
	const vec3 backflipper0joint = vec3(backflipper0size.x * 0.5, backflipper0size.y, backflipper0size.z * 0.5);
	const vec3 backflipper0offset = vec3(0, bodysize.y * -0.5, 0);
	backflipper0 = new bodypart(&body, backflipper0offset, backflipper0size, backflipper0joint, { rotation() });
	const vec3 backflipper1size = vec3(0.05, 0.1, 0.2);
	const vec3 backflipper1joint = vec3(backflipper1size.x * 0.5, backflipper1size.y, backflipper1size.z * 0.5);
	const vec3 backflipper1offset = vec3(0, (backflipper1size.y + backflipper0size.y) * -0.5,0);
	backflipper1 = new bodypart(&body, backflipper1offset, backflipper1size, backflipper1joint, { rotation() });

	body.childs = std::vector<bodypart*>({ flipper0, flipper1,backflipper0 });
	backflipper0->childs = std::vector<bodypart*>({ backflipper1 });
}
