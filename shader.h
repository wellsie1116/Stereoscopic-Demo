/*
 * shader.h
 *
 *  Created on: Oct 31, 2009
 *      Author: Kevin Wells
 */

#ifndef SHADER_H_
#define SHADER_H_

/**
 * Attempts to load a shader program from the given files and will call the init callback if the program links successfully
 */
int shader_init(const char* vShaderFile, const char* fShaderFile, void (*initCallback)(int program));

#endif /* SHADER_H_ */
