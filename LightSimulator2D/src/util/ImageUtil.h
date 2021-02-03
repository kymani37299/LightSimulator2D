#pragma once

#define STB_IMAGE_IMPLEMENTATION

#include "stb/stb_image.h"

struct ImageData
{
	unsigned char* data;
	int height;
	int width;
	int bpp;
};

namespace ImageUtil
{
	ImageData* LoadImage(const char* path, bool flip = true)
	{
		stbi_set_flip_vertically_on_load(flip);
		int width,height,bpp;
		unsigned char* image_data = stbi_load(path, &width, &height, &bpp, 4);
		return new ImageData{ image_data,height,width,bpp };
	}

	void FreeImage(unsigned char* image_data)
	{
		stbi_image_free(image_data);
	}
}