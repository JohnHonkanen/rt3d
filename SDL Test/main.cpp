// Based loosly on the first triangle OpenGL tutorial
// http://www.opengl.org/wiki/Tutorial:_OpenGL_3.1_The_First_Triangle_%28C%2B%2B/Win%29
// This program will render two triangles
// Most of the OpenGL code for dealing with buffer objects, etc has been moved to a 
// utility library, to make creation and display of mesh objects as simple as possible

// Windows specific: Uncomment the following line to open a console window for debug output
#if _DEBUG
#pragma comment(linker, "/subsystem:\"console\" /entry:\"WinMainCRTStartup\"")
#endif

#include "rt3d.h"
#include "glm\glm.hpp"
#include "glm\gtc\matrix_transform.hpp"
#include "glm\gtc\type_ptr.hpp"
#include <stack>

using namespace std;

#define DEG_TO_RAD 0.017453293

stack<glm::mat4> mvStack;


// Globals
// Real programs don't use globals :-D
// Data would normally be read from files
GLuint cubeVertCount = 36;
GLfloat cubeVerts[] = { 
0, 0, 0,        1, 1, 0,        1, 0, 0,        0, 0, 0,        0, 1, 0,
1, 1, 0,        0, 0, 0,        0, 1, 1,        0, 1, 0,        0, 0, 0,
0, 0, 1,        0, 1, 1,        0, 1, 0,        1, 1, 1,        1, 1, 0,
0, 1, 0,        0, 1, 1,        1, 1, 1,        1, 0, 0,        1, 1, 0,
1, 1, 1,        1, 0, 0,        1, 1, 1,        1, 0, 1,        0, 0, 0,
1, 0, 0,        1, 0, 1,        0, 0, 0,        1, 0, 1,        0, 0, 1,
0, 0, 1,        1, 0, 1,        1, 1, 1,        0, 0, 1,        1, 1, 1,
0, 1, 1
};

GLfloat cubeNorms[] = {
	0, 0, -1,       0, 0, -1,       0, 0, -1,       0, 0, -1,       0, 0, -1,
	0, 0, -1,       -1, 0, 0,       -1, 0, 0,       -1, 0, 0,       -1, 0, 0,
	-1, 0, 0,       -1, 0, 0,       0, 1, 0,        0, 1, 0,        0, 1, 0,
	0, 1, 0,        0, 1, 0,        0, 1, 0,        1, 0, 0,        1, 0, 0,
	1, 0, 0,        1, 0, 0,        1, 0, 0,        1, 0, 0,        0, -1, 0,
	0, -1, 0,       0, -1, 0,       0, -1, 0,       0, -1, 0,       0, -1, 0,
	0, 0, 1,        0, 0, 1,        0, 0, 1,        0, 0, 1,        0, 0, 1,
0, 0, 1
};
GLfloat cubeColours[] = { 0.0f, 0.0f, 0.0f,
0.0f, 1.0f, 0.0f,
1.0f, 1.0f, 0.0f,
1.0f, 0.0f, 0.0f,
0.0f, 0.0f, 1.0f,
0.0f, 1.0f, 1.0f,
1.0f, 1.0f, 1.0f,
1.0f, 0.0f, 1.0f };

GLuint cubeIndexCount = 36;
GLuint cubeIndices[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22
, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35
}; // front


GLuint meshObjects[1];
GLuint textures[3];

GLuint shaderProgram;
GLfloat r = 0.0f;
GLfloat dx = 0.0f, dy = 0.0f, dz = 0.0f, scale = 1.0f;

glm::vec4 lightPosition(0.0f, 0.0f, 1.0f, 1.0f);
rt3d::lightStruct light0 = {
	{ 0.2f,0.2f,0.2f, 1.0f }, // ambient
	{ 0.7f, 0.7f, 0.7f, 0.7f }, // diffuse 
	{ 0.8f, 0.8f,0.8f, 1.0f }, // specular
	{ 0.0f, 0.0f, 1.0f, 1.0f }  // position
};

rt3d::materialStruct material0 = {
	{ 0.4f, 0.2f, 0.2f, 1.0f }, // ambient
	{ 0.8f, 0.5f, 0.5f, 1.0f }, // diffuse
	{ 1.0f, 0.8f, 0.8f, 1.0f }, // specular
	2.0f  // shininess
};


rt3d::materialStruct material1 = {
	{ 0.4f, 0.2f, 0.2f, 0.3f }, // ambient
	{ 0.8f, 0.5f, 0.5f, 0.3f }, // diffuse
	{ 1.0f, 0.8f, 0.8f, 0.3f }, // specular
	2.0f  // shininess
};

