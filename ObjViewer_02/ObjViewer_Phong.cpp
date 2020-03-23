//
//
//

#include "OpenGLcore.h"
#include <GLFW/glfw3.h>

// les repertoires d'includes sont:
// ../libs/glfw-3.3/include			fenetrage
// ../libs/glew-2.1.0/include		extensions OpenGL
// ../libs/stb						gestion des images (entre autre)

// les repertoires des libs sont (en 64-bit):
// ../libs/glfw-3.3/lib-vc2015
// ../libs/glew-2.1.0/lib/Release/x64

// Pensez a copier les dll dans le repertoire x64/Debug, cad:
// glfw-3.3/lib-vc2015/glfw3.dll
// glew-2.1.0/bin/Release/x64/glew32.dll		si pas GLEW_STATIC

// _WIN32 indique un programme Windows
// _MSC_VER indique la version du compilateur VC++
#if defined(_WIN32) && defined(_MSC_VER)
#pragma comment(lib, "glfw3dll.lib")
#pragma comment(lib, "glew32s.lib")			// glew32.lib si pas GLEW_STATIC
#pragma comment(lib, "opengl32.lib")
#elif defined(__APPLE__)
#elif defined(__linux__)
#endif

#include <iostream>
#include <fstream>

#include "../common/GLShader.h"
#include "mat4.h"
#include "Texture.h"
#include "Mesh.h"


struct Application
{
	Mesh* object;
	GLShader opaqueShader;

	void Initialize()
	{
		GLenum error = glewInit();
		if (error != GLEW_OK) {
			std::cout << "erreur d'initialisation de GLEW!"
				<< std::endl;
		}

		std::cout << "Version : " << glGetString(GL_VERSION) << std::endl;
		std::cout << "Vendor : " << glGetString(GL_VENDOR) << std::endl;
		std::cout << "Renderer : " << glGetString(GL_RENDERER) << std::endl;

		// on utilise un texture manager afin de ne pas recharger une texture deja en memoire
		// de meme on va definir une ou plusieurs textures par defaut
		Texture::SetupManager();

		opaqueShader.LoadVertexShader("opaque.vs.glsl");
		opaqueShader.LoadFragmentShader("opaque.fs.glsl");
		opaqueShader.Create();

		object = new Mesh;

		Mesh::ParseObj(object, "../data/suzanne.obj");

		int32_t program = opaqueShader.GetProgram();
		glUseProgram(program);
		// on connait deja les attributs que l'on doit assigner dans le shader
		int32_t positionLocation = glGetAttribLocation(program, "a_Position");
		int32_t normalLocation = glGetAttribLocation(program, "a_Normal");
		int32_t texcoordsLocation = glGetAttribLocation(program, "a_TexCoords");
		int32_t colorLocation = glGetAttribLocation(program, "a_Color");

		glGenVertexArrays(1, &object->mesh.VAO);
		glBindVertexArray(object->mesh.VAO);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, object->mesh.IBO);

		glBindBuffer(GL_ARRAY_BUFFER, object->mesh.VBO);
		// Specifie la structure des donnees envoyees au GPU
		glVertexAttribPointer(positionLocation, 3, GL_FLOAT, false, sizeof(Vertex), 0);
		glVertexAttribPointer(normalLocation, 3, GL_FLOAT, false, sizeof(Vertex), (void*)offsetof(Vertex, normal));
		glVertexAttribPointer(texcoordsLocation, 2, GL_FLOAT, false, sizeof(Vertex), (void*)offsetof(Vertex, texcoords));
		glVertexAttribPointer(colorLocation, 3, GL_UNSIGNED_BYTE, true, sizeof(Vertex), (void*)offsetof(Vertex, color));
		// indique que les donnees sont sous forme de tableau
		glEnableVertexAttribArray(positionLocation);
		glEnableVertexAttribArray(normalLocation);
		glEnableVertexAttribArray(texcoordsLocation);
		glEnableVertexAttribArray(colorLocation);

		// ATTENTION, les instructions suivantes ne detruisent pas immediatement les VBO/IBO
		// Ceci parcequ'ils sont référencés par le VAO. Ils ne seront détruit qu'au moment
		// de la destruction du VAO
		glBindVertexArray(0);
		DeleteBufferObject(object->mesh.VBO);
		DeleteBufferObject(object->mesh.IBO);

