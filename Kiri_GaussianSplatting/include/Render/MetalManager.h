#pragma once

#if defined(__APPLE__)

#include <iostream>
#include <Metal/Metal.hpp>
#include "Render/ShaderInput.h"
#include <vector>
#include <span>


class MetalManager {

private:
    MTL::Device* device = nullptr;
    NS::AutoreleasePool* pool = nullptr;

    MTL::Buffer* positionBuffer = nullptr;
    MTL::Buffer* bezierBuffer = nullptr;
    MTL::Buffer* anchorMatrixBuffer = nullptr;
    MTL::Buffer* transformModelBuffer = nullptr;
    MTL::Buffer* viewMatrixBuffer = nullptr;
    MTL::Buffer* renderInfoBuffer = nullptr;
    MTL::Buffer* viewPositionBuffer = nullptr;

    MTL::Library* library = nullptr;
    MTL::Function* function = nullptr;
    MTL::ComputePipelineState* pipelineState = nullptr;
    MTL::CommandQueue* commandQueue = nullptr;
    MTL::CommandBuffer* commandBuffer = nullptr;
    MTL::ComputeCommandEncoder* encoder = nullptr;

    void ReleaseResources();
    void InitResourcesOnce(const ShaderInput& input);
    void RewriteBuffer(const std::vector<float>& position, const ShaderInput& input);
    void EncodeDispatch(int pointCount);

public:
    MetalManager();
    ~MetalManager() ;
    void Init();
    void RunComputeAnimate(const ShaderInput& input , 
                        const std::vector<float>& position ,
                         std::span<float>& posViewBufferSpan);

};

#endif
