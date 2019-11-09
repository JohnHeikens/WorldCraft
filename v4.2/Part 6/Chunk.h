//typedef unsigned short ushort;
#include "LightMap.h"
#include "Texture.h"
//#include "../Noise/PerlinNoise.h"
//#include "../Noise/FastNoiseSIMD/FastNoiseSIMD.h"
//#include "../Noise/PerlinNoise.h"
#include "LayerNoiseSimplex.h"
//#include "../Noise/RealNoise/LayerNoise.h"
#include "WorldConstants.h"
#include "ActionContainer.h"
#include "Mesh.h"
#include "Math.h"
#include "graphics.h"
//#include "main.h"
#include <time.h>

#pragma once


extern int seed, ElevationPower,MoisturePower,sealevel;
//extern fp MinE, MaxE, MinM, MaxM;

void InitializeTerrain();
fp* GetHeightMap(int x, int y, int w, int h, const int power, biome* biomes);
class Chunk
{
public:
	fp* FluidVertices = nullptr;
	fp* CulledSolidVertices = nullptr;
	fp* UnCulledSolidVertices = nullptr;

	uint FluidSize = 0;//triangle count
	uint CulledSolidSize = 0;//triangle count
	uint UnCulledSolidSize = 0;//triangle count

	block data[ChunkScale3];//an 3d array that stores all the blocks

	BrightnessValue LightMap[ChunkScale3];//an 3d array that stores all the light levels

	int xPos;//the x position of the chunk
	int yPos;//the y position of the chunk

	bool Populated = false;//wether the chunk is populated(trees, actions)
	bool Changed = true;//wether the chunk needs to recalculate mesh positions and light

	biome* BiomeMap;
	int* Positions = nullptr;//markers to places where anything needs to be placed
	artifact* Artifacts = nullptr;//what needs to be placed
	int PositionCount;//marker count

	Chunk();
	Chunk(int OffsetX, int OffsetY);

	void DeleteMesh();
	void DeleteData();
	void LoadData();
	void GenerateData();
	void GenerateMesh();
	void Populate();
	static void PlaceArtifact(int x, int y, int z, artifact artifact, const bool addaction);
	static void SetSphere(const bool* place, const fp x, const fp y, const fp z, const fp radius, const block b, const bool addaction = false);
	//called on deletion
	~Chunk();
private:
	void CalculateLightMap();
	void GeneratePlaneXY1(const int& x1, const int& y1, const int& z,LightIntensity intensity, const fp& textureX,const fp& textureY, fp*& fptr);
	void GeneratePlaneXY0(const int& x1, const int& y1, const int& z, LightIntensity intensity, const fp& textureX,const fp& textureY, fp*& fptr);
	void GeneratePlaneXZ0(const int& x1, const int& y, const int& z1, LightIntensity intensity, const fp& textureX,const fp& textureY, fp*& fptr);
	void GeneratePlaneXZ1(const int& x1, const int& y, const int& z1, LightIntensity intensity, const fp& textureX,const fp& textureY, fp*& fptr);
	void GeneratePlaneYZ0(const int& x, const int& y1, const int& z1, LightIntensity intensity, const fp& textureX,const fp& textureY, fp*& fptr);
	void GeneratePlaneYZ1(const int& x, const int& y1, const int& z1, LightIntensity intensity, const fp& textureX,const fp& textureY, fp*& fptr);
	void GenerateFluidPlaneXY1(const int& x1, const int& y1, const int& z, LightIntensity intensity, fp*& fptr);
	void GenerateFluidPlaneXY0(const int& x1, const int& y1, const int& z, LightIntensity intensity, fp*& fptr);
	void GenerateFluidPlaneXZ0(const int& x1, const int& y, const int& z1, LightIntensity intensity, fp*& fptr);
	void GenerateFluidPlaneXZ1(const int& x1, const int& y, const int& z1, LightIntensity intensity, fp*& fptr);
	void GenerateFluidPlaneYZ0(const int& x, const int& y1, const int& z1, LightIntensity intensity, fp*& fptr);
	void GenerateFluidPlaneYZ1(const int& x, const int& y1, const int& z1, LightIntensity intensity, fp*& fptr);
	void GenerateCross(const int& x1, const int& y1, const int& z1, LightIntensity intensity, const fp& textureX,const fp& textureY, fp*& fptr);
};
#define FNV_32_PRIME 16777619u
unsigned int FNVHash32(const int input1, const int input2);
extern void SetBlockRange(int x0, int y0, int z0, int x1, int y1, int z1, block value, const bool addaction = false);
void SetBlockImgSize(vec3 v00, vec3 v10, vec3 v11, Image* image, std::vector<color> colors, std::vector<block> blocks, const bool addaction);
void SetBlockImgStep(vec3 start, vec3 xstep, vec3 ystep, Image* image, std::vector<color> colors, std::vector<block> blocks, const bool addaction);
block GetBlock(vec3 pos);
block GetBlock(int x, int y, int z);
vec3 GetLightLevel(int x, int y, int z);
Chunk* MapOnChunk(int& x, int& y, const int& z,int& ChunkX, int& ChunkY, int& ChunkIndex);
void SetBlock(vec3 pos, block b, const bool addaction = false);
void SetBlock(int x, int y, int z, block value, const bool addaction = false);