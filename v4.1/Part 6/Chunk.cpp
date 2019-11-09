#include "Chunk.h"
using namespace std;
LayerNoiseSimplex* HeightNoise;
LayerNoiseSimplex* MoistureNoise;
int Seed;//the seed of the world; different seed = different noise
int Power;//the power of the elevation(1 = e, 2 = e*e, 3 = e*e*e etc.)
fp MinE, MaxE;//elevation limits(0 to 1)
fp MinM, MaxM;//moisture limits(0 to 1)
int SeaLevel = 25;//62 in minecraft
inline Biome GetBiome(fp e, fp m) {
	return

		(e < 0.1) ? OCEAN :

		(e < 0.12) ? BEACH :

		(e > 0.8) ?
		(m < 0.1) ? SCORCHED :
		(m < 0.2) ? BARE :
		(m < 0.5) ? TUNDRA :
		SNOW :

		(e > 0.6) ?
		(m < 0.33) ? TEMPERATE_DESERT :
		(m < 0.66) ? SHRUBLAND :
		TAIGA :

		(e > 0.3) ?
		(m < 0.16) ? TEMPERATE_DESERT :
		(m < 0.50) ? GRASSLAND :
		(m < 0.83) ? TEMPERATE_DECIDUOUS_FOREST :
		TEMPERATE_RAIN_FOREST :

		(m < 0.16) ? SUBTROPICAL_DESERT :
		(m < 0.33) ? GRASSLAND :
		(m < 0.66) ? TROPICAL_SEASONAL_FOREST :
		TROPICAL_RAIN_FOREST;
}
void InitializeTerrain()
{
	HeightNoise = new LayerNoiseSimplex(Seed, new fp[Octaves] {1, .5f, .25f, .125f, .06125f, .036125f}, Octaves, MinE, MaxE, FREQUENCY);
	MoistureNoise = new LayerNoiseSimplex(Seed + 1, new fp[BIOME_OCTAVES] {1, .75f, .33f, .33f, .33f, .5f}, BIOME_OCTAVES, MinM, MaxM, BIOME_FREQUENCY);
}
Biome GetBiome(int X, int Y)
{
	fp elevation = HeightNoise->Evaluate2d(X, Y);
	Pow(elevation, Power);
	fp moisture = MoistureNoise->Evaluate2d(X, Y);
	return GetBiome(elevation, moisture);
}

