#include "human.h"
color humancolor = color(0xff, 0x80, 0x80);

void human::Draw(GraphicsObject* graphics, mat4x4 transform, const vec3 camera, const vec3 direction)
{
	if (visible) {
		head->Draw(graphics, transform, &humancolor, camera, direction);
	}
}


human::human(vec3 pos)
{
	health = maxhumanhealth;
	gravity = false;
	position = pos;
	const vec3 headsize = vec3(8) * humanpixelsize;
	head = DBG_NEW bodypart(NULL, pos, headsize, vec3(headsize.x * .5, headsize.y * .5, 0), { rotation(zup::left),rotation(zup::up) });
	const vec3 bodysize = vec3(8, 4, 12) * humanpixelsize;
	body = bodypart(head, vec3(), bodysize, vec3(bodysize.x * 0.5,bodysize.y * 0.5,bodysize.z), { rotation(zup::left),rotation() });
	head->childs = {&body};
	const vec3 legsize = vec3(4, 4, 12) * humanpixelsize;
	const vec3 armsize = vec3(4, 4, 12) * humanpixelsize;
	const vec3 legjoint = vec3(legsize.x * .5, legsize.y * .5, legsize.z);//mid above the leg.
	const vec3 armjoint = vec3(armsize.x * .5, armsize.y * .5, armsize.x * .5);//in the midst of a imaginary square on top of the arm.
	const vec3 legoffset = vec3((bodysize.x - legsize.x) * 0.5, (bodysize.y - legsize.y) * 0.5, -bodysize.z);
	const vec3 armoffset = vec3((bodysize.x + armsize.x) * 0.5, (bodysize.y - armsize.y) * 0.5, (- armsize.z) * 0.5);
	arm0 = DBG_NEW bodypart(&body, vec3(-armoffset.x, armoffset.y, armoffset.z), armsize, armjoint, { rotation(zup::left) });
	arm1 = DBG_NEW bodypart(&body, vec3(armoffset.x, armoffset.y, armoffset.z), armsize, armjoint, { rotation(zup::right) });
	leg0 = DBG_NEW bodypart(&body, vec3(-legoffset.x, legoffset.y, legoffset.z), legsize, legjoint, { rotation(zup::left) });
	leg1 = DBG_NEW bodypart(&body, vec3(legoffset.x, legoffset.y, legoffset.z), legsize, legjoint, { rotation(zup::right) });//covered
	body.childs = std::vector<bodypart*>({ arm0,arm1,leg0,leg1 });
	hitboxsize = vec3(humanwidth, humanwidth, 1.8);//1.8 heigh
	hitboxmiddlepoint = vec3(humanwidth * 0.5, humanwidth * 0.5, legsize.z + body.rotationcentre.z);
}

hitbox human::CalculateHitBox(vec3 pos) const
{
	hitbox h = hitbox();
	//calculate head centered
	//middle point of hitbox is always right below the head
	//hitbox width = 0.6 so 0.3 - headpos, 0.3 + headpos
	//calculate headpos
	h.p000.x = pos.x - humanwidth * 0.5;
	h.p000.y = pos.y - humanwidth * 0.5;
	h.p111.x = pos.x + humanwidth * 0.5;
	h.p111.y = pos.y + humanwidth * 0.5;
	h.p000.z = pos.z;
	h.p111.z = pos.z + currentheight;
	return h;
}

