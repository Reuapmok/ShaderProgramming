#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "stb_image.h"

#include <glm/glm/glm.hpp>
#include <glm/glm/gtc/matrix_transform.hpp>
#include <glm/glm/gtc/type_ptr.hpp>


#include <iostream>

#include "stb_image.h"
#include "Camera.h"
#include "Shader.h"
#include "TextRenderer.h"
#include "tables.h"




// FreeType
//#include <ft2build.h>
//#include FT_FREETYPE_H

#include <iostream>


// CALLBACK FUNCTIONS
//
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);
unsigned int loadTexture(const char *path);

// SOFT SHADOW 
void renderScene(const Shader &shader);
void renderCube();
void renderQuad();

void softShadowMapping();

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

// meshes
unsigned int planeVAO;

// quality control
unsigned int amount = 0;// 10;


// WINDOW SETTINGS
//
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// CAMERA
//
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;
int scrollInput = 0;


// TIME
//
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;

float wholeTime = 0.0f;



// VERTICES POINTS 96 x 96 for MARCHING CUBES
//
float dummyVertices[96 * 96 * 2];
void dummyFunction();

const float SLICE_HEIGHT = (256.0f * 2.0f / (96.0f * 256.0f));


// LIGTHING INFO
// 
//glm::vec3 lightPos(0.0f, 3.0f, 2.0f);
glm::vec3 lightPos(-0.5f, 1.5f, 2.0f);


// PARTICLE SYSTEM INFO
//
bool swapVBO = true;
const int MAX_PARTICLES = 10000;
int renderVertexNumber = 0;


bool marchingCubes = false;
bool particles = false;
bool softShadowMap = false;
bool tesselation = false;
bool wireframeMode = false;

float innerTesselationFactor = 2.0f;
float outerTesselationFactor = 2.0f;


