// ==========================================================================
// Barebones OpenGL Core Profile Boilerplate
//    using the GLFW windowing system (http://www.glfw.org)
//
// Loosely based on
//  - Chris Wellons' example (https://github.com/skeeto/opengl-demo) and
//  - Camilla Berglund's example (http://www.glfw.org/docs/latest/quick.html)
//
// Author:  Sonny Chan, University of Calgary
// Date:    December 2015
// ==========================================================================

#include <iostream>
#include <fstream>
#include <string>
#include <iterator>
#include <algorithm>
#include <vector>
#include <cstdlib>
#include <ctime>

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

// specify that we want the OpenGL core profile before including GLFW headers
#define GLFW_INCLUDE_GLCOREARB
#define GL_GLEXT_PROTOTYPES
#include <GLFW/glfw3.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include "camera.h"

#define PI 3.141592653589793238462643383

using namespace std;
using namespace glm;

//Forward definitions
bool CheckGLErrors(string location);
void QueryGLVersion();
string LoadSource(const string &filename);
GLuint CompileShader(GLenum shaderType, const string &source);
GLuint LinkProgram(GLuint vertexShader, GLuint fragmentShader);

vec2 mousePos;
bool mousePressed = false;
bool motion = true;

vec3 sunCenter = vec3(0.0);
vec3 earthCenter = vec3(0.0);
vec3 moonCenter = vec3(0.0);
int mode = 1;

Camera cam;
float speed = 0.05;

GLFWwindow* window = 0;

// --------------------------------------------------------------------------
// GLFW callback functions

// reports GLFW errors
void ErrorCallback(int error, const char* description)
{
    cout << "GLFW ERROR " << error << ":" << endl;
    cout << description << endl;
}

// handles keyboard input events
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);

    if(key == GLFW_KEY_SPACE && action == GLFW_PRESS)
    	motion = !motion;
    if(key == GLFW_KEY_UP) 
    	speed *= 1.5;
    if(key == GLFW_KEY_DOWN) 
    	speed *= 0.5;
    if(key == GLFW_KEY_1)  {
    	mode = 1;
    	cam.polarPos.z = 50.0;
    }
    if(key == GLFW_KEY_2) {
    	mode = 2;
    	cam.polarPos.z = 10.0;
    }
    if(key == GLFW_KEY_3) {
    	mode = 3;
    	cam.polarPos.z = 5.0;
    }
}

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
	if( (action == GLFW_PRESS) || (action == GLFW_RELEASE) )
		mousePressed = !mousePressed;
}

void mousePosCallback(GLFWwindow* window, double xpos, double ypos)
{
	int vp[4];
	glGetIntegerv(GL_VIEWPORT, vp);

	vec2 newPos = vec2(xpos/(double)vp[2], -ypos/(double)vp[3])*2.f - vec2(1.f);

	vec2 diff = newPos - mousePos;
	if(mousePressed)
		cam.translateCamera(-diff.y, diff.x, 0.0);

	mousePos = newPos;
}

void resizeCallback(GLFWwindow* window, int width, int height)
{
	int vp[4];
	glGetIntegerv(GL_VIEWPORT, vp);

	glViewport(0, 0, width, height);
}

void mouseScrollCallback(GLFWwindow* window, double xOffset, double yOffset) {
	cam.translateCamera(0.0, 0.0, -yOffset);
}




//==========================================================================
// TUTORIAL STUFF


//vec2 and vec3 are part of the glm math library. 
//Include in your own project by putting the glm directory in your project, 
//and including glm/glm.hpp as I have at the top of the file.
//"using namespace glm;" will allow you to avoid writing everyting as glm::vec2
vector<vec3> points;
vector<vec2> uvs;

//Structs are simply acting as namespaces
//Access the values like so: VAO::LINES
struct VAO{
	enum {GEOMETRY=0, COUNT};		//Enumeration assigns each name a value going up
										//LINES=0, COUNT=1
};

