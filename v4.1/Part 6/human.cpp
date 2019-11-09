#include "human.h"
color humancolor = color(0xff, 0x80, 0x80);

void human::Draw(GraphicsObject* graphics, mat4x4 transform, const vec3 camera, const vec3 direction)
{
	if (visible) {
		body.Draw(graphics, transform, &humancolor, camera, direction);
	}
}


human::human(vec3 pos)
{
	gravity = false;
	position = pos;
	const vec3 bodysize = vec3(8, 4, 12) * humanpixelsize;
	body = bodypart(NULL, pos, bodysize, bodysize * 0.5, {rotation()});
	const vec3 headsize = vec3(8) * humanpixelsize;
	head = new bodypart(&body, vec3(0, 0, (bodysize.z + headsize.z)*.5), headsize, headsize * .5, { rotation(zup::left),rotation(zup::up) });
	const vec3 legsize = vec3(4, 4, 12) * humanpixelsize;
	const vec3 armsize = vec3(4, 4, 12) * humanpixelsize;
	const vec3 legjoint = vec3(legsize.x * .5, legsize.y * .5, legsize.z);//mid above the leg.
	const vec3 armjoint = vec3(armsize.x * .5, armsize.y * .5, armsize.x * .5);//in the midst of a imaginary square on top of the arm.
	const vec3 legoffset = vec3((bodysize.x - legsize.x) * 0.5, (bodysize.y - legsize.y) * 0.5, -(bodysize.z * 0.5));
	const vec3 armoffset = vec3((bodysize.x + armsize.x) * 0.5, (bodysize.y - armsize.y) * 0.5, (bodysize.z - armsize.z) * 0.5);
	arm0 = new bodypart(&body, vec3(-armoffset.x, armoffset.y, armoffset.z), armsize, armjoint, { rotation(zup::left) });
	arm1 = new bodypart(&body, vec3(armoffset.x, armoffset.y, armoffset.z), armsize, armjoint, { rotation(zup::right) });
	leg0 = new bodypart(&body, vec3(-legoffset.x, legoffset.y, legoffset.z), legsize, legjoint, { rotation(zup::left) });
	leg1 = new bodypart(&body, vec3(legoffset.x, legoffset.y, legoffset.z), legsize, legjoint, { rotation(zup::right) });//covered
	body.childs = std::vector<bodypart*>({ head, arm0,arm1,leg0,leg1 });
	hitboxsize = vec3(humanwidth, humanwidth, 1.8);
	hitboxmiddlepoint = vec3(humanwidth * 0.5, humanwidth * 0.5, legsize.z + body.rotationcentre.z);
}
