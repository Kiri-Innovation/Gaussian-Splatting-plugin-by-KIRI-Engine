#include "Render/GaussianRenderer.h"
#include "Common/Utils.h"
#include "stb_image_write.h"
#include "Common/Global.h"
#include "chrono"
#include <algorithm>
#include <iomanip> 
#include <Plugin/3DGS_PF.h>
#include "UI/UIPanel.h"
#include "UI/ColorGradientUI.h"
#include "UI/CmdArbitraryCallBackHandler.h"
#include "Render/GaussianSorter.h"


GaussianRenderer::GaussianRenderer() {
    
    g_openGLManager.BindContext();

    computeShaderProgramInfo = g_openGLManager.CreateShaderProgram(ShaderProgramType::PointAnimate);
    shaderProgramInfo = g_openGLManager.CreateShaderProgram(ShaderProgramType::Main);
    postEffectProgramInfo = g_openGLManager.CreateShaderProgram(ShaderProgramType::PostEffectDof);
    postEffectGlowBrightProgramInfo = g_openGLManager.CreateShaderProgram(ShaderProgramType::PostEffectGlowBright);
    gaussianBlurVerticalProgramInfo = g_openGLManager.CreateShaderProgram(ShaderProgramType::GaussianBlurVertical);
    gaussianBlurHorizontalProgramInfo = g_openGLManager.CreateShaderProgram(ShaderProgramType::GaussianBlurHorizontal);
    postEffectGlowBrightUpsampleProgramInfo = g_openGLManager.CreateShaderProgram(ShaderProgramType::PostEffectGlowUpSample);
    postEffectGlowBlendProgramInfo = g_openGLManager.CreateShaderProgram(ShaderProgramType::PostEffectGlowBlend);

    //computeShaderProgramInfo = g_openGLManager.CreateComputeProgram();
    splatMeshInfo     = g_openGLManager.CreateSplatMesh();
    g_openGLManager.GetGLError();

    g_openGLManager.UnbindContext();

};

GaussianRenderer::~GaussianRenderer() {
    g_openGLManager.BindContext();

    g_openGLManager.DeleteRenderTarget(renderTargetInfo);
    g_openGLManager.DeleteSplatMesh(splatMeshInfo);
    g_openGLManager.DeleteShaderProgram(shaderProgramInfo);

    g_openGLManager.UnbindContext();
};