struct VBO{
	enum {POINTS=0, NORMALS, UVS, INDICES, COUNT};	//POINTS=0, COLOR=1, COUNT=2
};

struct SHADER{
	enum {DEFAULT=0, COUNT};		//LINE=0, COUNT=1
};

GLuint vbo [VBO::COUNT];		//Array which stores OpenGL's vertex buffer object handles
GLuint vao [VAO::COUNT];		//Array which stores Vertex Array Object handles
GLuint shader [SHADER::COUNT];		//Array which stores shader program handles

//Gets handles from OpenGL
void generateIDs()
{
	glGenVertexArrays(VAO::COUNT, vao);		//Tells OpenGL to create VAO::COUNT many
														// Vertex Array Objects, and store their
														// handles in vao array
	glGenBuffers(VBO::COUNT, vbo);		//Tells OpenGL to create VBO::COUNT many
													//Vertex Buffer Objects and store their
													//handles in vbo array
}

//Clean up IDs when you're done using them
void deleteIDs()
{
	for(int i=0; i<SHADER::COUNT; i++)
	{
		glDeleteProgram(shader[i]);
	}
	
	glDeleteVertexArrays(VAO::COUNT, vao);
	glDeleteBuffers(VBO::COUNT, vbo);	
}


//Describe the setup of the Vertex Array Object
bool initVAO()
{
	glBindVertexArray(vao[VAO::GEOMETRY]);		//Set the active Vertex Array

	glEnableVertexAttribArray(0);		//Tell opengl you're using layout attribute 0 (For shader input)
	glBindBuffer( GL_ARRAY_BUFFER, vbo[VBO::POINTS] );		//Set the active Vertex Buffer
	glVertexAttribPointer(
		0,				//Attribute
		3,				//Size # Components
		GL_FLOAT,	//Type
		GL_FALSE, 	//Normalized?
		sizeof(vec3),	//Stride
		(void*)0			//Offset
		);

	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[VBO::NORMALS]);
	glVertexAttribPointer(
		1,				//Attribute
		3,				//Size # Components
		GL_FLOAT,	//Type
		GL_FALSE, 	//Normalized?
		sizeof(vec3),	//Stride
		(void*)0			//Offset
		);
	
	glEnableVertexAttribArray(2);		//Tell opengl you're using layout attribute 1
	glBindBuffer(GL_ARRAY_BUFFER, vbo[VBO::UVS]);
	glVertexAttribPointer(
		2,
		2,
		GL_FLOAT,
		GL_FALSE,
		sizeof(vec2),
		(void*)0
		);	

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[VBO::INDICES]);

	return !CheckGLErrors("initVAO");		//Check for errors in initialize
}


//Loads buffers with data
bool loadBuffer(const vector<vec3>& points, const vector<vec3> normals, 
				const vector<vec2>& uvs, const vector<unsigned int>& indices)
{
	glBindBuffer(GL_ARRAY_BUFFER, vbo[VBO::POINTS]);
	glBufferData(
		GL_ARRAY_BUFFER,				//Which buffer you're loading too
		sizeof(vec3)*points.size(),	//Size of data in array (in bytes)
		&points[0],							//Start of array (&points[0] will give you pointer to start of vector)
		GL_DYNAMIC_DRAW						//GL_DYNAMIC_DRAW if you're changing the data often
												//GL_STATIC_DRAW if you're changing seldomly
		);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[VBO::NORMALS]);
	glBufferData(
		GL_ARRAY_BUFFER,				//Which buffer you're loading too
		sizeof(vec3)*normals.size(),	//Size of data in array (in bytes)
		&normals[0],							//Start of array (&points[0] will give you pointer to start of vector)
		GL_DYNAMIC_DRAW						//GL_DYNAMIC_DRAW if you're changing the data often
												//GL_STATIC_DRAW if you're changing seldomly
		);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[VBO::UVS]);
	glBufferData(
		GL_ARRAY_BUFFER,
		sizeof(vec2)*uvs.size(),
		&uvs[0],
		GL_STATIC_DRAW
		);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[VBO::INDICES]);
	glBufferData(
		GL_ELEMENT_ARRAY_BUFFER,
		sizeof(unsigned int)*indices.size(),
		&indices[0],
		GL_STATIC_DRAW
		);

	return !CheckGLErrors("loadBuffer");	
}

