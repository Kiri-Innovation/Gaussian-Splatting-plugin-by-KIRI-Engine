#pragma once

#include "vector"
#include "string"
#include <memory>
#include "AEConfig.h"
#include "Render/OpenGLManager.h"
#include "array"
#include "Render/GaussianTypes.h"
#include "span"

class GaussianModel{
public : 
	GaussianModel();
	~GaussianModel();
	void Init(const std::vector<StandardGaussian>& gaussianData);
	//std::string GetFilePath();

public:
	int splatCount = 0 ;
	TextureBufferInfo tboInfo;
	TextureBufferInfo viewPositionBufferInfo;
	SSBOInfo SSBOinfo;

	//TextureBufferInfo testBuffer;
	std::vector<float> position;

	std::vector<int> depthIndexBuffer;
	TextureBufferInfo depthIndexTboInfo;

private:
	std::vector<float> depthBuffer;
	std::vector<float>zViewBuffer;
	std::vector<int> countBuffer;
	std::vector<int> sortKeyBuffer;
	std::vector<int> depthIndexSrcBuffer;
	std::vector<bool> viewableBuffer;
	std::string currentPlyPath;

	int bucketCount = 1 << 17;
};


