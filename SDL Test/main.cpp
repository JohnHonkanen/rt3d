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
GLuint cubeVertCount = 8;
GLfloat cubeVerts[] = { -0.5, -0.5f, -0.5f,
-0.5, 0.5f, -0.5f,
0.5, 0.5f, -0.5f,
0.5, -0.5f, -0.5f,
-0.5, -0.5f, 0.5f,
-0.5, 0.5f, 0.5f,
0.5, 0.5f, 0.5f,
0.5, -0.5f, 0.5f };
GLfloat cubeColours[] = { 0.0f, 0.0f, 0.0f,
0.0f, 1.0f, 0.0f,
1.0f, 1.0f, 0.0f,
1.0f, 0.0f, 0.0f,
0.0f, 0.0f, 1.0f,
0.0f, 1.0f, 1.0f,
1.0f, 1.0f, 1.0f,
1.0f, 0.0f, 1.0f };

GLuint cubeIndexCount = 36;
GLuint cubeIndices[] = { 0,1,2, 0,2,3, // back  
1,0,5, 0,4,5, // left					
6,3,2, 3,6,7, // right
1,5,6, 1,6,2, // top
0,3,4, 3,7,4, // bottom
6,5,4, 7,6,4 }; // front


GLuint meshObjects[1];

GLuint mvpShaderProgram;
GLuint mvpShaderProgram2;
glm::mat4 MVP;
GLfloat r = 0.0f, red = 0.0f, green = 0.0f, blue = 0.0f;
GLfloat px = 0.0f, py = 0.0f;

rt3d::lightStruct light0 = {
	{ 0.2f,0.2f,0.2f, 1.0f }, // ambient
	{ 0.7f, 0.7f, 0.7f, 1.0f }, // diffuse
	{ 0.8f, 0.8f, 0.8f, 1.0f }, // specular
	{ px, py, 1.0f, 1.0f }  // position
};

rt3d::materialStruct material0 = {
	{ 0.4f, 0.2f, 0.2f, 1.0f }, // ambient
	{ 0.8f, 0.5f, 0.5f, 1.0f }, // diffuse
	{ 1.0f, 0.8f, 0.8f, 1.0f }, // specular
	2.0f  // shininess
};


rt3d::materialStruct material1 = {
	{ 1.0f, 1.0f, 0.2f, 1.0f }, // ambient
	{ 0.0f, 0.5f, 0.5f, 1.0f }, // diffuse
	{ 0.0f, 0.8f, 0.8f, 1.0f }, // specular
	2.0f  // shininess
};

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
        800, 600, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN );
	if (!window) // Check window was created OK
        rt3d::exitFatalError("Unable to create window");
 
    context = SDL_GL_CreateContext(window); // Create opengl context and attach to window
    SDL_GL_SetSwapInterval(1); // set swap buffers to sync with monitor's vertical refresh rate
	return window;
}

void init(void) {
	glEnable(GL_DEPTH_TEST);
	
	// For this simple example we'll be using the most basic of shader programs
	//rt3d::initShaders("minimal.vert","minimal.frag");
	mvpShaderProgram = rt3d::initShaders("phong.vert", "phong.frag");
	mvpShaderProgram2 = rt3d::initShaders("gouraud.vert", "simple.frag");
	MVP = glm::mat4(1.0);
	// Going to create our mesh objects here
	meshObjects[0] = rt3d::createMesh(cubeVertCount, cubeVerts, nullptr, cubeVerts,
		nullptr, cubeIndexCount, cubeIndices);


	glUseProgram(mvpShaderProgram);
	rt3d::setLight(mvpShaderProgram, light0);
	rt3d::setMaterial(mvpShaderProgram, material0);

	glUseProgram(mvpShaderProgram2);
	rt3d::setLight(mvpShaderProgram2, light0);
	rt3d::setMaterial(mvpShaderProgram2, material1);


}