//Compile and link shaders, storing the program ID in shader array
bool initShader()
{	
	string vertexSource = LoadSource("vertex.glsl");		//Put vertex file text into string
	string fragmentSource = LoadSource("fragment.glsl");		//Put fragment file text into string

	GLuint vertexID = CompileShader(GL_VERTEX_SHADER, vertexSource);
	GLuint fragmentID = CompileShader(GL_FRAGMENT_SHADER, fragmentSource);
	
	shader[SHADER::DEFAULT] = LinkProgram(vertexID, fragmentID);	//Link and store program ID in shader array

	return !CheckGLErrors("initShader");
}

//For reference:
//	https://open.gl/textures
GLuint createTexture(const char* filename)
{
	int components;
	GLuint texID;
	int tWidth, tHeight;

	//stbi_set_flip_vertically_on_load(true);
	unsigned char* data = stbi_load(filename, &tWidth, &tHeight, &components, 0);
	
	if(data != NULL)
	{
		glGenTextures(1, &texID);
		glBindTexture(GL_TEXTURE_2D, texID);

		if(components==3)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, tWidth, tHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		else if(components==4)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tWidth, tHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		//Clean up
		glBindTexture(GL_TEXTURE_2D, 0);
		stbi_image_free(data);

		return texID;
	} 
	
	return 0;	//Error
}

//Use program before loading texture
//	texUnit can be - GL_TEXTURE0, GL_TEXTURE1, etc...
bool loadTexture(GLuint texID, GLuint texUnit, GLuint program, const char* uniformName)
{
	glActiveTexture(texUnit);
	glBindTexture(GL_TEXTURE_2D, texID);
	
	GLuint uniformLocation = glGetUniformLocation(program, uniformName);
	glUniform1i(uniformLocation, 0);
		
	return !CheckGLErrors("loadTexture");
}

// fun fact: did you know planets are just elaborate spheres? Believe it.
void generateSphere(vector<vec3>& positions, vector<vec3>& normals, 
					vector<vec2>& uvs, vector<unsigned int>& indices,
					vec3 center, float radius, int divisions)
{
	float step = 1.f / (float)(divisions - 1);
	float u = 0.f;

	// Traversing the planes of time and space
	for (int i = 0; i < divisions; i++) {
		float v = 0.f;

		//Traversing the planes of time and space (again)
		for (int j = 0; j < divisions; j++) {
			vec3 pos = vec3(	radius * cos(2.f * PI * u) * sin(PI * v),
								radius * sin(2.f * PI * u) * sin(PI * v),
								radius * cos(PI * v)) + center;

			vec3 normal = normalize(pos - center);
			
			positions.push_back(pos);
			normals.push_back(normal);
			uvs.push_back(vec2(u, v));

			v += step;
		}

		u += step;
	}

	for(int i = 0; i < divisions - 1; i++)
	{
		for(int j = 0; j < divisions - 1; j++)
		{
			unsigned int p00 = i * divisions + j;
			unsigned int p01 = i * divisions + j + 1;
			unsigned int p10 = (i + 1) * divisions + j;
			unsigned int p11 = (i + 1) * divisions + j + 1;

			indices.push_back(p00);
			indices.push_back(p10);
			indices.push_back(p01);

			indices.push_back(p01);
			indices.push_back(p10);
			indices.push_back(p11);
		}
	}
}

//Initialization
void initGL()
{

//	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	//Only call these once - don't call again every time you change geometry
	generateIDs();		//Create VertexArrayObjects and Vertex Buffer Objects and store their handles
	initShader();		//Create shader and store program ID

	initVAO();			//Describe setup of Vertex Array Objects and Vertex Buffer Object

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
}

