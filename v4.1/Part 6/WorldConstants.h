#include "GlobalFunctions.h"
#include "gamemode.h"
#include "biome.h"
//#include <glad/glad.h>
#pragma once
constexpr int TexturePixelSize = 0x10;
//constexpr int DataSize = 0x200;
//constexpr fp TextSize = (fp)TexturePixelSize / DataSize;
constexpr int TextSize = TexturePixelSize;
//constexpr fp LastRowY = 1 - TextSize;
constexpr int dir8x[8]{ 1,1,0,-1,-1,-1,0,1 };//the eight directions(x coördinate, counterclockwise)
constexpr int dir8y[8]{ 0,1,1,1,0,-1,-1,-1 };//the eight directions(y coördinate, counterclockwise)
#define VerticesCap 0x100000//a milion
//block data
//enumerator: each value = prev. value + 1
enum block : byte
{
	Air, Leaves, Glass, RedFlower, BlueFlower, YellowFlower, Tallgrass, Fire, Water, Lava, GlowStone, Dirt, Sand, Grass, Stone, OakWood, Ice, Bedrock, OakPlanks, Bricks, BookShelf, Furnace, Cobweb, ScorchedStone, Snow, BirchWood, BirchPlanks, PalmWood, PalmPlanks, CactusBlock, BlockCount
};
enum BlockType {
	Invisible, Solid, Cross, Fluid, Transparent
};
enum audio 
{
	auNothing, auWood, auStone, auSand, auGravel, auGrass
};

const fp MAX_NOISE_OUTPUT = 0.707f;//sqrt(0.5)
const fp NOISE_MULT = 1 / MAX_NOISE_OUTPUT;
const fp NOISE_MULT_HALF = .5f / MAX_NOISE_OUTPUT;


constexpr int ChunkScaleHor = 0x10,
ChunkScaleHorMin = ChunkScaleHor - 1,
ChunkScaleVert = 0x100,
ChunkScale2 = ChunkScaleHor * ChunkScaleHor,
ChunkScale3 = ChunkScale2 * ChunkScaleVert,
StrideY = ChunkScaleHor,
StrideZ = ChunkScale2;


const int CloudLevel = 127, Maxheight = 0xff, Bedrocklayers = 4,
MinGenHeight = 0,//SeaLevel - (Maxheight - SeaLevel) / 9.0f
MaxGenHeight = Maxheight, GenDiff = MaxGenHeight - MinGenHeight;

typedef byte BrightnessValue;
constexpr fp BrightnessValueToFp = 1.0f / 0xff;
typedef unsigned short BlockIndex;//an unsigned short that stores the position in a chunk
//typedef unsigned int BlockIndex;//an int that stores the position in a chunk

