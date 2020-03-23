#pragma once

#include "mat4.h"

struct Vertex
{
	vec3 position;			//  3x4 octets = 12
	vec3 normal;			// +3x4 octets = 24
	vec2 texcoords;			// +2x4 octets = 32
	uint8_t color[4];		// +4 octets = 36 non formellement nécessaire
};