GLfloat cubeTexCoords[] = { 0, 0,   1, 1,   1, 0,   0, 0,   0, 1,   1, 1,   0, 0,   1, 1,   1, 0,   0, 0,
0, 1,   1, 1,   0, 0,   1, 1,   1, 0,   0, 0,   0, 1,   1, 1,   0, 0,   0, 1,
1, 1,   0, 0,   1, 1,   1, 0,   0, 0,   1, 0,   1, 1,   0, 0,   1, 1,   0, 1,
0, 0,   1, 0,   1, 1,   0, 0,   1, 1,   0, 1
};

glm::vec3 eye(0.0f, 4.0f, 4.0f);
glm::vec3 at(0.0f, 0.0f, 0.0f);
glm::vec3 up(0.0f, 1.0f, 0.0f);
glm::vec3 player(0.0f, 0.0f, -10.0f);

GLuint loadBitmap(char*);

glm::vec3 moveForward(glm::vec3 cam, GLfloat angle, GLfloat d) {
	return glm::vec3(cam.x + d*std::sin(angle*DEG_TO_RAD),
		cam.y, cam.z - d*std::cos(angle*DEG_TO_RAD));
}

glm::vec3 moveRight(glm::vec3 pos, GLfloat angle, GLfloat d) {
	return glm::vec3(pos.x + d*std::cos(angle*DEG_TO_RAD),
		pos.y, pos.z + d*std::sin(angle*DEG_TO_RAD));
}


// Set up rendering context
SDL_Window * setupRC(SDL_GLContext &context) {
	SDL_Window * window;
	if (SDL_Init(SDL_INIT_VIDEO) < 0) // Initialize video
		rt3d::exitFatalError("Unable to initialize SDL");

	// Request an OpenGL 3.0 context.
	// Not able to use SDL to choose profile (yet), should default to core profile on 3.2 or later
	// If you request a context not supported by your drivers, no OpenGL context will be created

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);  // double buffering on
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4); // Turn on x4 multisampling anti-aliasing (MSAA)

	// Create 800x600 window
	window = SDL_CreateWindow("SDL/GLM/OpenGL Demo", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		800, 600, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
	if (!window) // Check window was created OK
		rt3d::exitFatalError("Unable to create window");

	context = SDL_GL_CreateContext(window); // Create opengl context and attach to window
	SDL_GL_SetSwapInterval(1); // set swap buffers to sync with monitor's vertical refresh rate
	return window;
}

void init(void) {
	glEnable(GL_DEPTH_TEST);

	textures[0] = loadBitmap("fabric.bmp");
	textures[1] = loadBitmap("studdedmetal.bmp");
	textures[2]= loadBitmap("groundsoil.bmp");
	// For this simple example we'll be using the most basic of shader programs
	//rt3d::initShaders("minimal.vert","minimal.frag");
	shaderProgram = rt3d::initShaders("phong-tex.vert", "phong-tex.frag");
	// Going to create our mesh objects here
	meshObjects[0] = rt3d::createMesh(cubeVertCount, cubeVerts, nullptr, cubeNorms, cubeTexCoords, cubeIndexCount, cubeIndices);


	glUseProgram(shaderProgram);
	rt3d::setLight(shaderProgram, light0);
	rt3d::setMaterial(shaderProgram, material1);

}

GLuint loadBitmap(char *fname) {

	GLuint texID;
	glGenTextures(1, &texID); // Generate Texture IDs

	SDL_Surface *tmpSurface;
	tmpSurface = SDL_LoadBMP(fname);
	if (!tmpSurface)
		std::cout << "Error Loading Bitmap" << std::endl;

	//bind texture and set parameters
	glBindTexture(GL_TEXTURE_2D, texID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	SDL_PixelFormat *format = tmpSurface->format;
	GLuint externalFormat, internalFormat;

	if (format->Amask) {
		internalFormat = GL_RGBA;
		externalFormat = (format->Rmask < format->Bmask) ? GL_RGBA : GL_BGRA;
	}
	else {
		internalFormat = GL_RGB;
		externalFormat = (format->Rmask < format->Bmask) ? GL_RGB : GL_BGR;
	}

	glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, tmpSurface->w, tmpSurface->h, 0, externalFormat, GL_UNSIGNED_BYTE, tmpSurface->pixels);
	glGenerateMipmap(GL_TEXTURE_2D);
	SDL_FreeSurface(tmpSurface);
	return texID;
}

void update() {
	const Uint8 *keys = SDL_GetKeyboardState(NULL);
	if (keys[SDL_SCANCODE_W]) {
		player = moveForward(player, r, 0.1f);
	}
	if (keys[SDL_SCANCODE_S]) {
		player = moveForward(player, r, -0.1f);
	}
	if (keys[SDL_SCANCODE_A]) {
		player = moveRight(player, r, -0.1f);
	}

	if (keys[SDL_SCANCODE_D]) {
		player = moveRight(player, r, 0.1f);
	}
	if (keys[SDL_SCANCODE_R]) eye.y += 0.1;
	if (keys[SDL_SCANCODE_F]) eye.y -= 0.1;
	if (keys[SDL_SCANCODE_COMMA]) r -= 0.1f;
	if (keys[SDL_SCANCODE_PERIOD]) r += 0.1f;

	eye.x = player.x;
	eye.y = player.y + 3.0f;
	eye.z = player.z + 10.0f;
}