constexpr int blocktexturewidth = 0x10;
const std::wstring blockname[]
{
	L"air",
	L"leaves",
	L"glass",
	L"redflower",
	L"blueflower",
	L"yellowflower",
	L"tallgrass",
	L"fire",
	L"water",
	L"lava",
	L"glowstone",
	L"dirt",
	L"sand",
	L"grass",
	L"stone",
	L"oakwood",
	L"ice",
	L"bedrock",
	L"oakplanks",
	L"bricks",
	L"bookshelf",
	L"furnace",
	L"cobweb",
	L"scorchedstone",
	L"snow",
	L"birchwood",
	L"birchplanks",
	L"palmwood",
	L"palmplanks",
	L"cactusblock",
};
//texture sources:
//https://hammerchisel.wordpress.com/2017/11/23/woods/
//side x,y, top x, y, bottom xy
static const fp textureIndices[]
{
0,0,0,0,0,0,			//air
64,48,64,48,64,48,		//leaves
16,48,16,48,16,48,		//glass
192,0,0,0,0,0,			//red flower
16,256,0,0,0,0,			//blue flower
208,0,0,0,0,0,			//yellow flower
208,256,0,0,0,0,		//tall grass
304,48,0,0,0,0,			//fire
0,0,0,0,0,0,			//water
176,288,176,288,176,288,//lava
144,96,144,96,144,96,	//glowstone
32,0,32,0,32,0,			//dirt
32,16,32,16,32,16,		//sand
48,0,0,0,32,0,			//grass
16,0,16,0,16,0,			//stone
64,16,80,16,80,16,		//oak wood
48,64,48,64,48,64,		//Ice
16,16,16,16,16,16,		//bedrock
64,0,64,0,64,0,			//oak planks
112,0,112,0,112,0,		//bricks
48,32,80,16,80,16,		//bookshelf
224,48,208,48,208,48,	//furnace
176,0,0,0,0,0,			//cobweb
240,336,240,336,240,336,//scorched stone
32,64,32,64,32,64,		//snow
80,112,304,256,304,256,	//birch wood
112,192,112,192,112,192,//birch planks
64,112,288,256,288,256,	//palm wood
96,192,96,192,96,192,	//palm planks
96,64,80,64,80,64,		//cactus block
};
static const BlockType GetBlockType[]{
Invisible,		//air
Transparent,	//leaves
Transparent,	//glass
Cross,			//red flower
Cross,			//blue flower
Cross,			//yellow flower
Cross,			//tall grass
Cross,			//fire
Fluid,			//water
Fluid,			//lava
Solid,			//glowstone
Solid,			//dirt
Solid,			//sand
Solid,			//grass
Solid,			//stone
Solid,			//wood
Solid,			//Ice
Solid,			//bedrock
Solid,			//oak planks
Solid,			//bricks
Solid,			//bookshelf
Solid,			//furnace
Cross,			//cobweb
Solid,			//scorched stone
Solid,			//snow
Solid,			//birch wood
Solid,			//birch planks
Solid,			//palm wood
Solid,			//palm planks
Solid,			//cactus block
};
constexpr audio blockaudio[]
{
	auNothing,//air
	auGrass,//leaves
	auNothing,//glass
	auGrass,//red flower
	auGrass,//blue flower
	auGrass,//yellow flower
	auGrass,//tall grass
	auNothing,//fire
	auNothing,//water
	auNothing,//lava
	auNothing,//glowstone
	auNothing,//dirt
	auSand,//sand
	auGrass,//grass
	auStone,//stone
	auWood,//wood
	auNothing,//Ice
	auNothing,//bedrock
	auWood,//oak planks
	auStone,//bricks
	auNothing,//bookshelf
	auStone,//furnace
	auNothing,//cobweb
	auStone,//scorched stone
	auNothing,//snow
	auWood,//birch wood
	auWood,//birch planks
	auWood,//palm wood
	auWood,//palm planks
	auNothing,//cactus block
};
static const fp Opacity[]{
1,		//air
0.75f,	//leaves
0.9f,	//glass
0.9f,	//red flower
0.9f,	//blue flower
0.9f,	//yellow flower
0.85f,	//tall grass
1,		//fire
0.95f,	//water
1,		//lava
1,		//glowstone
0,		//dirt
0,		//sand
0,		//grass
0,		//stone
0,		//wood
0,		//Ice
0,		//bedrock
0,		//oak planks
0,		//bricks
0,		//bookshelf
0,		//furnace
0.9f,	//cobweb
0,		//scorched stone
0,		//snow
0,		//birch wood
0,		//birch planks
0,		//palm wood
0,		//palm planks
0,		//cactus block
};
static const fp GetBounceMultiplier[]
{
0,		//air
0,		//leaves
0,		//glass
0,		//red flower
0,		//blue flower
0,		//yellow flower
0,		//tall grass
0,		//fire
0,		//water
0,		//lava
0,		//glowstone
0,		//dirt
0,		//sand
0,		//grass
0,		//stone
0,		//wood
0,		//Ice
0,		//bedrock
0,		//oak planks
0,		//bricks
0,		//bookshelf
0,		//furnace
0,		//cobweb
0,		//scorched stone
0,		//snow
0,		//birch wood
0,		//birch planks
0,		//palm wood
0,		//palm planks
0,		//cactus block
};
static const bool IsRaycastVisible[]
{
false,	//air
true,	//leaves
true,	//glass
true,	//red flower
true,	//blue flower
true,	//yellow flower
true,	//tall grass
true,	//fire
false,	//water
false,	//lava
true,	//glowstone
true,	//dirt
true,	//sand
true,	//grass
true,	//stone
true,	//wood
true,	//Ice
true,	//bedrock
true,	//oak planks
true,	//bricks
true,	//bookshelf
true,	//furnace
true,	//cobweb
true,	//scorched stone
true,	//snow
true,	//birch wood
true,	//birch planks
true,	//palm wood
true,	//palm planks
true,	//cactus block
};
static const bool IsSolidBlock[]{
false,	//air
true,	//leaves
true,	//glass
false,	//red flower
false,	//blue flower
false,	//yellow flower
false,	//tall grass
false,	//fire
false,	//water
false,	//lava
true,	//glowstone
true,	//dirt
true,	//sand
true,	//grass
true,	//stone
true,	//wood
true,	//Ice
true,	//bedrock
true,	//oak planks
true,	//bricks
true,	//bookshelf
true,	//furnace
false,	//cobweb
true,	//scorched stone
true,	//snow
true,	//birch wood
true,	//birch planks
true,	//palm wood
true,	//palm planks
true,	//cactus block
};
static const bool DrawAllSides[]{
false,	//air
false,	//leaves
false,	//glass
false,	//red flower
false,	//blue flower
false,	//yellow flower
false,	//tall grass
false,	//fire
true,	//water
true,	//lava
true,	//glowstone
true,	//dirt
true,	//sand
true,	//grass
true,	//stone
true,	//wood
true,	//Ice
true,	//bedrock
true,	//oak planks
true,	//bricks
true,	//bookshelf
true,	//furnace
false,	//cobweb
true,	//scorched stone
true,	//snow
true,	//birch wood
true,	//birch planks
true,	//palm wood
true,	//palm planks
true,	//cactus block
};

const int Octaves = 6;//the amount of noise layers
const int Smoothness = 0x100;//smallest layer: 4
const fp FREQUENCY = 1.0f / Smoothness;