int main()
{
	// glfw: initialize and configure
	// ------------------------------
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4.3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// glfw window creation
	// --------------------
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	// SOFT SHADOW
	glfwSetKeyCallback(window, key_callback);	// key Callback

	// tell GLFW to capture our mouse
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// glad: load all OpenGL function pointers
	// ---------------------------------------
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	// configure global opengl state
	// -----------------------------
	glEnable(GL_DEPTH_TEST);



	//	densityShader	density_vertex_shader.vs	density_fragment_shader
	Shader screenShader("noise.vert", "noise.frag");
	screenShader.linkProgram();

	// TRANSFORM SHADER PROGRAM
	// ------------------------------------
	Shader marchingCubesShader("marchingCubes.vert", "marchingCubes.frag", "marchingCubes.geom");
	marchingCubesShader.linkProgram();

	// THE TRANSFORM FEEDBACK		[/// DEACTIVATED ///]
	// -----------------------------------------------------------------
	//const char* feedbackVaryings[] = { "outValue" };
	//glTransformFeedbackVaryings(testShader.ID, 1, feedbackVaryings, GL_INTERLEAVED_ATTRIBS);



	// PARTICLE SYSTEM UPDATE SHADER
	// -------------------------------------
	Shader particleShader("ps_update.vert", nullptr, "ps_update.geom");

	// the transform feedback Varyings
	const GLchar* Varyings[4];
	Varyings[0] = "Type1";
	Varyings[1] = "Position1";
	Varyings[2] = "Velocity1";
	Varyings[3] = "Age1";

	glTransformFeedbackVaryings(particleShader.ID, 4, Varyings, GL_INTERLEAVED_ATTRIBS);
	particleShader.linkProgram();


	// PARTICLE SYSTEM RENDER SHADER
	// ------------------------------------
	Shader particleRenderShader("ps_render.vert", "ps_render.frag", "ps_render.geom");
	particleRenderShader.linkProgram();


	// ------------------------------
	//		PARTICLE INFO
	// ------------------------------
	//
	struct Particle
	{
		float type;									// particle type (spawner, normal...)
		glm::vec3 position;							// particle world position
		glm::vec3 velocity;							// particle velocity
		float lifeTime;								// particle lifetime
	};


	// INITIALIZE PARTICLE SYSTEM
	// --------------------------------------------------------------
	Particle Particles[MAX_PARTICLES];				//set size of Array
	memset(Particles, 0, sizeof(Particles));		// zero out Array values


	// ADD FIRST TWO SPAWNER ARRAY TO PARTICLE SYSTEM
	Particles[0].type = 1.0f;
	Particles[0].position = glm::vec3(0.0f, 2.0f, 1.5f);
	Particles[0].velocity = glm::vec3(0.0f, 0.0f, 0.0f);
	Particles[0].lifeTime = 5.0f;

	Particles[1].type = 1.0f;
	Particles[1].position = glm::vec3(0.0f, 2.1f, 1.5f);
	Particles[1].velocity = glm::vec3(0.0f, 0.0f, 0.0f);
	Particles[1].lifeTime = 5.0f;

	// VAO			->			IMPORTANT! is always needed for rendering
	unsigned int particleVAO;
	glGenVertexArrays(1, &particleVAO);

	bool isFirst = true;
	unsigned int currVB = 0;									// Current Vertex Buffer	 HANDLER ID
	unsigned int currTFB = 1;									// Current Transform Feedback Buffer HANDLER ID
	GLuint particleBuffer[2];									// Particle Buffer
	GLuint transformFeedback[2];								// Transform Feedback Buffer

	glGenTransformFeedbacks(2, transformFeedback);				// generate two Transform Feedback Buffer
	glGenBuffers(2, particleBuffer);							// generate two Vertex Buffer


	for (unsigned int i = 0; i < 2; i++) {
		glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, transformFeedback[i]);			// BIND TF-Buffers
		glBindBuffer(GL_ARRAY_BUFFER, particleBuffer[i]);								// BIND V-Buffers
		glBufferData(GL_ARRAY_BUFFER, sizeof(Particles), &Particles, GL_DYNAMIC_DRAW);	// FILL BUFFER AND SET SIZE
		glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, particleBuffer[i]);			// BIND BUFFER TO INDEX 0
	}


	// Additionally for debugging purposes
	// ATOMIC COUNTER	
	unsigned int ac_buffer;
	glGenBuffers(1, &ac_buffer);
	glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, ac_buffer);								// bind buffer
	glBufferData(GL_ATOMIC_COUNTER_BUFFER, sizeof(GLuint), NULL, GL_DYNAMIC_DRAW);	// storage capacity
	glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);										// unbind the buffer


	// texture for particle billboarding
	unsigned int texSnow = loadTexture("resources/snow.png");
	particleRenderShader.use();
	particleRenderShader.setInt("texSnow", 0);


	// MARCHING CUBES SCREEN QUAD
	//
	float densityQuadVertices[] = { // vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
   // positions   // texCoords
   -1.0f,  1.0f,  0.0f, 1.0f,
   -1.0f, -1.0f,  0.0f, 0.0f,
	1.0f, -1.0f,  1.0f, 0.0f,

   -1.0f,  1.0f,  0.0f, 1.0f,
	1.0f, -1.0f,  1.0f, 0.0f,
	1.0f,  1.0f,  1.0f, 1.0f
	};
	// screen quad VAO
	unsigned int densityQuadVAO, densityQuadVBO;
	glGenVertexArrays(1, &densityQuadVAO);
	glGenBuffers(1, &densityQuadVBO);
	glBindVertexArray(densityQuadVAO);
	glBindBuffer(GL_ARRAY_BUFFER, densityQuadVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(densityQuadVertices), &densityQuadVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

	// set the vertices for the marching cubes
	dummyFunction();




	// STEP 3 
	// ------
	// Create input VBO and vertex format

	// draw a "dummy" vertex buffer
	// BUFFER TO PASS 96x96 Vertices to
	// -------------------------------
	unsigned int dummyVAO, dummyVBO;
	glGenVertexArrays(1, &dummyVAO);
	glGenBuffers(1, &dummyVBO);
	glBindVertexArray(dummyVAO);
	glBindBuffer(GL_ARRAY_BUFFER, dummyVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(dummyVertices), &dummyVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);

	//glVertexAttribPointer(0, 1, GL_FLOAT, GL_FALSE, 0, 0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);





	unsigned int diffuseMap = loadTexture("bricks2.jpg");
	unsigned int normalMap = loadTexture("bricks2_normal.jpg");
	unsigned int heightMap = loadTexture("bricks2_disp.jpg");

	//unsigned int diffuseMap = loadTexture("bricks2.jpg");
	//unsigned int normalMap = loadTexture("bricks2_normal.jpg");
	//unsigned int heightMap = loadTexture("bricks2_disp.jpg");




	// load and create a texture 
	// -------------------------
	screenShader.use();
	screenShader.setInt("screenTexture", 0);

	marchingCubesShader.use();
	marchingCubesShader.setInt("screenTexture", 0);
	marchingCubesShader.setInt("diffuseMap", 1);
	marchingCubesShader.setInt("normalMap", 2);
	marchingCubesShader.setInt("depthMap", 3);


	//	[ 1 ]	UNIFORM BUFFER OBJECTS FOR MARCHING CUBES
	//
	unsigned int uniformBlockMCIndex = glGetUniformBlockIndex(marchingCubesShader.ID, "g_mc_lut");
	glUniformBlockBinding(marchingCubesShader.ID, uniformBlockMCIndex, 0);


	//	[ 2 ]	 CREATE THE UNIFORM BUFFER OBJECTS
	//
	unsigned int ubog_mc_lut;
	glGenBuffers(1, &ubog_mc_lut);

	glBindBuffer(GL_UNIFORM_BUFFER, ubog_mc_lut);
	glBufferData(GL_UNIFORM_BUFFER, (256 + 12 * 6) * 16, NULL, GL_STATIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	glBindBufferRange(GL_UNIFORM_BUFFER, 0, ubog_mc_lut, 0, (256 + 12 * 6) * 16);

	//	[ 3 ]	FILL THE BUFFER
	//
	glBindBuffer(GL_UNIFORM_BUFFER, ubog_mc_lut);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, 16 * 256, &case_to_numpolys);							// NUM TO POLYS
	glBufferSubData(GL_UNIFORM_BUFFER, (16 * 256), 16 * 12, &cornerAmask0123);					// CORNERMASK	A0123
	glBufferSubData(GL_UNIFORM_BUFFER, (16 * 256) + (16 * 12), 16 * 12, &cornerAmask4567);		// CORNERMASK	A4567
	glBufferSubData(GL_UNIFORM_BUFFER, (16 * 256) + (16 * 12) * 2, 16 * 12, &cornerBmask0123);	// CORNERMASK	B0123
	glBufferSubData(GL_UNIFORM_BUFFER, (16 * 256) + (16 * 12) * 3, 16 * 12, &cornerBmask4567);	// CORNERMASK	B0123
	glBufferSubData(GL_UNIFORM_BUFFER, (16 * 256) + (16 * 12) * 4, 16 * 12, &vec_start);		// CORNERMASK	B0123
	glBufferSubData(GL_UNIFORM_BUFFER, (16 * 256) + (16 * 12) * 5, 16 * 12, &vec_dir);			// CORNERMASK	B0123
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	glEnable(GL_TEXTURE_3D);	// enable 3D texturing


	// [ 1 ]	UNIFORM BUFFER OBJECTS FOR MARCHING CUBES
	unsigned int uniformBlockTriTableIndex = glGetUniformBlockIndex(marchingCubesShader.ID, "g_mc_lut2");
	glUniformBlockBinding(marchingCubesShader.ID, uniformBlockTriTableIndex, 1);

	// [ 2 ]	 CREATE THE UNIFORM BUFFER OBJECTS
	unsigned int ubog_mc_lut2;
	glGenBuffers(1, &ubog_mc_lut2);

	glBindBuffer(GL_UNIFORM_BUFFER, ubog_mc_lut2);
	glBufferData(GL_UNIFORM_BUFFER, (1280 * 16), NULL, GL_STATIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	glBindBufferRange(GL_UNIFORM_BUFFER, 1, ubog_mc_lut2, 0, (1280 * 16));

	// [ 3 ] FILL THE BUFFER
	glBindBuffer(GL_UNIFORM_BUFFER, ubog_mc_lut2);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, (1280 * 16), &g_triTable);			// TRI_TABLE
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	//
	// -------------------------------------------------


	// ----------------------------------
	//		 3D TEXTURE FRAMEBUFFER 
	// ----------------------------------
	//
	// The framebuffer, which regroups 0, 1, or more textures, and 0 or 1 depth buffer.
	unsigned int framebuffer;
	glGenFramebuffers(1, &framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

	// create a color attachment 3D-texture
	unsigned int texture3D;
	glGenTextures(1, &texture3D);
	glBindTexture(GL_TEXTURE_3D, texture3D);

	glTexImage3D(GL_TEXTURE_3D, 0, GL_R16F, 96, 96, 256, 0, GL_RED, GL_FLOAT, NULL);
	//glTexImage3D(GL_TEXTURE_3D, 0, GL_RED, 800, 600, 0, GL_RED, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	//glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);	// clamp on X
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);	// clamp on Y
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_REPEAT);

	// glFramebufferTexture3D
	glFramebufferTexture3D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_3D, texture3D, 0, 0);

	GLenum DrawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, DrawBuffers); // "1" is the size of DrawBuffers

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
	// execute victory dance
	glBindFramebuffer(GL_FRAMEBUFFER, 0);


	//glDeleteFramebuffers(1, &framebuffer);

	//glDisable(GL_DEPTH_TEST);

	// Set OpenGL options
	//glEnable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// build and compile shaders
	// -------------------------
	Shader shader("shadow_mapping.vert", "shadow_mapping.frag");
	shader.linkProgram();

	Shader simpleDepthShader("shadow_mapping_depth.vert", "shadow_mapping_depth.frag");
	simpleDepthShader.linkProgram();

	Shader blurShader("blurDepth.vert", "blurDepth.frag");
	blurShader.linkProgram();



	// Compile and setup the shader
	Shader textShader("text.vert", "text.frag");
	textShader.linkProgram();
	glm::mat4 projection = glm::ortho(0.0f, static_cast<GLfloat>(SCR_WIDTH), 0.0f, static_cast<GLfloat>(SCR_HEIGHT));
	textShader.use();
	textShader.setMat4("projection", projection);

	TextRenderer FPS = TextRenderer();


	// set up vertex data (and buffer(s)) and configure vertex attributes
	// ------------------------------------------------------------------
	float planeVertices[] = {
		// positions            // normals         // texcoords
		 25.0f, -0.5f,  25.0f,  0.0f, 1.0f, 0.0f,  25.0f,  0.0f,
		-25.0f, -0.5f,  25.0f,  0.0f, 1.0f, 0.0f,   0.0f,  0.0f,
		-25.0f, -0.5f, -25.0f,  0.0f, 1.0f, 0.0f,   0.0f, 25.0f,

		 25.0f, -0.5f,  25.0f,  0.0f, 1.0f, 0.0f,  25.0f,  0.0f,
		-25.0f, -0.5f, -25.0f,  0.0f, 1.0f, 0.0f,   0.0f, 25.0f,
		 25.0f, -0.5f, -25.0f,  0.0f, 1.0f, 0.0f,  25.0f, 25.0f
	};
	// plane VAO
	unsigned int planeVBO;
	glGenVertexArrays(1, &planeVAO);
	glGenBuffers(1, &planeVBO);
	glBindVertexArray(planeVAO);
	glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), planeVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glBindVertexArray(0);

	// load textures
	// -------------
	unsigned int woodTexture = loadTexture("resources/textures/bricks.jpg");


	// configure depth map FBO
	// -----------------------
	const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
	unsigned int depthMapFBO;
	glGenFramebuffers(1, &depthMapFBO);

	// -----------------
	//	DEPTH TEXTURE
	// -----------------
	unsigned int depthMap;
	glGenTextures(1, &depthMap);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	float borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

	// attach depth texture as FBO's depth buffer
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
	//glDrawBuffer(GL_NONE);		// as we are not rendering any color
	//glReadBuffer(GL_NONE);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		return false;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);



	// ----------------------
	//	DEPTH COLOR TEXTURE
	// ----------------------
	unsigned int depthColorMap;
	glGenTextures(1, &depthColorMap);
	glBindTexture(GL_TEXTURE_2D, depthColorMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);


	// AND ATTACH IT TO THE GL_COLOR_ATTACHMENT0
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, depthColorMap, 0);
	//glDrawBuffer(GL_NONE);		// as we are not rendering any color
	//glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);




	float quadVertices[] = { // vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
   // positions   // texCoords
   -1.0f,  1.0f,  0.0f, 1.0f,
   -1.0f, -1.0f,  0.0f, 0.0f,
	1.0f, -1.0f,  1.0f, 0.0f,

   -1.0f,  1.0f,  0.0f, 1.0f,
	1.0f, -1.0f,  1.0f, 0.0f,
	1.0f,  1.0f,  1.0f, 1.0f
	};
	// screen quad VAO
	unsigned int quadVAO1, quadVBO1;
	glGenVertexArrays(1, &quadVAO1);
	glGenBuffers(1, &quadVBO1);
	glBindVertexArray(quadVAO1);
	glBindBuffer(GL_ARRAY_BUFFER, quadVBO1);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));


	unsigned int blurMapFBO;

	// Creating the blur FBO
	glGenFramebuffers(1, &blurMapFBO);
	unsigned int blurMap;
	glGenTextures(1, &blurMap);
	glBindTexture(GL_TEXTURE_2D, blurMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	unsigned int blurMap1;
	glGenTextures(1, &blurMap1);
	glBindTexture(GL_TEXTURE_2D, blurMap1);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	//glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
	//glGenerateMipmap(GL_TEXTURE_2D);


	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	//float borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
	//glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

	//glGenerateMipmap(GL_TEXTURE_2D);

	// AND ATTACH IT TO THE GL_COLOR_ATTACHMENT0
	//glBindFramebuffer(GL_FRAMEBUFFER, blurMapFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, quadVBO1);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, blurMap, 0);
	//glDrawBuffer(GL_NONE);		// as we are not rendering any color
	//glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);


	// ping-pong-framebuffer for blurring
	unsigned int pingpongFBO[2];
	unsigned int pingpongColorbuffers[2];
	glGenFramebuffers(2, pingpongFBO);
	glGenTextures(2, pingpongColorbuffers);
	for (unsigned int i = 0; i < 2; i++)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[i]);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, pingpongColorbuffers[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RG32F, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_RG, GL_FLOAT, NULL);
		//													(bi)linear filtering
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // we clamp to the edge as the blur filter would otherwise sample repeated texture values!
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pingpongColorbuffers[i], 0);
		// also check if framebuffers are complete (no need for depth buffer)
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			std::cout << "Framebuffer not complete!" << std::endl;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);


	// shader configuration
	// --------------------
	shader.use();
	shader.setInt("diffuseTexture", 0);		// map object texture
	shader.setInt("shadowMap", 1);			// map shadowMap texture

	blurShader.use();
	blurShader.setInt("image", 0);		// map object texture




	// TESSELATION
	Shader tesselationShader("tesselationVertexShader.vert", "tesselationFragmentShader.frag", "tesselationGeometryShader.geom", "tesselationControlShader.tesc", "tesselationEvaluationShader.tese");
	//Shader tesselationShader("tesselationVertexShader.vert", "tesselationFragmentShader.frag");

	tesselationShader.linkProgram();
	unsigned int terrainHeightmap = loadTexture("resources/textures/terrain_heightmap.png");
	//tesselationShader.use();

	//float vertices[] = {
	//	-1.0f, 1.0f, 0.0f,
	//	-1.0f, -1.0f, 0.0f,
	//	1.0f, -1.0f, 0.0f,
	//
	//	-1.0f, 1.0f, 0.0f,
	//	1.0f, -1.0f, 0.0f,
	//	1.0f, 1.0f, 0.0f,
	//};

	float vertices[] = {
		0.0f, 1.0f, 0.0f,
		-1.0f, -1.0f, 0.0f,
		1.0f, -1.0f, 0.0f,
	};


	unsigned int VBO, VAO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	// bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	// note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
	// VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
	glBindVertexArray(0);




	// FPS counter
	int frames = 0;
	int printFrame = 0;
	double lastTime = glfwGetTime();




	// ---------------------------
	//			RENDER LOOP
	// ---------------------------
	//
	//
	while (!glfwWindowShouldClose(window))
	{
		// per-frame time logic
		// --------------------
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		++frames;

		if (currentFrame - lastTime >= 1.0)
		{
			printFrame = frames;
			frames = 0;
			lastTime += 1.0;
		}


		// PROCESS INPUT
		// --------------
		processInput(window);

		//glDisable(GL_DEPTH_TEST);
		//glEnable(GL_DEPTH_TEST);

		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		if (softShadowMap) {

			// render
			// ------

			// 1. render depth of scene to texture (from light's perspective)
			// --------------------------------------------------------------
			glm::mat4 lightProjectionMatrix, lightViewMatrix;
			glm::mat4 lightSpaceMatrix;
			float near_plane = 1.0f, far_plane = 7.5f;

			lightProjectionMatrix = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane); // orthographic 
			lightViewMatrix = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
			// light space transformation matrix	->	transforms each world-spacevector into light-space
			lightSpaceMatrix = lightProjectionMatrix * lightViewMatrix;
			// render scene from light's point of view
			simpleDepthShader.use();
			simpleDepthShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);

			glEnable(GL_CULL_FACE);
			// set viewport to shadow-map size
			glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
			glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
			glClear(GL_DEPTH_BUFFER_BIT);
			// set active texture for the shader
			glEnable(GL_DEPTH_TEST); // enable depth testing (is disabled for rendering screen-space quad)
			//glActiveTexture(GL_TEXTURE0);
			//glBindTexture(GL_TEXTURE_2D, woodTexture);
			//glCullFace(GL_FRONT);
			//glCullFace(GL_BACK);
			renderScene(simpleDepthShader);
			//glCullFace(GL_FRONT);
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glDisable(GL_CULL_FACE);
			// reset viewport
			glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);



			glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


			glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


			glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);

			// 2. blur bright fragments with two-pass Gaussian Blur 
		   // --------------------------------------------------
			bool horizontal = true, first_iteration = true;
			//unsigned int amount = 2;// 10;
			blurShader.use();
			//glBindVertexArray(quadVAO);
			for (unsigned int i = 0; i < amount; i++)
			{
				glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[horizontal]);
				blurShader.setInt("horizontal", horizontal);
				glBindTexture(GL_TEXTURE_2D, first_iteration ? depthColorMap : pingpongColorbuffers[!horizontal]);  // bind texture of other framebuffer (or scene if first iteration)
				renderQuad();
				//glDrawArrays(GL_TRIANGLES, 0, 6);
				horizontal = !horizontal;
				if (first_iteration)
					first_iteration = false;
			}
			//glBindVertexArray(0);
			glBindFramebuffer(GL_FRAMEBUFFER, 0);




			// 2. render scene as normal using the generated depth/shadow map  
			// --------------------------------------------------------------
			glm::mat4 biasMatrix(
				0.5, 0.0, 0.0, 0.0,
				0.0, 0.5, 0.0, 0.0,
				0.0, 0.0, 0.5, 0.0,
				0.5, 0.5, 0.5, 1.0
			);
			glm::mat4 depthBiasMVP = biasMatrix * lightSpaceMatrix;

			glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			shader.use();
			glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
			glm::mat4 view = camera.GetViewMatrix();
			shader.setMat4("projection", projection);
			shader.setMat4("view", view);
			// set light uniforms
			shader.setVec3("viewPos", camera.Position);
			shader.setVec3("lightPos", lightPos);
			shader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
			//shader.setMat4("depthBiasMVP", depthBiasMVP);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, woodTexture);
			glActiveTexture(GL_TEXTURE1);
			if (amount != 0)
			{
				glBindTexture(GL_TEXTURE_2D, pingpongColorbuffers[!horizontal]);
			}
			else
			{
				glBindTexture(GL_TEXTURE_2D, depthColorMap);
			}
			renderScene(shader);
		}

		if (marchingCubes) {
			// ------------------------------------------
			//		 TODO: CALCULATE ONLY WHEN > 10 slices	
			// ------------------------------------------
			glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
			glViewport(0, 0, 96, 96);


			// -------------------------------------
			//		RENDER TO 3D TEXTURE
			// -------------------------------------
			screenShader.use();
			glBindVertexArray(densityQuadVAO);
			glBindTexture(GL_TEXTURE_3D, texture3D);

			// ------------------------------
			for (int i = 0; i < 256; i++)
			{
				// layer size
				//screenShader.setInt("layer", i + int(camera.Position.y / SLICE_HEIGHT));
				screenShader.setInt("layer", i + scrollInput);
				glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, texture3D, 0, i);
				glDrawArrays(GL_TRIANGLES, 0, 6);
			}
			//
			//-------------------------------------
			glBindVertexArray(0);
			// now bind back to default framebuffer and draw a quad plane with the attached framebuffer color texture
			glBindFramebuffer(GL_FRAMEBUFFER, 0);

			glViewport(0, 0, 800, 600);


			// ---------------------------------------
			//		MARCHING CUBES RENDER
			// ---------------------------------------

			marchingCubesShader.use();
			glEnable(GL_DEPTH_TEST);

			marchingCubesShader.setVec3("lightPos", lightPos);

			glm::mat4 projectionTest = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
			marchingCubesShader.setMat4("projection", projectionTest);

			// camera/view transformation
			glm::mat4 viewTest = camera.GetViewMatrix();
			marchingCubesShader.setMat4("view", viewTest);

			glm::mat4 model = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
			marchingCubesShader.setMat4("model", model);


			marchingCubesShader.setVec3("viewPos", camera.Position);			// NOT MINE

			glBindVertexArray(dummyVAO);
			//glBindSampler(texture3D, sampler_state);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_3D, texture3D);


			// bind textures on corresponding texture units
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, diffuseMap);

			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, normalMap);

			glActiveTexture(GL_TEXTURE3);
			glBindTexture(GL_TEXTURE_2D, heightMap);


			glDrawArraysInstanced(GL_POINTS, 0, (96 * 96), 256);
			//glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			//glBindSampler(texture3D, 0);
			glBindVertexArray(0);
		}


		// ------------------------------------
		if (particles) {
			// ------------------------
			//		DRAW PARTICLES
			// ------------------------
			//

			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glViewport(0, 0, 800, 600);

			// draw particles
			particleShader.use();
			particleShader.setFloat("elapsedTime", deltaTime);
			//std::cout << "elapsed time: " << deltaTime  << std::endl;

			wholeTime += deltaTime;			// WOULD NEED TO BE STOPPED FROM OVERFLOW
			particleShader.setFloat("wholeTime", wholeTime);
			//std::cout << "whole time: " << wholeTime  << std::endl;

			glBindVertexArray(particleVAO);


			// -------------------------------------------
			//	PARTICLE SYSTEM TRANSFORM FEEDBACK 
			// -------------------------------------------

			// RESET ATOMIC COUNTER TO ZERO
			//
			glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, ac_buffer);
			glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 4, ac_buffer);

			GLuint* ptr = (GLuint*)glMapBufferRange(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(GLuint),
				GL_MAP_WRITE_BIT |
				GL_MAP_INVALIDATE_BUFFER_BIT |
				GL_MAP_UNSYNCHRONIZED_BIT);
			ptr[0] = 0;		// resetting value to zero
			glUnmapBuffer(GL_ATOMIC_COUNTER_BUFFER);
			glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);
			glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 4, ac_buffer);


			// RENDER

			// Swap buffers
			currVB = currTFB;
			currTFB = (currTFB + 1) & 0x1;


			//------------------------
			//	UPDATE PARTICLE SYSTEM
			// ------------------------
			//
			// DISCARD RASTERIZE PRIMITIVES (We dont render to screen anyway)
			glEnable(GL_RASTERIZER_DISCARD);
			glBindBuffer(GL_ARRAY_BUFFER, particleBuffer[currVB]);								// BIND ARRAY BUFFER
			glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, transformFeedback[currTFB]);			// BIND TRANSFORM FEEDBACK BUFFER


			// set up vertex attributes of the Vertex buffer
			glEnableVertexAttribArray(0);
			glEnableVertexAttribArray(1);
			glEnableVertexAttribArray(2);
			glEnableVertexAttribArray(3);

			glVertexAttribPointer(0, 1, GL_FLOAT, GL_FALSE, sizeof(Particle), 0); // type
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), (const GLvoid*)4);	// position
			glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), (const GLvoid*)16);	// velocity
			glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(Particle), (const GLvoid*)28);	// lifetime 
																									// size

			// BEGIN TRANSFORM FEEDBACK CAPTURING
			glBeginTransformFeedback(GL_POINTS);

			// IF IT IS THE FIRST DRAW CALL WE NEED NORMAL DRAW CALL
			if (isFirst)
			{
				glDrawArrays(GL_POINTS, 0, 1);
				isFirst = false;
			}
			else
			{
				// We dont know how many particles will end up in the feedback buffer
				glDrawTransformFeedback(GL_POINTS, transformFeedback[currVB]);
			}
			// END TRANSFORM FEEDBACK CAPTURING
			glEndTransformFeedback();

			// DISABLE stuff
			glDisableVertexAttribArray(0);
			glDisableVertexAttribArray(1);
			glDisableVertexAttribArray(2);
			glDisableVertexAttribArray(3);

			// GET COUNTER NUMBER FROM SHADER
			unsigned int userCounters[1];
			glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, ac_buffer);
			glGetBufferSubData(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(GLuint), userCounters);
			glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);
			int renderVertexNumber = userCounters[0];
			glUnmapBuffer(GL_ATOMIC_COUNTER_BUFFER);

			//std::cout << renderVertexNumber << " why " << std::endl;

			// -------------------
			//	RENDER PARTICLES
			// -------------------
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, texSnow);

			particleRenderShader.use();

			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

			// WE ENABLE RASTERIZING AGAIN AS WE WANT TO RENDER TO SCREEN
			glDisable(GL_RASTERIZER_DISCARD);

			// MODEL, VIEW, PROJECTION
			glm::mat4 projectionPS = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
			particleRenderShader.setMat4("projection", projectionPS);

			glm::mat4 viewPS = camera.GetViewMatrix();
			particleRenderShader.setMat4("view", viewPS);

			glm::mat4 modelPS = glm::mat4(1.0f);						// make sure to initialize matrix to identity matrix first
			particleRenderShader.setMat4("model", modelPS);

			particleRenderShader.setVec3("cameraPos", camera.Position);


			glBindBuffer(GL_ARRAY_BUFFER, particleBuffer[currTFB]);		// BIND RETRIEVED BUFFER FORM TRANSFORM FEEDBACK AS INPUT

			glEnableVertexAttribArray(0);
			glEnableVertexAttribArray(1);

			glVertexAttribPointer(0, 1, GL_FLOAT, GL_FALSE, sizeof(Particle), 0);					// type
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), (const GLvoid*)4);	// position

			// DRAW WITH BUFFER FROM TRANSFORM FEEDBACK
			glDrawTransformFeedback(GL_POINTS, transformFeedback[currTFB]);

			glDisableVertexAttribArray(0);
			glDisableVertexAttribArray(1);

			glDisable(GL_BLEND);
		}


		if (tesselation) {
			if (wireframeMode)
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

			// draw our first triangle
			tesselationShader.use();
			shader.setVec3("lightPos", lightPos);

			glm::mat4 projectionTess = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
			glm::mat4 viewTess = camera.GetViewMatrix();
			tesselationShader.setMat4("projection", projectionTess);
			tesselationShader.setMat4("view", viewTess);

			glm::mat4 modelTess = glm::mat4(1.0f);
			tesselationShader.setMat4("model", modelTess);

			// set Tesselation Faktor
			tesselationShader.setVec4("tesselationFactor", glm::vec4(innerTesselationFactor, outerTesselationFactor, 1.0f, 1.0f));

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, terrainHeightmap);

			glBindVertexArray(VAO); // seeing as we only have a single VAO there's no need to bind it every time, but we'll do so to keep things a bit more organized
			//glDrawArrays(GL_TRIANGLES, 0, 3);
			glDrawArrays(GL_PATCHES, 0, 6);
			//glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
			glBindVertexArray(0); // seeing as we only have a single VAO there's no need to bind it every time, but we'll do so to keep things a bit more organized
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}


		glEnable(GL_BLEND);
		glDepthMask(GL_FALSE);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		FPS.RenderText(textShader, std::to_string(printFrame), 25.0f, 25.0f, 1.0f, glm::vec3(0.5, 0.8f, 0.2f));
		glDepthMask(GL_TRUE);
		glDisable(GL_BLEND);

		// std::to_string(printFrame)

		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		// -------------------------------------------------------------------------------
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// optional: de-allocate all resources once they've outlived their purpose:
	// ------------------------------------------------------------------------
	glDeleteVertexArrays(1, &densityQuadVAO);
	glDeleteBuffers(1, &densityQuadVBO);

	// glfw: terminate, clearing all previously allocated GLFW resources.
	// ------------------------------------------------------------------
	glfwTerminate();
	return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, deltaTime);

	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
		scrollInput += 4.0f;
	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
		scrollInput -= 4.0f;
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (action == GLFW_RELEASE) return;
	if (key == GLFW_KEY_1) {

		marchingCubes = !marchingCubes;
	}
	if (key == GLFW_KEY_2) {


		particles = !particles;
	}
	if (key == GLFW_KEY_3) {

		softShadowMap = !softShadowMap;
	}
	if (key == GLFW_KEY_4) {

		tesselation = !tesselation;
	}

	if (tesselation && key == GLFW_KEY_M)
	{
		wireframeMode = !wireframeMode;
	}

	if (tesselation && key == GLFW_KEY_I) {

		innerTesselationFactor += 0.2f;
	}

	if (tesselation && key == GLFW_KEY_O) {

		outerTesselationFactor += 0.2f;
	}
	if (key == GLFW_KEY_R) {

		amount += 2;
	}
	if (key == GLFW_KEY_T) {

		if (amount > 0)
		{
			amount -= 2;

		}	// Toggle fullscreen flag.


	}

}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(yoffset);
}