		glUseProgram(0);
	}

	void Render(const uint32_t width, const uint32_t height)
	{
		glClearColor(0.973f, 0.514f, 0.475f, 1.f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Defini le viewport en pleine fenetre
		glViewport(0, 0, width, height);

		// En 3D il est usuel d'activer le depth test pour trier les faces, et cacher les faces arrières
		// Par défaut OpenGL considère que les faces anti-horaires sont visibles (Counter Clockwise, CCW)
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);

		uint32_t program = opaqueShader.GetProgram();
		glUseProgram(program);

		// calcul des matrices world (une simple rotation), view et projection
		// ces matrices sont communes à tous les SubMesh
		mat4 world, view, perspective;
		// objet dans le monde
		world.rotationUp((float)glfwGetTime());
		// camera
		// notez que la camera est positionnee en repere main droite avec +Z hors ecran
		vec3 cameraPosition = { 0.f, 0.f, 3.f };
		// l'inverse d'une translation T(x) est T(-x)
		vec3 inverseCameraPosition{ -cameraPosition.x, -cameraPosition.y, -cameraPosition.z };
		// la view matrix contient alors la translation a appliquer aux objets comme si la camera se trouvait a l'origine
		view.translation(inverseCameraPosition);
		// projection
		perspective.perspective(45.f, (float)width / (float)height, 0.1f, 1000.f);

		int32_t worldLocation = glGetUniformLocation(program, "u_WorldMatrix");
		glUniformMatrix4fv(worldLocation, 1, false, world.m);
		int32_t viewLocation = glGetUniformLocation(program, "u_ViewMatrix");
		glUniformMatrix4fv(viewLocation, 1, false, view.m);
		int32_t projectionLocation = glGetUniformLocation(program, "u_ProjectionMatrix");
		glUniformMatrix4fv(projectionLocation, 1, false, perspective.m);

		// On va maintenant affecter les valeurs du matériau à chaque SubMesh
		// On peut éventuellement optimiser cette boucle en triant les SubMesh par materialID
		// On ne devra modifier les uniformes que lorsque le materialID change
		int32_t ambientLocation = glGetUniformLocation(program, "u_Material.AmbientColor");
		int32_t diffuseLocation = glGetUniformLocation(program, "u_Material.DiffuseColor");
		int32_t specularLocation = glGetUniformLocation(program, "u_Material.SpecularColor");
		int32_t shininessLocation = glGetUniformLocation(program, "u_Material.Shininess");

		// position de la camera
		int32_t camPosLocation = glGetUniformLocation(program, "u_CameraPosition");
		glUniform3fv(camPosLocation, 1, &cameraPosition.x);

		SubMesh& mesh = object->mesh;
		glUniform3fv(ambientLocation, 1, &object->material.ambientColor.x);
		glUniform3fv(diffuseLocation, 1, &object->material.diffuseColor.x);
		glUniform3fv(specularLocation, 1, &object->material.specularColor.x);
		glUniform1f(shininessLocation, object->material.shininess);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, object->material.diffuseTexture);

		// bind implicitement les VBO et IBO rattaches, ainsi que les definitions d'attributs
		glBindVertexArray(object->mesh.VAO);
		// dessine les triangles
		glDrawElements(GL_TRIANGLES, object->mesh.indicesCount, GL_UNSIGNED_INT, 0);
	}

	void Shutdown()
	{
		object->Destroy();
		delete object;

		// On n'oublie pas de détruire les objets OpenGL

		Texture::PurgeTextures();

		opaqueShader.Destroy();
	}
};

void KeyboardCallback(GLFWwindow* , int, int , int, int )
{

}

int main(void)
{
	GLFWwindow* window;

	/* Initialize the library */
	if (!glfwInit())
		return -1;

	/* Create a windowed mode window and its OpenGL context */
	window = glfwCreateWindow(960, 720, "OBJ Viewer avec speculaire type Phong", NULL, NULL);
	if (!window)
	{
		glfwTerminate();
		return -1;
	}

	Application app;

	/* Make the window's context current */
	glfwMakeContextCurrent(window);
	glfwSetWindowUserPointer(window, &app);

	// inputs
	glfwSetKeyCallback(window, KeyboardCallback);

	// toutes nos initialisations vont ici
	app.Initialize();

	/* Loop until the user closes the window */
	while (!glfwWindowShouldClose(window))
	{
		/* Render here */
		int width, height;
		glfwGetWindowSize(window, &width, &height);

		app.Render(width, height);

		/* Swap front and back buffers */
		glfwSwapBuffers(window);

		/* Poll for and process events */
		glfwPollEvents();
	}

	// ne pas oublier de liberer la memoire etc...
	app.Shutdown();

	glfwTerminate();
	return 0;
}