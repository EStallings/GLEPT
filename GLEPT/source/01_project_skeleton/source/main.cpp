#include "platform.hpp"

// third-party libraries
#include <direct.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

// standard C++ libraries
#include <cassert>
#include <iostream>
#include <stdexcept>
#include <cmath>
#include <math.h>
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>

// Geometry
#include "objLoader.h"
#include "obj_parser.h"
using namespace std;

// tdogl classes
#include "Program.h"

// constants
const glm::vec2 SCREEN_SIZE(800, 600);

// globals
GLFWwindow* gWindow = NULL;
tdogl::Program* gProgram = NULL;
GLuint gVAO = 0;
GLuint gVBO = 0;


GLuint   program_object;
GLuint	 program_object2;
GLuint   vertex_shader;
GLuint   fragment_shader;

GLuint framebufferID;
GLuint colorTextureID;



// read a file to string
std::string readFile(const char *filePath) {
	std::string content;
	std::ifstream fileStream(filePath, std::ios::in);

	if (!fileStream.is_open()) {
		std::cerr << "Could not read file " << filePath << " File does not exist." << std::endl;
		return "";
	}

	std::string line = "";
	while (!fileStream.eof()) {
		std::getline(fileStream, line);
		content.append(line + "\n");
	}
	fileStream.close();
	return content;
}


float *VertsToFloat3(obj_vector** verts, int vertCount){
	int arraySize = vertCount * 3;
	float *output = (float*)malloc(arraySize * sizeof(float));

	// Each vert
	for (int i = 0; i < vertCount; i++){
		// Each coord
		for (int k = 0; k < 3; k++){
			//printf("%f", verts[i]->e[k]);
			output[i * 3 + k] = verts[i]->e[k];
		}
	}

	return output;
}

float *GetObjectMaterials(objLoader* object, int faceCount){
	int arraySize = object->materialCount * 3;
	float *materials = (float*)malloc(arraySize * sizeof(float));


	// Each material
	for (int i = 0; i < object->materialCount; i++){

		// Each color channel
		for (int k = 0; k < 3; k++){
			materials[i * 3 + k] = object->materialList[i]->diff[k];
			//materials[i * 3 + k] = object->materialList[object->faceList[i]->material_index]->diff[k];
		}
	}

	return materials;
}

static double* getFaceNormals(objLoader objData, int faceCount){
	/*int arraySize = objData->normalCount * 3;
	double *normals = (double*)malloc(arraySize * sizeof(double));

	for (int i = 0; i < faceCount; i++){
	for (int k = 0; k < 3; k++){
	normals[3*i + k] = objData->normalList[3 * (objData->faceList[i]->normal_index) + k];
	}
	}*/
}

int *FacesToMats(objLoader* object, int faceCount){
	int arraySize = faceCount;
	int *faceMats = (int*)malloc(arraySize * sizeof(int));

	// Each face
	for (int i = 0; i < faceCount; i++){
		faceMats[i] = object->faceList[i]->material_index;
	}

	return faceMats;
}

int *FacesToVerts(obj_face** faces, int faceCount){
	int arraySize = faceCount * 3;
	int *output = (int*)malloc(arraySize * sizeof(int));

	// Each face
	for (int i = 0; i < faceCount; i++){

		// Each vert
		for (int k = 0; k < 3; k++){
			output[i * 3 + k] = faces[i]->vertex_index[k];//printf("   Vert: %d", output[i * 3 + k]);
		}
	}

	return output;
}

objLoader * parseObj(){
	objLoader *objData = new objLoader();
	objData->load("test.obj");

	//
	const int faceAmount = objData->faceCount;

	printf("Number of vertices: %i\n", objData->vertexCount);
	printf("Number of faces: %i\n", objData->faceCount);
	printf("Number of materials: %i\n", objData->materialCount);
	printf("\n");

	printf("Number of faces: %i\n", objData->faceCount);
	printf("Material List: %d\n", objData->materialList[objData->faceList[0]->material_index]->diff);

	return objData;
}

// Log information
static void printProgramInfoLog(GLuint obj)
{
	GLint infologLength = 0, charsWritten = 0;
	glGetProgramiv(obj, GL_INFO_LOG_LENGTH, &infologLength);
	if (infologLength > 2) {
		GLchar* infoLog = new GLchar[infologLength];
		glGetProgramInfoLog(obj, infologLength, &charsWritten, infoLog);
		std::cerr << infoLog << std::endl;
		delete infoLog;
	}
}