//Draws buffers to screen
void render(Camera* cam, mat4 perspectiveMatrix, mat4 modelview, int startElement, int numElements)
{
	
	//Don't need to call these on every draw, so long as they don't change
	glUseProgram(shader[SHADER::DEFAULT]);		//Use LINE program
	glBindVertexArray(vao[VAO::GEOMETRY]);		//Use the LINES vertex array

	glUseProgram(shader[SHADER::DEFAULT]);

	mat4 camMatrix = cam->getMatrix();

	glUniformMatrix4fv(glGetUniformLocation(shader[SHADER::DEFAULT], "cameraMatrix"),
						1,
						false,
						&camMatrix[0][0]);

	glUniformMatrix4fv(glGetUniformLocation(shader[SHADER::DEFAULT], "perspectiveMatrix"),
						1,
						false,
						&perspectiveMatrix[0][0]);

	glUniformMatrix4fv(glGetUniformLocation(shader[SHADER::DEFAULT], "modelviewMatrix"),
						1,
						false,
						&modelview[0][0]);
	
	CheckGLErrors("loadUniforms");

	glDrawElements(
			GL_TRIANGLES,		//What shape we're drawing	- GL_TRIANGLES, GL_LINES, GL_POINTS, GL_QUADS, GL_TRIANGLE_STRIP
			numElements,		//How many indices
			GL_UNSIGNED_INT,	//Type
			(void*)0			//Offset
			);

	CheckGLErrors("render");
}

void rotatePlanet(vector<vec3>& points, vector<vec3>& normals, vec3 center, vec3 axis, float theta) {
	axis = normalize(axis);
	float x = axis.x;
	float y = axis.y;
	float z = axis.z;
	float x2 = x * x;
	float y2 = y * y;
	float z2 = z * z;

	mat3 rMat = mat3(	cos(theta) + x2 * (1 - cos(theta)), x * y * (1 - cos(theta)) - z * sin(theta), x * z * (1 - cos(theta)) + y * sin(theta),
						y * x * (1 - cos(theta)) + z * sin(theta), cos(theta) + y2 * (1 - cos(theta)), y * z * (1 - cos(theta)) - x * sin(theta),
						z * x * (1 - cos(theta)) - y * sin(theta), z * y * (1 - cos(theta)) + x * sin(theta), cos(theta) + z2 * (1 - cos(theta)));

	for (int i = 0; i < points.size(); i++) {
		points[i] = (rMat * (points[i] - center)) + center;
		normals[i] = normalize(points[i] - center);
	}

}

void orbitPlanet(vector<vec3>& points, vector<vec3>& normals, vec3& childCenter, vec3 parentCenter, vec3 axis, float theta) {
	axis = normalize(axis);
	float x = axis.x;
	float y = axis.y;
	float z = axis.z;
	float x2 = x * x;
	float y2 = y * y;
	float z2 = z * z;

	rotatePlanet(points, normals, childCenter, axis, -theta);

	mat3 rMat = mat3(	cos(theta) + x2 * (1 - cos(theta)), x * y * (1 - cos(theta)) - z * sin(theta), x * z * (1 - cos(theta)) + y * sin(theta),
						y * x * (1 - cos(theta)) + z * sin(theta), cos(theta) + y2 * (1 - cos(theta)), y * z * (1 - cos(theta)) - x * sin(theta),
						z * x * (1 - cos(theta)) - y * sin(theta), z * y * (1 - cos(theta)) + x * sin(theta), cos(theta) + z2 * (1 - cos(theta)));

	childCenter = (rMat * (childCenter - parentCenter)) + parentCenter;

	for (int i = 0; i < points.size(); i++) {
		points[i] = (rMat * (points[i] - parentCenter)) + parentCenter;
		normals[i] = normalize(points[i] - childCenter);
	}

}




