#include "parrot.h"
//color parrotcolor = color(210, 149, 20);
void parrot::Update()
{
	fp cosyaw = cos(body.rotations[1].angle);
	fp sinyaw = sin(body.rotations[1].angle);
	vec3 direction = vec3(cosyaw, sinyaw, 0);
	block b = GetBlock(position + direction);
	speed += 0.005 * direction;
	fp mult = b == Air ? 0.01 : 0.05;
	rotationyawspeed += (RANDFP - 0.5) * mult;
	rotationyawspeed *= 0.99;
	speed.z += (RANDFP - 0.5) * mult;
	//speed.z *= 0.99;
	body.rotations[1].angle += rotationyawspeed;//yaw

	//time += 2;
	//flipper rotation
	/*flipper0->rotations[0].angle = sin(time);
	flipper0->changed = true;
	flipper1->rotations[0].angle = -sin(time);
	flipper1->changed = true;
	backflipper0->rotations[0].angle = sin(time * 0.5);
	backflipper0->changed = true;
	backflipper1->rotations[0].angle = sin(time * 0.5);
	backflipper1->changed = true;
	*/

	if (this->falling && !this->sinking)
	{
		speed.z += GravityForcePerTick * AirFriction;
	}
}
void parrot::Draw(GraphicsObject* graphics, mat4x4 transform, const vec3 camera, const vec3 direction)
{
	body.Draw(graphics, transform, &c, camera, direction);
}


parrot::parrot(vec3 pos)
{//y+ = front
	gravity = true;
	position = pos;
	hitboxsize = vec3(0.5, 0.5, 0.9);
	hitboxmiddlepoint = hitboxsize * 0.5;
	const vec3 bodysize = vec3(0.1875, 0.1875, 0.3125);
	body = bodypart(NULL, pos, bodysize, bodysize * 0.5, { rotation(zup::right), rotation() });
	body.rotations[0].angle = M_PI * 0.2;//pitch
	const vec3 sidewingsize = vec3(0.0625, 0.1875, 0.3125);
	const vec3 sidewingjoint = vec3(0, sidewingsize.y * .5, sidewingsize.z);
	const vec3 sidewingoffset = vec3(bodysize.x * .5, 0, bodysize.z * 0.4);
	wing0 = new bodypart(&body, vec3(-sidewingoffset.x,sidewingoffset.y,sidewingoffset.z), sidewingsize, vec3(sidewingsize.x - sidewingjoint.x,sidewingjoint.y,sidewingjoint.z), { rotation(zup::left,M_PI * 0.1),rotation(zup::up) });
	wing1 = new bodypart(&body, vec3(sidewingoffset.x, sidewingoffset.y, sidewingoffset.z), sidewingsize, vec3(sidewingjoint.x, sidewingjoint.y, sidewingjoint.z), { rotation(zup::left,M_PI * 0.1),rotation(zup::up) });
	const vec3 head0size = vec3(0.125, 0.125, 0.1875);
	const vec3 head0joint = vec3(head0size.x * .5, head0size.y * .5, 0);
	const vec3 head0offset = vec3(0, 0, bodysize.y * .5);
	head0 = new bodypart(&body, head0offset, head0size, head0joint, { rotation(zup::right,M_PI * -.2) });
	body.childs = std::vector<bodypart*>({ wing0, wing1,head0 });
	c = color::FromBrightness(0xff + (rand() & 0xff));
}