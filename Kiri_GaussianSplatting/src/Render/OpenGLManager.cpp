#include "Render/OpenGLManager.h"
#include "Common/Utils.h"
#include "Render/Shader.h"
#include <thread> 
#include "memory.h"
#include <list>
#include "Render/GaussianTypes.h"
#include "Common/Global.h"



OpenGLManager::OpenGLManager() {

#if defined(__APPLE__)
    InitOpenGLMac();
#elif defined(_WIN32)
    InitOpenGLWin();
#endif
}


OpenGLManager::~OpenGLManager() {
    //BindContext();

    std::list<std::pair<int, TextureBufferInfo>> textureBufferList(
        textureBuffers.begin(),
        textureBuffers.end()
    );
    for (auto& [uid, info] : textureBufferList) {
        DeleteTextureBuffer(info);
    }
    std::list<std::pair<int, SplatMeshInfo>>  splatMeshList(
        splatMeshes.begin(),
        splatMeshes.end()
    );
    for (auto& [uid, info] : splatMeshList) {
        DeleteSplatMesh (info);
    }
    std::list<std::pair<int, ShaderProgramInfo>>  shaderProgramList(
        shaderPrograms.begin(),
        shaderPrograms.end()
    );
    for (auto& [uid, info] : shaderProgramList) {
        DeleteShaderProgram(info);
    }
    std::list<std::pair<int, RenderTargetInfo>>  renderTargetList(
        renderTargets.begin(),
        renderTargets.end()
    );
    for (auto& [uid, info] : renderTargetList) {
        DeleteRenderTarget(info);
    }
    std::list<std::pair<int, SSBOInfo>>  SSBOList(
        SSBOs.begin(),
        SSBOs.end()
    );
    for (auto& [uid, info] : SSBOList) {
        DeleteSSBO(info);
    }
    std::list<std::pair<int, UBOInfo>>  UBOList(
        UBOs.begin(),
        UBOs.end()
    );
    for (auto& [uid, info] : UBOList) {
        DeleteUBO(info);
    }
    
#if defined(__APPLE__)
    CGLDestroyContext(context);
#elif defined(_WIN32)
    glfwTerminate();
#endif

    //UnbindContext();
}


#if defined(_WIN32)
inline void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    PLOGI << key << std::endl;
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
}
#endif

inline void get_thread_id_str() {
    PLOGI  << "current thread id " << std::this_thread::get_id();
}

void OpenGLManager::InitOpenGLWin() {

#if defined(_WIN32)
    // Init GLFW
    glfwInit();
    // Set all the required options for GLFW
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(500, 500, "LearnOpenGL", NULL, NULL);

    if (!window) {
        PLOGI <<"GLFW window create failed";
        return;
    }

    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, key_callback);

    if (gladLoadGL(glfwGetProcAddress) == 0) {
        PLOGI <<"gladLoadGL failed";
    }
  
    glEnable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);

    const char* version = (const char*)glGetString(GL_VERSION);
    PLOGI << "OpenGL version: " << version << std::endl;

    glViewport(0, 0, 500, 500);


    for (int i = 0; i < 3; i++) {
        glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, i, &workGroupCount[i]);
    }
    PLOGD << "GL_MAX_COMPUTE_WORK_GROUP_COUNT: "
        << workGroupCount[0] << " , "
        << workGroupCount[1] << " , "
        << workGroupCount[2];

    for (int i = 0; i < 3; i++) {
        glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, i, &workGroupSize[i]);
    }
    PLOGD << "GL_MAX_COMPUTE_WORK_GROUP_SIZE: "
        << workGroupSize[0] << " , "
        << workGroupSize[1] << " , "
        << workGroupSize[2];

    

    glfwHideWindow(window);
    GetGLError();
    get_thread_id_str();

#endif
}

