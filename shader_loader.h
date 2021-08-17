#ifndef SHADER_LOADER_H
#define SHADER_LOADER_H

#include <stdlib.h>
#include <stdio.h>
#include <GL/glew.h>

unsigned int sl_compile_and_link_shaders(char* vertex_shader_source_path, char* fragment_shader_source_path);

#endif
