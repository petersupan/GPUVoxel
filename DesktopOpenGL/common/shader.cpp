#include <stdio.h>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <algorithm>
using namespace std;

#include <stdlib.h>
#include <string.h>

#include <GL/glew.h>

#include "shader.hpp"
#include <common/GLError.h>

unsigned int LoadComputeShader(const char* compute_file_name, const char* contentPath) {
	std::string computeShaderPath = contentPath;
	computeShaderPath += std::string(compute_file_name);

	GLuint shaderId = glCreateShader(GL_COMPUTE_SHADER);
	GLuint programId = glCreateProgram();

	std::string ComputeShaderCode;
	std::ifstream ComputeShaderStream(computeShaderPath.c_str(), std::ios::in);
	if(ComputeShaderStream.is_open()){
		std::string Line = "";
		while(getline(ComputeShaderStream, Line))
			ComputeShaderCode += "\n" + Line;
		ComputeShaderStream.close();
	}else{
		printf("Impossible to open %s. Are you in the right directory ? Don't forget to read the FAQ !\n", computeShaderPath.c_str());
		getchar();
		return 0;
	}

	// Compile Vertex Shader
	printf("Compiling shader : %s\n", computeShaderPath.c_str());
	char const * ComputeSourcePointer = ComputeShaderCode.c_str();
	glShaderSource(shaderId, 1, &ComputeSourcePointer , NULL);

	glCompileShader(shaderId);
    int rvalue;
    glGetShaderiv(shaderId, GL_COMPILE_STATUS, &rvalue);
    if (!rvalue) {
        fprintf(stderr, "Error in compiling the compute shader\n");
        GLchar log[10240];
        GLsizei length;
        glGetShaderInfoLog(shaderId, 10239, &length, log);
        fprintf(stderr, "Compiler log:\n%s\n", log);
        exit(40);
    }
    glAttachShader(programId, shaderId);

	glLinkProgram(programId);
    glGetProgramiv(programId, GL_LINK_STATUS, &rvalue);
    if (!rvalue) {
        fprintf(stderr, "Error in linking compute shader program\n");
        GLchar log[10240];
        GLsizei length;
        glGetProgramInfoLog(programId, 10239, &length, log);
        fprintf(stderr, "Linker log:\n%s\n", log);
        exit(41);
    }   
    glUseProgram(programId);

	return programId;
}


GLuint LoadShaders(const char * vertex_file_name,const char * fragment_file_name, const char* contentPath){

    std::string vertexShaderPath = contentPath;
	vertexShaderPath += std::string(vertex_file_name);

    std::string fragmentShaderPath = contentPath;
	fragmentShaderPath += std::string(fragment_file_name);

	// Create the shaders
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	// Read the Vertex Shader code from the file
	std::string VertexShaderCode;
	std::ifstream VertexShaderStream(vertexShaderPath.c_str(), std::ios::in);
	if(VertexShaderStream.is_open()){
		std::string Line = "";
		while(getline(VertexShaderStream, Line))
			VertexShaderCode += "\n" + Line;
		VertexShaderStream.close();
	}else{
		printf("Impossible to open %s. Are you in the right directory ? Don't forget to read the FAQ !\n", vertexShaderPath.c_str());
		getchar();
		return 0;
	}

	// Read the Fragment Shader code from the file
	std::string FragmentShaderCode;
	std::ifstream FragmentShaderStream(fragmentShaderPath.c_str(), std::ios::in);
	if(FragmentShaderStream.is_open()){
		std::string Line = "";
		while(getline(FragmentShaderStream, Line))
			FragmentShaderCode += "\n" + Line;
		FragmentShaderStream.close();
	}



	GLint Result = GL_FALSE;
	int InfoLogLength;



	// Compile Vertex Shader
	printf("Compiling shader : %s\n", vertexShaderPath.c_str());
	char const * VertexSourcePointer = VertexShaderCode.c_str();
	glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
	glCompileShader(VertexShaderID);

	// Check Vertex Shader
	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if ( InfoLogLength > 0 ){
		std::vector<char> VertexShaderErrorMessage(InfoLogLength+1);
		glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
		printf("%s\n", &VertexShaderErrorMessage[0]);
	}



	// Compile Fragment Shader
	printf("Compiling shader : %s\n", fragmentShaderPath.c_str());
	char const * FragmentSourcePointer = FragmentShaderCode.c_str();
	glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
	glCompileShader(FragmentShaderID);

	// Check Fragment Shader
	glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if ( InfoLogLength > 0 ){
		std::vector<char> FragmentShaderErrorMessage(InfoLogLength+1);
		glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
		printf("%s\n", &FragmentShaderErrorMessage[0]);
	}



	// Link the program
	printf("Linking program\n");
	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
	glLinkProgram(ProgramID);

	// Check the program
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if ( InfoLogLength > 0 ){
		std::vector<char> ProgramErrorMessage(InfoLogLength+1);
		glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
		printf("%s\n", &ProgramErrorMessage[0]);
	}

	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);

	return ProgramID;
}


