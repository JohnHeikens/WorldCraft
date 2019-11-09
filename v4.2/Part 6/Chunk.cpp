#include "Chunk.h"
using namespace std;
LayerNoiseSimplex* HeightNoise;
LayerNoiseSimplex* MoistureNoise;
int seed;//the seed of the world; different seed = different noise
int ElevationPower;//the power of the elevation(1 = e, 2 = e*e, 3 = e*e*e etc.)
int MoisturePower;
//fp MinE, MaxE;//elevation limits(0 to 1)
//fp MinM, MaxM;//moisture limits(0 to 1)
int sealevel;//62 in minecraft

std::wstring modfilename = L"data\\mods\\standard\\worldgen";
std::wstring* blockname = nullptr;
std::wstring* biomename = nullptr;
std::wstring* artifactname = nullptr;
fp* textureIndices = nullptr;
BlockType* GetBlockType = nullptr;
fp* Opacity = nullptr;
fp* GetBounceMultiplier = nullptr;
bool* IsRaycastVisible = nullptr;
bool* IsSolidBlock = nullptr;
bool* DrawAllSides = nullptr;
bool* HasAir = nullptr;
bool* cullbackface = nullptr;
bool* IsVisibleSurrounded = nullptr;
int* chance = nullptr;
block* artifactblocks = nullptr;
block* TopBlock = nullptr;
block* LayerBlock = nullptr;
color* aircolor = nullptr;
fp* fogdistance = nullptr;
int BlockCount= 0, ArtifactCount = 0, biomecount = 0;
int TextSize = 0;
block transparentblock;//water normally
block filloceanwith, fillgroundwith, fillskywith;
int biomemapscale;

vec3 worldspawn;

//images
Image* fullheart;
Image* halfheart;
Image* emptyheart;

artifact* artifacts;
color* biomecolor;
biome* biomemap;
std::vector<function*> artifactfunctions;
function* fstart;
function* fnextframe;

void InitializeTerrain()
{
	//HeightNoise = DBG_NEW LayerNoiseSimplex(Seed, DBG_NEW fp[Octaves] {1, .5f, .25f, .125f, .06125f, .036125f}, Octaves, MinE, MaxE, FREQUENCY);
	//MoistureNoise = DBG_NEW LayerNoiseSimplex(Seed + 1, DBG_NEW fp[BIOME_OCTAVES] {1, .75f, .33f, .33f, .33f, .5f}, BIOME_OCTAVES, MinM, MaxM, BIOME_FREQUENCY);
}


