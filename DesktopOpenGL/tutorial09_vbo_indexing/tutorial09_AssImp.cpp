// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <string>

// Include GLEW
#include <GL/glew.h>

//#define MINGW_COMPILER
#define SCREENWIDTH 960
#define SCREENHEIGHT 720

// Include GLFW
#include <glfw3.h>
GLFWwindow* window;

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include <common/shader.hpp>
#include <common/texture.hpp>
#include <common/controls.hpp>
#include <common/objloader.hpp>
#include <common/vboindexer.hpp>
#include <common/Mesh.hpp>
#include <common/GLError.h>
#include <common/RenderState.hpp>

// Include AntTweakBar
#include <AntTweakBar.h>

#define PATHTOCONTENT "../tutorial09_vbo_indexing/"

// Create and compile our GLSL program from the shaders
	std::string contentPath = PATHTOCONTENT;

const char *faceFile[6] = {
	"cm_left.bmp",
	"cm_right.bmp",
	"cm_top.bmp",
	"cm_bottom.bmp",
	"cm_back.bmp",
	"cm_front.bmp"
};

float shadowMagicNumber = 0.003;
unsigned char textureToShow = 0;
unsigned char layerToShow = 0;


struct Scene
{
	std::vector<RenderState*>* objects;
	std::vector<Mesh*>* meshes;
	std::vector<ShaderEffect*>* effects;

	Scene(std::vector<RenderState*>* _obj,
		std::vector<Mesh*>* _meshes,
		std::vector<ShaderEffect*>* _effects) :
			objects(_obj),
			meshes(_meshes),
			effects(_effects)
	{}
};

void renderObjects(Scene& scene, glm::mat4x4& viewMatrix, glm::mat4x4& projectionMatrix, glm::vec3& lightPos, glm::mat4& lightMatrix)
{
	std::vector<RenderState*>* objects = scene.objects;
	#ifdef MINGW_COMPILER
	glm::mat4 modelMatrix = glm::rotate(glm::mat4(1.0f), -90.0f, glm::vec3(1.0f, 0.0f, 0.0f));
	#else
	glm::mat4 modelMatrix = glm::mat4(1.0f);
	#endif
	
    check_gl_error();
	for(int i = 0; i < objects->size(); i++)
	{
		RenderState* rs = (*objects)[i];
		
		unsigned int meshId = (*objects)[i]->meshId;
		Mesh* m = (*scene.meshes)[meshId];
		modelMatrix = m->modelMatrix;
		glm::mat4 MVP = projectionMatrix * viewMatrix * modelMatrix;
		// Use our shader
		unsigned int effectId = (*objects)[i]->shaderEffectId;
		ShaderEffect* effect = (*scene.effects)[effectId];
		glUseProgram(effect->programId);
        check_gl_error();
		rs->setParameters(effect);
		// Send our transformation to the currently bound shader,
		// in the "MVP" uniform
		glUniformMatrix4fv(effect->MVPId, 1, GL_FALSE, &MVP[0][0]);
		glUniformMatrix4fv(effect->MId, 1, GL_FALSE, &modelMatrix[0][0]);
		
		glUniformMatrix4fv(effect->VId, 1, GL_FALSE, &viewMatrix[0][0]);
        check_gl_error();

		//glUniform3f(effect.lightPositionId, lightPos.x, lightPos.y, lightPos.z);
		//glUniform1f(effect.shadowMagicNumberId, shadowMagicNumber);
        check_gl_error();

        if (effect->lightMatrixId != 0xffffffff){
			glm::mat4 lm = lightMatrix * modelMatrix;
			glUniformMatrix4fv(effect->lightMatrixId, 1, GL_FALSE, &lm[0][0]);
		}
		//glUniform1f(effect.shininessId, rs.shininess);
		//glUniform1f(effect.metalnessId, rs.metalness);
		//glUniform1f(effect.specularityId, rs.specularity);
		//glUniform3f(effect.specColorId, rs.specularMaterialColor.r,rs.specularMaterialColor.g, rs.specularMaterialColor.b);
  //      glUniform1i(effect.isShadowCasterId, rs.isShadowCaster);
  //      check_gl_error();

		m->bindBuffersAndDraw();


		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
		glDisableVertexAttribArray(2);
		glDisableVertexAttribArray(3);
		glDisableVertexAttribArray(4);
		check_gl_error();
	}
}