// renders the 3D scene
// --------------------
void renderScene(const Shader &shader)
{
	// floor
	glm::mat4 model = glm::mat4(1.0f);
	shader.setMat4("model", model);
	glBindVertexArray(planeVAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	// cubes
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(-1.0f, 0.5f, 0.0));
	model = glm::rotate(model, glm::radians(45.0f), glm::normalize(glm::vec3(1.0, 0.0, 1.0)));
	model = glm::scale(model, glm::vec3(0.5f));
	shader.setMat4("model", model);
	renderCube();
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(2.0f, 0.0f, 1.0));
	model = glm::scale(model, glm::vec3(0.5f));
	shader.setMat4("model", model);
	renderCube();
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(-1.0f, 0.0f, 2.0));
	model = glm::rotate(model, glm::radians(60.0f), glm::normalize(glm::vec3(1.0, 0.0, 1.0)));
	model = glm::scale(model, glm::vec3(0.25));
	shader.setMat4("model", model);
	renderCube();
}


// renderCube() renders a 1x1 3D cube in NDC.
// -------------------------------------------------
unsigned int cubeVAO = 0;
unsigned int cubeVBO = 0;
void renderCube()
{
	// initialize (if necessary)
	if (cubeVAO == 0)
	{
		float vertices[] = {
			// back face
			-1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
			 1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
			 1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f, // bottom-right         
			 1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
			-1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
			-1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, // top-left
			// front face
			-1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
			 1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, // bottom-right
			 1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
			 1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
			-1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, // top-left
			-1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
			// left face
			-1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
			-1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-left
			-1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
			-1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
			-1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-right
			-1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
			// right face
			 1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
			 1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
			 1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-right         
			 1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
			 1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
			 1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-left     
			// bottom face
			-1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
			 1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f, // top-left
			 1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
			 1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
			-1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, // bottom-right
			-1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
			// top face
			-1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
			 1.0f,  1.0f , 1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
			 1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f, // top-right     
			 1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
			-1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
			-1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f  // bottom-left        
		};
		glGenVertexArrays(1, &cubeVAO);
		glGenBuffers(1, &cubeVBO);
		// fill buffer
		glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
		// link vertex attributes
		glBindVertexArray(cubeVAO);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}
	// render Cube
	glBindVertexArray(cubeVAO);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);
}