// Create the vertex and fragment shaders
GLuint LoadShader(string vertex_path, string fragment_path) {
	GLuint vertShader = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragShader = glCreateShader(GL_FRAGMENT_SHADER);

	// Read shaders
	std::string vertShaderStr = readFile(vertex_path.c_str());
	std::string fragShaderStr = readFile(fragment_path.c_str());
	const char *vertShaderSrc = vertShaderStr.c_str();
	const char *fragShaderSrc = fragShaderStr.c_str();

	GLint result = GL_FALSE;
	int logLength;

	// Compile vertex shader
	std::cout << "Compiling vertex shader." << std::endl;
	glShaderSource(vertShader, 1, &vertShaderSrc, NULL);
	glCompileShader(vertShader);

	// Check vertex shader
	glGetShaderiv(vertShader, GL_COMPILE_STATUS, &result);
	glGetShaderiv(vertShader, GL_INFO_LOG_LENGTH, &logLength);


	// Compile fragment shader
	std::cout << "Compiling fragment shader." << std::endl;
	//std::cout << fragShaderSrc << std::endl;
	glShaderSource(fragShader, 1, &fragShaderSrc, NULL);
	glCompileShader(fragShader);

	GLint compiled;
	glGetShaderiv(fragShader, GL_COMPILE_STATUS, &compiled);
	//mada_check_gl_error();
	if (!compiled)
	{
		GLint length;
		glGetShaderiv(fragShader, GL_INFO_LOG_LENGTH, &length);
		//mada_check_gl_error();
		if (length > 0)
		{
			GLint infoLength;
			char* infoBuf = (char*)malloc(sizeof(char)* length);
			glGetShaderInfoLog(fragShader, length, &infoLength, infoBuf);
			printf("ERROR: %s", infoBuf);
			//mada_check_gl_error();
			//mada_log(logERROR, infoBuf);
			//SysUtils::error("Error compiling shader. See log for info.");
			free(infoBuf);
		}
		//SysUtils::error("Failed to compile shader. No further info available.");
	}

	// Check fragment shader
	glGetShaderiv(fragShader, GL_COMPILE_STATUS, &result);
	glGetShaderiv(fragShader, GL_INFO_LOG_LENGTH, &logLength);

	std::cout << "Linking program" << std::endl;
	GLuint program = glCreateProgram();
	glAttachShader(program, vertShader);
	glAttachShader(program, fragShader);
	glLinkProgram(program);

	printProgramInfoLog(program);   // verifies if all this is ok so far

	glGetProgramiv(program, GL_LINK_STATUS, &result);
	glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);
	std::vector<char> programError((logLength > 1) ? logLength : 1);
	glGetProgramInfoLog(program, logLength, NULL, &programError[0]);
	std::cout << &programError[0] << std::endl;

	glDeleteShader(vertShader);
	glDeleteShader(fragShader);

	return program;
}

// loads the vertex shader and fragment shader, and links them to make the global gProgram
static void LoadShaders() {
    std::vector<tdogl::Shader> shaders;
    shaders.push_back(tdogl::Shader::shaderFromFile(ResourcePath("vertex-shader.txt"), GL_VERTEX_SHADER));
	shaders.push_back(tdogl::Shader::shaderFromFile(ResourcePath("fragment-shader.txt"), program_object));
    gProgram = new tdogl::Program(shaders);
}


// Our GL INITS
bool init(void){
	glClearColor(0.7f, 0.7f, 0.7f, 0.5f);	// Black Background

	glUseProgram(program_object);

	/* PARSING OBJECTS BITCHES */
	std::cout << "Starting to send memory: " << std::endl;
	objLoader* loadedObject = parseObj();

	// export verts
	float* vertArray = VertsToFloat3(loadedObject->vertexList, loadedObject->vertexCount);

	// export faces
	int* faceArray = FacesToVerts(loadedObject->faceList, loadedObject->faceCount);
	float* materials = GetObjectMaterials(loadedObject, loadedObject->faceCount);
	int* faceMats = FacesToMats(loadedObject, loadedObject->faceCount);

	//create texture A
	glEnable(GL_TEXTURE_2D);
	glGenTextures(1, &colorTextureID);
	glBindTexture(GL_TEXTURE_2D, colorTextureID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 512, 512, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

	//create fboA and attach texture A to it
	glGenFramebuffersEXT(1, &framebufferID);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, framebufferID);
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, colorTextureID, 0);

	GLenum status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
	if (status != GL_FRAMEBUFFER_COMPLETE_EXT)
		printf("NOT COMPLETE!!!!\n\n");

	printf("GL_VERSION:%s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));

	glUniform1fv(glGetUniformLocation(program_object, "verts"), loadedObject->vertexCount * 3, vertArray);
	glUniform1iv(glGetUniformLocation(program_object, "faces"), loadedObject->faceCount * 3, faceArray);
	glUniform1iv(glGetUniformLocation(program_object, "faceMat"), loadedObject->faceCount, faceMats);
	glUniform1fv(glGetUniformLocation(program_object, "Materials"), loadedObject->materialCount * 3, materials);
	glUniform1i(glGetUniformLocation(program_object, "faceCount"), loadedObject->faceCount);
	std::cout << "Finished sending memory: FaceCount:" << loadedObject->faceCount << std::endl;


	// -----------------------------------------//
	// GEOMETRY //
	GLuint blockIndex = glGetUniformBlockIndex(program_object, "BlobSettings");
	GLint blockSize;

	glGetActiveUniformBlockiv(program_object, blockIndex, GL_UNIFORM_BLOCK_DATA_SIZE, &blockSize);

	GLubyte * blockBuffer = (GLubyte *)malloc(blockSize);

	// Query for the offsets of each block variable
	const GLchar *names[] = { "InnerColor", "OuterColor",
		"RadiusInner", "RadiusOuter" };

	GLuint indices[4];
	glGetUniformIndices(program_object, 4, names, indices);

	GLint offset[4];
	glGetActiveUniformsiv(program_object, 4, indices, GL_UNIFORM_OFFSET, offset);

	//Place the data into the buffer at the appropriate offsets.
	GLfloat outerColor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
	GLfloat innerColor[] = { 1.0f, 1.0f, 0.75f, 1.0f };
	GLfloat innerRadius = 0.25f, outerRadius = 0.45f;

	memcpy(blockBuffer + offset[0], innerColor,
		4 * sizeof(GLfloat));
	memcpy(blockBuffer + offset[1], outerColor,
		4 * sizeof(GLfloat));
	memcpy(blockBuffer + offset[2], &innerRadius,
		sizeof(GLfloat));
	memcpy(blockBuffer + offset[3], &outerRadius,
		sizeof(GLfloat));

	//Create the OpenGL buffer object and copy the data into it.
	GLuint uboHandle;

	glGenBuffers(1, &uboHandle);
	glBindBuffer(GL_UNIFORM_BUFFER_EXT, uboHandle);
	glBufferData(GL_UNIFORM_BUFFER_EXT, blockSize, blockBuffer,
		GL_DYNAMIC_DRAW);

	// Bind the buffer object to the uniform block.
	glBindBufferBase(GL_UNIFORM_BUFFER_EXT, blockIndex, uboHandle);

	return true;
}

