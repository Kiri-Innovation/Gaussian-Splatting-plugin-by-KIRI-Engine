#pragma once 

#if defined(_WIN32)
#include <glad/gl.h>
#include <GLFW/glfw3.h>
#endif

#if defined(__APPLE__)
//#include <OpenGL/CGLTypes.h>
#include <OpenGL/CGLCurrent.h>
//#include <OpenGL/CGLMacro.h>
//#include <OpenGL/CGLContext.h>
#include <OpenGL/OpenGL.h>
#include <OpenGL/gl3.h>
#include <pthread.h>
#endif

#include <glm/glm.hpp>
#include <memory.h>
#include "Shader.h"


typedef struct TextureBufferInfo {
	uint64_t uniqueID = -1;
	GLuint TBO;
	GLuint TBOTexure;
}TextureBufferInfo;

typedef struct SSBOInfo {
	uint64_t uniqueID = -1;
	GLuint SSBO;
}SSBOInfo;

typedef struct UBOInfo {
	uint64_t uniqueID = -1;
	GLuint UBO;
}UBOInfo;

typedef struct SplatMeshInfo {
	uint64_t uniqueID = -1;
	GLuint VAO;
	GLuint VBO;
	GLuint EBO;
}SplatMeshInfo;

typedef struct ShaderProgramInfo {
	uint64_t uniqueID = -1;
	GLuint program;
}ShaderProgramInfo;

typedef struct RenderTargetInfo {
	uint64_t uniqueID = -1;
	GLuint FBO;
	GLuint finalOutputTexture;
	GLuint colorTexture;
	GLuint depthTexture;
	GLuint depthRBO;
}RenderTargetInfo;

class OpenGLManager {

public : 
	OpenGLManager();
	~OpenGLManager();

	void BindContext();
	void UnbindContext();
	void GetGLError();

	TextureBufferInfo  CreateTextureBuffer(int byteCount, const void* dataPtr , bool isStatic);
	SSBOInfo		   CreateSSBO(int byteCount, const void* dataPtr);
	UBOInfo			   CreateUBO(int byteCount, const void* dataPtr);
	SplatMeshInfo      CreateSplatMesh();
	RenderTargetInfo   CreateRenderTarget(int width, int height);
	ShaderProgramInfo  CreateShaderProgram(ShaderProgramType type);
	//ShaderProgramInfo  CreateComputeProgram();


	void DeleteRenderTarget(RenderTargetInfo & info);
	void DeleteTextureBuffer(TextureBufferInfo& info);
	void DeleteShaderProgram(ShaderProgramInfo& info);
	void DeleteSplatMesh(SplatMeshInfo& info);
	void DeleteSSBO(SSBOInfo& info);
	void DeleteUBO(UBOInfo& info);
	GLint* GetWorkGroupCount();
	GLint* GetWorkGroupSize();

private :
	void InitOpenGLWin();
    void InitOpenGLMac();

	int uniqueID = 0;

	GLuint CompileShader(GLenum type, const char* source);
	void LogRenderInfoUniformIndex(GLuint program, const char* uniformName);

	std::unordered_map<int , TextureBufferInfo>  textureBuffers = {};
	std::unordered_map<int , SplatMeshInfo>      splatMeshes = {};
	std::unordered_map<int , RenderTargetInfo>   renderTargets = {};
	std::unordered_map<int , ShaderProgramInfo>  shaderPrograms = {};
	std::unordered_map<int , SSBOInfo>			 SSBOs = {};
	std::unordered_map<int , UBOInfo>			 UBOs = {};


	GLint workGroupCount[3] = { 0 };
	GLint workGroupSize[3] = { 0 };

#if defined(_WIN32)
	GLFWwindow* window = NULL;
#elif defined(__APPLE__)
    CGLContextObj context = NULL;
#endif

};
