#include "biomeeditor.h"

biomeeditor::biomeeditor(int x, int y, int width, int height):form(x,y,width,height)
{
	obj = new GraphicsObject();
	Image* img = new Image(width, height);
	childs.push_back(new PictureBox(left, top, width, height, img));
	obj = GraphicsObject::FromImage(*img);
	biomes = new Biome[width * height];
}

void biomeeditor::Draw(const GraphicsObject& obj)
{
	for (int j = 0; j < height; j++) 
	{
		for (int i = 0; i < width; i++)
		{
			obj.FillPixel(i + left, j + top, BiomeColor[biomes[i + j * width]]);
		}

	}
}

void biomeeditor::Run()
{
}
