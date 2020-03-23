#pragma once

#include "Vertex.h"
#include "Material.h"

struct SubMesh
{
	uint32_t VBO;
	uint32_t IBO;
	uint32_t vertexCount;
	uint32_t indicesCount;
};

struct Mesh
{
	SubMesh mesh;
	Material material;

	void Destroy();

	static bool ParseObj(Mesh* obj, const char* filepath);
};