void OpenGLManager::InitOpenGLMac() {

#if defined(__APPLE__)
    CGLError err;
    CGLPixelFormatObj pixelFormat = NULL;
    GLint numVers = 0;

    CGLPixelFormatAttribute pixelFormatAttributes[] = {
      kCGLPFAOpenGLProfile, (CGLPixelFormatAttribute) kCGLOGLPVersion_GL4_Core,
      kCGLPFAColorSize, (CGLPixelFormatAttribute) 24,
      kCGLPFAAlphaSize, (CGLPixelFormatAttribute) 8,
      kCGLPFAAccelerated,
      kCGLPFADoubleBuffer,
      kCGLPFASampleBuffers, (CGLPixelFormatAttribute) 1,
      kCGLPFASamples, (CGLPixelFormatAttribute) 4,
      (CGLPixelFormatAttribute) 0,
    };

    CGLPixelFormatObj pix;
    GLint npix;
    if (CGLChoosePixelFormat(pixelFormatAttributes, &pix, &npix) != kCGLNoError || !pix) {
        PLOGI <<"Failed to choose pixel format";
        return;
    } else {
        PLOGI <<"Chosen pixel format, total options: " + std::to_string(npix);
    }

    err = CGLCreateContext(pix, nullptr, &context);
    if (err != kCGLNoError || !context) {
        PLOGI <<"Failed to create CGL context";
        CGLDestroyPixelFormat(pix);
        return;
    }
    else{
        PLOGI <<"creat CGL context success";
    }
    
    CGLDestroyPixelFormat(pix);
    CGLSetCurrentContext(context);
    const GLubyte* vendor   = glGetString(GL_VENDOR);
    const GLubyte* renderer = glGetString(GL_RENDERER);
    const GLubyte* version  = glGetString(GL_VERSION);
    const GLubyte* glslVer  = glGetString(GL_SHADING_LANGUAGE_VERSION);
    PLOGI <<"OpenGL Vendor: ";
    PLOGI <<(const char*)vendor;
    PLOGI <<"OpenGL Renderer: ";
    PLOGI <<(const char*)renderer;
    PLOGI <<"OpenGL Version: ";  
    PLOGI <<(const char*)version;
    PLOGI <<"GLSL Version: ";
    PLOGI <<(const char*)glslVer;
    
    glEnable(GL_BLEND);
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);

    
    //GLint major = GLVersion.major;
    //GLint minor = GLVersion.minor;
    //PLOGI <<"GLAD reports OpenGL version:" + std::to_string(major) + "." + std::to_string(minor));

#endif
}

void OpenGLManager::GetGLError() {
    GLenum error = glGetError();

    if (error != GL_NO_ERROR) {
        PLOGE  << "OpenGL error occurred: " << error;
    }
}

void OpenGLManager::BindContext() {
#if defined(_WIN32)
    glfwMakeContextCurrent(window);
#elif defined(__APPLE__)
    CGLSetCurrentContext(context);
#endif
}

void OpenGLManager::UnbindContext() {
#if defined(_WIN32)
    glfwMakeContextCurrent(NULL);
#elif defined(__APPLE__)
    CGLSetCurrentContext(NULL);
#endif
}

TextureBufferInfo OpenGLManager::CreateTextureBuffer(int byteCount, const void* dataPtr , bool isStatic){
    //BindContext();

    TextureBufferInfo info;
    info.uniqueID = uniqueID++;

    glGenBuffers(1, &info.TBO);
    glBindBuffer(GL_TEXTURE_BUFFER, info.TBO);
    if (isStatic) {
        glBufferData(GL_TEXTURE_BUFFER, byteCount, dataPtr, GL_STATIC_DRAW);
    }
    else {
        glBufferData(GL_TEXTURE_BUFFER, byteCount, dataPtr, GL_DYNAMIC_READ);
    }

    glGenTextures(1, &info.TBOTexure);
    glBindTexture(GL_TEXTURE_BUFFER, info.TBOTexure);

    glTexBuffer(GL_TEXTURE_BUFFER, GL_R32F, info.TBO);

    //int floatCount = byteCount / sizeof(float);
    //std::vector<float> data(floatCount);
    //
    //glGetBufferSubData(GL_TEXTURE_BUFFER, 0, byteCount, data.data());
    //
    //std::ostringstream oss;
    //
    ////for (int i = 0; i< 10; i++) {
    ////    oss<< data[i] << std::endl;
    ////}
    //
    //PLOGI <<"CreateTextureBuffer");

    //PLOGI <<oss.str().c_str());

    glBindBuffer(GL_TEXTURE_BUFFER, 0);
    glBindTexture(GL_TEXTURE_BUFFER, 0);

    textureBuffers.insert(std::pair<uint64_t, TextureBufferInfo>(info.uniqueID , info));

    //UnbindContext();

    return info;
}