//indicates wether the block has air in it
static const bool HasAir[]{
true,	//air
false,	//leaves
false,	//glass
true,	//red flower
true,	//blue flower
true,	//yellow flower
true,	//tall grass
true,	//fire
false,	//water
false,	//lava
false,	//glowstone
false,	//dirt
false,	//sand
false,	//grass
false,	//stone
false,	//wood
false,	//Ice
false,	//bedrock
false,	//oak planks
false,	//bricks
false,	//bookshelf
false,	//furnace
true,	//cobweb
false,	//scorched stone
false,	//snow
false,	//birch wood
false,	//birch planks
false,	//palm wood
false,	//palm planks
false,	//cactus block
};
//indicates wether the block will be culled
static const bool cullbackface[]{
false,	//air
false,	//leaves
true,	//glass
false,	//red flower
false,	//blue flower
false,	//yellow flower
false,	//tall grass
false,	//fire
false,	//water
false,	//lava
true,	//glowstone
true,	//dirt
true,	//sand
true,	//grass
true,	//stone
true,	//wood
true,	//Ice
true,	//bedrock
true,	//oak planks
true,	//bricks
true,	//bookshelf
true,	//furnace
false,	//cobweb
true,	//scorched stone
true,	//snow
true,	//birch wood
true,	//birch planks
true,	//palm wood
true,	//palm planks
true,	//cactus block
};
//indicates wether the block is visible when surrounded
static const bool IsVisibleSurrounded[]{
true,	//air
true,	//leaves
true,	//glass
true,	//red flower
true,	//blue flower
true,	//yellow flower
true,	//tall grass
true,	//fire
true,	//water
true,	//lava
false,	//glowstone
false,	//dirt
false,	//sand
false,	//grass
false,	//stone
false,	//wood
false,	//Ice
false,	//bedrock
false,	//oak planks
false,	//bricks
false,	//bookshelf
false,	//furnace
true,	//cobweb
false,	//scorched stone
false,	//snow
false,	//birch wood
false,	//birch planks
false,	//palm wood
false,	//palm planks
false,	//cactus block
};

constexpr int TextIndiceCount = 6;//6 indices per texture
enum Artifact : byte {
	aOakTree,
	aBirchTree,
	aPalmTree,
	aSpruceTree,
	aRedFlower,
	aBlueFlower,
	aYellowFlower,
	aCactus,
	aPineTree,
	ArtifactCount
};
//x: artifact
//y: biome
//oak tree, birch tree, palm tree, spruce tree, red flower, blue flower, yellow flower, cactus, pine tree
static const int chance[ArtifactCount * BIOME_COUNT]
{
	0,	0,	0,	0,	0,	0,	0,	0,	0,		//ocean
	0,	0,	30,	30,	30,	30,	30,	10,	10,		//beach
	0,	0,	0,	0,	0,	0,	10,	10,	10,		//scorched
	0,	0,	0,	2,	2,	10,	10,	12,	12,		//bare
	2,	2,	2,	20,	20,	30,	40,	50,	50,		//tundra
	0,	0,	0,	10,	15,	30,	40,	42,	82,		//snow
	0,	0,	10,	10,	10,	10,	10,	30,	30,		//temperate desert
	10,	10,	10,	10,	30,	30,	30,	30,	30,		//shrubland
	10,	10,	10,	30,	50,	70,	90,	90,	110,	//taiga
	5,	10,	20,	20,	30,	30,	30,	30,	30,		//grassland
	30,	30,	50,	50,	90,	110,120,120,120,	//temperate deciduous forest
	20,	20,	40,	40,	70,	90,	100,101,101,	//temperate rain forest
	20,	20,	40,	40,	70,	90,	100,150,150,	//subtropical desert
	10,	30,	40,	40,	70,	90,	100,100,100,	//subtropical seasonal forest
	40,	40,	60,	60,	70,	80,	80,	85,	85,		//tropical rain forest
};
static const block TopBlock[BIOME_COUNT]
{
	Sand,			//ocean
	Sand,			//beach
	ScorchedStone,	//scorched
	Stone,			//bare
	Snow,			//tundra
	Snow,			//snow
	Sand,			//temperate desert
	Grass,			//shrubland
	Grass,			//taiga
	Grass,			//grassland
	Grass,			//temperate deciduous forest
	Grass,			//temperate rain forest
	Sand,			//subtropical desert
	Grass,			//subtropical seasonal forest
	Grass,			//tropical rain forest
};
static const block LayerBlock[BIOME_COUNT]
{
	Sand,			//ocean
	Sand,			//beach
	ScorchedStone,	//scorched
	Stone,			//bare
	Snow,			//tundra
	Stone,			//snow
	Sand,			//temperate desert
	Stone,			//shrubland
	Stone,			//taiga
	Dirt,			//grassland
	Dirt,			//temperate deciduous forest
	Dirt,			//temperate rain forest
	Sand,			//subtropical desert
	Dirt,			//subtropical seasonal forest
	Dirt,			//tropical rain forest
};