void GaussianRenderer::Render(GaussianModel& gaussianModel,
                              ShaderInput& shaderInput,
                              const AEStreamValueInfo& streamValue)
{

    std::lock_guard<std::recursive_mutex> lock(renderMutex);
    PLOGI <<"GaussianRenderer::Render ";
    PLOGI << shaderInput.renderBlock.splatCount;

    //// clean up unuse asset 
    //renderFrameCount++;
    //if (renderFrameCount % 100 == 0) {
    //    renderFrameCount = 1;
    //    g_assetManager.CleanUp();
    //}

    auto start = std::chrono::high_resolution_clock::now();

    GaussianRenderInfo& renderInfo = shaderInput.renderBlock;

    if (width != renderInfo.viewport[0] || height != renderInfo.viewport[1]) {
        g_openGLManager.DeleteRenderTarget(renderTargetInfo);
        width = renderInfo.viewport[0];
        height = renderInfo.viewport[1];
        renderTargetInfo = g_openGLManager.CreateRenderTarget(width, height);
        glowRenderTargetInfo = g_openGLManager.CreateGlowRenderTarget(width, height);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, renderTargetInfo.FBO);
    
    glViewport(0, 0, width, height);
    glClearColor(0., 0., 0., 0.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (gaussianModel.splatCount == 0 || shaderInput.splatEnable==0) {
        return;
    }
    
    // 1. ComputeAnimatedSplatData
    auto afterRunComputeTestBegin = std::chrono::high_resolution_clock::now();
    ComputeAnimatedSplatData(gaussianModel , shaderInput, posViewBufferSpan);
    auto afterRunComputeTestEnd = std::chrono::high_resolution_clock::now();
    
    // 2. sort depth from camera
    auto afterComputeDepthBufferBegin = std::chrono::high_resolution_clock::now();
    GaussianSorter::Sort(gaussianModel , shaderInput, streamValue , posViewBufferSpan , SortComputeType::CPU);
    auto afterComputeDepthBufferEnd = std::chrono::high_resolution_clock::now();

    // 3. UBO shader binding
    auto afterWriteUniformBufferBegin = std::chrono::high_resolution_clock::now();
    WriteUniformBuffer(gaussianModel, shaderInput);
    auto afterWriteUniformBufferEnd = std::chrono::high_resolution_clock::now();


    // 4. render pass
    auto afterGlFinishBegin = std::chrono::high_resolution_clock::now();
    {
        glUseProgram(shaderProgramInfo.program);

        
        glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
        // out
        glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderTargetInfo.GetCurrentColorTexture(), 0);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, renderTargetInfo.depthTexture, 0);

        GLenum postPassBuffers[] = { GL_COLOR_ATTACHMENT0 , GL_COLOR_ATTACHMENT1 };
        glDrawBuffers(std::size(postPassBuffers), postPassBuffers);

        glClear(GL_COLOR_BUFFER_BIT);

        // =========== TBO ===========
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_BUFFER, gaussianModel.depthIndexTboInfo.TBOTexure);
        glUniform1i(GetUniformLoc(shaderProgramInfo , "u_depthIndexData"), 0);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_BUFFER, gaussianModel.tboInfo.TBOTexure);
        glUniform1i(GetUniformLoc(shaderProgramInfo , "u_TBOsplats"), 1);

        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_BUFFER, gaussianModel.viewPositionBufferInfo.TBOTexure);
        glUniform1i(GetUniformLoc(shaderProgramInfo , "u_viewPositionBuffer"), 2);
        g_openGLManager.GetGLError();
        // =========== TBO ===========


        glBindVertexArray(splatMeshInfo.VAO);
        glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0, renderInfo.instanceCount);
        glBindVertexArray(0);
    }
    auto afterGlFinishEnd = std::chrono::high_resolution_clock::now();

    // 5. post effect :: dof
    auto dofBegin = std::chrono::high_resolution_clock::now();
    if (shaderInput.renderBlock.advancedDofEnable == 1.0)
    {
        glUseProgram(postEffectProgramInfo.program);

        // out 
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderTargetInfo.GetNextColorTexture(), 0);
        
        GLenum postPassBuffers[] = { GL_COLOR_ATTACHMENT0 };
        glDrawBuffers(std::size(postPassBuffers), postPassBuffers);

        glClear(GL_COLOR_BUFFER_BIT);

        // in 
        glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D, renderTargetInfo.GetCurrentColorTexture());
        glActiveTexture(GL_TEXTURE1); glBindTexture(GL_TEXTURE_2D, renderTargetInfo.depthTexture);

        //glUniform1i(glGetUniformLocation(postEffectProgramInfo.program, "u_RawColorTexture"), 0);
        //glUniform1i(glGetUniformLocation(postEffectProgramInfo.program, "u_DepthTexture"), 1);
        
        glUniform1i(GetUniformLoc(postEffectProgramInfo, "u_RawColorTexture"), 0);
        glUniform1i(GetUniformLoc(postEffectProgramInfo, "u_DepthTexture"), 1);
        // screen space render 
        glBindVertexArray(splatMeshInfo.VAO);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        glBindVertexArray(0);
        
        glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D, 0);
        glActiveTexture(GL_TEXTURE1); glBindTexture(GL_TEXTURE_2D, 0);

        //GLenum originAttachments[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
        //glDrawBuffers(std::size(originAttachments), originAttachments);

        //glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, renderTargetInfo.colorTexture, 0);
        //glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, renderTargetInfo.depthTexture, 0);
        
        renderTargetInfo.SwapColorTexture();

        g_openGLManager.GetGLError();
    }
    auto dofEnd = std::chrono::high_resolution_clock::now();

    // 5. post effect :: glow
    auto glowBegin = std::chrono::high_resolution_clock::now();
    //if (shaderInput.renderBlock.advancedGlowEnable == 1.0)
    if (shaderInput.renderBlock.advancedGlowEnable == 1.0)
    {
        PLOGD << "RENDER GLOW";
        GLboolean blendWasEnabled = glIsEnabled(GL_BLEND);
        glDisable(GL_BLEND);
        // 5.1 bright pass
        {
            PLOGD << "RENDER GLOW BRIGHT PASS";
            glUseProgram(postEffectGlowBrightProgramInfo.program);
            glBindFramebuffer(GL_FRAMEBUFFER, glowRenderTargetInfo.FBO);

            //glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D, renderTargetInfo.colorTexture);
            // out
            glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, glowRenderTargetInfo.glowBrightTexure,0);
            glClearColor(0.0f,0.0f,0.0f,0.0f);

            // in 
            glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D, renderTargetInfo.GetCurrentColorTexture());
            glUniform1i(GetUniformLoc(postEffectGlowBrightProgramInfo, "u_RawColorTexture"), 0);

            // screen space render 
            glBindVertexArray(splatMeshInfo.VAO);
            glDrawArrays(GL_TRIANGLES, 0, 3);
            glBindVertexArray(0);
            
            if (0)
            {
                std::vector<unsigned char> pixels(width * height * 4);
                
                glBindTexture(GL_TEXTURE_2D, glowRenderTargetInfo.glowBrightTexure);
                //glBindTexture(GL_TEXTURE_2D, renderTargetInfo.colorTexture);
                glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
                glBindTexture(GL_TEXTURE_2D, 0);
                
                int result = stbi_write_png("C:\\Users\\xiaoh\\Desktop\\glow_test_plugin\\glow_bright.png", width, height, 4, pixels.data(), 0);
            }
            
            //glActiveTexture(GL_TEXTURE1); glBindTexture(GL_TEXTURE_2D, 0);

            g_openGLManager.GetGLError();
        }
        // 5.2 generate gaussian blur mipmap
        //if (false)
        {
            PLOGD << "RENDER GLOW BRIGHT MIPMAP PASS";
            GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0 };
            glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
            glDrawBuffers(1, drawBuffers);

            for (int i = 0; i < GLOW_MIPMAP_LEVEL_COUNT; i++) {

                int inputWidth  = i == 0 ? width  : width  >> (i-1);
                int inputHeight = i == 0 ? height : height >> (i-1);
               
                int outputWidth  =  width  >> i ;
                int outputHeight =  height >> i ;

                float texelX = 1.0 / inputWidth;
                float texelY = 1.0 / inputHeight;

                // horizontal gaussian blur 
                {
                    glUseProgram(gaussianBlurHorizontalProgramInfo.program);
                    glViewport(0, 0, outputWidth, outputHeight);

                    // out
                    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, glowRenderTargetInfo.blurTempTexture[i], 0);
                    glClear(GL_COLOR_BUFFER_BIT);

                    // in
                    if (i == 0) {
                        glActiveTexture(GL_TEXTURE1); glBindTexture(GL_TEXTURE_2D, glowRenderTargetInfo.glowBrightTexure);
                    }
                    else {
                        glActiveTexture(GL_TEXTURE1); glBindTexture(GL_TEXTURE_2D, glowRenderTargetInfo.mipmapTexture[i-1]);
                    }

                    glUniform1i(GetUniformLoc(gaussianBlurHorizontalProgramInfo, "u_GlowBrightTexture"), 1);
                    glUniform2f(GetUniformLoc(gaussianBlurHorizontalProgramInfo, "u_TexelSize"), texelX, texelY);
                    glBindVertexArray(splatMeshInfo.VAO);
                    glDrawArrays(GL_TRIANGLES, 0, 3);
                }

                // vertical gaussian blur 
                {
                    glUseProgram(gaussianBlurVerticalProgramInfo.program);
                    // out
                    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, glowRenderTargetInfo.mipmapTexture[i], 0);
                    glClear(GL_COLOR_BUFFER_BIT);
                    // in
                    glActiveTexture(GL_TEXTURE1); glBindTexture(GL_TEXTURE_2D, glowRenderTargetInfo.blurTempTexture[i]);

                    glUniform1i(GetUniformLoc(gaussianBlurVerticalProgramInfo, "u_GlowBrightTexture"), 1);
                    glUniform2f(GetUniformLoc(gaussianBlurVerticalProgramInfo, "u_TexelSize"), texelX, texelY);
                    glBindVertexArray(splatMeshInfo.VAO);
                    glDrawArrays(GL_TRIANGLES, 0, 3);
                }

                // write test
                if (0)
                {
                    int saveWidth  = outputWidth;
                    int saveHeight = outputHeight;
                    std::vector<unsigned char> pixels(saveWidth* saveHeight * 4);
                    glBindTexture(GL_TEXTURE_2D, glowRenderTargetInfo.mipmapTexture[i]);
                    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
                    glBindTexture(GL_TEXTURE_2D, 0);
                    
                    std::string filename = "C:\\Users\\xiaoh\\Desktop\\glow_test_plugin\\glow_bright_blur_" + std::to_string(i) + ".png";
                    int result = stbi_write_png(filename.c_str(), saveWidth, saveHeight, 4, pixels.data(), 0);
                }
            }

            g_openGLManager.GetGLError();

        }
        // 5.3 upsample
        {
            PLOGD << "RENDER GLOW UPSAMPLE PASS";
            glUseProgram(postEffectGlowBrightUpsampleProgramInfo.program);

            // out
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, glowRenderTargetInfo.glowBrightTexure, 0);
            glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
            glViewport(0, 0, width, height);

            // in 
            for (int i = 0; i < GLOW_MIPMAP_LEVEL_COUNT; i++) {
                int level = 1 << i;
                glActiveTexture(GL_TEXTURE0 + i ); glBindTexture(GL_TEXTURE_2D, glowRenderTargetInfo.mipmapTexture[i]);
                std::string uniformName = "u_GlowBrightMipmapTexture" + std::to_string(level);
                glUniform1i(GetUniformLoc(postEffectGlowBrightUpsampleProgramInfo, uniformName.c_str()), i);
            }

            // screen space render 
            glBindVertexArray(splatMeshInfo.VAO);
            glDrawArrays(GL_TRIANGLES, 0, 3);
            glBindVertexArray(0);

            if (0)
            {
                std::vector<unsigned char> pixels(width * height * 4);
                
                glBindTexture(GL_TEXTURE_2D, glowRenderTargetInfo.glowBrightTexure);
                //glBindTexture(GL_TEXTURE_2D, renderTargetInfo.colorTexture);
                glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
                glBindTexture(GL_TEXTURE_2D, 0);
                
                int result = stbi_write_png("C:\\Users\\xiaoh\\Desktop\\glow_test_plugin\\glow_bright_upsample.png", width, height, 4, pixels.data(), 0);
            }
          
        }
        // 5.4 blend
        {
            PLOGD << "RENDER GLOW BLEND PASS";
            glUseProgram(postEffectGlowBlendProgramInfo.program);

            // out
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D , renderTargetInfo.GetNextColorTexture(), 0);
            glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

            //out
            glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D, renderTargetInfo.GetCurrentColorTexture());
            glUniform1i(GetUniformLoc(postEffectGlowBlendProgramInfo, "u_RawColorTexture"), 0);
            glActiveTexture(GL_TEXTURE1); glBindTexture(GL_TEXTURE_2D, glowRenderTargetInfo.glowBrightTexure);
            glUniform1i(GetUniformLoc(postEffectGlowBlendProgramInfo, "u_GlowBrightTexture"), 1);

            // screen space render 
            glBindVertexArray(splatMeshInfo.VAO);
            glDrawArrays(GL_TRIANGLES, 0, 3);
            glBindVertexArray(0);

            renderTargetInfo.SwapColorTexture();
            if (0)
            {
                std::vector<unsigned char> pixels(width * height * 4);

                glBindTexture(GL_TEXTURE_2D, renderTargetInfo.GetCurrentColorTexture());
                //glBindTexture(GL_TEXTURE_2D, renderTargetInfo.colorTexture);
                glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
                glBindTexture(GL_TEXTURE_2D, 0);

                int result = stbi_write_png("C:\\Users\\xiaoh\\Desktop\\glow_test_plugin\\glow_bright_blend.png", width, height, 4, pixels.data(), 0);
            }

        }
        if (blendWasEnabled) {
            glEnable(GL_BLEND);
        }
    }
    auto glowEnd = std::chrono::high_resolution_clock::now();


    std::ostringstream oss;
    g_openGLManager.GetGLError();
    
    glBindBuffer(GL_TEXTURE_BUFFER, 0);

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> elapsed = end - start;
    auto runComputeTestElapsed = std::chrono::duration<double, std::milli>(afterRunComputeTestEnd - afterRunComputeTestBegin);
    auto computeDepthBufferElapsed = std::chrono::duration<double, std::milli>(afterComputeDepthBufferEnd - afterComputeDepthBufferBegin);
    auto writeUniformBufferElapsed = std::chrono::duration<double, std::milli>(afterWriteUniformBufferEnd - afterWriteUniformBufferBegin);
    auto glFinishElapsed = std::chrono::duration<double, std::milli>(afterGlFinishEnd - afterGlFinishBegin);

    auto appendStageTiming = [&](const char* stageName, double stageMs) {
        double totalMs = elapsed.count();
        double relativePercent = totalMs > 0.0 ? (stageMs / totalMs) * 100.0 : 0.0;
        double fps = stageMs > 0.0 ? 1000.0 / stageMs : 0.0;
        oss << stageName
            << " : " << stageMs << " ms"
            << " | " << relativePercent << "%"
            << " | " << fps << " fps"
            << std::endl;
    };

    oss.str("");
    oss << "GaussianRenderer Stage Timing" << std::endl;
    appendStageTiming("RunComputeTest", runComputeTestElapsed.count());
    appendStageTiming("ComputeDepthBuffer", computeDepthBufferElapsed.count());
    appendStageTiming("WriteUniformBuffer", writeUniformBufferElapsed.count());
    appendStageTiming("glFinish", glFinishElapsed.count());
    oss << "Total"
        << " : " << elapsed.count() << " ms"
        << " | 100%"
        << " | " << (elapsed.count() > 0.0 ? 1000.0 / elapsed.count() : 0.0) << " fps"
        << std::endl;
    oss << " render frame rate : " << 1000 / elapsed.count() << " " << elapsed.count() << std::endl;
    PLOGI <<oss.str();

};

