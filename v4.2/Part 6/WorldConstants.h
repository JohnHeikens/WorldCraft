#include "gamemode.h"
#include "include.h"
#include "modfunctions.h"
constexpr int TextIndiceCount = 6;//6 indices per texture

//#include <glad/glad.h>
#pragma once
//constexpr fp LastRowY = 1 - TextSize;
constexpr int dir8x[8]{ 1,1,0,-1,-1,-1,0,1 };//the eight directions(x coördinate, counterclockwise)
constexpr int dir8y[8]{ 0,1,1,1,0,-1,-1,-1 };//the eight directions(y coördinate, counterclockwise)
#define VerticesCap 0x100000//a milion
//block data
//enumerator: each value = prev. value + 1
typedef byte block;
typedef byte artifact;
typedef byte biome;

enum knownblocks
{
	air, water, dirt, grass
};
enum BlockType {
	Invisible, Solid, Cross, Fluid, Transparent, BlockTypeCount
};
enum audio 
{
	auNothing, auWood, auStone, auSand, auGravel, auGrass
};

extern Image* fullheart;
extern Image* halfheart;
extern Image* emptyheart;

//extern block* blocks;
extern artifact* artifacts;
extern std::vector<function*> artifactfunctions;
extern function* fnextframe;
extern function* fstart;
extern block* artifactblocks;
extern vec3 worldspawn;//if no other spawnpoints exist

extern int sealevel;
//constexpr int TexturePixelSize = 0x10;
//constexpr int DataSize = 0x200;
//constexpr fp TextSize = (fp)TexturePixelSize / DataSize;
extern int TextSize;

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
MinGenHeight = 0,//sealevel - (Maxheight - sealevel) / 9.0f
MaxGenHeight = Maxheight, GenDiff = MaxGenHeight - MinGenHeight;

typedef byte BrightnessValue;
constexpr fp BrightnessValueToFp = 1.0f / 0xff;
typedef unsigned short BlockIndex;//an unsigned short that stores the position in a chunk
//typedef unsigned int BlockIndex;//an int that stores the position in a chunk

extern std::wstring modfilename;

//arrays
extern std::wstring* blockname;
extern std::wstring* biomename;
extern std::wstring* artifactname;
extern fp* textureIndices;
extern BlockType* GetBlockType;
extern fp* Opacity;
extern fp* GetBounceMultiplier;
extern bool* IsRaycastVisible;
extern bool* IsSolidBlock;
extern bool* DrawAllSides;
extern bool* HasAir;
extern bool* cullbackface;
extern bool* IsVisibleSurrounded;
extern int* chance;
extern block* TopBlock;
extern block* LayerBlock;
extern color* aircolor;
extern fp* fogdistance;
extern int BlockCount, ArtifactCount;
extern block filloceanwith, fillgroundwith;
extern block transparentblock;
extern color* biomecolor;
extern int biomecount;
extern biome* biomemap;
extern int biomemapscale;
extern Texture* BlockTextures;

extern LayerNoiseSimplex* HeightNoise;
extern LayerNoiseSimplex* MoistureNoise;

extern int seed;
extern int ElevationPower, MoisturePower;
constexpr int blocktexturewidth = 0x10;

const std::wstring blocktypename[]
{
	L"invisible", L"solid", L"cross", L"fluid", L"transparent"
};
template<typename t>
inline int searcharray(t in, t* ptr, int count)
{
	int index = 0;
	while (index < count) 
	{
		if (in == ptr[index])return index;
		index++;
	}
	return -1;
}
inline BlockType ParseBlockType(std::wstring in) 
{
	for (int i = 0; i < BlockTypeCount; i++)
	{
		if (in == blocktypename[i])return (BlockType)i;
	}
}
inline biome ParseBiome(std::wstring in)
{
	for (biome i = 0; i < biomecount; i++)
	{
		if (in == biomename[i])return i;
	}
}
inline block GetBlockByID(std::wstring ID)
{
	long id;
	if (strtol(ID, id, 10))
	{
		return (block)id;
	}
	else
	{
		makelowercase(ID);
		for (int i = 0; i < BlockCount; i++)
		{
			if (ID == blockname[i])
			{
				return (block)i;
			}
		}	
		return -1;
	}
}
inline gamemode GetGamemodeByID(std::wstring ID)
{
	long id;
	if (strtol(ID, id, 10))
	{
		return (gamemode)id;
	}
	else
	{
		makelowercase(ID);
		for (int i = 0; i < gamemodecount; i++)
		{
			if (ID == gamemodename[i])
			{
				return (gamemode)i;
			}
		}
	}
}

