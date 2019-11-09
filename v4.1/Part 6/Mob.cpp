#include "mob.h"

mob::mob()
{
}

void mob::Draw(GraphicsObject* graphics, mat4x4 transform, const vec3 camera, const vec3 direction)
{
	
}

void mob::Update()
{
}
bool mob::CollideHitbox(vec3 pos)
{
	vec3 v000 = pos - hitboxmiddlepoint;
	vec3 v111 = v000 + hitboxsize;
	int FromX = (int)floor(v000.x);
	int FromY = (int)floor(v000.y);
	int ToX = (int)floor(v111.x);
	int ToY = (int)floor(v111.y);
	int FromZ = floor(v000.z);
	int ToZ = floor(v111.z);
	//check hitbox
	for (int pZ = FromZ; pZ <= ToZ; pZ++)
	{
		for (int pY = FromY; pY <= ToY; pY++)
		{
			for (int pX = FromX; pX <= ToX; pX++)
			{
				block block = GetBlock(pX, pY, pZ);
				if (IsSolidBlock[block])
					return true;
			}
		}
	}
	return false;
}

void mob::Physics() 
{
	if (gravity)
	{
		vec3 newposition = position + speed;
		//check hitbox
		fp BounceMultiplier = 0.0;
		bool xyz = CollideHitbox(newposition);
		bool xy = CollideHitbox(vec3(newposition.x, newposition.y, position.z));
		bool xz = CollideHitbox(vec3(newposition.x, position.y, newposition.z));
		bool yz = CollideHitbox(vec3(position.x, newposition.y, newposition.z));
		bool cx = CollideHitbox(vec3(newposition.x, position.y, position.z));
		bool cy = CollideHitbox(vec3(position.x, newposition.y, position.z));
		bool cz = CollideHitbox(vec3(position.x, position.y, newposition.z));
		bool newfalling = true;
		//CanJump = true;
		if (xyz)//at least one axis collided
		{
			if (!xy)//z collided
			{
				speed.x *= 0.7;
				speed.y *= 0.7;
				if (!xz || !yz) //corner
				{
					speed = vec3();
				}
				else
				{
					if (speed.z < 0)//margin 0
					{
						fp zfeet = position.z - hitboxmiddlepoint.z;
						int fzfeet = floor(zfeet);
						zfeet = zfeet - fzfeet + floatepsilon < 0 ? fzfeet - 1 : fzfeet;
						position.z = zfeet + hitboxmiddlepoint.z + floatepsilon;
						newfalling = false;//hit ground
						BounceMultiplier = GetBounceMultiplier[GetBlock(floor(position.x), floor(position.y), floor(zfeet - 1))];
						//if (BounceMultiplier > 0 && Pressed(JUMP_KEY))
						//{
							//BounceMultiplier = (BounceMultiplier - 1) / 2;//higher bounce
							//CanJump = false;
						//}
					}
					speed.z *= BounceMultiplier;
				}
			}
			else if (!xz)//y collided
			{
				if (!yz) //corner
				{
					speed = vec3();
				}
				else
				{
					speed.y *= BounceMultiplier;
				}
			}
			else if (!yz)//x collided
			{
				speed.x *= BounceMultiplier;
			}
			else if (!cx) //y and z collided
			{
				speed.y *= BounceMultiplier;
				speed.z *= BounceMultiplier;
			}
			else if (!cy) //x and z collided
			{
				speed.x *= BounceMultiplier;
				speed.z *= BounceMultiplier;
			}
			else if (!cz) //x and y collided
			{
				speed.x *= BounceMultiplier;
				speed.y *= BounceMultiplier;
			}
			else //x, y and z collided
			{
				speed *= BounceMultiplier;
			}
		}
		newposition = position + speed;
		int BlockUnderZ = floor(newposition.z - hitboxmiddlepoint.z - 0.001f);
		int FromX = (int)floor(newposition.x - hitboxmiddlepoint.x);
		int FromY = (int)floor(newposition.y - hitboxmiddlepoint.y);
		int ToX = (int)floor(newposition.x - hitboxmiddlepoint.x + hitboxsize.x);
		int ToY = (int)floor(newposition.y - hitboxmiddlepoint.y + hitboxsize.y);
		sinking = false;
		for (int pY = FromY; pY <= ToY; pY++)
		{
			for (int pX = FromX; pX <= ToX; pX++)
			{
				block block = GetBlock(pX, pY, BlockUnderZ);
				bool solid = IsSolidBlock[block];
				if (solid)
				{
					newfalling = false;
					sinking = false;
					goto BlockUnder;
				}
				else if (GetBlockType[block] == Fluid)
				{
					sinking = true;
				}
			}
		}
		if (newfalling && sneaking && !falling)
		{
			speed = vec3();//prevent from going off the edge
			newfalling = false;
		}
	BlockUnder:
		position += speed;
		if (newfalling) {
			speed.z -= GravityForcePerTick;//pow(gravitymult, 1.0f / FPS);
		}
		speed *= newfalling ? sinking ? FluidFriction : AirFriction : GroundFriction;
		falling = newfalling;
	}
	else {
		position += speed;
		speed *= AirFriction;
	}
	body.translate = position;
	body.changed = true;
}