fp* GetHeightMap(int x, int y, const int w, const int l, biome* Biomes)
{
	const int size = w * l;
	fp* HeightMap = DBG_NEW fp[size];
	fp* eValues = HeightNoise->Evaluate2d(x, y, w, l);
	fp* mValues = MoistureNoise->Evaluate2d(x, y, w, l);
	fp* mPtr = mValues;//pointer to moisture values
	fp* eEndPtr = eValues + size;
	fp* HeightMapPtr = HeightMap;
	for (fp* ptr = eValues; ptr < eEndPtr; ptr++) {
		//pow(e,i)
		*ptr = PowIntSimple(*ptr, ElevationPower);
		*mPtr = PowIntSimple(*mPtr, MoisturePower);
		biome biome = GetBiomeClimate(*ptr, *mPtr++);
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
void Chunk::DeleteMesh()
{
	delete[] CulledSolidVertices;
	delete[] UnCulledSolidVertices;
	delete[] FluidVertices;
	CulledSolidVertices = nullptr;
	UnCulledSolidVertices = nullptr;
	FluidVertices = nullptr;
}
void Chunk::DeleteData()
{
	delete[] Artifacts;
	delete[] Positions;
	Artifacts = nullptr;
	Positions = nullptr;
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
	if (this->Artifacts) 
	{
		DeleteData();
	}
	int Positions[MAX_ARTIFACTS * 3];//temporary list of positions
	artifact Artifacts[MAX_ARTIFACTS];//temporary list of artifacts
	int PositionCount = 0;
	int difference = Maxheight - Bedrocklayers;
	biome* chunkbiomemap = DBG_NEW biome[ChunkScale2];
	fp* HeightMap = GetHeightMap(ScaleOffsetX, ScaleOffsetY, ChunkScaleHor, ChunkScaleHor, chunkbiomemap);
	biome* biomePtr = chunkbiomemap;

	//initialize chunk seed
	int Chunkseed = FNVHash32(xPos, yPos);
	srand(Chunkseed);

	//fill region below or equal to minheight with bedrock
	for (int j = 0; j < ChunkScaleHor; j++) {
		for (int i = 0; i < ChunkScaleHor; i++) {
			//get map data
			int height = HeightMap[i + j * StrideY];
			biome ActiveBiome = *biomePtr++;

			int r = rand() % 1000;//1000 possibilities
			const int* ChancePtr = chance + (int)ActiveBiome * ArtifactCount;

			//determine if artifact is placed and what type
			for (int artIndex = 0; artIndex < ArtifactCount; artIndex++)
			{
				if (r < ChancePtr[artIndex])
				{
					Artifacts[PositionCount / 3] = (artifact)artIndex;
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
						*ActiveBlock = fillgroundwith;
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
					if (k < sealevel)
					{
						*ActiveBlock = filloceanwith;
					}
					else
					{
						*ActiveBlock = fillskywith;
					}
				}
			}
		}
	}
	delete[] HeightMap;
	delete[] chunkbiomemap;
	//positions
	this->PositionCount = PositionCount;

	//copy artifact data
	int* posPtr = DBG_NEW int[PositionCount];
	memcpy(posPtr, Positions, PositionCount * sizeof(int));
	this->Positions = posPtr;
	//artifacts
	artifact* artifactPtr = DBG_NEW artifact[PositionCount / 3];
	memcpy(artifactPtr, Artifacts, PositionCount / 3 * sizeof(artifact));
	this->Artifacts = artifactPtr;
}
Chunk::~Chunk()
{
	DeleteMesh();
	if (Artifacts) 
	{
		DeleteData();
	}
}

const int MAX_CULLED_SOLID_VERTICES = 0x100000;//a million
const int MAX_UNCULLED_SOLID_VERTICES = 0x100000;//a million
const int MAX_FLUID_VERTICES = 0x10000;//65 thousand
void Chunk::GenerateMesh() {
	if (UnCulledSolidVertices) 
	{
		DeleteMesh();
	}
	fp* CulledSolidVerticesContainer = DBG_NEW fp[MAX_CULLED_SOLID_VERTICES];
	fp* UnCulledSolidVerticesContainer = DBG_NEW fp[MAX_UNCULLED_SOLID_VERTICES];
	fp* FluidVerticesContainer = DBG_NEW fp[MAX_FLUID_VERTICES];
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
				if (block > 0)
				{
					if (block == transparentblock)
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
void Chunk::PlaceArtifact(int x, int y, int z, artifact artifact, const bool addaction = false)
{
	function* f = artifactfunctions[artifact];
	if(f)
	{
		*(int*)f->arguments[0]->var = x;
		*(int*)f->arguments[1]->var = y;
		*(int*)f->arguments[2]->var = z;
		*(bool*)f->arguments[3]->var = addaction;
		f->execute();
	}
	else {
		SetBlock(x, y, z,artifactblocks[artifact], addaction);
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
		artifact aartifact = Artifacts[i / 3];
		int TreeI = Positions[i++];
		int TreeJ = Positions[i++];
		int TreeK = Positions[i++] + 1;//origin 1 higher
		BlockIndex index = TreeI + TreeJ * StrideY + TreeK * StrideZ;
		int TreeX = TreeI + ScaleOffsetX;
		int TreeY = TreeJ + ScaleOffsetY;
		
		PlaceArtifact(TreeX, TreeY, TreeK,aartifact);
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
	DeleteData();
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