inline biome GetBiomeClimate(cfp e, cfp m)
{
	return *(biomemap + ((int)(e * biomemapscale) * biomemapscale + (int)(m * biomemapscale)));
}
inline biome GetBiome(cint X, cint Y)
{
	cfp elevation = HeightNoise->Evaluate2d(X, Y);
	PowIntSimple(elevation, ElevationPower);
	cfp moisture = MoistureNoise->Evaluate2d(X, Y);
	PowIntSimple(moisture, MoisturePower);
	return GetBiomeClimate(elevation, moisture);
}

static class world 
{
public:
	inline static void CalculateWorldSpawn()
	{
		for (int i = 0; i < 100; i++) {

			//try random positions
			vec2 pos = vec2::getrotatedvector(RANDFP * math::PI2) * RANDFP * 0x100;
			int px = (int)pos.x, py = (int)pos.y;
			biome spawnbiome = GetBiome(px, py);
			if (IsSolidBlock[TopBlock[spawnbiome]])
			{
				//valid spawn
				fp elevation = HeightNoise->Evaluate2d(px, py);
				int z = (int)(PowIntSimple(elevation, ElevationPower) * GenDiff + MinGenHeight) + 1;
				if (z > sealevel) 
				{
					worldspawn = vec3(px + 0.5, py + 0.5, z);
					return;
				}
			}
		}
		MessageBox(NULL, L"could not find a world spawn", L"Error", MB_OK);

	}
	inline static void UnloadMod() 
	{
		delete[] chance;
		delete[] biomecolor;
		delete[] TopBlock;
		delete[] LayerBlock;
		delete[] artifactname;
		delete[] biomename;
		delete[] blockname;
		delete[] textureIndices;
		delete[] GetBlockType;
		delete[] Opacity;
		delete[] GetBounceMultiplier;
		delete[] IsRaycastVisible;
		delete[] IsSolidBlock;
		delete[] DrawAllSides;
		delete[] HasAir;
		delete[] cullbackface;
		delete[] IsVisibleSurrounded;
		delete[] aircolor;
		delete[] fogdistance;
	}
	inline static void LoadMod(std::wstring filename) 
	{
		if (chance) 
		{
			UnloadMod();
		}
		modfilename = filename;
		hsfreader reader = hsfreader::fromfile(filename);
		//other resources
		std::wstring folder = getdirectorypath(filename) + L"\\";
		hsfcontent otherresources = reader.getcontent(L"other files");
		//load block properties
		LoadBlocks(folder + otherresources.data[0]);

		//load images
		//load block textures
		BlockTextures = Image::FromFile(folder + otherresources.data[3], true);

		fullheart = Image::FromFile(folder + L"fullheart.bmp", true);
		halfheart = Image::FromFile(folder + L"halfheart.bmp", true);
		emptyheart = Image::FromFile(folder + L"emptyheart.bmp", true);
		//load major blocks
		hsfcontent majorblockcontent = reader.getcontent(L"major blocks");
		fillgroundwith = GetBlockByID(majorblockcontent.data[0]);
		filloceanwith = GetBlockByID(majorblockcontent.data[1]);
		//load noise maps
		for (int j = 0; j < 2; j++) {
			hsfcontent noisecontent;
			LayerNoiseSimplex** s;
			if (j == 0) 
			{
				noisecontent = reader.getcontent(L"elevation noise");
				s = &HeightNoise;
				ElevationPower = stoi(noisecontent.data[2]);
			}
			else 
			{
				noisecontent = reader.getcontent(L"moisture noise");
				s = &MoistureNoise;
				MoisturePower = stoi(noisecontent.data[2]);
			}
			fp min, max, frequency;
			int power, octaves;
			fp* weights;
			min = wcstof(noisecontent.data[0].c_str(), 0);
			max = wcstof(noisecontent.data[1].c_str(), 0);
			frequency = wcstof(noisecontent.data[3].c_str(), 0);
			octaves = stoi(noisecontent.data[4]);
			weights = DBG_NEW fp[octaves];
			for (int i = 0; i < octaves; i++)
			{
				weights[i] = wcstof(noisecontent.data[i + 5].c_str(), 0);
			}
			*s = DBG_NEW LayerNoiseSimplex(seed, weights, octaves, min, max, frequency);
			delete[] weights;
		}
		hsfcontent worldsettingcontent = reader.getcontent(L"world settings");
		if (worldsettingcontent.data.size() != 2)
		{
			MessageBox(NULL, L"wrong number of values in world settings", L"Error", MB_OK);
			throw 0;
		}
		else
		{
			sealevel = stoi(worldsettingcontent.data[0]);
			TextSize = stoi(worldsettingcontent.data[1]);

		}
		//load biome names
		hsfcontent biomenamecontent = reader.getcontent(L"biomes");
		biomecount = biomenamecontent.vertsize;
		biomename = DBG_NEW std::wstring[biomecount];
		for (int i = 0; i < biomecount; i++) 
		{
			biomename[i] = biomenamecontent.data[i];
		}
		//load biome colors
		hsfcontent biomecolorcontent = reader.getcontent(L"biome colors");
		biomecolor = DBG_NEW color[biomecount];
		for (int i = 0; i < biomecount; i++)
		{
			std::vector<std::wstring> components = split_string(biomecolorcontent.data[i].substr(4, biomecolorcontent.data[i].length() - 5), L",");
			biomecolor[i].a = 0xff;
			biomecolor[i].r = stoi(components[0]);
			biomecolor[i].g = stoi(components[1]);
			biomecolor[i].b = stoi(components[2]);
		}
		//load biome map
		Image* biometex = Image::FromFile(folder + otherresources.data[2], false);
		biomemapscale = biometex->Width;
		int biomemapsize = biomemapscale * biomemapscale;
		biomemap = DBG_NEW biome[biomemapsize];
		for (int i = 0; i < biomemapsize; i++) 
		{
			for (biome b = 0; b < biomecount; b++) 
			{
				int j = searcharray<color>(biometex->colors[i], biomecolor, biomecount);
				if (j == -1) 
				{
					MessageBox(NULL, (L"biome undefined at x = \"" + std::to_wstring(i % biomemapscale) + L", y = " + std::to_wstring(i/biomemapscale) + L"\"").c_str(), L"Error", MB_OK);
					throw 0;
				}
				biomemap[i] = j;
			}
		}
		delete[] biometex->colors;
		delete biometex;
		//load terrain blocks
		//top
		hsfcontent topblockcontent = reader.getcontent(L"top blocks");
		TopBlock = DBG_NEW block[biomecount];
		for (int i = 0; i < biomecount; i++) 
		{
			TopBlock[i] = GetBlockByID(topblockcontent.data[i]);
		}
		//layer
		hsfcontent layerblockcontent = reader.getcontent(L"layer blocks");
		LayerBlock = DBG_NEW block[biomecount];
		for (int i = 0; i < biomecount; i++)
		{
			LayerBlock[i] = GetBlockByID(layerblockcontent.data[i]);
		}
		//load artifact names
		hsfcontent artifactnamecontent = reader.getcontent(L"artifact name");
		ArtifactCount = artifactnamecontent.data.size();
		artifactname = DBG_NEW std::wstring[ArtifactCount];
		for (int i = 0; i < ArtifactCount; i++)
		{
			artifactname[i] = artifactnamecontent.data[i];
		}
		//load artifact chances
		hsfcontent artifactcontent = reader.getcontent(L"artifact chance");
		int tablesize = artifactcontent.data.size();
		chance = DBG_NEW int[tablesize];
		int total;
		//calculate chance table
		//they will be summed(chance to get less than this far)
		int* chanceptr = chance;
		int k = 0;
		if (artifactcontent.data.size() != biomecount * ArtifactCount) 
		{
			MessageBox(NULL, L"invalid chance array size", L"Error", MB_OK);
			throw 0;
		}
		for (int j = 0; j < biomecount; j++) 
		{
			int total = 0;
			for (int i = 0; i < ArtifactCount; i++, k++)
			{
				total += stoi(artifactcontent.data[k]);
				*chanceptr++ = total;
			}

		}
		//calculate world spawn
		CalculateWorldSpawn();

		//load artifact functions
		std::wstring code = StringToWString(readalltext(folder + otherresources.data[1]));
		LoadCode(code);
		if (fstart)
		{
			fstart->execute();
		}
	}