fp* GetHeightMap(int x, int y, const int w, const int l, Biome* Biomes)
{
	const int size = w * l;
	fp* HeightMap = new fp[size];
	fp* eValues = HeightNoise->Evaluate2d(x, y, w, l);
	fp* mValues = MoistureNoise->Evaluate2d(x, y, w, l);
	fp* mPtr = mValues;//pointer to moisture values
	fp* eEndPtr = eValues + size;
	fp* HeightMapPtr = HeightMap;
	for (fp* ptr = eValues; ptr < eEndPtr; ptr++) {
		//pow(e,i)
		Pow(*ptr, Power);
		Biome biome = GetBiome(*ptr, *mPtr++);
			*Biomes++ = biome;
			//*Biomes++ = GetBiome(*ptr, *mPtr++);
		*HeightMapPtr++ = *ptr
			* GenDiff + MinGenHeight;
	}
	//delete[] mValues, eValues; does not give the expected behaviour.
	//sources:
	//https://stackoverflow.com/questions/6694745/how-to-delete-multiple-dynamically-allocated-arrays-in-a-single-delete-statement
	//http://stackoverflow.com/questions/3037655/c-delete-syntax
	delete[] mValues,delete[] eValues;
	return HeightMap;
}
//#include "../Noise/PerlinNoise.h"
//#include "../Noise/FastNoiseSIMD/FastNoiseSIMD.h"
unsigned int FNVHash32(const int input1, const int input2)
{
	unsigned int hash = 2166136261u;
	const unsigned char* pBuf = (unsigned char *)&input1;

	for (int i = 0; i < 4; ++i)
	{
		hash *= FNV_32_PRIME;
		hash ^= *pBuf++;
	}

	pBuf = (unsigned char *)&input2;

	for (int i = 0; i < 4; ++i)
	{
		hash *= FNV_32_PRIME;
		hash ^= *pBuf++;
	}

	return hash;
}
Chunk::Chunk() {}
Chunk::Chunk(int OffsetX, int OffsetY)
{
		this->xPos = OffsetX;
		this->yPos = OffsetY;
		this->Populated = false;
		LoadData();
}
//loads data.
//if file exists, load from file, else generate
void Chunk::LoadData()
{
	GenerateData();
}
void Chunk::GenerateData() {
	block* ptr = this->data;
	int ScaleOffsetX = xPos * ChunkScaleHor;
	int ScaleOffsetY = yPos * ChunkScaleHor;
	fp NoiseScale = 10;
	const int MAX_ARTIFACTS = ChunkScale2;
	int Positions[MAX_ARTIFACTS * 3];
	Artifact Artifacts[MAX_ARTIFACTS];
	int PositionCount = 0;
	int difference = Maxheight - Bedrocklayers;
	Biome* BiomeMap = new Biome[ChunkScale2];
	fp* HeightMap = GetHeightMap(ScaleOffsetX, ScaleOffsetY, ChunkScaleHor, ChunkScaleHor, BiomeMap);
	Biome* biomePtr = BiomeMap;

	//initialize chunk seed
	int Chunkseed = FNVHash32(xPos, yPos);
	srand(Chunkseed);

	//fill region below or equal to minheight with bedrock
	for (int j = 0; j < ChunkScaleHor; j++) {
		for (int i = 0; i < ChunkScaleHor; i++) {
			//get map data
			int height = HeightMap[i + j * StrideY];
			Biome ActiveBiome = *biomePtr++;

			int r = rand() % 1000;//1000 possibilities
			const int* ChancePtr = chance + (int)ActiveBiome * ArtifactCount;

			//determine if artifact is placed and what type
			for (int artIndex = 0; artIndex < ArtifactCount; artIndex++)
			{
				if (r < ChancePtr[artIndex])
				{
					Artifacts[PositionCount / 3] = (Artifact)artIndex;
					Positions[PositionCount++] = i;
					Positions[PositionCount++] = j;
					Positions[PositionCount++] = height;
					break;//else all the other artifacts will also be placed
				}
			}

			for (int k = 0; k < ChunkScaleVert; k++)
			{
				block* ActiveBlock = data + i + j * StrideY + k * StrideZ;
				if (k < height)
				{
					const int GroundLayers = 4;
					if (k < height - GroundLayers)
					{
						*ActiveBlock = Stone;
					}
					else 
					{
						*ActiveBlock = LayerBlock[ActiveBiome];
					}
				}
				else if (k == height) 
				{
					*ActiveBlock = TopBlock[ActiveBiome];
				}
				else {
					if (k < SeaLevel)
					{
						*ActiveBlock = Water;
					}
					else
					{
						*ActiveBlock = Air;
					}
				}
			}
		}
	}
	delete[] HeightMap;
	delete[] BiomeMap;
	//positions
	this->PositionCount = PositionCount;

	//copy artifact data
	int* posPtr = new int[PositionCount];
	memcpy(posPtr, Positions, PositionCount * sizeof(int));
	this->Positions = posPtr;
	//artifacts
	Artifact* artifactPtr = new Artifact[PositionCount / 3];
	memcpy(artifactPtr, Artifacts, PositionCount / 3 * sizeof(Artifact));
	this->Artifacts = artifactPtr;
}
Chunk::~Chunk()
{
	delete[] CulledSolidVertices;
	delete[] UnCulledSolidVertices;
	delete[] FluidVertices;
}
//https://minecraft.gamepedia.com/Tree
//https://github.com/ferreusveritas/DynamicTrees/wiki/World-generation
void Chunk::PlaceBirchTree(int x, int y, int z, const bool addaction = false)
{
	int TrunkHeight = rand() % 3 + 2;
	int bottomz = z + TrunkHeight;
	int TrunkTopZ = bottomz + 3;
	//trunk
	SetBlockRange(x, y, z, x + 1, y + 1, TrunkTopZ, block::BirchWood);

	//layer 1
	//-y
	SetBlock(x, y - 1, bottomz + 3, Leaves);
	//+y
	SetBlock(x, y + 1, bottomz + 3, Leaves);
	//x
	SetBlockRange(x - 1, y, bottomz + 3, x + 2, y + 1, bottomz + 4, Leaves);

	//layer 2
	//-y
	SetBlockRange(x - 1, y - 1, bottomz + 2, x + 2, y, bottomz + 3, Leaves);
	//+y
	SetBlockRange(x - 1, y + 1, bottomz + 2, x + 2, y + 2, bottomz + 3, Leaves);
	//-x
	SetBlock(x - 1, y, bottomz + 2, Leaves);
	//+x
	SetBlock(x + 1, y, bottomz + 2, Leaves);

	//layer 3
	//-x
	SetBlockRange(x - 2, y - 1, bottomz + 1, x, y + 2, bottomz + 2, Leaves);
	//+x
	SetBlockRange(x + 1, y - 1, bottomz + 1, x + 3, y + 2, bottomz + 2, Leaves);
	//-y
	SetBlockRange(x - 1, y - 2, bottomz + 1, x + 2, y - 1, bottomz + 2, Leaves);
	//fitting block
	SetBlock(x, y - 1, bottomz + 1, Leaves);
	//+y
	SetBlockRange(x - 1, y + 2, bottomz + 1, x + 2, y + 3, bottomz + 2, Leaves);
	//fitting blocks:
	//+y
	SetBlock(x, y + 1, bottomz + 2, Leaves);
	//-y
	SetBlock(x, y - 1, bottomz + 2, Leaves);

	//layer 4
	//-x
	SetBlockRange(x - 2, y - 2, bottomz, x, y + 3, bottomz + 1, Leaves);
	//+x
	SetBlockRange(x + 1, y - 2, bottomz, x + 3, y + 3, bottomz + 1, Leaves);
	//y
	SetBlock(x, y - 2, bottomz, Leaves);
	SetBlock(x, y - 1, bottomz, Leaves);
	SetBlock(x, y + 1, bottomz, Leaves);
	SetBlock(x, y + 2, bottomz, Leaves);
}
void Chunk::PlaceOakTree(int x, int y, int z, const bool addaction = false)
{
	int TrunkHeight = rand() % 3 + 2;
	int TrunkTopZ = z + TrunkHeight;
	int woodtopz = TrunkTopZ + rand() % 4;
	SetBlockRange(x, y, z, x + 1, y + 1, woodtopz, block::OakWood, addaction);//trunk
	for (int az = TrunkTopZ; az < woodtopz; az++) {
		int branchcount = rand() % 4 == 0 ? 0 : 1;
		//place branches
		for (int i = 0; i < branchcount; i++)
		{
			int direction = rand() % 8;//the direction of the branch
			int length = rand() % 3 + 1;
			const int stepx = dir8x[direction];
			const int stepy = dir8y[direction];
			int ax = x, ay = y;
			for (int j = 1; j <= length; j++)
			{
				ax += stepx, ay += stepy;
				SetBlock(ax, ay, az, block::OakWood, addaction);
			}
			//leaves on the end of the branch
			SetSphere(HasAir, ax, ay, az, RANDFP * 2, block::Leaves, addaction);
		}
	}
	//leaves on top of main branch
	SetSphere(HasAir, x, y, woodtopz, 2.5, block::Leaves, addaction);
}
void Chunk::PlacePalmTree(int x, int y, int z, const bool addaction = false)
{
	int TrunkHeight = rand() % 3 + 3;
	int TrunkTopZ = z + TrunkHeight;
	SetBlockRange(x, y, z, x + 1, y + 1, TrunkTopZ, block::PalmWood, addaction);

	//cross
	SetBlock(x - 2, y, TrunkTopZ, Leaves, addaction);//x row
	SetBlock(x - 1, y, TrunkTopZ, Leaves, addaction);
	SetBlock(x + 1, y, TrunkTopZ, Leaves, addaction);
	SetBlock(x + 2, y, TrunkTopZ, Leaves, addaction);
	SetBlockRange(x, y - 2, TrunkTopZ, x + 1, y + 3, TrunkTopZ + 1, Leaves, addaction);//y row
	//top leaf
	SetBlock(x, y, TrunkTopZ + 1, Leaves, addaction);
	//hanging leaves
	int LeafHeight = TrunkTopZ - 1;
	SetBlock(x - 3, y, LeafHeight, Leaves, addaction);
	SetBlock(x, y - 3, LeafHeight, Leaves, addaction);
	SetBlock(x + 3, y, LeafHeight, Leaves, addaction);
	SetBlock(x, y + 3, LeafHeight, Leaves, addaction);
}
//best height: 2
void Chunk::PlaceSpruceTree(int x, int y, int z, const bool addaction = false)
{
	int TrunkHeight = rand() % 3 + 1;
	int TrunkTopZ = z + TrunkHeight + 3;
	SetBlockRange(x, y, z, x + 1, y + 1, TrunkTopZ, block::OakWood, addaction);

	//top cross
	SetBlock(x - 1, y, TrunkTopZ, Leaves, addaction);//x row
	SetBlock(x + 1, y, TrunkTopZ, Leaves, addaction);
	SetBlock(x, y - 1, TrunkTopZ, Leaves, addaction);//y row
	SetBlock(x, y, TrunkTopZ, Leaves, addaction);
	SetBlock(x, y + 1, TrunkTopZ, Leaves, addaction);
	//top leaf
	SetBlock(x, y, TrunkTopZ + 1, Leaves, addaction);
	//circles of leaves
	//small circle
	int SmallCircleHeight = TrunkTopZ - 2;
	SetBlock(x - 1, y, SmallCircleHeight, Leaves, addaction);//cross
	SetBlock(x, y - 1, SmallCircleHeight, Leaves, addaction);
	SetBlock(x + 1, y, SmallCircleHeight, Leaves, addaction);
	SetBlock(x, y + 1, SmallCircleHeight, Leaves, addaction);
	//big circle
	int BigCircleHeight = TrunkTopZ - 3;
	SetBlockRange(x - 2, y - 1, BigCircleHeight, x, y + 2, BigCircleHeight + 1, Leaves, addaction);//-x
	SetBlockRange(x + 1, y - 1, BigCircleHeight, x + 3, y + 2, BigCircleHeight + 1, Leaves, addaction);//+x
	SetBlockRange(x - 1, y - 2, BigCircleHeight, x + 2, y - 1, BigCircleHeight + 1, Leaves, addaction);//-y
	SetBlockRange(x - 1, y + 2, BigCircleHeight, x + 2, y + 3, BigCircleHeight + 1, Leaves, addaction);//+y
	SetBlock(x, y - 1, BigCircleHeight, Leaves, addaction);//fitting block: -y
	SetBlock(x, y + 1, BigCircleHeight, Leaves, addaction);//fitting block: +y
}
void Chunk::PlaceCactus(int x, int y, int z, const bool addaction = false)
{
	int TrunkHeight = rand() % 3 + 2;
	SetBlockRange(x, y, z, x + 1, y + 1, z + TrunkHeight, block::CactusBlock, addaction);
}
//tall spruce
void Chunk::PlacePineTree(int x, int y, int z, const bool addaction = false)
{
	const int TrunkHeight = rand() % 5 + 5;//high trunks
	const int TrunkTopZ = z + TrunkHeight;
	const int LeaveLayers = 4;//4 layers of leaves
	const int TrunkBottomZ = TrunkTopZ - LeaveLayers + 1;
	//set wood
	SetBlockRange(x, y, z, x + 1, y + 1, TrunkTopZ + 1, block::OakWood, addaction);
	//set leaves
	//-x
	SetBlockRange(x - 1, y - 1, TrunkBottomZ, x, y + 2, TrunkTopZ + 1, block::Leaves, addaction);
	//+x
	SetBlockRange(x + 1, y - 1, TrunkBottomZ, x + 2, y + 2, TrunkTopZ + 1, block::Leaves, addaction);
	//-y
	SetBlockRange(x, y - 1, TrunkBottomZ, x + 1, y, TrunkTopZ + 1, block::Leaves, addaction);
	//+y
	SetBlockRange(x, y + 1, TrunkBottomZ, x + 1, y + 2, TrunkTopZ + 1, block::Leaves, addaction);
	//top block
	SetBlock(x, y, TrunkTopZ + 1, block::Leaves, addaction);
}

