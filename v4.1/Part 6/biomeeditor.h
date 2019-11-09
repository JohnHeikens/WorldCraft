#pragma once
#include "include.h"
#include "WorldConstants.h"
class biomeeditor: public form
{
public:
	GraphicsObject* obj;
	Biome* biomes;
	biomeeditor(int x, int y, int width, int height);
	void Draw(const GraphicsObject& obj);
	void Run();
};