// ==========================================================================
// PROGRAM ENTRY POINT

int main(int argc, char *argv[])
{   
    // initialize the GLFW windowing system
    if (!glfwInit()) {
        cout << "ERROR: GLFW failed to initilize, TERMINATING" << endl;
        return -1;
    }
    glfwSetErrorCallback(ErrorCallback);

    // attempt to create a window with an OpenGL 4.1 core profile context
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    window = glfwCreateWindow(1024, 1024, "CPSC 453 OpenGL Boilerplate", 0, 0);
    if (!window) {
        cout << "Program failed to create GLFW window, TERMINATING" << endl;
        glfwTerminate();
        return -1;
    }

    // set keyboard callback function and make our context current (active)
    glfwSetKeyCallback(window, keyCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetCursorPosCallback(window, mousePosCallback);
    glfwSetScrollCallback(window, mouseScrollCallback);
    glfwSetWindowSizeCallback(window, resizeCallback);
    glfwMakeContextCurrent(window);

    // query and print out information about our OpenGL environment
    QueryGLVersion();

	initGL();


	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	float distScale = 35.0 / 149597870.7; // AU in km
	float radScale = 1.0 / 6378.1; // E in km

	// make sun
	vector<vec3> sunPoints;
	vector<vec3> sunNormals;
	vector<vec2> sunUvs;
	vector<unsigned int> sunIndices;
	sunCenter = vec3(0.0);
	float sunRadius = pow(radScale * 696000.0, 0.5);
	generateSphere(sunPoints, sunNormals, sunUvs, sunIndices, sunCenter, sunRadius, 96);
	GLuint sun = createTexture("sun.jpg");

	// make earth
	vector<vec3> earthPoints;
	vector<vec3> earthNormals;
	vector<vec2> earthUvs;
	vector<unsigned int> earthIndices;
	earthCenter = vec3(distScale * 149597890, 0.0, 0.0);
	float earthRadius = pow(radScale * 6378.1, 0.5);
	generateSphere(earthPoints, earthNormals, earthUvs, earthIndices, earthCenter, earthRadius, 72);
	GLuint earth = createTexture("earth.jpg");

	// make moon
	vector<vec3> moonPoints;
	vector<vec3> moonNormals;
	vector<vec2> moonUvs;
	vector<unsigned int> moonIndices;
	moonCenter = earthCenter - vec3((20 * distScale * 384399.0), 0.0, 0.0);
	float moonRadius = pow(radScale * 1737.1 / 2, 0.5);
	generateSphere(moonPoints, moonNormals, moonUvs, moonIndices, moonCenter, moonRadius, 48);
	GLuint moon = createTexture("moonyy.jpg");
	
	// make space
	vector<vec3> spacePoints;
	vector<vec3> spaceNormals;
	vector<vec2> spaceUvs;
	vector<unsigned int> spaceIndices;
	vec3 spaceCenter = vec3(0.0);
	generateSphere(spacePoints, spaceNormals, spaceUvs, spaceIndices, spaceCenter, 400.0, 128);
	GLuint space = createTexture("space1.png");
	
	
	// direction, position
	cam = Camera(vec3(-1.63994, 0.0607855, 50.0), vec3(0.0, 0.0, 0.0), sunRadius);
	//float fovy, float aspect, float zNear, float zFar
	mat4 perspectiveMatrix = perspective(radians(80.f), 1.f, 0.1f, 1000.f); 

	float scale; 
	float sunRot;
	float earthOrb;
	float earthRot;
	float moonOrb; 
	float moonRot;
	float spaceRot;
	GLuint diffUniformLocation;

    // run an event-triggered main loop
    while (!glfwWindowShouldClose(window))
    {
    	glClearColor(0.f, 0.f, 0.f, 0.f);		// Color to clear the screen with (R, G, B, Alpha)
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);		// Clear color and depth buffers (Haven't covered yet)
		
		// cout << "x: " << cam.polarPos.x << " y: " << cam.polarPos.y << " z: " << cam.polarPos.z << endl;

		// cout << cam.polarPos.z << endl;

		scale = speed * PI;
		sunRot = scale / 25.38;
		earthOrb = scale / 365;
		earthRot = -scale;
		moonOrb = scale / 27.32;
		moonRot = scale / 27.32;
		spaceRot = scale / 5000;

		if(mode == 1)
			cam = Camera(cam.polarPos, -sunCenter, sunRadius);
		if(mode == 2)
			cam = Camera(cam.polarPos, -earthCenter, earthRadius);
		if(mode == 3)
			cam = Camera(cam.polarPos, -moonCenter, moonRadius);

        // call function to draw our scene
        if(motion) {
        	rotatePlanet(sunPoints, sunNormals, sunCenter, vec3(0.0, 0.0, 1.0), sunRot);
        	orbitPlanet(earthPoints, earthNormals, earthCenter, sunCenter, vec3(0.0, 0.0, 1.0), earthOrb);
        	rotatePlanet(earthPoints, earthNormals, earthCenter, vec3(0.0, 0.0, 1.0), earthRot);
        	orbitPlanet(moonPoints, moonNormals, moonCenter, earthCenter, vec3(0.0, 0.0, 1.0), moonOrb);
        	rotatePlanet(moonPoints, moonNormals, moonCenter, vec3(0.0, 0.0, 1.0), moonRot);
        	rotatePlanet(spacePoints, spaceNormals, spaceCenter, vec3(0.0, 0.0, 1.0), spaceRot);
        }

        glUseProgram(shader[SHADER::DEFAULT]);

        loadBuffer(sunPoints, sunNormals, sunUvs, sunIndices);
        loadTexture(sun, GL_TEXTURE0, shader[SHADER::DEFAULT], "texSphere");
        diffUniformLocation = glGetUniformLocation(shader[SHADER::DEFAULT], "diffuse");
        glUniform1i(diffUniformLocation, false); // change this to sunDiffuse or something in your free time because this is sloppy
        render(&cam, perspectiveMatrix, mat4(1.f), 0, sunIndices.size());
        

        loadBuffer(earthPoints, earthNormals, earthUvs, earthIndices);
        loadTexture(earth, GL_TEXTURE0, shader[SHADER::DEFAULT], "texSphere");
        diffUniformLocation = glGetUniformLocation(shader[SHADER::DEFAULT], "diffuse");
        glUniform1i(diffUniformLocation, true);
        render(&cam, perspectiveMatrix, mat4(1.f), 0, earthIndices.size());
        
        
        loadBuffer(moonPoints, moonNormals, moonUvs, moonIndices);
        loadTexture(moon, GL_TEXTURE0, shader[SHADER::DEFAULT], "texSphere");
        diffUniformLocation = glGetUniformLocation(shader[SHADER::DEFAULT], "diffuse");
        glUniform1i(diffUniformLocation, true);
        render(&cam, perspectiveMatrix, mat4(1.f), 0, moonIndices.size());
        

		loadBuffer(spacePoints, spaceNormals, spaceUvs, spaceIndices);
        loadTexture(space, GL_TEXTURE0, shader[SHADER::DEFAULT], "texSphere");
        diffUniformLocation = glGetUniformLocation(shader[SHADER::DEFAULT], "diffuse");
        glUniform1i(diffUniformLocation, false);
        render(&cam, perspectiveMatrix, mat4(1.f), 0, spaceIndices.size());
        

        // scene is rendered to the back buffer, so swap to front for display
        glfwSwapBuffers(window);

        // sleep until next event before drawing again
        glfwPollEvents();
	}

	// clean up allocated resources before exit
   	deleteIDs();
	glfwDestroyWindow(window);
   	glfwTerminate();

   return 0;
}

