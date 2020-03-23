
#include <iostream>

#include "tiny_obj_loader.h"

#include "OpenGLcore.h"
#include "Material.h"
#include "Mesh.h"
#include "Texture.h"

// materiau par defaut (couleur ambiante, couleur diffuse, couleur speculaire, shininess, tex ambient, tex diffuse, tex specular)
Material Material::defaultMaterial = { { 0.2f, 0.2f, 0.2f }, { 1.f, 0.f, 1.f }, { 1.f, 1.f, 1.f }, 256.f, 0, 1, 0 };

void Mesh::Destroy()
{
	// On n'oublie pas de détruire les objets OpenGL
	DeleteBufferObject(mesh.VBO);
}

bool Mesh::ParseObj(Mesh* obj, const char* filepath)
{
	std::string warning, error;

	memset(obj, 0, sizeof(Mesh));

	std::map<std::string, int> material_map;
	std::vector<tinyobj::material_t> materials;
	std::string mtlPath = filepath;

	{
		size_t off = mtlPath.rfind("/");
		mtlPath.resize(off);
		std::vector<tinyobj::shape_t> shapes;
		tinyobj::attrib_t attrib;

		bool ok = tinyobj::LoadObj(&attrib, &shapes, &materials, &warning, &error, filepath, mtlPath.c_str());
		if (warning.length())
			std::cout << "[warning]: " << warning << std::endl;
		if (error.length())
			std::cout << "[error]: " << error << std::endl;

		obj->material = Material::defaultMaterial;

		// On ne gère qu'une seule shape dans cet exemple
		// note: attention à ne pas utiliser memset avec des classes polymorphiques (virtual) 
		// vous risquez d'écraser les pointeurs vers la table virtuelle (vtable)
		memset(&obj->mesh, 0, sizeof(SubMesh));

		const tinyobj::shape_t& shape = shapes.front();

		{
			// buffer temporaire, on va tout stocker côté GPU
			Vertex* vertices = new Vertex[shape.mesh.indices.size()];
			obj->mesh.vertexCount = 0;

			int materialId = -1;
			int faceId = 0;
			for (const tinyobj::index_t& index : shape.mesh.indices)
			{
				Vertex &v = vertices[obj->mesh.vertexCount];

				v.position.x = attrib.vertices[3 * index.vertex_index + 0];
				v.position.y = attrib.vertices[3 * index.vertex_index + 1];
				v.position.z = attrib.vertices[3 * index.vertex_index + 2];

				if (index.normal_index > -1) {
					v.normal.x = attrib.normals[3 * index.normal_index + 0];
					v.normal.y = attrib.normals[3 * index.normal_index + 1];
					v.normal.z = attrib.normals[3 * index.normal_index + 2];
				}
				else
				{
					// todo : générer des normales
					bool useSmoothingGroup = false;
					if (shape.mesh.smoothing_group_ids.size() != 0)
						useSmoothingGroup = true;
				}

				v.texcoords = {0.f, 0.f};
				if (index.texcoord_index > -1) {
					v.texcoords.x = attrib.texcoords[2 * index.texcoord_index + 0];
					v.texcoords.y = attrib.texcoords[2 * index.texcoord_index + 1];
					// Important à savoir
					// contrairement à OpenGL, les textures dans les logiciels 2D et 3D
					// ont pour origine le coin haut-gauche de l'écran
					// Il est donc souvent nécessaire de convertir la cordonnées v (y)
					// en C++ ou dans le shader, par exemple ici 
					v.texcoords.y = 1.f - v.texcoords.y;
				}

				// tinyobj loader affecte du blanc par defaut lorsqu'il ne trouve pas de couleur
				// c'est généralement le cas car les couleurs sont une extension non standard du format OBJ
				// ce qui rend cet attribut purement optionnel
				// notez que les couleurs sont volontairement converties en RGBA8 pour gagner de la place en memoire
				v.color[0] = uint8_t(attrib.colors[3 * index.vertex_index + 0] * 255.99f);
				v.color[1] = uint8_t(attrib.colors[3 * index.vertex_index + 1] * 255.99f);
				v.color[2] = uint8_t(attrib.colors[3 * index.vertex_index + 2] * 255.99f);
				v.color[3] = 255;

				++obj->mesh.vertexCount;
				faceId++;
			}

			// notez que je ne cree pas le VAO ici
			// Un VAO fait le lien entre le VBO (+ EBO/IBO) et les attributs d'un vertex shader
			obj->mesh.VBO = CreateBufferObject(BufferType::VBO, sizeof(Vertex) * obj->mesh.vertexCount, vertices);
			
			// important, bien libérer la mémoire des buffers temporaires
			delete[] vertices;
		}
	}

	return true;
}