int GaussianRenderer::GetUniformLoc(const ShaderProgramInfo& shaderProgramInfo , const std::string& uniformName) {
    auto it = uniformLocCache.find(uniformName);
    if (it != uniformLocCache.end()) {
        PLOGI <<"find uniform Loc " + std::to_string(it->second) ;
        return it->second;
    }
    else {
        int locId = glGetUniformLocation(shaderProgramInfo.program, uniformName.c_str());
        if(locId == GL_INVALID_INDEX) {
            PLOGI <<"no loc " + uniformName;
            locId = glGetUniformBlockIndex(shaderProgramInfo.program, uniformName.c_str());
            if (locId == GL_INVALID_INDEX) {

                PLOGI <<"no UniformBlock loc " + uniformName;
            }
        }

        g_openGLManager.GetGLError();
        uniformLocCache[uniformName] = locId;
        return locId;
    }
}
void GaussianRenderer::WriteUniformBuffer(const GaussianModel& gaussainModel, 
                                          const ShaderInput& shaderInput) {

    g_openGLManager.GetGLError();

    // =========== UBO ===========
    //renderInfo Uniform
    int binding = 0;

    GLint renderInfoUBOIdx = glGetUniformBlockIndex(shaderProgramInfo.program, "RenderInfoBuffer");
    glUniformBlockBinding(shaderProgramInfo.program, renderInfoUBOIdx, binding);
    GetRenderInfoUBO(shaderInput.renderBlock, renderInfoUBO);
    glBindBufferBase(GL_UNIFORM_BUFFER, binding , renderInfoUBO.UBO);
    binding++;
    g_openGLManager.GetGLError();


    GLint colorGradientCursorUBOIdx = glGetUniformBlockIndex(shaderProgramInfo.program, "ColorGradientBuffer");
    glUniformBlockBinding(shaderProgramInfo.program, colorGradientCursorUBOIdx, binding);
    GetColorGradientCursorUBO(shaderInput.colorGradientBlock ,colorGradientCursorUBO);
    glBindBufferBase(GL_UNIFORM_BUFFER, binding, colorGradientCursorUBO.UBO);
    binding++;
    g_openGLManager.GetGLError();

    GLint bezierCurveUBOIdx = glGetUniformBlockIndex(shaderProgramInfo.program, "BezierCurveBuffer");
    glUniformBlockBinding(shaderProgramInfo.program, bezierCurveUBOIdx, binding);
    GetBezierCurveInfoUBO(shaderInput.bezierCurveBlock, bezierCurveUBO);
    glBindBufferBase(GL_UNIFORM_BUFFER, binding, bezierCurveUBO.UBO);
    binding++;
    g_openGLManager.GetGLError();

    PLOGI <<"finish WriteUniformBuffe";

}


