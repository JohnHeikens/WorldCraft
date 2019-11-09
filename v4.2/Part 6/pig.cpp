#include "pig.h"
color pigcolor = color(0xff, 0x80, 0x80);
void pig::Update()
{
	//grass block beneath
	vec3 blockpos = vec3(position.x, position.y, position.z - hitboxmiddlepoint.z - pixelscale);
	if (GetBlock(blockpos) == grass)
	{
		speed *= 0.8;
		if (rand() % 200 == 0) //once in 200 ticks(10 seconds)
		{
			//eat grass block
			SetBlock(blockpos, dirt, true);
		}
	}
	else 
	{
		fp cosa = cos(body.rotations[0].angle);
		fp sina = sin(body.rotations[0].angle);
		vec3 direction = vec3(-sina, cosa, 0);
		block b = GetBlock(position + direction);
		fp mult = b == air ? 0.01 : 0.05;
		speed += 0.005 * direction;
		rotationspeed += (RANDFP - 0.5) * mult;
		rotationspeed *= 0.99;
		body.rotations[0].angle += rotationspeed;
	}
	//rotate feet
	if (gravity && !falling) {
		fp l = this->speed.length();
		/*if (backward) {
			traveleddistance -= l;
		}*/
		//if (forward) {
			traveleddistance += l;
		//}
		const fp swingspermeter = 5;
		const fp swingradians = 0.5;
		leg00->rotations[0].angle = leg10->rotations[0].angle=leg01->rotations[0].angle = leg11->rotations[0].angle = sin(traveleddistance * swingspermeter) * swingradians;
		leg00->changed = leg10->changed = leg01->changed = leg11->changed = true;
	}
}
void pig::Draw(GraphicsObject* graphics, mat4x4 transform, const vec3 camera, const vec3 direction)
{
	body.Draw(graphics, transform, &pigcolor, camera, direction);
}


pig::pig(vec3 pos)
{
	gravity = true;
	position = pos;
	hitboxsize = vec3(0.9);
	const vec3 bodysize = vec3(0.625, 1, 0.5);
	body = bodypart(NULL,pos,bodysize,bodysize * 0.5, { rotation() });
	const vec3 headsize = vec3(0.5);
	head = DBG_NEW bodypart(&body, vec3(0, bodysize.y * 0.5 + headsize.z * 0.2, headsize.z * 0.2), headsize, headsize * .5, { rotation( zup::left) });
	const vec3 legsize = vec3(0.25, 0.25, 0.375);
	hitboxmiddlepoint = vec3(0, 0, bodysize.z * 0.5 + legsize.z);
	const vec3 legjoint = vec3(legsize.x * .5, legsize.y * .5, legsize.z);
	const fp offy = pixelscale;
	const vec3 legoffset = vec3((bodysize.x - legsize.x) * 0.5, (bodysize.y -legsize.y)* 0.5, -(bodysize.z * 0.5));
	leg00 = DBG_NEW bodypart(&body, vec3(-legoffset.x , -legoffset.y - offy,legoffset.z),legsize,legjoint, { rotation(zup::left) });
	leg10 = DBG_NEW bodypart(&body, vec3(legoffset.x , -legoffset.y - offy, legoffset.z), legsize, legjoint, { rotation(zup::right) });
	leg01 = DBG_NEW bodypart(&body, vec3(-legoffset.x , legoffset.y - offy, legoffset.z), legsize, legjoint, { rotation(zup::left) });
	leg11 = DBG_NEW bodypart(&body, vec3(legoffset.x , legoffset.y - offy, legoffset.z), legsize, legjoint, { rotation(zup::right) });//covered
	body.childs = std::vector<bodypart*>({head, leg00,leg10,leg01,leg11});
}
