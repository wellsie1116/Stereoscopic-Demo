/*
 * tex.h
 *
 *  Created on: Oct 25, 2009
 *      Author: Kevin Wells
 */

#ifndef TEX_H_
#define TEX_H_

/**
 * This structure holds info about the TGA image before we load it into OpenGL.
 *
 * @author Tom Arnold
 */
typedef struct
{
	unsigned char depth;
	short int w;
	short int h;
	unsigned char* data;
} tga_data_t;


//loads a texture from the given filename
tga_data_t* tga_data_load(char* fn);


#endif /* TEX_H_ */
