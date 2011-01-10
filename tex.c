/*
 * tex.c
 *
 *  Created on: Oct 25, 2009
 *      Author: Kevin Wells
 */
#include "tex.h"

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

// Eliminate extraneous calls to sizeof().
const size_t size_uchar = sizeof(unsigned char);
const size_t size_sint = sizeof(short int);

/**
 * Load a TGA image into an info structure.
 *
 * @param fn The TGA image to load.
 * @return An info structure.
 * @author Tom Arnold
 */
tga_data_t*
tga_data_load(char* fn)
{
	tga_data_t* tga = NULL;
	FILE* fh = NULL;
	int md, t;

	fh = fopen(fn, "rb");

	if (!fh)
		return NULL;

	tga = (tga_data_t*)malloc(sizeof(tga_data_t));

	// Load information about the tga, aka the header.
	{
		// Seek to the width.
		fseek(fh, 12, SEEK_SET);
		fread(&tga->w, size_sint, 1, fh);

		// Seek to the height.
		fseek(fh, 14, SEEK_SET);
		fread(&tga->h, size_sint, 1, fh);

		// Seek to the depth.
		fseek(fh, 16, SEEK_SET);
		fread(&tga->depth, size_sint, 1, fh);
	}

	// Load the actual image data.
	{
		// Mode = components per pixel.
		md = tga->depth / 8;

		// Total bytes = h * w * md.
		t = tga->h * tga->w * md;

		// Allocate memory for the image data.
		tga->data = malloc(size_uchar * t);

		// Seek to the image data.
		fseek(fh, 18, SEEK_SET);
		fread(tga->data, size_uchar, t, fh);

		// We're done reading.
		fclose(fh);

		// Mode 3 = RGB, Mode 4 = RGBA
		// TGA stores RGB(A) as BGR(A) so
		// we need to swap red and blue.
		if (md >= 3)
		{
			unsigned char aux;
			int i;
			for (i = 0; i < t; i += md)
			{
				aux = tga->data[i];
				tga->data[i] = tga->data[i + 2];
				tga->data[i + 2] = aux;
			}
		}
	}

	return tga;
}
