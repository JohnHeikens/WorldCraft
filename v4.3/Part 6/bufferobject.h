#include <GlobalFunctions.h>
#pragma once
template<typename t>
struct bufferobject 
{
	t* buffer;
	int size;//the length of the buffer in t's
	int stride;//the step to take in t's
	void Resize(int newsize, const bool keep = false)
	{
		t* newbuffer = new t[newsize];
		if (keep) 
		{
			memcpy(newbuffer,buffer,sizeof(t) * size)
		}
		size = newsize;
		delete[] buffer;
	}
};