// ==========================================================================
// SUPPORT FUNCTION DEFINITIONS

// --------------------------------------------------------------------------
// OpenGL utility functions

void QueryGLVersion()
{
    // query opengl version and renderer information
    string version  = reinterpret_cast<const char *>(glGetString(GL_VERSION));
    string glslver  = reinterpret_cast<const char *>(glGetString(GL_SHADING_LANGUAGE_VERSION));
    string renderer = reinterpret_cast<const char *>(glGetString(GL_RENDERER));

    cout << "OpenGL [ " << version << " ] "
         << "with GLSL [ " << glslver << " ] "
         << "on renderer [ " << renderer << " ]" << endl;
}

bool CheckGLErrors(string location)
{
    bool error = false;
    for (GLenum flag = glGetError(); flag != GL_NO_ERROR; flag = glGetError())
    {
        cout << "OpenGL ERROR:  ";
        switch (flag) {
        case GL_INVALID_ENUM:
            cout << location << ": " << "GL_INVALID_ENUM" << endl; break;
        case GL_INVALID_VALUE:
            cout << location << ": " << "GL_INVALID_VALUE" << endl; break;
        case GL_INVALID_OPERATION:
            cout << location << ": " << "GL_INVALID_OPERATION" << endl; break;
        case GL_INVALID_FRAMEBUFFER_OPERATION:
            cout << location << ": " << "GL_INVALID_FRAMEBUFFER_OPERATION" << endl; break;
        case GL_OUT_OF_MEMORY:
            cout << location << ": " << "GL_OUT_OF_MEMORY" << endl; break;
        default:
            cout << "[unknown error code]" << endl;
        }
        error = true;
    }
    return error;
}

