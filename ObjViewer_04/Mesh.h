#pragma once

#include "Vertex.h"
#include "Material.h"

struct SubMesh
{
	uint32_t VAO;	// notez qu'il faut cr�er un VAO par SubMesh (VBO) quand bien meme on utilise le meme shader
	uint32_t VBO;	// ceci parceque l'identifiant du VBO est logiquement different a chaque fois
	uint32_t IBO;
	uint32_t verticesCount;
	uint32_t indicesCount;
};

struct Mesh
{
	SubMesh mesh;
	Material material;

	void Destroy();

	static bool ParseObj(Mesh* obj, const char* filepath);
};


