#pragma once 


#include <glm/glm.hpp>
#include <memory>
#include <sstream>
#include <iomanip>
#include <map>
#include "AE_Effect.h"
#include "AE_EffectCB.h"
#include "AE_Macros.h"
#include "AE_EffectCBSuites.h"
#include "AE_GeneralPlug.h"

typedef struct  GaussianRenderInfo {
	
	glm::mat4x4 modelMatrix;	
	glm::mat4x4 anchorModelMatrix;
	glm::mat4x4 transformModelMatrix;

	glm::mat4x4 viewMatrix;			   
	glm::mat4x4 projectionMatrix;	   
									   
	float viewport[2];				   
	float focalPixelX;				   
	int instanceCount;				   
									   
	glm::vec4 cameraPos;			   

	// do not change the following member order for std 140 layout
	glm::vec4 colorShapeCenter;
	glm::vec4 colorShapeScaleXYZ;
	int shDegree;
	float colorEnable;
	float colorShapeSize;
	float colorShapeFeather;

	glm::vec4 cropShapeCenter;
	glm::vec4 cropShapeScaleXYZ;
	float cropEnable;
	int cropInvert;
	float cropShapeSize;
	float cropShapeFeather;

	glm::vec4 splatScaleShapeCenter;
	glm::vec4 splatScaleShapeScaleXYZ;
	float splatScaleEnable;
	float splatScaleSize;
	float splatScaleShapeSize;
	float splatScaleShapeFeather;

	glm::vec4 splatNoiseShapeCenter;
	glm::vec4 splatNoiseShapeScaleXYZ;
	float splatNoiseEnable;
	float splatNoiseShapeSize;
	float splatNoiseShapeFeather;
	int	  splatNoiseOctaves;
	float splatNoisePersistence;
	float splatNoiseLacunarity;
	float splatNoiseStrength;
	float splatNoiseStrengthX;
	float splatNoiseStrengthY;
	float splatNoiseStrengthZ;
	float pad_0;

	float splatOpacityEnable;
	glm::vec4 splatOpacityShapeCenter;
	glm::vec4 splatOpacityShapeScaleXYZ;
	float splatOpacityShapeFeather;
	float splatOpacityShapeSize;
	float splatMaxOpacity;
	float splatMinOpacity;

	glm::vec4 splatDisplacementOffset;
	glm::vec4 splatDisplacementRotation;
	glm::vec4 splatDisplacementShapeScale;
	glm::vec4 splatDisplacementShapeCenter;
	float splatDisplacementEnable;
	float splatDisplacementScale;
	float splatDisplacementShapeSize;
	float splatDisplacementShapeFeather;

	glm::vec4 splatDenseShapeCenter;
	glm::vec4 splatDenseShapeScaleXYZ;
	float splatDenseDensity;
	float splatDenseEnable;
	float splatDenseShapeSize;
	float splatDenseShapeFeather;


	float advancedSplatCropNear;
	float advancedSplatCropFar;
	float advancedCameraFocalLength;
	float advancedSplatCropMaxScale;

	float advancedSplatCropMinScale;
	float advancedDofEnable;
	float advancedDofFocusDistance;
	float advancedDofAperture;

	float advancedDofBlurLevel;
	float advancedGlowEnable;
	float advancedGlowBlendMode;
	float advancedGlowRadius;

	float advancedGlowThreshold;
	float advancedGlowSmooth;
	float splatInvertSphereEnable;
	float splatInvertSphereRaduis;

	glm::vec4 splatInvertSphereCenter;
	float splatInvertSphereIntensity;
	float splatInvertSphereDistance;
	float splatInvertSphereCompression;
	// do not change the above member order for std 140 layout

	int splatCount;
	float focalPixelY;
		
}GaussianRenderInfo;

typedef struct RenderResult {
	int  width;
	int  height;
	float  ratio_x;
	float  ratio_y;
	std::shared_ptr<unsigned char> pixelPtr;
	bool isRenderSucces;
}RenderResult;

typedef struct PLYData {
	size_t vertexCount = 0;
	size_t propertyCount = 0;
	std::vector<float> buffer; 
	std::map<std::string, int> propMap;
}PLYData;

