#include "mob.h"

mob::mob()
{
}

void mob::TakeDamage(cfp damage)
{
	health -= damage;
}

void mob::Death()
{
	despawn = true;
}

void mob::Draw(GraphicsObject* graphics, mat4x4 transform, const vec3 camera, const vec3 direction)
{
	
}

void mob::Update()
{
}
void mob::RecalculateBodyPartTransforms()
{
	body.translate = position;
	body.changed = true;
}
hitbox mob::CalculateHitBox(vec3 pos) const
{
	hitbox h = hitbox();
	h.p000 = pos - hitboxmiddlepoint;
	h.p111 = h.p000 + hitboxsize;
	return h;
}
bool mob::CollideHitbox(const vec3 pos)
{
	hitbox h = CalculateHitBox(pos);
	return CollideHitbox(h);
}
bool mob::CollideHitbox(const hitbox h)
{
	int FromX = (int)floor(h.p000.x);
	int FromY = (int)floor(h.p000.y);
	int FromZ = (int)floor(h.p000.z);
	int ToX = (int)floor(h.p111.x);
	int ToY = (int)floor(h.p111.y);
	int ToZ = (int)floor(h.p111.z);
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
		if (falling)
		{
			if (position.z > heighestfallpoint)
			{
				heighestfallpoint = position.z;
			}
		}
		vec3 newposition = position + speed;
		//check hitbox
		fp BounceMultiplier = 0.0;
		hitbox newh = CalculateHitBox(newposition);
		bool xyz = CollideHitbox(newh);
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
						hitbox cur = CalculateHitBox(position);
						fp zfeet = cur.p000.z;
						const fp diffz = position.z - zfeet;
						int fzfeet = floor(zfeet);
						zfeet = zfeet - fzfeet + math::fpepsilon < 0 ? fzfeet - 1 : fzfeet;
						position.z = zfeet + diffz + math::fpepsilon;
						newfalling = false;//hit ground
						//calculate fall damage
						const fp falldistance = heighestfallpoint - position.z;
						//https://minecraft.gamepedia.com/Damage#Fall_damage
						const fp falldamage = falldistance - 3;
						//take damage
						if (falldamage > 0) 
						{
							TakeDamage(falldamage);
						}
						//reset fall point
						heighestfallpoint = -INFINITY;
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
		hitbox h = CalculateHitBox(newposition);
		int BlockUnderZ = (int)floor(h.p000.z - 0.001f);
		int FromX = (int)floor(h.p000.x);
		int FromY = (int)floor(h.p000.y);
		int ToX = (int)floor(h.p111.x);
		int ToY = (int)floor(h.p111.y);
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
	//body.translate = position;
	//body.changed = true;
}
