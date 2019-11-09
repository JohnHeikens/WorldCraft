#include "modfunctions.h"
#include "Chunk.h"

void fsetblock::execute() const
{
	SetBlock(*(int*)arguments[0]->var, *(int*)arguments[1]->var, * (int*)arguments[2]->var, *(int*)arguments[3]->var, *(bool*)arguments[4]->var);
}

void fsetblockrange::execute() const
{
	SetBlockRange(*(int*)arguments[0]->var, *(int*)arguments[1]->var, *(int*)arguments[2]->var, *(int*)arguments[3]->var, *(int*)arguments[4]->var, *(int*)arguments[5]->var, *(int*)arguments[6]->var, *(bool *)arguments[7]->var);
}

void fsetsphere::execute() const
{
	Chunk::SetSphere(HasAir, *(int*)arguments[0]->var, *(int*)arguments[1]->var, *(int*)arguments[2]->var, *(double*)arguments[3]->var, *(int*)arguments[4]->var, *(bool*)arguments[5]->var);
}

void frand::execute() const
{
	*(int*)result->var = rand();
}

void frandfp::execute() const
{
	*(double*)result->var = RANDFP;
}

void fgetblock::execute() const
{
	*(int*)result->var = GetBlock(*(int*)arguments[0]->var, *(int*)arguments[1]->var, *(int*)arguments[2]->var);
}