// draws a single frame
static void Render() {
    // clear everything
    glClearColor(0, 0, 0, 1); // black
    glClear(GL_COLOR_BUFFER_BIT);
    
    // bind the program (the shaders)
    glUseProgram(program_object);
        
    // bind the VAO (the triangle)
    glBindVertexArray(gVAO);
    
    // draw the VAO
    glDrawArrays(GL_TRIANGLES, 0, 3);
    
    // unbind the VAO
    glBindVertexArray(0);
    
    // unbind the program
    glUseProgram(0);
    
    // swap the display buffers (displays what was just drawn)
    glfwSwapBuffers(gWindow);
}

void OnError(int errorCode, const char* msg) {
    throw std::runtime_error(msg);
}

// the program starts here
void AppMain() {
    // initialise GLFW
    glfwSetErrorCallback(OnError);
    if(!glfwInit())
        throw std::runtime_error("glfwInit failed");
    
    // open a window with GLFW
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    gWindow = glfwCreateWindow((int)SCREEN_SIZE.x, (int)SCREEN_SIZE.y, "OpenGL Tutorial", NULL, NULL);
    if(!gWindow)
        throw std::runtime_error("glfwCreateWindow failed. Can your hardware handle OpenGL 3.2?");

    // GLFW settings
    glfwMakeContextCurrent(gWindow);
    
    // initialise GLEW
    glewExperimental = GL_TRUE; //stops glew crashing on OSX :-/
    if(glewInit() != GLEW_OK)
        throw std::runtime_error("glewInit failed");

    // print out some info about the graphics drivers
    std::cout << "OpenGL version: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "GLSL version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
    std::cout << "Vendor: " << glGetString(GL_VENDOR) << std::endl;
    std::cout << "Renderer: " << glGetString(GL_RENDERER) << std::endl;

    // make sure OpenGL version 3.2 API is available
    if(!GLEW_VERSION_3_2)
        throw std::runtime_error("OpenGL 3.2 API is not available.");

    // load vertex and fragment shaders into opengl
	char currentPath[FILENAME_MAX];
	_getcwd(currentPath, sizeof(currentPath));

	string something = currentPath;
	string something2 = currentPath;

	//aprogram_object2 = LoadShader("./resources/VertexShader.vert", "./resources/FragmentShader2.frag");
	program_object = LoadShader(something.append("\\resources\\VertexShader.vert"), something2.append("\\resources\\FragmentShader.frag"));

    // create buffer and fill it with the points of the triangle
    //LoadTriangle();

    // run while the window is open
    while(!glfwWindowShouldClose(gWindow)){
        // process pending events
        glfwPollEvents();
        
        // draw one frame
        Render();
    }

    // clean up and exit
	//glfwTerminate();
}


int main(int argc, char *argv[]) {
    //try {
    AppMain();
    //} catch (const std::exception& e){
        //std::cerr << "ERROR: " << e.what() << std::endl;
        //return EXIT_FAILURE;
    //}

    return EXIT_SUCCESS;
}
