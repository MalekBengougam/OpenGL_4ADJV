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

struct Framebuffer
{
	uint32_t FBO;
	uint32_t colorBuffer;
	uint32_t depthBuffer;
	uint16_t width;
	uint16_t height;

	Framebuffer() : FBO(0), colorBuffer(0), depthBuffer(0) {}

	void CreateFramebuffer(const uint32_t w, const uint32_t h, bool useDepth = false)
	{
		width = (uint16_t)w;
		height = (uint16_t)h;

		// on bascule sur le sampler 0 (par defaut)
		glActiveTexture(GL_TEXTURE0);
		// creation de la texture servant de color buffer 
		glGenTextures(1, &colorBuffer);
		glBindTexture(GL_TEXTURE_2D, colorBuffer);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		if (useDepth) {
			glGenTextures(1, &depthBuffer);
			glBindTexture(GL_TEXTURE_2D, depthBuffer);
			// format interne (3eme param) indique le format de stockage en memoire video, combine usage et taille des données
			// format (externe, 7eme et 8eme param) indique le format des donnees en RAM
			// notez que si le format interne est different du format les pilotes OpenGL peuvent proceder a une conversion couteuse
			// sauf en OpenGL ES / WebGL ou les conversions sont interdites
			// le format interne d'un depth buffer peut etre GL_DEPTH_COMPONENT16/24/32 en int, ou GL_DEPTH_COMPONENT32F en float
			// le format doit correspondre le plus possible ici GL_DEPTH_COMPONENT + GL_UNSIGNED_INT (GL_FLOAT si le format interne est GL_DEPTH_COMPONENT32F) 
			glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, width, height, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, nullptr);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		}

		// Cette ligne n'est pas necessaire ici, mais il s'agit de montrer que le FBO ne necessite pas qu'une texture
		// soit Bind pour etre utilisable comme attachment
		glBindTexture(GL_TEXTURE_2D, 0);

		glGenFramebuffers(1, &FBO);
		glBindFramebuffer(GL_FRAMEBUFFER, FBO);
		// On attache ensuite le lod 0 (dernier param) de la texture 'colorBuffer' (4eme param) qui est de type GL_TEXTURE_2D (3eme param)
		// comme 'color attachment #0' (2eme param) de notre FBO precedemment bind comme GL_FRAMEBUFFER (1er param)
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorBuffer, 0);

		if (useDepth) {
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthBuffer, 0);
		}

		GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if (status != GL_FRAMEBUFFER_COMPLETE) {
			std::cout << "Framebuffer invalide, code erreur = " << status << std::endl;
		}
	}

	void DestroyFramebuffer()
	{
		if (depthBuffer)
			glDeleteTextures(1, &depthBuffer);
		if (colorBuffer)
			glDeleteTextures(1, &colorBuffer);
		if (FBO)
			glDeleteFramebuffers(1, &FBO);
		depthBuffer = 0;
		colorBuffer = 0;
		FBO = 0;
	}

	void EnableRender()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, FBO);
		glViewport(0, 0, width, height);
	}

	// force le rendu vers le backbuffer
	static void RenderToBackBuffer(const uint32_t w = 0, const uint32_t h = 0)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		if (w != 0 && h != 0)
			glViewport(0, 0, w, h);
	}
};

struct Application
{
	Mesh* object;
	uint32_t quadVAO;

	GLShader opaqueShader;
	GLShader copyShader;			// remplace glBlitFrameBuffer

	// dimensions du back buffer / Fenetre
	int32_t width;
	int32_t height;

	Framebuffer offscreenBuffer;	// rendu hors ecran

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
		copyShader.LoadVertexShader("copy.vs.glsl");
		copyShader.LoadFragmentShader("copy.fs.glsl");
		copyShader.Create();

		object = new Mesh;

		Mesh::ParseObj(object, "../data/lightning/lightning_obj.obj");

		int32_t program = opaqueShader.GetProgram();
		glUseProgram(program);
		// on connait deja les attributs que l'on doit assigner dans le shader
		int32_t positionLocation = glGetAttribLocation(program, "a_Position");
		int32_t normalLocation = glGetAttribLocation(program, "a_Normal");
		int32_t texcoordsLocation = glGetAttribLocation(program, "a_TexCoords");
		int32_t colorLocation = glGetAttribLocation(program, "a_Color");

		for (uint32_t i = 0; i < object->meshCount; i++)
		{
			SubMesh& mesh = object->meshes[i];

			glGenVertexArrays(1, &mesh.VAO);
			glBindVertexArray(mesh.VAO);

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.IBO);