GLuint LoadShaders(const char * vertex_file_name, const char* geometry_file_name, const char * fragment_file_name, const char* contentPath){

    std::string vertexShaderPath = contentPath;
	vertexShaderPath += std::string(vertex_file_name);

    std::string fragmentShaderPath = contentPath;
	fragmentShaderPath += std::string(fragment_file_name);

    std::string geometryShaderPath = contentPath;
	geometryShaderPath += std::string(geometry_file_name);

	// Create the shaders
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
	GLuint GeometryShaderID = glCreateShader(GL_GEOMETRY_SHADER);

	// Read the Vertex Shader code from the file
	std::string VertexShaderCode;
	std::ifstream VertexShaderStream(vertexShaderPath.c_str(), std::ios::in);
	if(VertexShaderStream.is_open()){
		std::string Line = "";
		while(getline(VertexShaderStream, Line))
			VertexShaderCode += "\n" + Line;
		VertexShaderStream.close();
	}else{
		printf("Impossible to open %s. Are you in the right directory ? Don't forget to read the FAQ !\n", vertexShaderPath.c_str());
		getchar();
		return 0;
	}

	// Read the Fragment Shader code from the file
	std::string FragmentShaderCode;
	std::ifstream FragmentShaderStream(fragmentShaderPath.c_str(), std::ios::in);
	if(FragmentShaderStream.is_open()){
		std::string Line = "";
		while(getline(FragmentShaderStream, Line))
			FragmentShaderCode += "\n" + Line;
		FragmentShaderStream.close();
	}else{
		printf("Impossible to open %s. Are you in the right directory ? Don't forget to read the FAQ !\n", fragmentShaderPath.c_str());
		getchar();
		return 0;
	}

		// Read the Fragment Shader code from the file
	std::string GeometryShaderCode;
	std::ifstream GeometryShaderStream(geometryShaderPath.c_str(), std::ios::in);
	if(GeometryShaderStream.is_open()){
		std::string Line = "";
		while(getline(GeometryShaderStream, Line))
			GeometryShaderCode += "\n" + Line;
		GeometryShaderStream.close();
	}else{
		printf("Impossible to open %s. Are you in the right directory ? Don't forget to read the FAQ !\n", geometryShaderPath.c_str());
		getchar();
		return 0;
	}



	GLint Result = GL_FALSE;
	int InfoLogLength;



	// Compile Vertex Shader
	printf("Compiling shader : %s\n", vertexShaderPath.c_str());
	char const * VertexSourcePointer = VertexShaderCode.c_str();
	glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
	glCompileShader(VertexShaderID);

	// Check Vertex Shader
	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if ( InfoLogLength > 0 ){
		std::vector<char> VertexShaderErrorMessage(InfoLogLength+1);
		glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
		printf("%s\n", &VertexShaderErrorMessage[0]);
	}



	// Compile Fragment Shader
	printf("Compiling shader : %s\n", fragmentShaderPath.c_str());
	char const * FragmentSourcePointer = FragmentShaderCode.c_str();
	glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
	glCompileShader(FragmentShaderID);

	// Check Fragment Shader
	glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if ( InfoLogLength > 0 ){
		std::vector<char> FragmentShaderErrorMessage(InfoLogLength+1);
		glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
		printf("%s\n", &FragmentShaderErrorMessage[0]);
	}

		// Compile Geometry Shader
	printf("Compiling shader : %s\n", geometryShaderPath.c_str());
	char const * GeometrySourcePointer = GeometryShaderCode.c_str();
	glShaderSource(GeometryShaderID, 1, &GeometrySourcePointer , NULL);
	glCompileShader(GeometryShaderID);

	// Check Geometry Shader
	glGetShaderiv(GeometryShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(GeometryShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if ( InfoLogLength > 0 ){
		std::vector<char> GeometryShaderErrorMessage(InfoLogLength+1);
		glGetShaderInfoLog(GeometryShaderID, InfoLogLength, NULL, &GeometryShaderErrorMessage[0]);
		printf("%s\n", &GeometryShaderErrorMessage[0]);
	}



	// Link the program
	printf("Linking program\n");
	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, GeometryShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
	glLinkProgram(ProgramID);

	// Check the program
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if ( InfoLogLength > 0 ){
		std::vector<char> ProgramErrorMessage(InfoLogLength+1);
		glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
		printf("%s\n", &ProgramErrorMessage[0]);
	}

	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);
	glDeleteShader(GeometryShaderID);

	return ProgramID;
}

//AllShaderEffect::AllShaderEffect(unsigned int _programId)
//{
//	programId = _programId;
//	glUseProgram(programId);
//	// Get a handle for our matrix uniforms
//
//	MVPId = glGetUniformLocation(programId, "MVP");
//	VId = glGetUniformLocation(programId, "V");
//	MId = glGetUniformLocation(programId, "M");
//	textureSamplerId = glGetUniformLocation(programId, "myTextureSampler");
//	ndotl_ndotvSamplerId = glGetUniformLocation(programId, "ndotvSampler");
//	ndotl_ndothSamplerId = glGetUniformLocation(programId, "ndothSampler");
//	ndotl_vdotlSamplerId = glGetUniformLocation(programId, "vdotlSampler");
//	volumeSamplerId = glGetUniformLocation(programId, "volumeSampler");
//	fluxId = glGetUniformLocation(programId, "fluxSampler");
//	
//	cubeSamplerId = glGetUniformLocation(programId, "cubeSampler");
//	lightPositionId = glGetUniformLocation(programId, "LightPosition_worldspace");
//	specColorId = glGetUniformLocation(programId, "MaterialSpecularColor");
//	shininessId = glGetUniformLocation(programId, "shininess");
//	metalnessId = glGetUniformLocation(programId, "metalness");
//	specularityId = glGetUniformLocation(programId, "specularity");
//    lightMatrixId = glGetUniformLocation(programId, "modelToLightProjectionMatrix");
//    isShadowCasterId = glGetUniformLocation(programId, "isShadowCaster");
//	shadowMagicNumberId = glGetUniformLocation(programId, "shadowMagicNumber");
//}

ShaderEffect::ShaderEffect(unsigned int _programId)
{
	lightMatrixId = 0xffffffff;
	programId = _programId;
	glUseProgram(programId);
	MVPId = glGetUniformLocation(programId, "MVP");
	VId = glGetUniformLocation(programId, "V");
	MId = glGetUniformLocation(programId, "M");
}

SimpleShaderEffect::SimpleShaderEffect(unsigned int _programId) : ShaderEffect(_programId)
{
	textureSamplerId = glGetUniformLocation(_programId, "myTextureSampler");
	printf("SimpleShaderEffect created, texid %i\n", textureSamplerId);
	lightPositionId = glGetUniformLocation(_programId, "LightPosition_worldspace");
}

VolumeRenderingShaderEffect::VolumeRenderingShaderEffect(unsigned int _programId) : ShaderEffect(_programId)
{
	volumeSamplerId = glGetUniformLocation(_programId, "volumeSampler");
	layerId = glGetUniformLocation(_programId, "layer");

}

// DONE Implementation of a shadow mapping Shader Effet
ShadowMappingShaderEffect::ShadowMappingShaderEffect(unsigned int _programId) : SimpleShaderEffect(_programId)
{
	depthTextureSamplerId = glGetUniformLocation(_programId, "depthSampler");
	ssaoResultSamplerId = glGetUniformLocation(_programId, "SSAOResultSampler");
	lightMatrixId = glGetUniformLocation(programId, "modelToLightProjectionMatrix");
	shadowMagicNumberId = glGetUniformLocation(programId, "shadowMagicNumber");
}

LandscapeGenerationShaderEffect::LandscapeGenerationShaderEffect(unsigned int _programId) : SimpleShaderEffect(_programId)
{
	imageId = glGetUniformLocation(_programId, "outImage");
}

LandscapeImShaderEffect::LandscapeImShaderEffect(unsigned int _programId) : SimpleShaderEffect(_programId)
{
	heightSamplerId = glGetUniformLocation(_programId, "heightSampler");
	cameraPositionId = glGetUniformLocation(_programId, "cameraPosition");
}


