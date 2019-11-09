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

#define Pow(e,pow)\
fp p = e;\
for(int i = 1; i<pow;i++)\
p*=e;\
e = p;

extern int Seed, Power,SeaLevel;
extern fp MinE, MaxE, MinM, MaxM;

Biome GetBiome(int PosX, int PosY);
void InitializeTerrain();
fp* GetHeightMap(int x, int y, int w, int h, const int power, Biome* biomes);
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

	Biome* BiomeMap;
	int* Positions;//markers to places where anything needs to be placed
	Artifact* Artifacts;//what needs to be placed
	int PositionCount;//marker count

	Chunk();
	Chunk(int OffsetX, int OffsetY);

	void LoadData(); 
	void GenerateData();
	void GenerateMesh();
	void Populate();
	static void PlaceOakTree(int x, int y, int z, const bool addaction);
	static void PlacePalmTree(int x, int y, int z, const bool addaction);
	static void PlaceSpruceTree(int x, int y, int z, const bool addaction);
	static void PlacePineTree(int x, int y, int z, const bool addaction);
	static void PlaceBirchTree(int x, int y, int z, const bool addaction);
	static void PlaceCactus(int x, int y, int z, const bool addaction);
	static void PlaceArtifact(int x, int y, int z, Artifact artifact, const bool addaction);
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
Chunk* MapOnChunk(int& x, int& y, const int& z, int& ChunkIndex);
void SetBlock(vec3 pos, block b, const bool addaction = false);
void SetBlock(int x, int y, int z, block value, const bool addaction = false);