void update() {
	const Uint8 *keys = SDL_GetKeyboardState(NULL);
	if (keys[SDL_SCANCODE_Q]) r -= 0.1;
	if (keys[SDL_SCANCODE_E]) r += 0.1;

	if (keys[SDL_SCANCODE_A]) px -= 0.1;
	if (keys[SDL_SCANCODE_D]) px += 0.1;

	if (keys[SDL_SCANCODE_W]) py += 0.1;
	if (keys[SDL_SCANCODE_S]) py -= 0.1;

	if (keys[SDL_SCANCODE_T]) red -= 0.1;
	if (keys[SDL_SCANCODE_G]) red += 0.1;

	if (keys[SDL_SCANCODE_Y]) green -= 0.1;
	if (keys[SDL_SCANCODE_H]) green += 0.1;

	if (keys[SDL_SCANCODE_U]) blue -= 0.1;
	if (keys[SDL_SCANCODE_J]) blue += 0.1;

	if (keys[SDL_SCANCODE_R]) {
		r = 0;
		px = 0;
		py = 0;
	}

	if (red < 0.0f || red > 1.0f)
		red = 0.0f;
	if (green < 0.0f || green > 1.0f)
		green = 0.0f;
	if (blue < 0.0f || blue > 1.0f)
		blue = 0.0f;	
}

void draw(SDL_Window * window) {

	light0 = {
		{ red,green,blue, 1.0f }, // ambient
		{ 0.7f, 0.7f, 0.7f, 1.0f }, // diffuse
		{ 0.8f, 0.8f, 0.8f, 1.0f }, // specular
		{ px, py, 1.0f, 1.0f }  // position
	};
	// clear the screen
	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	// set up projection matrix


	glUseProgram(mvpShaderProgram);
	rt3d::setLight(mvpShaderProgram, light0);
	rt3d::setMaterial(mvpShaderProgram, material0);

	
	
	
	glm::mat4 projection(1.0);
	projection = glm::perspective(float(60.0f*DEG_TO_RAD), 800.0f / 600.0f, 1.0f, 50.0f);
	rt3d::setUniformMatrix4fv(mvpShaderProgram, "projection", glm::value_ptr(projection));

	// render the sun
	glm::mat4 modelview(1.0);
	mvStack.push(modelview); // push modelview to stack
	mvStack.top() = glm::translate(mvStack.top(), glm::vec3(0.0f, 0.0f, -4.0f));
	mvStack.top() = glm::rotate(mvStack.top(), r, glm::vec3(0.0f, 1.0f, 0.0f));
	mvStack.top() = glm::scale(mvStack.top(), glm::vec3(0.5f, 0.5f, 0.5f));
	rt3d::setUniformMatrix4fv(mvpShaderProgram, "modelview", glm::value_ptr(mvStack.top()));
	rt3d::drawIndexedMesh(meshObjects[0], cubeIndexCount, GL_TRIANGLES);
	// cube
	mvStack.pop();
	glUseProgram(mvpShaderProgram2);
	rt3d::setLight(mvpShaderProgram2, light0);
	rt3d::setMaterial(mvpShaderProgram2, material1);

	rt3d::setUniformMatrix4fv(mvpShaderProgram2, "projection", glm::value_ptr(projection));
	// render the sun
	mvStack.push(modelview);
	mvStack.push(mvStack.top());// push modelview to stack
	mvStack.top() = glm::translate(mvStack.top(), glm::vec3(2.0f, 0.0f, -4.0f));
	mvStack.top() = glm::rotate(mvStack.top(), r, glm::vec3(0.0f, 1.0f, 0.0f));
	mvStack.top() = glm::scale(mvStack.top(), glm::vec3(0.5f, 0.5f, 0.5f));
	rt3d::setUniformMatrix4fv(mvpShaderProgram2, "modelview", glm::value_ptr(mvStack.top()));
	rt3d::drawIndexedMesh(meshObjects[0], cubeIndexCount, GL_TRIANGLES);

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
		exit (1);
	}
	cout << glGetString(GL_VERSION) << endl;

	init();

	bool running = true; // set running to true
	SDL_Event sdlEvent;  // variable to detect SDL events
	while (running)	{	// the event loop
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