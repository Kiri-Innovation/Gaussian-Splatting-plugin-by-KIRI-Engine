#include "Render/MetalManager.h"
#include "Common/Utils.h"
#include "Render/Shader.h"
#include <chrono>
#include <algorithm>
#include <cstring>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>
#include <span>
#include "Render/Shader.h"
#include "Render/ShaderInput.h"

#if defined(__APPLE__)

namespace {
void LogMetalError(const char* stage, NS::Error* error)
{
    if (error != nullptr && error->localizedDescription() != nullptr) {
        PLOGE << stage << ": " << error->localizedDescription()->utf8String();
    }
    else {
        PLOGE << stage;
    }
}
}

MetalManager::MetalManager() {
    PLOGI <<"Construct MetalManager";
}   

MetalManager::~MetalManager() {
    PLOGI <<"~MetalManager";
    ReleaseResources();
};

void MetalManager::ReleaseResources()
{
    // commandBuffer and encoder are autoreleased objects from Metal. We do not
    // own them, so just clear the cached pointers before draining the pool.
    encoder = nullptr;
    commandBuffer = nullptr;

    if(commandQueue != nullptr){
        commandQueue->release();
        commandQueue = nullptr;
    }

    if(pipelineState != nullptr){
        pipelineState->release();
        pipelineState = nullptr;
    }

    if(function != nullptr){
        function->release();
        function = nullptr;
    }

    if(library != nullptr){
        library->release();
        library = nullptr;
    }

    if(bezierBuffer != nullptr){
        bezierBuffer->release();
        bezierBuffer = nullptr;
    }

    if(anchorMatrixBuffer != nullptr){
        anchorMatrixBuffer->release();
        anchorMatrixBuffer = nullptr;
    }

    if(transformModelBuffer != nullptr){
        transformModelBuffer->release();
        transformModelBuffer = nullptr;
    }

    if(viewMatrixBuffer != nullptr){
        viewMatrixBuffer->release();
        viewMatrixBuffer = nullptr;
    }

    if(device != nullptr){
        device->release();
        device = nullptr;
    }

    if(pool != nullptr){
        pool->release();
        pool = nullptr;
    }
}

void MetalManager::Init() {
    PLOGI << "Initializing MetalManager";
    
    if(device != nullptr){
        PLOGI << "Metal device already initialized";
        return;
    }

    device = MTL::CreateSystemDefaultDevice();

    if (device == nullptr) {
           PLOGE << "Metal not supported\n";
    }
    else{
        PLOGI << "Using Metal device: " <<  device->name()->utf8String();
    }
    PLOGI << "MetalManager Constructed";
    //RunComputeTest();
}



void MetalManager::InitResourcesOnce(const ShaderInput& input){
    NS::Error* error = nullptr;
    if (pool == nullptr) {
        pool = NS::AutoreleasePool::alloc()->init();
    }

    if (library == nullptr) {
        const char* metalComputeShder = GetShaderProgramSource(ShaderProgramType::PointAnimate).comp;
        //NS::String* sourceString = NS::String::string(GetMetalShader().c_str(), NS::UTF8StringEncoding);
        NS::String* sourceString = NS::String::string(metalComputeShder, NS::UTF8StringEncoding);
        library = device->newLibrary(sourceString, nullptr, &error);
    }

    if (function == nullptr) {
        function = library->newFunction(MTLSTR("compute_splat_animate"));
    }

    if (pipelineState == nullptr) {
        pipelineState = device->newComputePipelineState(function, &error);
    }

    if (commandQueue == nullptr) {
        commandQueue = device->newCommandQueue();
    }
    LogMetalError("InitResourcesOnce", error);
}


void MetalManager::RewriteBuffer(const std::vector<float>& position, const ShaderInput& input){
     NS::Error* error = nullptr;

    const size_t bufferSize = position.size() * sizeof(float);
    const size_t valueCount = position.size();
    const size_t bezierBufferSize = input.bezierCurveBlock.size() * sizeof(BezierCurveInfoGpu);

    // rewritte buffer 
    if (positionBuffer == nullptr ){
        positionBuffer = device->newBuffer(bufferSize, MTL::ResourceStorageModeShared);
    }
    else if (positionBuffer->allocatedSize() != bufferSize) {
        positionBuffer->release();
        positionBuffer = device->newBuffer(bufferSize, MTL::ResourceStorageModeShared);
    }
    std::memcpy(positionBuffer->contents(), position.data(), bufferSize);

   
    if (bezierBuffer == nullptr ){
         bezierBuffer = device->newBuffer(bezierBufferSize, MTL::ResourceStorageModeShared);
    }
    else if (bezierBuffer->allocatedSize() != bezierBufferSize) {
        bezierBuffer->release();
        bezierBuffer = device->newBuffer(bezierBufferSize, MTL::ResourceStorageModeShared);
    }
    std::memcpy(bezierBuffer->contents(), input.bezierCurveBlock.data(), bezierBufferSize);

    if (renderInfoBuffer == nullptr) {
        renderInfoBuffer = device->newBuffer(sizeof(GaussianRenderInfo), MTL::ResourceStorageModeShared);
    }
    std::memcpy(renderInfoBuffer->contents(), &input.renderBlock, sizeof(GaussianRenderInfo));

    if (viewPositionBuffer == nullptr) {
        viewPositionBuffer = device->newBuffer(sizeof(float) * valueCount, MTL::ResourceStorageModeShared);
    }
    else if (viewPositionBuffer->allocatedSize() != sizeof(float) * valueCount) {
        viewPositionBuffer->release();
        viewPositionBuffer = device->newBuffer(sizeof(float) * valueCount, MTL::ResourceStorageModeShared);
    }
    
    LogMetalError("RewriteBuffer", error);
}


void MetalManager::EncodeDispatch(int pointCount){
    commandBuffer = commandQueue->commandBuffer();
    encoder = commandBuffer->computeCommandEncoder();

    encoder->setComputePipelineState(pipelineState);
    encoder->setBuffer(positionBuffer, 0, 0);
    encoder->setBuffer(bezierBuffer, 0, 1);
    encoder->setBuffer(renderInfoBuffer, 0, 2);
    encoder->setBuffer(viewPositionBuffer, 0, 3);

    const NS::UInteger threadgroupWidth = std::min<NS::UInteger>(
        pointCount,
        pipelineState->maxTotalThreadsPerThreadgroup()
    );

    encoder->dispatchThreads(
        MTL::Size::Make(pointCount, 1, 1),
        MTL::Size::Make(threadgroupWidth, 1, 1)
    );
    encoder->endEncoding();

    commandBuffer->commit();
    commandBuffer->waitUntilCompleted();
}


void MetalManager::RunComputeAnimate(const ShaderInput& input, /* in */
                                  const std::vector<float>& position,  /* in */
                                  std::span<float>& posViewBufferSpan /* out */
                                ) {

    PLOGD << "Running Metal compute shader for animated splat data";
    auto runComputeTestBegin = std::chrono::high_resolution_clock::now();

    NS::Error* error = nullptr;

    if (device == nullptr) {
        PLOGE << "Metal compute test skipped: device is null";
        return;
    }

    InitResourcesOnce(input);

    // ========== 2. RewriteBuffer ==========
   
    const size_t valueCount = position.size();
   
    RewriteBuffer(position, input);
    EncodeDispatch(valueCount / 3 );

    // read results back to CPU
    float* viewPosition = static_cast<float*>(viewPositionBuffer->contents());
    posViewBufferSpan = std::span<float>(viewPosition, valueCount);

}

#endif