const int MAX_CULLED_SOLID_VERTICES = 0x100000;//a million
const int MAX_UNCULLED_SOLID_VERTICES = 0x100000;//a million
const int MAX_FLUID_VERTICES = 0x10000;//65 thousand
void Chunk::GenerateMesh() {
	fp* CulledSolidVerticesContainer = new fp[MAX_CULLED_SOLID_VERTICES];
	fp* UnCulledSolidVerticesContainer = new fp[MAX_UNCULLED_SOLID_VERTICES];
	fp* FluidVerticesContainer = new fp[MAX_FLUID_VERTICES];
	CalculateLightMap();
	//delete[] vertices;
	Changed = false;
	int CulledSolidPlaneCount = 0;
	int UnCulledSolidPlaneCount = 0;
	int FluidPlaneCount = 0;
	fp* CulledSolidPtr = CulledSolidVerticesContainer;
	fp* UnCulledSolidPtr = UnCulledSolidVerticesContainer;
	fp* FluidPtr = FluidVerticesContainer;
	int ScaleOffsetX = xPos * ChunkScaleHor;
	int ScaleOffsetY = yPos * ChunkScaleHor;
	LightIntensity intensity = LightIntensity();
	for (int j = 0; j < ChunkScaleHor; j++) {
	//for (int j = 1; j < ChunkScaleHor - 1; j++) {
			int RealY = j + ScaleOffsetY;
		for (int i = 0; i < ChunkScaleHor; i++) {
		//for (int i = 1; i < ChunkScaleHor - 1; i++) {
				int RealX = i + ScaleOffsetX;
			BrightnessValue* lightptr = LightMap + i + j * StrideY + (ChunkScaleVert - 1) * StrideZ;
			for (int k = ChunkScaleVert - 1; k >= 0; k--) {
				int index = i + j * StrideY + k * StrideZ;
				block* ptr = data + index;
				block block = *ptr;
				//intensity.Shift();
				fp top = *lightptr * BrightnessValueToFp;
				intensity.b001 = top;
				intensity.b101 = top;
				intensity.b011 = top;
				intensity.b111 = top;
				if (k > 0) {
					lightptr -= StrideZ;//z--
					fp bottom = *lightptr * BrightnessValueToFp;
					intensity.b000 = bottom;
					intensity.b100 = bottom;
					intensity.b010 = bottom;
					intensity.b110 = bottom;
				}
				//std::copy(&intensity.b000, &intensity.b001, &intensity.b001);
				//if(block < MinSolid && block != Leaves)
				if (block > Air)
				{
					if (block == Water)
					{
						if (HasAir[GetBlock(RealX-1,RealY,k)])
						{
							GenerateFluidPlaneYZ0(RealX, RealY, k, intensity, FluidPtr);
							FluidPlaneCount ++;
						}
						if (HasAir[GetBlock(RealX, RealY-1, k)])
						{
							GenerateFluidPlaneXZ0(RealX, RealY, k, intensity, FluidPtr);
							FluidPlaneCount ++;
						}
						if (k > 0 && HasAir[GetBlock(RealX , RealY, k-1)])
						{
							GenerateFluidPlaneXY0(RealX, RealY, k, intensity, FluidPtr);
							FluidPlaneCount ++;
						}
						if (HasAir[GetBlock(RealX + 1, RealY, k)])
						{
							GenerateFluidPlaneYZ1(RealX + 1, RealY, k, intensity, FluidPtr);
							FluidPlaneCount ++;
						}
						if (HasAir[GetBlock(RealX , RealY+1, k)])
						{
							GenerateFluidPlaneXZ1(RealX, RealY + 1, k, intensity, FluidPtr);
							FluidPlaneCount ++;
						}
						if (HasAir[GetBlock(RealX, RealY, k + 1)])
						{
							GenerateFluidPlaneXY1(RealX, RealY, k + 1, intensity, FluidPtr);
							FluidPlaneCount ++;
						}
						continue;
					}
					fp TextureSideX = textureIndices[block * TextIndiceCount];
					fp TextureSideY = textureIndices[block * TextIndiceCount + 1];
					if (GetBlockType[block] == Cross) {//cross
						GenerateCross(RealX, RealY, k, intensity, TextureSideX, TextureSideY, UnCulledSolidPtr);
						UnCulledSolidPlaneCount += 2;//two planes
						continue;
					}
					fp** activePtr;
					int* activePlaneCount;
					if (cullbackface[block]) {
						activePtr = &CulledSolidPtr;
						activePlaneCount = &CulledSolidPlaneCount;
					}
					else 
					{
						activePtr = &UnCulledSolidPtr;
						activePlaneCount = &UnCulledSolidPlaneCount;
					}
					if (IsVisibleSurrounded[GetBlock(RealX - 1, RealY, k)]) {//x--
						GeneratePlaneYZ0(RealX, RealY, k, intensity, TextureSideX, TextureSideY, *activePtr);
						(*activePlaneCount)++;
					}
					if (IsVisibleSurrounded[GetBlock(RealX, RealY - 1, k)]) {//y--
						GeneratePlaneXZ0(RealX, RealY, k, intensity, TextureSideX, TextureSideY, *activePtr);
						(*activePlaneCount)++;
					}
					if (k > 0 && IsVisibleSurrounded[GetBlock(RealX, RealY, k - 1)]) {//z--
						fp TextureBottomX = textureIndices[block * TextIndiceCount + 4];
						fp TextureBottomY = textureIndices[block * TextIndiceCount + 5];
						GeneratePlaneXY0(RealX, RealY, k, intensity, TextureBottomX, TextureBottomY, *activePtr);
						(*activePlaneCount)++;
					}
					fp TextureTopX = textureIndices[block * TextIndiceCount + 2];
					fp TextureTopY = textureIndices[block * TextIndiceCount + 3];
					if (DrawAllSides[block]) {
						if (IsVisibleSurrounded[GetBlock(RealX + 1, RealY, k)]) {//x++
							GeneratePlaneYZ1(RealX + 1, RealY, k, intensity, TextureSideX, TextureSideY, *activePtr);
							(*activePlaneCount)++;
						}
						if (IsVisibleSurrounded[GetBlock(RealX, RealY + 1, k)]) {//y++
							GeneratePlaneXZ1(RealX, RealY + 1, k, intensity, TextureSideX, TextureSideY, *activePtr);
							(*activePlaneCount)++;
						}
						if (IsVisibleSurrounded[GetBlock(RealX, RealY, k + 1)]) {//z++
							GeneratePlaneXY1(RealX, RealY, k + 1, intensity, TextureTopX, TextureTopY, *activePtr);
							(*activePlaneCount)++;
						}
					}
					else {//blocks with transparent parts in it
						//==air because leaves would be drawn twice every block
						if (!IsSolidBlock[GetBlock(RealX + 1, RealY, k)]) {//x++
							GeneratePlaneYZ1(RealX + 1, RealY, k, intensity, TextureSideX, TextureSideY, *activePtr);
							(*activePlaneCount)++;
						}
						if (!IsSolidBlock[GetBlock(RealX, RealY + 1, k)]) {//y++
							GeneratePlaneXZ1(RealX, RealY + 1, k, intensity, TextureSideX, TextureSideY, *activePtr);
							(*activePlaneCount)++;
						}
						if (!IsSolidBlock[GetBlock(RealX, RealY, k + 1)]) {//z++
							GeneratePlaneXY1(RealX, RealY, k + 1, intensity, TextureTopX, TextureTopY, *activePtr);
							(*activePlaneCount)++;
						}
					}
				}
			}
		}
	}
	//culled solid
	CulledSolidSize = CulledSolidPlaneCount * 2;
	CulledSolidVertices = (fp*)ResizeArray((byte*)CulledSolidVerticesContainer, CulledSolidPlaneCount * ((3 + 2 + 3) * 4) * sizeof(fp));
	//unculled solid
	UnCulledSolidSize = UnCulledSolidPlaneCount * 2;
	UnCulledSolidVertices = (fp*)ResizeArray((byte*)UnCulledSolidVerticesContainer, UnCulledSolidPlaneCount * ((3 + 2 + 3) * 4) * sizeof(fp));
	//fluid
	FluidSize = FluidPlaneCount * 2;
	FluidVertices = (fp*)ResizeArray((byte*)FluidVerticesContainer, FluidPlaneCount * ((3 + 3) * 4) * sizeof(fp));
}
void Chunk::PlaceArtifact(int x, int y, int z, Artifact artifact, const bool addaction = false)
{
	int ChunkIndex;
	switch (artifact)
	{
	case aOakTree:
		PlaceOakTree(x, y, z, addaction);
		return;
	case aBirchTree:
		PlaceBirchTree(x, y, z, addaction);
		return;
	case aPalmTree:
		PlacePalmTree(x, y, z, addaction);
		return;
	case aSpruceTree:
		PlaceSpruceTree(x, y, z, addaction);
		return;
	case aPineTree:
		PlacePineTree(x, y, z, addaction);
		return;
	case aCactus:
		PlaceCactus(x, y, z, addaction);
		return;
	case aBlueFlower:
		SetBlock(x, y, z, block::BlueFlower, addaction);
		return;
	case aRedFlower:
		SetBlock(x, y, z, block::RedFlower, addaction);
		return;
	case aYellowFlower:
		SetBlock(x, y, z, block::YellowFlower, addaction);
		return;
	default:
		return;
	}

}
void Chunk::SetSphere(const bool* place, const fp x, const fp y, const fp z, const fp radius,const block b, const bool addaction)
{
	const fp radius2 = radius * radius;
	const int minx = floor(x - radius);
	const int miny = floor(y - radius);
	const int minz = floor(z - radius);
	const int maxx = ceil(x + radius);
	const int maxy = ceil(y + radius);
	const int maxz = ceil(z + radius);
	fp dz = z - minz;
	for (int az = minz; az <= maxz; az++)
	{
		const fp dz2 = dz * dz;
		fp dy = y - miny;
		for (int ay = miny; ay <= maxy; ay++)
		{
			const fp dy2 = dy * dy;
			const fp dzy2 = dz2 + dy2;
			fp dx = x - minx;
			for (int ax = minx; ax <= maxx; ax++)
			{
				const fp dx2 = dx * dx;
				if (dzy2 + dx2 <= radius2 && place[GetBlock(ax, ay, az)])
				{
					SetBlock(ax, ay, az, b, addaction);
				}
				dx--;
			}
			dy--;
		}
		dz--;
	}
}
void Chunk::Populate() 
{
	//initialize chunk seed
	int seed = FNVHash32(xPos, yPos);
	srand(seed);

	//load structures
	block* ptr = this->data;
	int ScaleOffsetX = xPos * ChunkScaleHor;
	int ScaleOffsetY = yPos * ChunkScaleHor;

	for (int i = 0; i < PositionCount;)
	{
		Artifact artifact = Artifacts[i / 3];
		int TreeI = Positions[i++];
		int TreeJ = Positions[i++];
		int TreeK = Positions[i++] + 1;//origin 1 higher
		BlockIndex index = TreeI + TreeJ * StrideY + TreeK * StrideZ;
		int TreeX = TreeI + ScaleOffsetX;
		int TreeY = TreeJ + ScaleOffsetY;
		
		PlaceArtifact(TreeX, TreeY, TreeK,artifact);
	placed:;
	}
	ActionContainer* container = GetContainer(xPos, yPos);
	if (container != NULL) {
		std::list<Action*>* actions = container->actions;
		for (auto ActiveAction = actions->begin(); ActiveAction != actions->end(); ActiveAction++)
		{
			Action a = **ActiveAction;
			*(data + a.index) = a.b;
		}
	}
	delete[] Positions;
	delete[] Artifacts;
	Changed = true;
	Populated = true;
}
//https://stackoverflow.com/questions/4535133/how-does-minecraft-perform-lighting
void Chunk::CalculateLightMap() {
	block* bBegin = data + (ChunkScaleVert - 1) * StrideZ;//z plus
	//the lowest block to check: 1, because there is nothing underneath 0
	fp AboveBlockOpacity;
	BrightnessValue MinBrightness = 0x80;
	BrightnessValue SunLight = 0xff;
	for (int j = 0; j < ChunkScaleHor; j++) {//y++
		block* jBlockPlus = bBegin + j * StrideY;
		BrightnessValue* YPlus = LightMap + j * StrideY;
		for (int i = 0; i < ChunkScaleHor; i++) {//x++
			BrightnessValue* bvLowest = YPlus + i;
			BrightnessValue* bvActive = bvLowest + (ChunkScaleVert - 1) * StrideZ;
			//bv: csv - 1 to 0
			*bvActive = SunLight;//set all upper values to sunlight
			bvActive -= StrideZ;
			//blockabove: csv -1 to 1
			block* AboveBlock = jBlockPlus + i;
			while (bvActive >= bvLowest)
			{
				AboveBlockOpacity = Opacity[*AboveBlock];//x y
				*bvActive = AboveBlockOpacity * *(bvActive + StrideZ);
				if (*bvActive < MinBrightness) {
					while (bvActive >= bvLowest) {//fill the rest with minimum
						*bvActive = MinBrightness;
						bvActive -= StrideZ;//z--
					}
					goto next;
				}
				bvActive -= StrideZ;
				AboveBlock -= StrideZ;
			}
		next:;
		}
	}
}
//inverted mtexture y coordinates
//[axis] 0 is inverted to let the normals point to the outside
void Chunk::GeneratePlaneXY0(const int& x1, const int& y1, const int& z, LightIntensity intensity, const fp& textureX, const fp& textureY, fp*& fptr) {
	const fp textureX1 = textureX + TextSize;
	const fp textureY1 = textureY + TextSize;
	const int x2 = x1 + 1;
	const int y2 = y1 + 1;
	*fptr++ = x1;	*fptr++ = y1;	*fptr++ = z; *fptr++ = textureX; *fptr++ = textureY1; *fptr++ = intensity.b000; *fptr++ = intensity.b000; *fptr++ = intensity.b000;//000,00,0
	*fptr++ = x1;	*fptr++ = y2;	*fptr++ = z; *fptr++ = textureX; *fptr++ = textureY; *fptr++ = intensity.b010; *fptr++ = intensity.b010; *fptr++ = intensity.b010;//010,01,2
	*fptr++ = x2;	*fptr++ = y1;	*fptr++ = z; *fptr++ = textureX1; *fptr++ = textureY1; *fptr++ = intensity.b100; *fptr++ = intensity.b100; *fptr++ = intensity.b100;//100,10,1
	*fptr++ = x2;	*fptr++ = y2;	*fptr++ = z; *fptr++ = textureX1; *fptr++ = textureY; *fptr++ = intensity.b110; *fptr++ = intensity.b110; *fptr++ = intensity.b110;//110,11,3
}
void Chunk::GeneratePlaneXY1(const int& x1, const int& y1, const int& z, LightIntensity intensity, const fp& textureX, const fp& textureY, fp*& fptr) {
	const fp textureX1 = textureX + TextSize;
	const fp textureY1 = textureY + TextSize;
	const int x2 = x1 + 1;
	const int y2 = y1 + 1;
	*fptr++ = x1;	*fptr++ = y1;	*fptr++ = z; *fptr++ = textureX; *fptr++ = textureY1; *fptr++ = intensity.b001; *fptr++ = intensity.b001; *fptr++ = intensity.b001;//000,00,0
	*fptr++ = x2;	*fptr++ = y1;	*fptr++ = z; *fptr++ = textureX1; *fptr++ = textureY1; *fptr++ = intensity.b101; *fptr++ = intensity.b101; *fptr++ = intensity.b101;//100,10,1
	*fptr++ = x1;	*fptr++ = y2;	*fptr++ = z; *fptr++ = textureX; *fptr++ = textureY; *fptr++ = intensity.b011; *fptr++ = intensity.b011; *fptr++ = intensity.b011;//010,01,2
	*fptr++ = x2;	*fptr++ = y2;	*fptr++ = z; *fptr++ = textureX1; *fptr++ = textureY; *fptr++ = intensity.b111; *fptr++ = intensity.b111; *fptr++ = intensity.b111;//110,11,3
}
void Chunk::GeneratePlaneXZ0(const int& x1, const int& y, const int& z1, LightIntensity intensity, const fp& textureX, const fp& textureY, fp*& fptr) {
	const fp textureX1 = textureX + TextSize;
	const fp textureY1 = textureY + TextSize;
	const int x2 = x1 + 1;
	const int z2 = z1 + 1;
	*fptr++ = x1;	*fptr++ = y;	*fptr++ = z1; *fptr++ = textureX; *fptr++ = textureY1; *fptr++ = intensity.b000; *fptr++ = intensity.b000; *fptr++ = intensity.b000;//000,00,0
	*fptr++ = x2;	*fptr++ = y;	*fptr++ = z1; *fptr++ = textureX1; *fptr++ = textureY1; *fptr++ = intensity.b100; *fptr++ = intensity.b100; *fptr++ = intensity.b100;//100,10,1
	*fptr++ = x1;	*fptr++ = y;	*fptr++ = z2; *fptr++ = textureX; *fptr++ = textureY; *fptr++ = intensity.b001; *fptr++ = intensity.b001; *fptr++ = intensity.b001;//001,01,2
	*fptr++ = x2;	*fptr++ = y;	*fptr++ = z2; *fptr++ = textureX1; *fptr++ = textureY; *fptr++ = intensity.b101; *fptr++ = intensity.b101; *fptr++ = intensity.b101;//101,11,3
}
void Chunk::GeneratePlaneXZ1(const int& x1, const int& y, const int& z1, LightIntensity intensity, const fp& textureX, const fp& textureY, fp*& fptr) {
	const fp textureX1 = textureX + TextSize;
	const fp textureY1 = textureY + TextSize;
	const int x2 = x1 + 1;
	const int z2 = z1 + 1;
	*fptr++ = x1;	*fptr++ = y;	*fptr++ = z1; *fptr++ = textureX; *fptr++ = textureY1; *fptr++ = intensity.b010; *fptr++ = intensity.b010; *fptr++ = intensity.b010;//010,00,0
	*fptr++ = x1;	*fptr++ = y;	*fptr++ = z2; *fptr++ = textureX; *fptr++ = textureY; *fptr++ = intensity.b011; *fptr++ = intensity.b011; *fptr++ = intensity.b011;//011,01,2
	*fptr++ = x2;	*fptr++ = y;	*fptr++ = z1; *fptr++ = textureX1; *fptr++ = textureY1; *fptr++ = intensity.b110; *fptr++ = intensity.b110; *fptr++ = intensity.b110;//110,10,1
	*fptr++ = x2;	*fptr++ = y;	*fptr++ = z2; *fptr++ = textureX1; *fptr++ = textureY; *fptr++ = intensity.b111; *fptr++ = intensity.b111; *fptr++ = intensity.b111;//111,11,3
}
void Chunk::GeneratePlaneYZ0(const int& x, const int& y1, const int& z1, LightIntensity intensity, const fp& textureX, const fp& textureY, fp*& fptr) {
	const fp textureX1 = textureX + TextSize;
	const fp textureY1 = textureY + TextSize;
	const int y2 = y1 + 1;
	const int z2 = z1 + 1;
	*fptr++ = x;	*fptr++ = y1;	*fptr++ = z1; *fptr++ = textureX; *fptr++ = textureY1; *fptr++ = intensity.b000; *fptr++ = intensity.b000; *fptr++ = intensity.b000;//000,00,0
	*fptr++ = x;	*fptr++ = y1;	*fptr++ = z2; *fptr++ = textureX; *fptr++ = textureY; *fptr++ = intensity.b001; *fptr++ = intensity.b001; *fptr++ = intensity.b001;//001,01,2
	*fptr++ = x;	*fptr++ = y2;	*fptr++ = z1; *fptr++ = textureX1; *fptr++ = textureY1; *fptr++ = intensity.b010; *fptr++ = intensity.b010; *fptr++ = intensity.b010;//010,10,1
	*fptr++ = x;	*fptr++ = y2;	*fptr++ = z2; *fptr++ = textureX1; *fptr++ = textureY; *fptr++ = intensity.b011; *fptr++ = intensity.b011; *fptr++ = intensity.b011;//011,11,3
}
void Chunk::GeneratePlaneYZ1(const int& x, const int& y1, const int& z1, LightIntensity intensity, const fp& textureX, const fp& textureY, fp*& fptr) {
	const fp textureX1 = textureX + TextSize;
	const fp textureY1 = textureY + TextSize;
	const int y2 = y1 + 1;
	const int z2 = z1 + 1;
	*fptr++ = x;	*fptr++ = y1;	*fptr++ = z1; *fptr++ = textureX; *fptr++ = textureY1; *fptr++ = intensity.b100; *fptr++ = intensity.b100; *fptr++ = intensity.b100;//100,00,0
	*fptr++ = x;	*fptr++ = y2;	*fptr++ = z1; *fptr++ = textureX1; *fptr++ = textureY1; *fptr++ = intensity.b110; *fptr++ = intensity.b110; *fptr++ = intensity.b110;//110,10,1
	*fptr++ = x;	*fptr++ = y1;	*fptr++ = z2; *fptr++ = textureX; *fptr++ = textureY; *fptr++ = intensity.b101; *fptr++ = intensity.b101; *fptr++ = intensity.b101;//101,01,2
	*fptr++ = x;	*fptr++ = y2;	*fptr++ = z2; *fptr++ = textureX1; *fptr++ = textureY; *fptr++ = intensity.b111; *fptr++ = intensity.b111; *fptr++ = intensity.b111;//111,11,3
}
void Chunk::GenerateFluidPlaneXY0(const int& x1, const int& y1, const int& z, LightIntensity intensity,  fp*& fptr) {
	const int x2 = x1 + 1;
	const int y2 = y1 + 1;
	*fptr++ = x1;	*fptr++ = y1;	*fptr++ = z; *fptr++ = intensity.b000; *fptr++ = intensity.b000; *fptr++ = intensity.b000;//000,00,0
	*fptr++ = x2;	*fptr++ = y1;	*fptr++ = z;  *fptr++ = intensity.b100; *fptr++ = intensity.b100; *fptr++ = intensity.b100;//100,10,1
	*fptr++ = x1;	*fptr++ = y2;	*fptr++ = z;  *fptr++ = intensity.b010; *fptr++ = intensity.b010; *fptr++ = intensity.b010;//010,01,2
	*fptr++ = x2;	*fptr++ = y2;	*fptr++ = z;  *fptr++ = intensity.b110; *fptr++ = intensity.b110; *fptr++ = intensity.b110;//110,11,3
}
void Chunk::GenerateFluidPlaneXY1(const int& x1, const int& y1, const int& z, LightIntensity intensity, fp*& fptr) {
	const int x2 = x1 + 1;
	const int y2 = y1 + 1;
	*fptr++ = x1;	*fptr++ = y1;	*fptr++ = z; *fptr++ = intensity.b001; *fptr++ = intensity.b001; *fptr++ = intensity.b001;//000,0
	*fptr++ = x2;	*fptr++ = y1;	*fptr++ = z;  *fptr++ = intensity.b101; *fptr++ = intensity.b101; *fptr++ = intensity.b101;//100,1
	*fptr++ = x1;	*fptr++ = y2;	*fptr++ = z;  *fptr++ = intensity.b011; *fptr++ = intensity.b011; *fptr++ = intensity.b011;//010,2
	*fptr++ = x2;	*fptr++ = y2;	*fptr++ = z;  *fptr++ = intensity.b111; *fptr++ = intensity.b111; *fptr++ = intensity.b111;//110,3
}
void Chunk::GenerateFluidPlaneXZ0(const int& x1, const int& y, const int& z1, LightIntensity intensity,  fp*& fptr) {
	const int x2 = x1 + 1;
	const int z2 = z1 + 1;
	*fptr++ = x1;	*fptr++ = y;	*fptr++ = z1; *fptr++ = intensity.b000; *fptr++ = intensity.b000; *fptr++ = intensity.b000;//000,00,0
	*fptr++ = x2;	*fptr++ = y;	*fptr++ = z1;  *fptr++ = intensity.b100; *fptr++ = intensity.b100; *fptr++ = intensity.b100;//100,10,1
	*fptr++ = x1;	*fptr++ = y;	*fptr++ = z2;  *fptr++ = intensity.b001; *fptr++ = intensity.b001; *fptr++ = intensity.b001;//001,01,2
	*fptr++ = x2;	*fptr++ = y;	*fptr++ = z2;  *fptr++ = intensity.b101; *fptr++ = intensity.b101; *fptr++ = intensity.b101;//101,11,3
}
void Chunk::GenerateFluidPlaneXZ1(const int& x1, const int& y, const int& z1, LightIntensity intensity,  fp*& fptr) {
	const int x2 = x1 + 1;
	const int z2 = z1 + 1;
	*fptr++ = x1;	*fptr++ = y;	*fptr++ = z1; *fptr++ = intensity.b010; *fptr++ = intensity.b010; *fptr++ = intensity.b010;//010,00,0
	*fptr++ = x2;	*fptr++ = y;	*fptr++ = z1;  *fptr++ = intensity.b110; *fptr++ = intensity.b110; *fptr++ = intensity.b110;//110,10,1
	*fptr++ = x1;	*fptr++ = y;	*fptr++ = z2;  *fptr++ = intensity.b011; *fptr++ = intensity.b011; *fptr++ = intensity.b011;//011,01,2
	*fptr++ = x2;	*fptr++ = y;	*fptr++ = z2;  *fptr++ = intensity.b111; *fptr++ = intensity.b111; *fptr++ = intensity.b111;//111,11,3
}
void Chunk::GenerateFluidPlaneYZ0(const int& x, const int& y1, const int& z1, LightIntensity intensity, fp*& fptr) {
	const int y2 = y1 + 1;
	const int z2 = z1 + 1;
	*fptr++ = x;	*fptr++ = y1;	*fptr++ = z1; *fptr++ = intensity.b000; *fptr++ = intensity.b000; *fptr++ = intensity.b000;//000,00,0
	*fptr++ = x;	*fptr++ = y2;	*fptr++ = z1;  *fptr++ = intensity.b010; *fptr++ = intensity.b010; *fptr++ = intensity.b010;//010,10,1
	*fptr++ = x;	*fptr++ = y1;	*fptr++ = z2;  *fptr++ = intensity.b001; *fptr++ = intensity.b001; *fptr++ = intensity.b001;//001,01,2
	*fptr++ = x;	*fptr++ = y2;	*fptr++ = z2;  *fptr++ = intensity.b011; *fptr++ = intensity.b011; *fptr++ = intensity.b011;//011,11,3
}
void Chunk::GenerateFluidPlaneYZ1(const int& x, const int& y1, const int& z1, LightIntensity intensity,  fp*& fptr) {
	const int y2 = y1 + 1;
	const int z2 = z1 + 1;
	*fptr++ = x;	*fptr++ = y1;	*fptr++ = z1; *fptr++ = intensity.b100; *fptr++ = intensity.b100; *fptr++ = intensity.b100;//100,00,0
	*fptr++ = x;	*fptr++ = y2;	*fptr++ = z1;  *fptr++ = intensity.b110; *fptr++ = intensity.b110; *fptr++ = intensity.b110;//110,10,1
	*fptr++ = x;	*fptr++ = y1;	*fptr++ = z2;  *fptr++ = intensity.b101; *fptr++ = intensity.b101; *fptr++ = intensity.b101;//101,01,2
	*fptr++ = x;	*fptr++ = y2;	*fptr++ = z2;  *fptr++ = intensity.b111; *fptr++ = intensity.b111; *fptr++ = intensity.b111;//111,11,3
}
void Chunk::GenerateCross(const int& x1, const int& y1, const int& z1,LightIntensity intensity, const fp& textureX, const fp& textureY, fp*& fptr) {
	const fp textureX1 = textureX + TextSize;
	const fp textureY1 = textureY + TextSize;
	const int x2 = x1 + 1;
	const int y2 = y1 + 1;
	const int z2 = z1 + 1;
	//xy to xy

	//00 to 11
	*fptr++ = x1;	*fptr++ = y1;	*fptr++ = z1; *fptr++ = textureX; *fptr++ = textureY1; *fptr++ = intensity.b000; *fptr++ = intensity.b000; *fptr++ = intensity.b000;//000,00,0
	*fptr++ = x2;	*fptr++ = y2;	*fptr++ = z1; *fptr++ = textureX1; *fptr++ = textureY1; *fptr++ = intensity.b110; *fptr++ = intensity.b110; *fptr++ = intensity.b110;//110,10,1
	*fptr++ = x1;	*fptr++ = y1;	*fptr++ = z2; *fptr++ = textureX; *fptr++ = textureY; *fptr++ = intensity.b001; *fptr++ = intensity.b001; *fptr++ = intensity.b001;//001,01,2
	*fptr++ = x2;	*fptr++ = y2;	*fptr++ = z2; *fptr++ = textureX1; *fptr++ = textureY; *fptr++ = intensity.b111; *fptr++ = intensity.b111; *fptr++ = intensity.b111;//111,11,3
	//01 to 10
	*fptr++ = x1;	*fptr++ = y2;	*fptr++ = z1; *fptr++ = textureX; *fptr++ = textureY1; *fptr++ = intensity.b010; *fptr++ = intensity.b010; *fptr++ = intensity.b010;//010,00,0
	*fptr++ = x2;	*fptr++ = y1;	*fptr++ = z1; *fptr++ = textureX1; *fptr++ = textureY1; *fptr++ = intensity.b100; *fptr++ = intensity.b100; *fptr++ = intensity.b100;//100,10,1
	*fptr++ = x1;	*fptr++ = y2;	*fptr++ = z2; *fptr++ = textureX; *fptr++ = textureY; *fptr++ = intensity.b011; *fptr++ = intensity.b011; *fptr++ = intensity.b011;//011,01,2
	*fptr++ = x2;	*fptr++ = y1;	*fptr++ = z2; *fptr++ = textureX1; *fptr++ = textureY; *fptr++ = intensity.b101; *fptr++ = intensity.b101; *fptr++ = intensity.b101;//101,11,3
}