SSBOInfo OpenGLManager::CreateSSBO(int byteCount, const void* dataPtr) {

#if defined(_WIN32)

    SSBOInfo info;
    info.uniqueID = uniqueID++;

    glGenBuffers(1, &info.SSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, info.SSBO);

    glBufferData(GL_SHADER_STORAGE_BUFFER, byteCount, dataPtr, GL_DYNAMIC_DRAW);

    //std::vector<StandardGaussian> testdata(1);
    // 
    //glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(StandardGaussian), testdata.data());
    ////std::ostringstream oss;
    ////oss << testdata[0] << " " << testdata[1] <<" "<< testdata[2] << " "<< testdata[3] << " " <<testdata[4];
    //PLOGI <<testdata[0].toString());
    //GetGLError();

    //glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, info.SSBO);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    SSBOs.insert(std::pair<uint64_t, SSBOInfo>(info.uniqueID, info));

    GetGLError();

    return info;
#endif
}

UBOInfo OpenGLManager::CreateUBO(int byteCount, const void* dataPtr) {

    UBOInfo info;
    info.uniqueID = uniqueID++;
    glGenBuffers(1, &info.UBO);
    glBindBuffer(GL_UNIFORM_BUFFER, info.UBO);

    glBufferData(GL_UNIFORM_BUFFER, byteCount , dataPtr, GL_DYNAMIC_DRAW);
    //glBindBufferBase(GL_UNIFORM_BUFFER, 0, uboID);

    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    UBOs.insert(std::pair<uint64_t, UBOInfo>(info.uniqueID, info));
    return info;
}

SplatMeshInfo OpenGLManager::CreateSplatMesh() {
    SplatMeshInfo info;
    info.uniqueID = uniqueID++;

    float vertices[] = {
      -0.5f,  -0.5f, -0.0f,
      0.5f,   -0.5f, -0.0f,
      0.5f,    0.5f, -0.0f  ,
      -0.5f,   0.5f, -0.0f  ,
    };

    unsigned int indices[] = {
        0, 1, 2 ,
        2, 3, 0
    };

    glGenVertexArrays(1, &info.VAO);
    glGenBuffers(1, &info.VBO);
    glGenBuffers(1, &info.EBO);

    glBindVertexArray(info.VAO);

    glBindBuffer(GL_ARRAY_BUFFER, info.VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, info.EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    splatMeshes.insert(std::pair<int, SplatMeshInfo>(info.uniqueID, info));

    return info;
}

RenderTargetInfo OpenGLManager::CreateRenderTarget(int width, int height) {
    RenderTargetInfo info;
    info.uniqueID = uniqueID++;

    glGenFramebuffers(1, &info.FBO);
    glBindFramebuffer(GL_FRAMEBUFFER, info.FBO);

    // final output
    glGenTextures(1, &info.finalOutputTexture);
    glBindTexture(GL_TEXTURE_2D, info.finalOutputTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, info.finalOutputTexture, 0);

    // raw color
    glGenTextures(1, &info.colorTexture);
    glBindTexture(GL_TEXTURE_2D, info.colorTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, info.colorTexture, 0);

    // depth buffer 
    glGenTextures(1, &info.depthTexture);
    glBindTexture(GL_TEXTURE_2D, info.depthTexture); // 【已修正】绑定正确的深度纹理ID

    //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, info.depthTexture, 0);


    // depth RBO which is required
    glGenRenderbuffers(1, &info.depthRBO);
    glBindRenderbuffer(GL_RENDERBUFFER, info.depthRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, info.depthRBO);

    // 5. 【核心新增】显式激活多目标渲染映射（让 location = 0 和 1 同时通电）
    GLenum attachments[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2};
    glDrawBuffers(std::size(attachments), attachments);

    // 6. 验证 FBO 完整性
    GLenum fboStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (fboStatus != GL_FRAMEBUFFER_COMPLETE) {
        std::ostringstream oss;
        oss << "ERROR::FRAMEBUFFER:: frame buffer is not completed! Status: " << fboStatus
            << " (FBO_ID="   << info.FBO
            << ", ColorTex=" << info.colorTexture
            << ", DepthTex=" << info.depthTexture
            << ", DepthRBO=" << info.depthRBO << ")";
        PLOGI << oss.str();

        // 优雅断后：失败时及时清理防止显存泄漏
        glDeleteFramebuffers(1, &info.FBO);
        glDeleteTextures(1, &info.colorTexture);
        glDeleteTextures(1, &info.depthTexture);
        glDeleteRenderbuffers(1, &info.depthRBO);
        return RenderTargetInfo(); // 返回空结构
    }

    // 7. 解绑状态机，保持上下文干净
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    // 8. 存入管理容器
    renderTargets.insert(std::pair<int, RenderTargetInfo>(info.uniqueID, info));

    return info;
}

GLuint OpenGLManager::CompileShader(GLenum type, const char* source) {

    GLuint shader = glCreateShader(type);

    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    int success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        PLOGE << "ERROR::SHADER_COMPILATION_ERROR of type: " << (type == GL_VERTEX_SHADER ? "VERTEX" : "FRAGMENT") << "\n" << infoLog << std::endl;
        
    }
    return shader;
}