void GaussianRenderer::GetColorGradientCursorUBO(
    const std::vector<ColorGradientInfoGpu>& colorGradientBlock,
    UBOInfo& colorGradientCursorUBO
){

    PLOGI <<"GetColorGradientCursorUBO";

    if (colorGradientCursorUBO.uniqueID == -1) {
        colorGradientCursorUBO = g_openGLManager.CreateUBO(
            colorGradientBlock.size() * sizeof(colorGradientBlock[0]), 
            colorGradientBlock.data());

        g_openGLManager.GetGLError();
    }
    else {
        glBindBuffer(GL_UNIFORM_BUFFER, colorGradientCursorUBO.UBO);
        glBufferSubData(
            GL_UNIFORM_BUFFER, 
            0,
            colorGradientBlock.size() * sizeof(colorGradientBlock[0]),
            colorGradientBlock.data());

        //glGetBufferSubData(GL_TEXTURE_BUFFER, 0, colorGradientCursorBuffer.size() * sizeof(float), colorGradientCursorBuffer.data());

        //std::ostringstream oss;
        //oss.str("");
        //oss << "TEST DATA " << std::fixed << std::setprecision(6) << colorGradientCursorBuffer[0] << " " << colorGradientCursorBuffer[1] ;
        //PLOGI <<oss.str());

        glBindBuffer(GL_UNIFORM_BUFFER, 0);
    }


    g_openGLManager.GetGLError();
}