			glBindBuffer(GL_ARRAY_BUFFER, mesh.VBO);
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
			DeleteBufferObject(mesh.VBO);
			DeleteBufferObject(mesh.IBO);
		}


		// force le framebuffer sRGB
		glEnable(GL_FRAMEBUFFER_SRGB);

		offscreenBuffer.CreateFramebuffer(width, height, true);
		
		{
			vec2 quad[] = { {-1.f, 1.f}, {-1.f, -1.f}, {1.f, 1.f}, {1.f, -1.f} };

			// VAO du carré plein ecran pour le shader de copie
			glGenVertexArrays(1, &quadVAO);
			glBindVertexArray(quadVAO);
			uint32_t vbo = CreateBufferObject(BufferType::VBO, sizeof(quad), quad);

			program = copyShader.GetProgram();
			glUseProgram(program);
			int32_t positionLocation = glGetAttribLocation(program, "a_Position");
			glVertexAttribPointer(positionLocation, 2, GL_FLOAT, false, sizeof(vec2), 0);
			glEnableVertexAttribArray(positionLocation);

			// maintenant que le VAO a enregistre le detail des attributs ainsi que la reference du VBO
			// on peut supprimer ce dernier car il ne nous servira plus de maniere explicite
			// attention a toujours desactiver les VAO avant d'agir sur un BO
			glBindVertexArray(0);
			DeleteBufferObject(vbo);
		}

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glUseProgram(0);
	}

	// la scene est rendue hors ecran
	void RenderOffscreen()
	{
		offscreenBuffer.EnableRender();

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

		// calcul des matrices model (une simple rotation), view (une translation inverse) et projection
		// ces matrices sont communes à tous les SubMesh
		mat4 world, view, perspective;
		world.rotationUp((float)glfwGetTime());
		vec3 position = { 0.f, 0.f, -100.f };
		view.translation(position);
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
		glUniform3fv(camPosLocation, 1, &position.x);

		for (uint32_t i = 0; i < object->meshCount; i++)
		{
			SubMesh& mesh = object->meshes[i];
			Material& mat = mesh.materialId > -1 ? object->materials[mesh.materialId] : Material::defaultMaterial;
			glUniform3fv(ambientLocation, 1, &mat.ambientColor.x);
			glUniform3fv(diffuseLocation, 1, &mat.diffuseColor.x);
			glUniform3fv(specularLocation, 1, &mat.specularColor.x);
			glUniform1f(shininessLocation, mat.shininess);

			// glActiveTexture() n'est pas strictement requis ici car nous n'avons qu'une texture à la fois
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, mat.diffuseTexture);

			// bind implicitement les VBO et IBO rattaches, ainsi que les definitions d'attributs
			glBindVertexArray(mesh.VAO);
			// dessine les triangles
			glDrawElements(GL_TRIANGLES, object->meshes[i].indicesCount, GL_UNSIGNED_INT, 0);
		}
	}

	void Render()
	{
		glEnable(GL_DEPTH_TEST);	// Active le test de profondeur (3D)
		RenderOffscreen();

		glDisable(GL_DEPTH_TEST);	// desactive le test de profondeur (2D)
		// on va maintenant dessiner un quadrilatere plein ecran
		// pour copier (sampler et inscrire dans le backbuffer) le color buffer du FBO
		Framebuffer::RenderToBackBuffer(width, height);

		uint32_t program = copyShader.GetProgram();
		glUseProgram(program);
		
		// on indique au shader que l'on va bind la texture sur le sampler 0 (TEXTURE0)
		// pas necessaire techniquement car c'est le sampler par defaut
		int32_t samplerLocation = glGetUniformLocation(program, "u_Texture");
		glUniform1i(samplerLocation, 0);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, offscreenBuffer.colorBuffer);

		glBindVertexArray(quadVAO);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	}

	void Resize(int w, int h)
	{
		if (width != w || height != h)
		{
			width = w;
			height = h;
			// le plus simple est de detruire et recreer tout
			offscreenBuffer.DestroyFramebuffer();
			offscreenBuffer.CreateFramebuffer(width, height, true);
		}
	}

	void Shutdown()
	{
		glDeleteVertexArrays(1, &quadVAO);
		quadVAO = 0;
		
		object->Destroy();
		delete object;

		// On n'oublie pas de détruire les objets OpenGL

		Texture::PurgeTextures();
		
		copyShader.Destroy();
		opaqueShader.Destroy();
	}
};


void ResizeCallback(GLFWwindow* window, int w, int h)
{
	Application* app = (Application *)glfwGetWindowUserPointer(window);
	app->Resize(w, h);
}

void MouseCallback(GLFWwindow* , int, int, int) {

}


int main(int argc, const char* argv[])
{
	GLFWwindow* window;

	/* Initialize the library */
	if (!glfwInit())
		return -1;

	/* Create a windowed mode window and its OpenGL context */
	window = glfwCreateWindow(960, 720, "OBJ Viewer FBO (Lighting FF-XIII)", NULL, NULL);
	if (!window)
	{
		glfwTerminate();
		return -1;
	}

	Application app;

	/* Make the window's context current */
	glfwMakeContextCurrent(window);

	// Passe l'adresse de notre application a la fenetre
	glfwSetWindowUserPointer(window, &app);

	// inputs
	glfwSetMouseButtonCallback(window, MouseCallback);

	// recuperation de la taille de la fenetre
	glfwGetWindowSize(window, &app.width, &app.height);

	// definition de la fonction a appeler lorsque la fenetre est redimensionnee
	// c'est necessaire afin de redimensionner egalement notre FBO
	glfwSetWindowSizeCallback(window, &ResizeCallback);

	// toutes nos initialisations vont ici
	app.Initialize();

	/* Loop until the user closes the window */
	while (!glfwWindowShouldClose(window))
	{
		/* Render here */
		glfwGetWindowSize(window, &app.width, &app.height);

		app.Render();

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