void OpenGLManager::LogRenderInfoUniformIndex(GLuint program, const char* uniformName) {
    GLuint index = GL_INVALID_INDEX;
    glGetUniformIndices(program, 1, &uniformName, &index);

    if (index == GL_INVALID_INDEX) {
        PLOGI <<std::string("uniform index not found: ") + uniformName;
        return;
    }

    GLint offset = -1;
    glGetActiveUniformsiv(program, 1, &index, GL_UNIFORM_OFFSET, &offset);

    std::ostringstream oss;
    oss << uniformName << " index = " << index << ", offset = " << offset;
    PLOGI <<oss.str();
}


ShaderProgramInfo OpenGLManager::CreateShaderProgram(ShaderProgramType type) {

    PLOGI <<"CreateShaderProgram";

    ShaderProgramInfo info;
    info.uniqueID = uniqueID++;

    ShaderProgramSource shaderSource = GetShaderProgramSource(type);

    if (shaderSource.comp == nullptr) {
        GLuint vertexShader = CompileShader(GL_VERTEX_SHADER, shaderSource.vert);
        GLuint fragmentShader = CompileShader(GL_FRAGMENT_SHADER, shaderSource.frag);

        info.program = glCreateProgram();

        glAttachShader(info.program, vertexShader);
        glAttachShader(info.program, fragmentShader);
        glLinkProgram(info.program);
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
    }
    else {
#ifdef _WIN32
        GLuint computeShader = CompileShader(GL_COMPUTE_SHADER, shaderSource.comp);

        info.program = glCreateProgram();
        glAttachShader(info.program, computeShader);
        glLinkProgram(info.program);
        glDeleteShader(computeShader);
#endif
    }

    int success;
    glGetProgramiv(info.program, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(info.program, 512, nullptr, infoLog);
        std::ostringstream oss;
        oss << "ERROR::PROGRAM_LINKING_ERROR\n" << infoLog << std::endl;
        PLOGI <<oss.str();
    }
    
    /*
    const char* names[] = {
        "u_renderInfo.modelMatrix",
        "u_renderInfo.viewMatrix",
        "u_renderInfo.projectionMatrix",
        "u_renderInfo.viewport",
        "u_renderInfo.focalPixelX",
        "u_renderInfo.instanceCount",
        "u_renderInfo.cameraPos",
        "u_renderInfo.colorShapeCenter",

        "u_renderInfo.shDegree",
        "u_renderInfo.colorEnable",
        "u_renderInfo.colorShapeSize",

        "u_renderInfo.cropEnable",
        "u_renderInfo.cropShapeCenter",
        "u_renderInfo.cropInvert",
        "u_renderInfo.cropShapeSize",

        "u_renderInfo.splatScaleEnable",
        "u_renderInfo.splatScaleSize",

        "u_renderInfo.splatNoiseShapeCenter",
        "u_renderInfo.splatNoiseEnable",
        "u_renderInfo.splatNoiseShapeSize",
        "u_renderInfo.splatNoiseOctaves",
        "u_renderInfo.splatNoisePersistence",
        "u_renderInfo.splatNoiseLacunarity",
        "u_renderInfo.splatNoiseStrength",

        "u_renderInfo.splatOpacityEnable",
        "u_renderInfo.splatOpacityShapeSize",
        "u_renderInfo.splatOpacityShapeCenter",
        "u_renderInfo.splatOpacity",

        //"u_colorGradient.currentCursorCount",
        //"u_colorGradient.colorGradientCursor[0].lerpFactor",
        //"u_colorGradient.colorGradientCursor[0].R",
        //"u_colorGradient.colorGradientCursor[0].G",
        //"u_colorGradient.colorGradientCursor[0].B",
        //
        //"u_colorGradient.colorGradientCursor[1].lerpFactor",
        //"u_colorGradient.colorGradientCursor[1].R",
        //"u_colorGradient.colorGradientCursor[1].G",
        //"u_colorGradient.colorGradientCursor[1].B",

        "u_colorRamp.currentPointCount",
        "u_colorRamp.bezierPointInfo[0].x",
        "u_colorRamp.bezierPointInfo[0].y",

        "u_splatScaleRamp.currentPointCount",
        "u_splatScaleRamp.bezierPointInfo[0].x",
        "u_splatScaleRamp.bezierPointInfo[0].y",
    };

    constexpr GLsizei count = sizeof(names) / sizeof(names[0]);

    std::ostringstream oss;
    for (int i = 0; i < count; ++i) {
        LogRenderInfoUniformIndex(info.program, names[i]);
    }
    GetGLError();
    */

    shaderPrograms.insert(std::pair<int, ShaderProgramInfo>(info.uniqueID, info));
    return info;
}
/*
ShaderProgramInfo OpenGLManager::CreateComputeProgram() {
  
#if defined(_WIN32)
    ShaderProgramInfo info;
    info.uniqueID = uniqueID++;

    std::string computeShaderSrc = GetComputeShader();
    GLuint  computeShader = CompileShader(GL_COMPUTE_SHADER, computeShaderSrc.data());

    info.program = glCreateProgram();
    glAttachShader(info.program, computeShader);

    glLinkProgram(info.program);

    int success;
    glGetProgramiv(info.program, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(info.program, 512, nullptr, infoLog);
        std::ostringstream oss;
        oss << "ERROR::PROGRAM_LINKING_ERROR\n" << infoLog << std::endl;
        PLOGI << oss.str();
    }

    glDeleteShader(computeShader);


    shaderPrograms.insert(std::pair<int, ShaderProgramInfo>(info.uniqueID, info));

    return info;
#endif

}
*/