void GaussianRenderer::GetBezierCurveInfoUBO(
    const std::vector<BezierCurveInfoGpu>& bezierCurveBlock,
    UBOInfo& bezierCurveUBO) {

    PLOGI <<"GetBezierCurveInfoUBO";

    if (bezierCurveUBO.uniqueID == -1) {
        bezierCurveUBO = g_openGLManager.CreateUBO(
            bezierCurveBlock.size() * sizeof(bezierCurveBlock[0]),
            bezierCurveBlock.data());

        g_openGLManager.GetGLError();
    }
    else {
        glBindBuffer(GL_UNIFORM_BUFFER, bezierCurveUBO.UBO);
        glBufferSubData(
            GL_UNIFORM_BUFFER,
            0,
            bezierCurveBlock.size() * sizeof(bezierCurveBlock[0]),
            bezierCurveBlock.data());

        //glGetBufferSubData(GL_TEXTURE_BUFFER, 0, colorGradientCursorBuffer.size() * sizeof(float), colorGradientCursorBuffer.data());

        //std::ostringstream oss;
        //oss.str("");
        //oss << "TEST DATA " << std::fixed << std::setprecision(6) << colorGradientCursorBuffer[0] << " " << colorGradientCursorBuffer[1] ;
        //PLOGI <<oss.str());

        glBindBuffer(GL_UNIFORM_BUFFER, 0);
    }


    g_openGLManager.GetGLError();
}