void draw(SDL_Window * window) {

	// clear the screen
	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glm::mat4 projection(1.0);
	projection = glm::perspective(float(60.0f*DEG_TO_RAD), 800.0f / 600.0f, 1.0f, 50.0f);

	rt3d::setUniformMatrix4fv(shaderProgram, "projection", glm::value_ptr(projection));
	glm::mat4 modelview(1.0);
	mvStack.push(modelview);
	at = moveForward(player, r, 1.0f);
	mvStack.top() = glm::lookAt(eye, at, up);

	glm::vec4 tmp = mvStack.top()*lightPosition;
	rt3d::setLightPos(shaderProgram, glm::value_ptr(tmp));

	// draw a cube for ground plane
	glBindTexture(GL_TEXTURE_2D, textures[0]);
	mvStack.push(mvStack.top());
	mvStack.top() = glm::translate(mvStack.top(), glm::vec3(-10.0f, -0.1f, -10.0f));
	mvStack.top() = glm::scale(mvStack.top(), glm::vec3(20.0f, 0.1f, 20.0f));
	rt3d::setUniformMatrix4fv(shaderProgram, "modelview", glm::value_ptr(mvStack.top()));
	rt3d::setMaterial(shaderProgram, material1);
	rt3d::drawIndexedMesh(meshObjects[0], cubeIndexCount, GL_TRIANGLES);
	mvStack.pop();

	//Player
	glBindTexture(GL_TEXTURE_2D, textures[0]);
	mvStack.push(mvStack.top());
	mvStack.top() = glm::translate(mvStack.top(), glm::vec3(player.x, player.y, player.z));
	mvStack.top() = glm::rotate(mvStack.top(), r, glm::vec3(0.0f, 1.0f, 0.0f));
	rt3d::setUniformMatrix4fv(shaderProgram, "modelview", glm::value_ptr(mvStack.top()));
	rt3d::setMaterial(shaderProgram, material1);
	rt3d::drawIndexedMesh(meshObjects[0], cubeIndexCount, GL_TRIANGLES);
	mvStack.pop();

	// Cubes for buildings Different texture & individual material properties & positions
	glBindTexture(GL_TEXTURE_2D, textures[1]);
	rt3d::materialStruct tmpMaterial = material0;
	for (int a = 0; a<4; a++) {
		for (int b = 0; b<4; b++) {
			tmpMaterial.ambient[0] = a * 0.33f;
			tmpMaterial.ambient[1] = b * 0.33f;
			tmpMaterial.diffuse[0] = a * 0.33f;
			tmpMaterial.diffuse[1] = b * 0.33f;
			tmpMaterial.specular[0] = a * 0.33f;
			tmpMaterial.specular[1] = b * 0.33f;
			rt3d::setMaterial(shaderProgram, tmpMaterial);
			mvStack.push(mvStack.top());
			mvStack.top() = glm::translate(mvStack.top(),
				glm::vec3(a*3.0f, 0.0f, b*3.0f));
			mvStack.top() = glm::scale(mvStack.top(),
				glm::vec3(1.0f, 1.0f*(a + 0.1f), scale));
			rt3d::setUniformMatrix4fv(shaderProgram, "modelview",
				glm::value_ptr(mvStack.top()));
			rt3d::drawIndexedMesh(meshObjects[0], cubeIndexCount, GL_TRIANGLES);
			mvStack.pop();
		}
	}


	SDL_GL_SwapWindow(window); // swap buffers
}


// Program entry point - SDL manages the actual WinMain entry point for us
int main(int argc, char *argv[]) {
	SDL_Window * hWindow; // window handle
	SDL_GLContext glContext; // OpenGL context handle
	hWindow = setupRC(glContext); // Create window and render context 

	// Required on Windows *only* init GLEW to access OpenGL beyond 1.1
	glewExperimental = GL_TRUE;
	GLenum err = glewInit();
	if (GLEW_OK != err) { // glewInit failed, something is seriously wrong
		std::cout << "glewInit failed, aborting." << endl;
		exit(1);
	}
	cout << glGetString(GL_VERSION) << endl;

	init();

	bool running = true; // set running to true
	SDL_Event sdlEvent;  // variable to detect SDL events
	while (running) {	// the event loop
		while (SDL_PollEvent(&sdlEvent)) {
			if (sdlEvent.type == SDL_QUIT)
				running = false;
		}
		update();
		draw(hWindow); // call the draw function
	}

	SDL_GL_DeleteContext(glContext);
	SDL_DestroyWindow(hWindow);
	SDL_Quit();
	return 0;
}