void OpenGLManager::DeleteRenderTarget(RenderTargetInfo& info) {
    auto it = renderTargets.find(info.uniqueID);
    if (it != renderTargets.end() ) {
        glDeleteFramebuffers(1, &info.FBO);
        glDeleteTextures(1, &info.finalOutputTexture);
        glDeleteTextures(1, &info.colorTexture);
        glDeleteTextures(1, &info.depthTexture);
        glDeleteRenderbuffers(1, &info.depthRBO);

        renderTargets.erase(it);
    }
}

void OpenGLManager::DeleteSplatMesh(SplatMeshInfo& info) {
    auto it = splatMeshes.find(info.uniqueID);
    if (it != splatMeshes.end()) {
        glDeleteVertexArrays(1, &info.VAO);
        glDeleteBuffers(1,      &info.VBO);
        glDeleteBuffers(1,      &info.EBO);

        splatMeshes.erase(it);
    }
}

void OpenGLManager::DeleteTextureBuffer(TextureBufferInfo& info) {
    //BindContext();

    auto it = textureBuffers.find(info.uniqueID);
    if (it != textureBuffers.end()) {
        glDeleteBuffers(1,  &info.TBO);
        glDeleteTextures(1, &info.TBOTexure);
    
        textureBuffers.erase(it);
    }

    //UnbindContext();
}

void OpenGLManager::DeleteShaderProgram(ShaderProgramInfo& info){
    auto it = shaderPrograms.find(info.uniqueID);
    if (it != shaderPrograms.end()) {
        glDeleteProgram(info.program);

        shaderPrograms.erase(it);
    }
}


void OpenGLManager::DeleteSSBO(SSBOInfo& info) {
    auto it = SSBOs.find(info.uniqueID);
    if (it != SSBOs.end()) {
        glDeleteBuffers(1, &info.SSBO);
        SSBOs.erase(it);
    }
}


void OpenGLManager::DeleteUBO(UBOInfo& info) {

    auto it = UBOs.find(info.uniqueID);
    if (it != UBOs.end()) {
        glDeleteBuffers(1, &info.UBO);
        UBOs.erase(it);
    }
}



GLint* OpenGLManager::GetWorkGroupCount() {
    return workGroupCount;
}
GLint* OpenGLManager::GetWorkGroupSize() {
    return workGroupSize;
}