void GaussianRenderer::GetRenderInfoUBO(const GaussianRenderInfo& renderInfo, UBOInfo& uboInfo) {

    PLOGI <<"GetRenderInfoUBO";

    if (uboInfo.uniqueID == -1) {
        uboInfo = g_openGLManager.CreateUBO(sizeof(renderInfo), &renderInfo);
        g_openGLManager.GetGLError();
    }
    else {
        glBindBuffer(GL_UNIFORM_BUFFER, uboInfo.UBO);

        glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(renderInfo), &renderInfo);

        //std::vector<GaussianRenderInfo> cpuData(1);
        //glGetBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(renderInfo), cpuData.data());
        //
        //std::ostringstream oss;
        //oss.str("");
        //oss << "UNIFORM BUFFER " << cpuData[0].colorShapeCenter.a << cpuData[0].colorShapeCenter.r << cpuData[0].colorShapeCenter.g << cpuData[0].colorShapeCenter.b;
        //PLOGI <<oss.str());

        g_openGLManager.GetGLError();

        glBindBuffer(GL_UNIFORM_BUFFER, 0);
    }
    g_openGLManager.GetGLError();
}



RenderResult GaussianRenderer::GetRenderResult(const ShaderInput& shaderInput) {

    PLOGI <<"GaussianRenderer::GetRenderResult";
    std::ostringstream oss;
    auto start = std::chrono::high_resolution_clock::now();

    auto beforeResize = std::chrono::high_resolution_clock::now();
    if (renderResult.width != width || renderResult.height != height) {
        renderResult.width = width;
        renderResult.height = height;

        unsigned int dataSize = width * height * 4;
        renderResult.pixelPtr.reset(new unsigned char[dataSize]);
    }
    auto afterResize = std::chrono::high_resolution_clock::now();

    glBindFramebuffer(GL_FRAMEBUFFER, renderTargetInfo.FBO);
    glFramebufferTexture2D( GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D, renderTargetInfo.GetCurrentColorTexture(), 0);
    glReadBuffer(GL_COLOR_ATTACHMENT0);

    // color buffer 
    auto beforeReadPixels = std::chrono::high_resolution_clock::now();
    glReadPixels(
        0, 0,                 
        width, height,        
        GL_RGBA,
        GL_UNSIGNED_BYTE,     //GL_RGBA8
        renderResult.pixelPtr.get()
    );
    auto afterReadPixels = std::chrono::high_resolution_clock::now();

    // depth buffer
    //glReadBuffer(GL_COLOR_ATTACHMENT1);
    //std::vector<unsigned char> depthResult(width * height * 4, 0);
    //glReadPixels(
    //    0, 0,
    //    width, height,
    //    GL_RGBA,
    //    GL_UNSIGNED_BYTE,     //GL_RGBA8   
    //    depthResult.data()
    //);


    //SavePng("D:/project/AE_GS_Test/render.png" , renderResult);
    //SaveJpg("D:/project/AE_GS_Test/render.jpg", renderResult);

    auto end = std::chrono::high_resolution_clock::now();
    double totalMs = std::chrono::duration<double, std::milli>(end - start).count();
    double resizeMs = std::chrono::duration<double, std::milli>(afterResize - beforeResize).count();
    double readPixelsMs = std::chrono::duration<double, std::milli>(afterReadPixels - beforeReadPixels).count();

    auto appendStageTiming = [&](const char* stageName, double stageMs) {
        double relativePercent = totalMs > 0.0 ? (stageMs / totalMs) * 100.0 : 0.0;
        double fps = stageMs > 0.0 ? 1000.0 / stageMs : 0.0;
        oss << stageName
            << " : " << stageMs << " ms"
            << " | " << relativePercent << "%"
            << " | " << fps << " fps"
            << std::endl;
    };

    oss << "GetRenderResult Stage Timing" << std::endl;
    appendStageTiming("ResizeRenderResultBuffer", resizeMs);
    appendStageTiming("glReadPixels", readPixelsMs);
    oss << "Total"
        << " : " << totalMs << " ms"
        << " | 100%"
        << " | " << (totalMs > 0.0 ? 1000.0 / totalMs : 0.0) << " fps"
        << std::endl;
    PLOGI <<oss.str();

    PLOGI <<"GaussianRenderer::GetRender    Result end";

    return renderResult;
};