	inline static void LoadCode(std::wstring code)
	{
		compiler c = compiler();
		area* a = DBG_NEW area();
		a->Parent = NULL;
		//get
		//rand
		a->functions.push_back(DBG_NEW frand());
		a->functions.push_back(DBG_NEW frandfp());
		//blocks
		a->functions.push_back(DBG_NEW fgetblock());
		//set
		a->functions.push_back(DBG_NEW fsetblock());
		a->functions.push_back(DBG_NEW fsetblockrange());
		a->functions.push_back(DBG_NEW fsetsphere());
		for (int i = 0; i < BlockCount; i++)
		{
			variable* v = DBG_NEW variable();
			v->name = blockname[i];
			v->type = tint;
			v->var = (void*)DBG_NEW int (i);
			a->variables.push_back(v);
		}
		area* locala = c.Compile(code,a);
		artifactfunctions = std::vector<function*>(ArtifactCount);
		artifactblocks = DBG_NEW block[ArtifactCount];
		for (int i = 0; i < ArtifactCount; i++)
		{
			artifactfunctions[i] = locala->FindFunctionName(L"place" + artifactname[i]);
			artifactblocks[i] = GetBlockByID(artifactname[i]);
		}
		fnextframe = locala->FindFunctionName(L"nextframe");
		fstart = locala->FindFunctionName(L"start");
	}
	inline static void LoadBlocks(std::wstring filename)
	{
		hsfreader reader = hsfreader::fromfile(filename);
		hsfcontent blockcontent = reader.getcontent(L"blocks");
		BlockCount = blockcontent.vertsize;
		int horsize = blockcontent.data.size() / blockcontent.vertsize;
		blockname = DBG_NEW std::wstring[BlockCount];
		textureIndices = DBG_NEW fp[BlockCount * 6];
		GetBlockType = DBG_NEW BlockType[BlockCount];
		Opacity = DBG_NEW fp[BlockCount];
		GetBounceMultiplier = DBG_NEW fp[BlockCount];
		IsRaycastVisible = DBG_NEW bool[BlockCount];
		IsSolidBlock = DBG_NEW bool[BlockCount];
		DrawAllSides = DBG_NEW bool[BlockCount];
		HasAir = DBG_NEW bool[BlockCount];
		cullbackface = DBG_NEW bool[BlockCount];
		IsVisibleSurrounded = DBG_NEW bool[BlockCount];
		aircolor = DBG_NEW color[BlockCount];
		fogdistance = DBG_NEW fp[BlockCount];
		fp * texptr = textureIndices;
		for (int i = 0; i < BlockCount; i++)
		{
			int plus = i * horsize;
			blockname[i] = blockcontent.data[plus];
			*texptr++ = wcstof(blockcontent.data[plus + 1].c_str(), 0);//texture coordinates
			*texptr++ = wcstof(blockcontent.data[plus + 2].c_str(), 0);
			*texptr++ = wcstof(blockcontent.data[plus + 3].c_str(), 0);
			*texptr++ = wcstof(blockcontent.data[plus + 4].c_str(), 0);
			*texptr++ = wcstof(blockcontent.data[plus + 5].c_str(), 0);
			*texptr++ = wcstof(blockcontent.data[plus + 6].c_str(), 0);
			GetBlockType[i] = ParseBlockType(blockcontent.data[plus + 7]);
			Opacity[i] = wcstof(blockcontent.data[plus + 8].c_str(), 0);
			GetBounceMultiplier[i] = wcstof(blockcontent.data[plus + 9].c_str(), 0);
			IsRaycastVisible[i] = blockcontent.data[plus + 10] == L"true";
			IsSolidBlock[i] = blockcontent.data[plus + 11] == L"true";
			DrawAllSides[i] = blockcontent.data[plus + 12] == L"true";
			HasAir[i] = blockcontent.data[plus + 13] == L"true";
			cullbackface[i] = blockcontent.data[plus + 14] == L"true";
			IsVisibleSurrounded[i] = blockcontent.data[plus + 15] == L"true";
			//air color
			std::vector<std::wstring> components = split_string(blockcontent.data[plus + 16].substr(4, blockcontent.data[plus + 16].length() - 5), L",");
			if (components.size() > 3) //alpha
			{
				aircolor[i] =color( wcstof(components[0].c_str(), 0),
					wcstof(components[1].c_str(), 0),
					wcstof(components[2].c_str(), 0),
					wcstof(components[3].c_str(), 0));
			}
			else 
			{
				aircolor[i] = color( 1.0,
					wcstof(components[0].c_str(), 0),
					wcstof(components[1].c_str(), 0),
					wcstof(components[2].c_str(), 0));
			}
			fogdistance[i] = wcstof(blockcontent.data[plus + 17].c_str(), 0);
		}
		hsfcontent transparentblockcontent = reader.getcontent(L"transparent block");
		if (transparentblockcontent.name == L"") 
		{
			MessageBox(NULL, L"did not find what the transparent block is", L"Error", MB_OK);
			throw 0;
		}
		transparentblock = GetBlockByID(transparentblockcontent.data[0]);
	}
};