// --------------------------------------------------------------------------
// OpenGL shader support functions

// reads a text file with the given name into a string
string LoadSource(const string &filename)
{
    string source;

    ifstream input(filename.c_str());
    if (input) {
        copy(istreambuf_iterator<char>(input),
             istreambuf_iterator<char>(),
             back_inserter(source));
        input.close();
    }
    else {
        cout << "ERROR: Could not load shader source from file "
             << filename << endl;
    }

    return source;
}

// creates and returns a shader object compiled from the given source
GLuint CompileShader(GLenum shaderType, const string &source)
{
    // allocate shader object name
    GLuint shaderObject = glCreateShader(shaderType);

    // try compiling the source as a shader of the given type
    const GLchar *source_ptr = source.c_str();
    glShaderSource(shaderObject, 1, &source_ptr, 0);
    glCompileShader(shaderObject);

    // retrieve compile status
    GLint status;
    glGetShaderiv(shaderObject, GL_COMPILE_STATUS, &status);
    if (status == GL_FALSE)
    {
        GLint length;
        glGetShaderiv(shaderObject, GL_INFO_LOG_LENGTH, &length);
        string info(length, ' ');
        glGetShaderInfoLog(shaderObject, info.length(), &length, &info[0]);
        cout << "ERROR compiling shader:" << endl << endl;
        cout << source << endl;
        cout << info << endl;
    }

    return shaderObject;
}

// creates and returns a program object linked from vertex and fragment shaders
GLuint LinkProgram(GLuint vertexShader, GLuint fragmentShader)
{
    // allocate program object name
    GLuint programObject = glCreateProgram();

    // attach provided shader objects to this program
    if (vertexShader)   glAttachShader(programObject, vertexShader);
    if (fragmentShader) glAttachShader(programObject, fragmentShader);

    // try linking the program with given attachments
    glLinkProgram(programObject);

    // retrieve link status
    GLint status;
    glGetProgramiv(programObject, GL_LINK_STATUS, &status);
    if (status == GL_FALSE)
    {
        GLint length;
        glGetProgramiv(programObject, GL_INFO_LOG_LENGTH, &length);
        string info(length, ' ');
        glGetProgramInfoLog(programObject, info.length(), &length, &info[0]);
        cout << "ERROR linking shader program:" << endl;
        cout << info << endl;
    }

    return programObject;
}


// ==========================================================================
