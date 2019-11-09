#pragma once
constexpr int BIOME_SMOOTHNESS = 0x100;//0x10 chunks
const fp BIOME_FREQUENCY = 1.0f / BIOME_SMOOTHNESS;
constexpr int BIOME_OCTAVES = 0x6;
enum Biome
{
	OCEAN,
	BEACH,
	SCORCHED,
	BARE,
	TUNDRA,
	SNOW,
	TEMPERATE_DESERT,
	SHRUBLAND,
	TAIGA,
	GRASSLAND,
	TEMPERATE_DECIDUOUS_FOREST,
	TEMPERATE_RAIN_FOREST,
	SUBTROPICAL_DESERT,
	TROPICAL_SEASONAL_FOREST,
	TROPICAL_RAIN_FOREST,
	BIOME_COUNT
};
const color BiomeColor[BIOME_COUNT]
{
	color(67,67,122),	//ocean
	color(160,145,119),	//beach
	color(85,85,85),	//scorched
	color(136,136,136),	//bare
	color(188,188,170),	//tundra
	color(222,222,229),	//snow
	color(201,210,155),	//temperate desert
	color(136,153,119),	//shrubland
	color(153,171,119),	//taiga
	color(136,171,85),	//grassland
	color(103,147,89),	//temperate deciduous forest
	color(67,136,85),	//temperate rain forest
	color(210,185,139),	//subtropical desert
	color(86,153,68),	//subtropical seasonal forest
	color(51,119,85),	//tropical rain forest
};