// DONE create FBO for Render to texture
unsigned int createFBO(int width, int height, int nrColorBuffers, std::vector<unsigned int>& textureIds)
{
    // create Framebuffer object
	GLuint framebufferName = 0;
	glGenFramebuffers(1, &framebufferName);
	glBindFramebuffer(GL_FRAMEBUFFER, framebufferName);
	check_gl_error();
    GLenum* drawBuffers = new GLenum[nrColorBuffers];
    
	// DONE create a texture to use as the depth buffer of the Framebuffer object
    GLuint renderedDepthTexture;
	glGenTextures(1, &renderedDepthTexture);
	glBindTexture(GL_TEXTURE_2D, renderedDepthTexture);
	// GL_DEPTH_COMPONENT is important to use the texture as depth buffer and as shadow map later
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
	glTexImage2D(GL_TEXTURE_2D, 1, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
	glTexImage2D(GL_TEXTURE_2D, 2, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
	glTexImage2D(GL_TEXTURE_2D, 3, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
	check_gl_error();
	glGenerateMipmap(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	// compare mode and compare func enable hardware shadow mapping. Otherwise the texture lookup would just
	// return the depth value and we would have to do the shadow comparison by ourselves
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
	// configure framebuffer
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, renderedDepthTexture, 0);
	// push back the depth texture to the textureids
	// this means the depth texture is id 0!
    textureIds.push_back(renderedDepthTexture);
    
	// texture to render to - the color buffers
    for (int i = 0; i < nrColorBuffers; i++)
    {
        GLuint renderedTexture;
        glGenTextures(1, &renderedTexture);
        glBindTexture(GL_TEXTURE_2D, renderedTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_FLOAT, 0);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);//GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);//GL_CLAMP_TO_EDGE);
        check_gl_error();
        textureIds.push_back(renderedTexture);
		// configure the Framebuffer to use the texture as color attachment i
        glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0+i, renderedTexture,0);
		// write into the drawbuffers array that the framebuffer has a color texture at attachment i
        drawBuffers[i] = GL_COLOR_ATTACHMENT0+i;
    }

	// set nrColorBuffers draw buffers for the Framebuffer object
	glDrawBuffers(nrColorBuffers, drawBuffers);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		printf("damn, end of createFBO\n");
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
    delete[] drawBuffers;
    return framebufferName;
}

float calculateDist(glm::vec3 center)
{
	glm::vec3 temp = center*center;
	float distance = sqrt(temp.x + temp.y + temp.z);
	return distance;
}

void initShaders(std::vector<ShaderEffect*>& shaderSets)
{
	// ########## load the shader programs ##########

	GLuint lsImProgramID = LoadShaders("Landscape.vertexshader", "LandscapeIntermediateTopDown.fragmentshader",  contentPath.c_str()); //LandscapeIntermediateTopDown.fragmentshader"
    LandscapeImShaderEffect* lsImProgram = new LandscapeImShaderEffect(lsImProgramID);
	shaderSets.push_back(lsImProgram);

	GLuint lsgenProgramID = LoadShaders("Landscape.vertexshader", "LandscapeTopDown.fragmentshader",  contentPath.c_str());/*LandscapeTopDown.fragmentshader*/
    LandscapeGenerationShaderEffect* lsgenProgram = new LandscapeGenerationShaderEffect(lsgenProgramID);
	shaderSets.push_back(lsgenProgram);

	// DONE Load shaders for just drawing a quad on the screen
	GLuint screenQuadProgramID = LoadShaders("texturedQuad.vertexshader", "texturedQuad.fragmentshader", contentPath.c_str());
	SimpleShaderEffect* screenQuadProgram = new SimpleShaderEffect(screenQuadProgramID);
	shaderSets.push_back(screenQuadProgram);
}

int main( void )
{
	// Initialise GLFW
	if( !glfwInit() )
	{
		fprintf( stderr, "Failed to initialize GLFW\n" );
		return -1;
	}

	glfwWindowHint(GLFW_SAMPLES, 8);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Open a window and create its OpenGL context
	window = glfwCreateWindow( SCREENWIDTH, SCREENHEIGHT, "Tutorial 09 - VBO Indexing", NULL, NULL);
	if( window == NULL ){
		fprintf( stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n" );
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	// Initialize GLEW
	glewExperimental = true; // Needed for core profile
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		return -1;
	}

	// Ensure we can capture the escape key being pressed below
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
	glfwSetCursorPos(window, SCREENWIDTH/2, SCREENHEIGHT/2);

	glm::vec3 lightSpecularColor = glm::vec3(0.3, 0.3, 0.3);
	glm::vec3 lightPos = glm::vec3(-4.64,6.7,-5.7);

	// create GUI for light position
	TwInit(TW_OPENGL_CORE, NULL);
	TwWindowSize(SCREENWIDTH, SCREENHEIGHT);
	TwBar * LightGUI = TwNewBar("Light Settings");
	TwAddVarRW(LightGUI, "LightPos X"  , TW_TYPE_FLOAT, &lightPos.x, "step=0.1");
	TwAddVarRW(LightGUI, "LightPos Y"  , TW_TYPE_FLOAT, &lightPos.y, "step=0.1");
	TwAddVarRW(LightGUI, "LightPos Z"  , TW_TYPE_FLOAT, &lightPos.z, "step=0.1");

	TwAddVarRW(LightGUI, "Shadow magic number"  , TW_TYPE_FLOAT, &shadowMagicNumber, "step=0.0001");
    
    TwAddVarRW(LightGUI, "Texture to show"  , TW_TYPE_UINT8, &textureToShow, "");

	TwAddVarRW(LightGUI, "Layer to show"  , TW_TYPE_UINT8, &layerToShow, "");

	// Set GLFW event callbacks. I removed glfwSetWindowSizeCallback for conciseness
	glfwSetMouseButtonCallback(window, (GLFWmousebuttonfun)TwEventMouseButtonGLFW); // - Directly redirect GLFW mouse button events to AntTweakBar
	glfwSetCursorPosCallback(window, (GLFWcursorposfun)TwEventMousePosGLFW);          // - Directly redirect GLFW mouse position events to AntTweakBar
	glfwSetScrollCallback(window, (GLFWscrollfun)TwEventMouseWheelGLFW);    // - Directly redirect GLFW mouse wheel events to AntTweakBar
	glfwSetKeyCallback(window, (GLFWkeyfun)TwEventKeyGLFW);                         // - Directly redirect GLFW key events to AntTweakBar
	glfwSetCharCallback(window, (GLFWcharfun)TwEventCharGLFW);                      // - Directly redirect GLFW char events to AntTweakBar

	// Dark blue background
	glClearColor(0.0f, 0.0f, 0.4f, 0.0f);

	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	glViewport(0, 0, SCREENWIDTH, SCREENHEIGHT);
	//
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS);

	//glEnable(GL_CULL_FACE);

	glEnable(GL_SAMPLE_ALPHA_TO_COVERAGE);

	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	std::vector<ShaderEffect*> shaderSets;
	std::vector<RenderState*> landscapeGenerationObjects;
	std::vector<RenderState*> landscapeImObjects;
	std::vector<RenderState*> landscapeShowObjects;	

	check_gl_error();

	std::vector<unsigned int> landscapeIds;
	unsigned int landscapeFBOId = createFBO(1024, 1024, 2, landscapeIds);

	initShaders(shaderSets);

	enum ShaderEffects {
		LANDSCAPE_INTERMEDIATE = 0,
		LANDSCAPE_GENERATE,
		TEXTURED_QUAD
	};

	check_gl_error();

// ########### Load the textures ################
	GLuint Texture1 = loadSoil("spongebob.DDS", contentPath.c_str());
	check_gl_error();

	GLuint Texture2 = loadSoil("lichenStone.dds", contentPath.c_str());
	check_gl_error();

	GLuint landscapeHeight = loadSoil(/*"D1.png"*/"D25_Height.png", contentPath.c_str());
	check_gl_error();

    GLuint landscapeColor = loadSoil(/*"C1W.png"*/"D25_Color_Blurred.png", contentPath.c_str());
	check_gl_error();

	GLuint cubeMapTex = loadSoilCubeMap(faceFile, contentPath.c_str());
	check_gl_error();
// ############## Load the meshes ###############
	std::vector<Mesh *> meshes;
	//std::vector<Mesh *> spongeBobMeshes;
	std::string modelPath = contentPath;
	#ifdef MINGW_COMPILER
        modelPath += std::string("ACGR_Scene_GI_Unwrap.dae");
	#else
        modelPath += std::string("ACGR_Scene_GI_Unwrap_II.3ds");
	#endif
	//Mesh::loadAssImp(modelPath.c_str(), meshes, true);	

	//int spongeMeshStartId = meshes.size();
	//std::string spongeBobPath = contentPath;
	//spongeBobPath += std::string("Spongebob/spongebob_bind.obj");
	//Mesh::loadAssImp(spongeBobPath.c_str(), meshes, true);

	//glm::mat4 spongeBobMatrix;
	//spongeBobMatrix = glm::translate(glm::mat4(1.0), glm::vec3(1.0, 1.0, 1.0));
	//
	//for(int i = 0; i < meshes.size(); i++){
	//	// DONE create a SimpleRenderstate for all objects which should cast shadows
	//	SimpleRenderState* rtts = new SimpleRenderState();
	//	rtts->meshId = i;
	//	rtts->shaderEffectId = STANDARDSHADING; // the Render to texture shader effect
	//	rtts->texId = Texture2;
	//	objects.push_back(rtts);
	//}

	//// apply the modelmatrix to the spongebob meshes
	//for(int i = spongeMeshStartId; i < meshes.size(); i++){
	//		meshes[i]->modelMatrix = spongeBobMatrix;
	//}
	//
	//// generate mesh VBOs
	//check_gl_error();
	//for (int i = 0; i < meshes.size(); i++)
	//{
	//	meshes[i]->generateVBOs();
	//}
	//check_gl_error();

	//// create a quad mesh which is about a quarter of the screen big and in the middle of the right side
	//Mesh debugQuad; debugQuad.createQuad(vec2(0.0, -0.5), vec2(1.0, 0.5));
	//debugQuad.generateVBOs();
	//meshes.push_back(&debugQuad);
	//// create a SimpleRenderstate for the quad (just has to show a texture
	//SimpleRenderState* debugQuadState = new SimpleRenderState(); 
	//debugQuadState->meshId = meshes.size()-1;
	//debugQuadState->shaderEffectId = TEXTURED_QUAD;
	//debugQuadState->texId = Texture2;
	//objects.push_back(debugQuadState);
	glDisable(GL_DEPTH_TEST);
	// the intermediate step for the landscape generation
	Mesh landscapeImQuad; landscapeImQuad.createQuad(vec2(-1.0, -1.0), vec2(1.0, 1.0));
	landscapeImQuad.generateVBOs();
	meshes.push_back(&landscapeImQuad);
	// create a SimpleRenderState with the SSAO Shaderset for the Quad
	LandscapeImRenderState* landscapeImQuadState = new LandscapeImRenderState(); 
	landscapeImQuadState->meshId = meshes.size()-1;
	landscapeImQuadState->shaderEffectId = LANDSCAPE_INTERMEDIATE;
	landscapeImQuadState->texId = landscapeColor;
	landscapeImQuadState->heightTexId = landscapeHeight;
	landscapeImObjects.push_back(landscapeImQuadState);

	// create a screen filled quad
	Mesh landscapeGenerationQuad; landscapeGenerationQuad.createQuad(vec2(-1.0, -1.0), vec2(1.0, 1.0));
	landscapeGenerationQuad.generateVBOs();
	meshes.push_back(&landscapeGenerationQuad);
	// create a SimpleRenderState with the SSAO Shaderset for the Quad
	LandscapeGenerationRenderState* landscapeGenerationQuadState = new LandscapeGenerationRenderState(); 
	landscapeGenerationQuadState->meshId = meshes.size()-1;
	landscapeGenerationQuadState->shaderEffectId = LANDSCAPE_GENERATE;
	landscapeGenerationQuadState->outImageId = landscapeIds[2];
	landscapeGenerationQuadState->texId = landscapeIds[1];
	landscapeGenerationObjects.push_back(landscapeGenerationQuadState);

	Mesh fullscreenQuad; fullscreenQuad.createQuad(vec2(-1.0, -1.0), vec2(1.0, 1.0));
	fullscreenQuad.generateVBOs();
	meshes.push_back(&fullscreenQuad);
	// create a SimpleRenderState with the SSAO Shaderset for the Quad
	SimpleRenderState* fullscreenQuadState = new SimpleRenderState(); 
	fullscreenQuadState->meshId = meshes.size()-1;
	fullscreenQuadState->shaderEffectId = TEXTURED_QUAD;
	fullscreenQuadState->texId = landscapeIds[2];
	landscapeShowObjects.push_back(fullscreenQuadState);

	// create the scenes
	enum Scenes {
		LANDSCAPE_INTERMEDIATE_PASS,
		LANDSCAPE_GENERATION_PASS,
		LANDSCAPE_SHOW_PASS
	};

	std::vector<Scene> scenes;
	// one scene for the standard rendering
	scenes.push_back(Scene(&landscapeImObjects, &meshes, &shaderSets));
	scenes.push_back(Scene(&landscapeGenerationObjects, &meshes, &shaderSets));
	scenes.push_back(Scene(&landscapeShowObjects, &meshes, &shaderSets));

	check_gl_error();


	// For speed computation
	double lastTime = glfwGetTime();
	int nbFrames = 0;

	do{ 
		// Apply the scene depth map to the textured quad object to debug.
		// With the gui we can change which texture we see.
		//SimpleRenderState* quadObj = static_cast<SimpleRenderState*>(objects[objects.size()-1]);
		//quadObj->texId = textureToShow;

		// Measure speed
		double currentTime = glfwGetTime();
		nbFrames++;
		if ( currentTime - lastTime >= 1.0 ){ // If last prinf() was more than 1sec ago
			// printf and reset
			printf("%f ms/frame\n", 1000.0/double(nbFrames));
			nbFrames = 0;
			lastTime += 1.0;
		}

		check_gl_error();
		// Clear the screen
		glClearColor(0.0, 0.0, 0.5, 1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		// Compute the MVP matrix from keyboard and mouse input
		computeMatricesFromInputs();
		glm::mat4 ProjectionMatrix = getProjectionMatrix();
		glm::mat4 ViewMatrix = getViewMatrix();
		check_gl_error();

        // compute the MVP matrix for the light
        // worldToView first
        glm::mat4 lightViewMatrix = glm::lookAt(lightPos, lightPos + glm::vec3(0.5, -1.0, 0.5), glm::vec3(0.0, 0.0, 1.0));
        glm::mat4 lightProjMatrix = glm::perspective(90.0f, 1.0f, 2.5f, 100.0f);
        glm::mat4 lightMVPMatrix = lightProjMatrix * lightViewMatrix;

		LandscapeImRenderState::cameraPosition = getPosition();
		glBindFramebuffer(GL_FRAMEBUFFER, landscapeFBOId);
		GLuint drawBuffers = GL_COLOR_ATTACHMENT0;
		//glDrawBuffers(1, &drawBuffers);
		glViewport(0,0,1024,1024);
		glClearColor(0.0, 0.0, 0.0, 0.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		renderObjects(scenes[LANDSCAPE_INTERMEDIATE_PASS], ViewMatrix, ProjectionMatrix, lightPos, lightMVPMatrix);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, SCREENWIDTH, SCREENHEIGHT);

		// clear landscape texture
		//glBindFramebuffer(GL_FRAMEBUFFER, landscapeFBOId);
		//glViewport(0,0,1024, 1024);
		//glClearColor(0.0, 0.0, 0.0, 0.0);
		//glClearBufferfv(landscapeFBOId, 1, 0);
		//glClear(GL_COLOR_BUFFER_BIT);
		//glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// generate landscape
		glViewport(0, 0, 32,32);
		
		renderObjects(scenes[LANDSCAPE_GENERATION_PASS], ViewMatrix, ProjectionMatrix, lightPos, lightMVPMatrix);

		glViewport(0, 0, SCREENWIDTH, SCREENHEIGHT);

		// set the scene constant variales ( light position)
		SimpleRenderState::lightPositionWorldSpace = lightPos;
		// render to the screen buffer
		renderObjects(scenes[LANDSCAPE_SHOW_PASS], ViewMatrix, ProjectionMatrix, lightPos, lightMVPMatrix);

		// draw gui
		TwDraw();
		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();

	} // Check if the ESC key was pressed or the window was closed
	while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
		   glfwWindowShouldClose(window) == 0 );

	// Cleanup VBO and shader
	//glDeleteProgram(programID);
	glDeleteTextures(1, &Texture1);
	//glDeleteTextures(1, &Texture2);
	//glDeleteTextures(1, &Texture3);
	glDeleteVertexArrays(1, &VertexArrayID);


	TwTerminate();
	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	return 0;
}

