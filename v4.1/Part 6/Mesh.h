#pragma once
#include "MeshConstants.h"

class Mesh {
public:
	Mesh();
	~Mesh();
	void Draw(int PlaneCount);
	void SetMesh(float* vertices, int Count);
	bool enabled;
private:
	uint VAO, VerticePointer;
};
void SolidParameters();
void FluidParameters();
void GenVerticeBuffers(uint* VerticeArray, uint* VAO);