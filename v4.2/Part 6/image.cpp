#include "image.h"
inline Image::Image()
{

}
//COPIES THE POINTER, NOT THE COLORS
Image::Image(color* ptr, int w, int h, int channelcount)
{
	Width = w;
	Height = h;
	colors = ptr;
	this->ChannelCount = channelcount;
}

Image::~Image()
{
}

inline color Image::GetPixel(int x, int y)
{
	return *(colors + x + y * Width);
}

inline void Image::SetPixel(int x, int y, color color)
{
	*(colors + x + y * Width) = color;
}

Image* Image::FromFile(std::string path, const bool flip)
{
	int w = 0, h = 0, channels = 0;
	std::vector<char> data = readBMP(path, w, h, channels);
	byte* c4 = new byte[w * h * 4];
	if (channels == 3)
	{
		InsertChannel((byte*)data.data(), (byte*)data.data() + w * h * 3, c4, 3, 4, 0xff);//fill alpha channel
	}
	else 
	{
		memcpy(c4, data.data(), w * h * sizeof(color));
	}
	if (flip) //flip img vertically
	{
		int w4 = w * 4;
		byte* flipped = new byte[w4 * h];
		byte* ptr = flipped;
		byte* endptr = flipped + w4 * h;
		byte* srcptr = c4 + (h - 1) * w4;
		while (ptr < endptr) 
		{
			memcpy(ptr, srcptr, w4);
			ptr += w4;
			srcptr -= w4;
		}
		delete[] c4;
		c4 = flipped;
	}
	return new Image((color*)c4, w, h, channels);
}

void Image::Save(std::string path)
{
	//write image to file
//source:
//https://stackoverflow.com/questions/2654480/writing-bmp-image-in-pure-c-c-without-other-libraries
//padding MUST be included (round to 4 bytes)
	std::ofstream stream;//output file stream
	stream.open(path, std::ios::binary);
	if (stream.good()) {
		// mimeType = "image/bmp";
		stream.seekp(0, stream.beg);

		byte file[14]
		{
			'B','M', // magic
			0,0,0,0, // size in bytes
			0,0, // app data
			0,0, // app data
			40 + 14,0,0,0 // start of data offset
		};
		byte info[40]
		{
			40,0,0,0, // info hd size
			0,0,0,0, // width
			0,0,0,0, // heigth
			1,0, // number color planes
			24,0, // bits per pixel
			0,0,0,0, // compression is none
			0,0,0,0, // image bits size
			0x13,0x0B,0,0, // horz resoluition in pixel / m
			0xC3,0x03,0,0, // vert resolutions (0x03C3 = 96 dpi, 0x0B13 = 72 dpi)
			0,0,0,0, // #colors in pallete
			0,0,0,0, // #important colors
		};
		int padSize = //(4 - (Width * 3) % 4) % 4;
			(4 - 3 * Width % 4) % 4;//how many bytes to add to be dividable by four?
		int sizeData = Width * Height * 3 + Height * padSize;
		int sizeAll = sizeData + sizeof(file) + sizeof(info);
		int strideX = 3;
		int strideY = Width * strideX + padSize;

		file[2] = (byte)(sizeAll);
		file[3] = (byte)(sizeAll >> 8);
		file[4] = (byte)(sizeAll >> 16);
		file[5] = (byte)(sizeAll >> 24);

		info[4] = (byte)(Width);
		info[5] = (byte)(Width >> 8);
		info[6] = (byte)(Width >> 16);
		info[7] = (byte)(Width >> 24);

		info[8] = (byte)(Height);
		info[9] = (byte)(Height >> 8);
		info[10] = (byte)(Height >> 16);
		info[11] = (byte)(Height >> 24);

		info[20] = (byte)(sizeData);
		info[21] = (byte)(sizeData >> 8);
		info[22] = (byte)(sizeData >> 16);
		info[23] = (byte)(sizeData >> 24);

		stream.write((char*)file, sizeof(file));
		stream.write((char*)info, sizeof(info));

		byte pad[3] = { 0,0,0 };
		sizeof(byte);
		byte* ptr = (byte*)colors;
		//stream.write((char*)data, (strideY + padSize) * Height);
		for (int y = 0; y < Height; y++)
		{
			byte* endPtr = ptr + Width * strideX;
			byte* marginPtr = ptr + strideY;
			//switch r with b
			while (ptr < endPtr)
			{
				stream.write((char*)(ptr + 2), 1);
				stream.write((char*)(ptr + 1), 1);
				stream.write((char*)ptr, 1);
				ptr += strideX;
			}
			//for (int i = 0; i < padSize; i++) 
			//{
			//	stream.write((char*)new char[1]{ 0 }, 1);
			//}
			while (ptr < marginPtr)
			{
				stream.write((char*)ptr++, 1);
			}
		}
		stream.close();
	}
}