void GaussianRenderer::SavePng(const char* filePath, const RenderResult& renderResult) {

    int result = stbi_write_png(filePath, renderResult.width, renderResult.height, 4, renderResult.pixelPtr.get(), renderResult.width * 4);
    std::ostringstream oss;
    if (result) {
        oss << "Image saved successfully: " << filePath << std::endl;
    }
    else {
        oss << "Error saving image!" << std::endl;
    }
    PLOGI <<oss.str().c_str();
}

void GaussianRenderer::SaveJpg(const char* filePath, const RenderResult& renderResult) {
    //stbi_flip_vertically_on_write(true);
    int result = stbi_write_jpg(filePath,
        renderResult.width,
        renderResult.height,
        4,
        renderResult.pixelPtr.get(),
        90);

    std::ostringstream oss;
    if (result) {
        oss << "Image saved successfully (JPG): " << filePath << std::endl;
    }
    else {
        oss << "Error saving JPG image!" << std::endl;
    }
    PLOGI <<oss.str().c_str();
}

void GaussianRenderer::ComputeAnimatedSplatData(
    const GaussianModel& gaussianModel, 
    const ShaderInput& shaderInput , 
    std::span<float>& posViewBufferSpan) {


    int bufferbyteSize = gaussianModel.position.size() * sizeof(float);
    if (currentSplatCount != gaussianModel.splatCount) {
        posViewBuffer.resize(gaussianModel.splatCount * 3);
        currentSplatCount = gaussianModel.splatCount;

#if defined(_WIN32)
        g_openGLManager.DeleteSSBO(positionSSBO);
        g_openGLManager.DeleteSSBO(viewPositionSSBO);

        positionSSBO = g_openGLManager.CreateSSBO(bufferbyteSize, gaussianModel.position.data());
        viewPositionSSBO = g_openGLManager.CreateSSBO(bufferbyteSize, gaussianModel.position.data());
        g_openGLManager.GetGLError();
#endif 
    }

#if defined(__APPLE__)
    g_metalManager.RunComputeAnimate(shaderInput, gaussianModel.position, posViewBufferSpan);
#else
    

    PLOGD << " write buffer to gpu";
    // write buffer to gpu
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, positionSSBO.SSBO);
    g_openGLManager.GetGLError();

    GetBezierCurveInfoUBO(shaderInput.bezierCurveBlock, bezierCurveUBO);
    glBindBufferBase(GL_UNIFORM_BUFFER, 1, bezierCurveUBO.UBO);
    g_openGLManager.GetGLError();

    GetRenderInfoUBO(shaderInput.renderBlock, renderInfoUBO);
    glBindBufferBase(GL_UNIFORM_BUFFER, 2, renderInfoUBO.UBO);
    g_openGLManager.GetGLError();

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, viewPositionSSBO.SSBO);
    g_openGLManager.GetGLError();

    glUseProgram(computeShaderProgramInfo.program);
    g_openGLManager.GetGLError();

    // group size may be need to be enlarged here
    GLuint localSize = 256;
    GLuint groupCount = (gaussianModel.splatCount + localSize - 1) / localSize;
    glDispatchCompute(groupCount, 1, 1);
    g_openGLManager.GetGLError();

    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_BUFFER_UPDATE_BARRIER_BIT);
    g_openGLManager.GetGLError();

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, viewPositionSSBO.SSBO);
    g_openGLManager.GetGLError();

    
    glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, bufferbyteSize, posViewBuffer.data());
    posViewBufferSpan = std::span<float>(posViewBuffer.data() , posViewBuffer.size());

    g_openGLManager.GetGLError();

    //if (posViewBuffer.data()) {
    //
    //    for (int i = 0; i < 10; i++) {
    //        PLOGD << posViewBufferSpan[i];
    //    }
    //}
    //else {
    //    PLOGE << "zViewBuffer is null";
    //}
    
     g_openGLManager.GetGLError();

#endif


}