typedef struct StandardGaussian {
	float pos[3];
	float sh[48];
	float opacity;
	float scale[3];
	float quat[4];


	std::string toString() const {
		std::stringstream ss;
		ss << std::fixed << std::setprecision(4);

		ss << "--- Gaussian Splat ---\n";
		ss << "Pos:     [" << pos[0] << ", " << pos[1] << ", " << pos[2] << "]\n";
		ss << "Opacity: " << opacity << "\n";
		ss << "Scale:   [" << scale[0] << ", " << scale[1] << ", " << scale[2] << "]\n";
		ss << "Rot(xyzw): [" << quat[0] << ", " << quat[1] << ", " << quat[2] << ", " << quat[3] << "]\n";

		for (int i = 0; i < 48; i++) {
			ss << "SH[ " << i << "]" << "   : [" << sh[i] << "] \n";
		}
		return ss.str();
	}
}StandardGaussian;

typedef struct AEStreamValueInfo {
	std::string filePath;			
	int splatEnable;				
	glm::vec3 transformPosition;	
	glm::vec3 transformScale;		
	glm::vec3 trasnformRotation;	

	glm::vec3 anchorPosition;		
	glm::vec3 anchorScale;			
	glm::vec3 anchorRotation;		

	// do not change the following member order for std 140 layout
	glm::vec4 colorShapeCenter;      
	glm::vec4 colorShapeScaleXYZ;
	int shDegree;
	float colorEnable;
	float colorShapeSize;
	float colorShapeFeather;

	glm::vec4 cropShapeCenter;
	glm::vec4 cropShapeScaleXYZ;
	float cropEnable;
	int cropInvert;
	float cropShapeSize;
	float cropShapeFeather;

	glm::vec4 splatScaleShapeCenter;
	glm::vec4 splatScaleShapeScaleXYZ;
	float splatScaleEnable;
	float splatScaleSize;
	float splatScaleShapeSize;
	float splatScaleShapeFeather;

	glm::vec4 splatNoiseShapeCenter;
	glm::vec4 splatNoiseShapeScaleXYZ;
	float splatNoiseEnable;
	float splatNoiseShapeSize;
	float splatNoiseShapeFeather;
	int	  splatNoiseOctaves;
	float splatNoisePersistence;
	float splatNoiseLacunarity;
	float splatNoiseStrength;
	float splatNoiseStrengthX;
	float splatNoiseStrengthY;
	float splatNoiseStrengthZ;
	float pad_0;

	float splatOpacityEnable;
	glm::vec4 splatOpacityShapeCenter;
	glm::vec4 splatOpacityShapeScaleXYZ;
	float splatOpacityShapeFeather;
	float splatOpacityShapeSize;
	float splatMaxOpacity;
	float splatMinOpacity;

	glm::vec4 splatDisplacementOffset;
	glm::vec4 splatDisplacementRotation;
	glm::vec4 splatDisplacementShapeScale;
	glm::vec4 splatDisplacementShapeCenter;
	float splatDisplacementEnable;
	float splatDisplacementScale;
	float splatDisplacementShapeSize;
	float splatDisplacementShapeFeather;

	glm::vec4 splatDenseShapeCenter;
	glm::vec4 splatDenseShapeScaleXYZ;
	float splatDenseDensity;
	float splatDenseEnable;
	float splatDenseShapeSize;
	float splatDenseShapeFeather;

	float advancedSplatCropNear;
	float advancedSplatCropFar;
	float advancedCameraFocalLength;
	float advancedSplatCropMaxScale;

	float advancedSplatCropMinScale;
	float advancedDofEnable;
	float advancedDofFocusDistance;
	float advancedDofAperture;

	float advancedDofBlurLevel;
	float advancedGlowEnable;
	float advancedGlowBlendMode;
	float advancedGlowRadius;

	float advancedGlowThreshold;
	float advancedGlowSmooth;
	float splatInvertSphereEnable;
	float splatInvertSphereRaduis;

	glm::vec4 splatInvertSphereCenter;
	float splatInvertSphereIntensity;
	float splatInvertSphereDistance;
	float splatInvertSphereCompression;

	float pad_end;
	// do not change the above member order for std 140 layout


}AEStreamValueInfo;