// renderQuad() renders a 1x1 XY quad in NDC
// -----------------------------------------
unsigned int quadVAO = 0;
unsigned int quadVBO;
void renderQuad()
{
	if (quadVAO == 0)
	{
		float quadVertices[] = {
			// positions        // texture Coords
			-1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
			-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
			 1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
			 1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
		};
		// setup plane VAO
		glGenVertexArrays(1, &quadVAO);
		glGenBuffers(1, &quadVBO);
		glBindVertexArray(quadVAO);
		glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	}
	glBindVertexArray(quadVAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
}

void dummyFunction()
{
	for (int y = 0; y < 96; y++)
	{
		for (int x = 0; x < (96 * 2); x++)
		{
			// x
			if (x % 2 == 0)
			{
				dummyVertices[x + (96 * 2 * y)] = x / 2 * 1.0f / 96.0f;

			}
			// y
			else if (x % 2 == 1)
			{
				dummyVertices[x + (96 * 2 * y)] = y * 1.0f / 96.0f;
			}
		}
	}
}


unsigned int loadTexture(char const * path)

{

	unsigned int textureID;

	glGenTextures(1, &textureID);



	int width, height, nrComponents;

	unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);

	if (data)

	{

		GLenum format;

		if (nrComponents == 1)

			format = GL_RED;

		else if (nrComponents == 3)

			format = GL_RGB;

		else if (nrComponents == 4)

			format = GL_RGBA;



		glBindTexture(GL_TEXTURE_2D, textureID);

		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);

		glGenerateMipmap(GL_TEXTURE_2D);



		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);



		stbi_image_free(data);

	}

	else

	{

		std::cout << "Texture failed to load at path: " << path << std::endl;

		stbi_image_free(data);

	}



	return textureID;

}

void softShadowMapping()
{

}