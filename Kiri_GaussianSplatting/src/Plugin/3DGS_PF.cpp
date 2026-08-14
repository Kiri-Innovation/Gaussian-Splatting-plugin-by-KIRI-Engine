
#include "Plugin/3DGS_PF.h"
#include "Common/Utils.h"
#include <algorithm>
#include <bitset> 

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL 
#include <glm/gtx/euler_angles.hpp>
#include <chrono>
#include <AE_GeneralPlug.h>
#include <AEFX_SuiteHelper.h>
#include <AEFX_SuiteHandlerTemplate.h>

#include <codecvt>
#include "UI/UIPanel.h"
#include "UI/ColorGradientUI.h"
#include <stddef.h>
#include "Render/ShaderInput.h"
#include "UI/CmdArbitraryCallBackHandler.h"
#include "Common/Global.h"
#include <cmath>

static PF_Err
GetCameraLayer
(PF_InData* in_data,
	AEGP_LayerH& cameraLayerH)
{
	PF_Err err = PF_Err_NONE;

	AEGP_SuiteHandler suites(in_data->pica_basicP);

	A_Time time;
	time.scale = in_data->time_scale;
	time.value = in_data->current_time;

	suites.PFInterfaceSuite1()->AEGP_GetEffectCamera(
		in_data->effect_ref,
		&time,
		&cameraLayerH
	);

	if (cameraLayerH != NULL) {
		PLOGI << "get camera Layer";
	}
	else {
		PLOGI << "not get camera Layer";
	}


	PLOGI << "get camera Layer";
	PLOGI << err;
	return err;

};




static PF_Err
GetCameraProperty
(PF_InData* in_data,
	AEGP_LayerStream layerStream,
	AEGP_StreamValue2& value)
{
	PF_Err err = PF_Err_NONE;

	PLOGI << "GetCameraProperty begin";

	AEGP_SuiteHandler suites(in_data->pica_basicP);

	AEGP_StreamRefH streamRefH = nullptr;

	//A_Time timePT;
	A_Time time;
	time.scale = in_data->time_scale;
	time.value = in_data->current_time;

	AEGP_LayerH _cameraLayerH = NULL;

	ERR(suites.PFInterfaceSuite1()->AEGP_GetEffectCamera(
		in_data->effect_ref,
		&time,
		&_cameraLayerH
	));

	ERR(suites.StreamSuite4()->AEGP_GetNewLayerStream(
		g_pluginID,
		_cameraLayerH,
		layerStream,
		&streamRefH
	));

	ERR(suites.StreamSuite4()->AEGP_GetNewStreamValue(
		g_pluginID,
		streamRefH,
		AEGP_LTimeMode_CompTime,
		//&timePT,
		&time,
		FALSE,
		&value
	));

	ERR(suites.StreamSuite4()->AEGP_DisposeStream(
		streamRefH
	));

	//std::ostringstream oss;
	//oss.str("");
	//oss << "GetCameraProperty end" << err;
	PLOGI << "GetCameraProperty end {}", err;

	return err;

};

static PF_Err
GetSelectedLayer(
	PF_InData* in_data,			 /* in */
	AEGP_LayerH& layerH			 /* out */
)
{
	std::ostringstream oss;

	PF_Err				err = PF_Err_NONE;
	AEGP_SuiteHandler	suites(in_data->pica_basicP);

	AEGP_LayerH currentLayerH = nullptr;
	ERR(suites.PFInterfaceSuite1()->AEGP_GetEffectLayer(
		in_data->effect_ref,
		&currentLayerH
	));

	AEGP_EffectRefH effectPH = nullptr;
	ERR(suites.EffectSuite5()->AEGP_GetLayerEffectByIndex(
		g_pluginID,
		currentLayerH,
		0,
		&effectPH));


	AEGP_StreamRefH streamH = nullptr;
	ERR(suites.StreamSuite2()->AEGP_GetNewEffectStreamByIndex(
		g_pluginID,
		effectPH,
		KIRI_LAYER,
		&streamH
	));

	AEGP_StreamValue val;
	AEFX_CLR_STRUCT(val);

	A_Time timeT;
	timeT.value = in_data->current_time;
	timeT.scale = in_data->time_scale;


	ERR(suites.StreamSuite2()->AEGP_GetNewStreamValue(
		g_pluginID,
		streamH,
		AEGP_LTimeMode_LayerTime,
		&timeT,
		FALSE,
		&val
	));

	PLOGI << " val.val.layer_id {}", val.val.layer_id;

	if (val.val.layer_id == AEGP_LayerIDVal_NONE) {
		layerH = NULL;
		ERR(suites.StreamSuite2()->AEGP_DisposeStreamValue(&val));
		return err;
	}

	AEGP_CompH compH = nullptr;
	ERR(suites.LayerSuite8()->AEGP_GetLayerParentComp(
		currentLayerH,
		&compH
	));

	if (compH == nullptr) {
		PLOGI << "compH is nullptr";
		return err;
	}

	ERR(suites.LayerSuite9()->AEGP_GetLayerFromLayerID(
		compH,
		val.val.layer_id,
		&layerH
	));

	if (layerH == nullptr) {
		PLOGI << "layerH  is nullptr";
		return err;
	}

	char layer_nameZ[256] = { 0 };
	char source_nameZ[256] = { 0 };
	ERR(suites.LayerSuite4()->AEGP_GetLayerName(
		layerH,
		layer_nameZ,
		source_nameZ
	));

	//oss.str("");

	//oss << "source_nameZ " << source_nameZ;
	PLOGI << " source_nameZ {}", source_nameZ;

	// release
	ERR(suites.StreamSuite2()->AEGP_DisposeStreamValue(&val));
	ERR(suites.StreamSuite2()->AEGP_DisposeStream(streamH));
	ERR(suites.EffectSuite5()->AEGP_DisposeEffect(effectPH));

	return err;
}

static PF_Err
CalcShaderInput(
	PF_InData* in_data,						  /* in  */
	PF_ParamDef* params[],					  /* in */
	AEStreamValueInfo& streamValueInfo,       /* in  */
	GaussianModel& gaussianModel,			  /* in  */
	ShaderInput& shaderInput				  /* out */
)
{
	PF_Err err = PF_Err_NONE;
	PLOGI << "begin CalcShaderInput";

	A_Matrix4 c2w;

	A_Time timeT;
	timeT.value = in_data->current_time;
	timeT.scale = in_data->time_scale;

	AEGP_SuiteHandler suites(in_data->pica_basicP);

	A_Matrix4   camMatrix;
	A_FpLong    zoom;
	A_short     cpWidth, cpHeight;

	std::ostringstream oss;

	ERR(suites.PFInterfaceSuite1()->AEGP_GetEffectCameraMatrix(
		in_data->effect_ref,
		&timeT,
		&camMatrix,
		&zoom,
		&cpWidth,
		&cpHeight
	));

	glm::mat4 cameraModel = glm::mat4(
		(float)camMatrix.mat[0][0], (float)camMatrix.mat[0][1], (float)camMatrix.mat[0][2], (float)camMatrix.mat[0][3],
		(float)camMatrix.mat[1][0], (float)camMatrix.mat[1][1], (float)camMatrix.mat[1][2], (float)camMatrix.mat[1][3],
		(float)camMatrix.mat[2][0], (float)camMatrix.mat[2][1], (float)camMatrix.mat[2][2], (float)camMatrix.mat[2][3],
		(float)camMatrix.mat[3][0], (float)camMatrix.mat[3][1], (float)camMatrix.mat[3][2], (float)camMatrix.mat[3][3]
	);

	//glm::mat4 rotateZ180 = glm::rotate(glm::mat4(1), glm::pi<float>(), { 0,1,0 });;
	cameraModel = glm::scale(cameraModel, glm::vec3(1, -1, -1));
	//cameraModel = cameraModel * rotateZ180;

	GaussianRenderInfo gaussianRenderInfo = {};

	gaussianRenderInfo.cameraPos[0] = cameraModel[3][0];
	gaussianRenderInfo.cameraPos[1] = cameraModel[3][1];
	gaussianRenderInfo.cameraPos[2] = cameraModel[3][2];

	glm::mat4 cameraView = glm::inverse(cameraModel);

	// align anchor TRS
	glm::mat4 anchorModelMatrix = glm::mat4(1);
	anchorModelMatrix = glm::translate(anchorModelMatrix, streamValueInfo.anchorPosition);
	anchorModelMatrix = anchorModelMatrix * glm::eulerAngleXYZ(glm::radians(streamValueInfo.anchorRotation.x),
		glm::radians(streamValueInfo.anchorRotation.y),
		glm::radians(-streamValueInfo.anchorRotation.z));
	anchorModelMatrix = glm::scale(anchorModelMatrix, glm::vec3(1000, 1000, 1000));
	anchorModelMatrix = glm::scale(anchorModelMatrix, streamValueInfo.anchorScale);

	// TRS
	glm::mat4 gaussianModelMatrix = glm::mat4(1.0f);
	glm::mat4 transformMatrix = glm::mat4(1.0f);
	transformMatrix = glm::translate(transformMatrix, streamValueInfo.transformPosition);

	glm::mat4 rotationMatrix = glm::eulerAngleXYZ(glm::radians(streamValueInfo.trasnformRotation.x),
		glm::radians(streamValueInfo.trasnformRotation.y),
		glm::radians(-streamValueInfo.trasnformRotation.z));
	transformMatrix = transformMatrix * rotationMatrix;

	// default scale
	transformMatrix = glm::scale(transformMatrix, streamValueInfo.transformScale);

	gaussianModelMatrix = transformMatrix * anchorModelMatrix;

	// camera_view 
	float width = in_data->width;
	float height = in_data->height;

	gaussianRenderInfo.viewport[0] = width;
	gaussianRenderInfo.viewport[1] = height;
	// radians 

	float fovY = 2.0f * glm::atan(height / (2.0f * streamValueInfo.advancedCameraFocalLength));

	float focalPixelY;

	float aspect = float(width) / float(height);

	gaussianRenderInfo.modelMatrix = gaussianModelMatrix;
	gaussianRenderInfo.anchorModelMatrix = anchorModelMatrix;
	gaussianRenderInfo.transformModelMatrix = transformMatrix;
	gaussianRenderInfo.viewMatrix = cameraView;

	gaussianRenderInfo.projectionMatrix = glm::perspective(fovY, aspect, streamValueInfo.advancedSplatCropNear, streamValueInfo.advancedSplatCropFar);

	// focal in pixel = 2 * cameraInfo.focalLength
	gaussianRenderInfo.focalPixelX = width * gaussianRenderInfo.projectionMatrix[0][0];
	gaussianRenderInfo.focalPixelY = height * gaussianRenderInfo.projectionMatrix[1][1];
	gaussianRenderInfo.splatCount = gaussianModel.splatCount;

	// gaussianRenderInfo memcpy
	int gaussianRenderInfoOffset = offsetof(GaussianRenderInfo, colorShapeCenter);
	int streamValueInfoOffsetBegin = offsetof(AEStreamValueInfo, colorShapeCenter);
	int streamValueInfoOffsetEnd = offsetof(AEStreamValueInfo, pad_end);//+  sizeof(AEStreamValueInfo::advancedDofBlurLevel);
	int memSize = streamValueInfoOffsetEnd - streamValueInfoOffsetBegin;
	memcpy(reinterpret_cast<char*>(&gaussianRenderInfo) + gaussianRenderInfoOffset,
		reinterpret_cast<char*>(&streamValueInfo) + streamValueInfoOffsetBegin,
		memSize);


	// ColorGradientInfoGpu memcpy
	std::vector<KIRIParamIdx> colorGradientUIIndex = {
		KIRI_RENDER_COLOR_GRADIENT
	};
	std::vector<ColorGradientInfoGpu> colorGradientBlock(colorGradientUIIndex.size());
	for (int i = 0; i < colorGradientUIIndex.size(); i++) {
		PF_ArbitraryH arbH = NULL;
		ColorGradientInfo* arbP = NULL;
		GetArbData(in_data, params, colorGradientUIIndex[i], &arbP, &arbH);
		colorGradientBlock[i] = arbP->ToGpu();
		PF_UNLOCK_HANDLE(arbH);
	}

	// BezierCurveInfoGpu memcpy
	std::vector<KIRIParamIdx> bezierCurveUIIndex = {
		KIRI_RENDER_COLOR_RAMP,
		KIRI_SPLAT_SCALE_RAMP,
		KIRI_SPLAT_OPACITY_RAMP,
		KIRI_SPLAT_DISPLACEMENT_OFFSET_RAMP,
		KIRI_SPLAT_DISPLACEMENT_SCALE_RAMP,
		KIRI_SPLAT_DISPLACEMENT_ROTATION_RAMP,
		KIRI_SPLAT_DENSE_SHAPE_RAMP
	};
	std::vector<BezierCurveInfoGpu> bezierCurveInfoGpu(bezierCurveUIIndex.size());
	for (int i = 0; i < bezierCurveUIIndex.size(); i++) {
		PF_ArbitraryH arbH = NULL;
		BezierCurveInfo* arbP = NULL;
		GetArbData(in_data, params, bezierCurveUIIndex[i], &arbP, &arbH);
		bezierCurveInfoGpu[i] = arbP->ToGpu();
		PF_UNLOCK_HANDLE(arbH);
	}

	shaderInput.splatEnable = streamValueInfo.splatEnable;
	shaderInput.renderBlock = gaussianRenderInfo;
	shaderInput.colorGradientBlock = colorGradientBlock;
	shaderInput.bezierCurveBlock = bezierCurveInfoGpu;

	PLOGI << "end ComputeShaderInput";

	return err;
}

static PF_Err
About(
	PF_InData* in_data,
	PF_OutData* out_data,
	PF_ParamDef* params[],
	PF_LayerDef* output)
{
	AEGP_SuiteHandler suites(in_data->pica_basicP);

	suites.ANSICallbacksSuite1()->sprintf(
		out_data->return_msg,
		"%s v%d.%d\r%s",
		STR(STRID_NAME),
		MAJOR_VERSION,
		MINOR_VERSION,
		STR(STRID_DESCRIPTION));

	return PF_Err_NONE;
}

static PF_Err
GlobalSetup(
	PF_InData* in_data,
	PF_OutData* out_data,
	PF_ParamDef* params[],
	PF_LayerDef* output)
{
	PLOGI << "GlobalSetup";

	PFAppSuite6* appSuiteP = nullptr;
	if (AEFX_AcquireSuite(
		in_data,
		out_data,
		kPFAppSuite,
		kPFAppSuiteVersion6,
		nullptr,
		reinterpret_cast<void**>(&appSuiteP)) == PF_Err_NONE &&
		appSuiteP != nullptr) {
		A_char langTagZ[PF_APP_LANG_TAG_SIZE] = {};
		if (appSuiteP->PF_AppGetLanguage(langTagZ) == PF_Err_NONE) {
			PLOGD << "language " << langTagZ;
			SetLanguageTypeFromTag(langTagZ);
		}
		AEFX_ReleaseSuite(in_data, out_data, kPFAppSuite, kPFAppSuiteVersion6, nullptr);
	}


	out_data->my_version = PF_VERSION(MAJOR_VERSION,
		MINOR_VERSION,
		BUG_VERSION,
		STAGE_VERSION,
		BUILD_VERSION);

	out_data->out_flags |= PF_OutFlag_FORCE_RERENDER |
		PF_OutFlag_CUSTOM_UI |
		PF_OutFlag_USE_OUTPUT_EXTENT |
		PF_OutFlag_SEQUENCE_DATA_NEEDS_FLATTENING;
	out_data->out_flags2 |= PF_OutFlag2_I_USE_3D_CAMERA | PF_OutFlag2_I_USE_3D_LIGHTS;

	return PF_Err_NONE;
}

static PF_Err
ParamsSetup(
	PF_InData* in_data,
	PF_OutData* out_data,
	PF_ParamDef* params[],
	PF_LayerDef* output)
{

	PLOGI << "ParamsSetup";

	PF_Err		err = PF_Err_NONE;
	PF_ParamDef	def;

	std::unordered_map<uint32_t, std::string> diskIdMap;

	//std::ostringstream oss;
	//for (auto e : magic_enum::enum_values<KIRIParamIdx>()) {
	//	auto name = std::string( magic_enum::enum_name(e));
	//	uint32_t diskID = GetDiskId(e);
	//
	//	auto [it, inserted] = diskIdMap.emplace(diskID, name);
	//	oss.str("");
	//	if (!inserted) {
	//		oss << "duplicate diskID: " << diskID
	//			<< " current=" << name
	//			<< " previous=" << it->second;
	//		PLOGI <<oss.str());
	//	}
	//	else {
	//		oss << name << " " << diskID;
	//		PLOGI <<oss.str());
	//	}
	//}

	ERR(SetupLayerInputUI(in_data, out_data));
	ERR(SetupAlignUI(in_data, out_data));
	ERR(SetupTransformUI(in_data, out_data));
	ERR(SetupEffectUI(in_data, out_data));
	ERR(SetupAdvancedUI(in_data, out_data));

	out_data->num_params = KIRI_NUM_PARAMS;

	PLOGI << "end ParamsSetup " << err;
	return err;
}


static PF_Err
GetSelectedPlyFile(
	PF_InData* in_data,				/* in */
	std::string& filePath			/* out */
)
{

	PF_Err		err = PF_Err_NONE;
	AEGP_SuiteHandler	suites(in_data->pica_basicP);


	AEGP_LayerH selectedLayerH = NULL;
	ERR(GetSelectedLayer(in_data, selectedLayerH));
	if (selectedLayerH == NULL) {
		filePath = "";
		return err;
	}

	AEGP_ItemH sourceItemH = NULL;
	ERR(suites.LayerSuite9()->AEGP_GetLayerSourceItem(
		selectedLayerH,
		&sourceItemH
	));
	if (sourceItemH == NULL) {
		return err;
	}

	AEGP_FootageH footageH = NULL;
	ERR(suites.FootageSuite5()->AEGP_GetMainFootageFromItem(
		sourceItemH,
		&footageH
	));
	if (footageH == NULL) {
		return err;
	}

	AEGP_MemHandle pathH = NULL;
	ERR(suites.FootageSuite5()->AEGP_GetFootagePath(
		footageH,
		0,
		AEGP_FOOTAGE_MAIN_FILE_INDEX,
		&pathH));

	if (pathH == NULL) {
		return err;
	}
	A_UTF16Char* unicode_path = nullptr;
	suites.MemorySuite1()->AEGP_LockMemHandle(pathH, reinterpret_cast<void**>(&unicode_path));
	std::u16string u16str(reinterpret_cast<const char16_t*>(unicode_path));
	std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> converter;
	filePath = converter.to_bytes(u16str);


	suites.MemorySuite1()->AEGP_UnlockMemHandle(pathH);
	suites.MemorySuite1()->AEGP_FreeMemHandle(pathH);

	return err;
}

inline static void
BilinearSampling(A_long		xL,
	A_long		yL,
	RenderResult* renderResult,
	PF_Pixel8* finalRGBA)
{
	int downsampleX = MIN(xL * renderResult->ratio_x, renderResult->width - 1);
	int downsampleY = MIN(renderResult->height - yL * renderResult->ratio_y, renderResult->height - 1);

	int downsampleX_1 = MAX(downsampleX - 1, 0);
	int downsampleX_2 = MIN(downsampleX + 1, renderResult->width - 1);

	int downsampleY_1 = MAX(downsampleY - 1, 0);
	int downsampleY_2 = MIN(downsampleY + 1, renderResult->height - 1);

	float dx = (downsampleX_2 - downsampleX) / (downsampleX_2 - downsampleX_1);
	float dy = (downsampleY_2 - downsampleY) / (downsampleY_2 - downsampleY_1);

	int headerIndexs[] = {
		(downsampleY_1 * renderResult->width + downsampleX_1) * 4 , //  downsampleX_1 ,  downsampleY_1  
		(downsampleY_1 * renderResult->width + downsampleX_2) * 4 , //  downsampleX_2 ,  downsampleY_1
		(downsampleY_2 * renderResult->width + downsampleX_1) * 4 , //  downsampleX_1 ,  downsampleY_2  
		(downsampleY_2 * renderResult->width + downsampleX_2) * 4 , //  downsampleX_2 ,  downsampleY_2 
	};
	A_u_char rgba[4] = {};
	for (int i = 0; i < 4; i++) {
		float top = std::lerp(renderResult->pixelPtr.get()[headerIndexs[1] + i], renderResult->pixelPtr.get()[headerIndexs[0] + i], dx);
		float bottom = std::lerp(renderResult->pixelPtr.get()[headerIndexs[3] + i], renderResult->pixelPtr.get()[headerIndexs[2] + i], dx); ;
		top = std::clamp(top, 0.f, 255.f);
		bottom = std::clamp(bottom, 0.f, 255.f);
		//rgba
		rgba[i] = (A_u_char)std::clamp(dy * top + (1 - dy) * bottom, 0.f, 255.f);
	}

	finalRGBA->red = rgba[0];
	finalRGBA->green = rgba[1];
	finalRGBA->blue = rgba[2];
	finalRGBA->alpha = rgba[3];
}


inline static void
DownSampling(A_long		xL,
	A_long		yL,
	RenderResult* renderResult,
	PF_Pixel8* finalRGBA)
{
	int downsampleX = MIN(xL * renderResult->ratio_x, renderResult->width - 1);
	int downsampleY = MIN(renderResult->height - yL * renderResult->ratio_y, renderResult->height - 1);

	int headerIndex = (downsampleY * renderResult->width + downsampleX) * 4;

	finalRGBA->alpha = renderResult->pixelPtr.get()[headerIndex + 3];
	memcpy(&finalRGBA->red, &renderResult->pixelPtr.get()[headerIndex + 0], 3 * sizeof(char));
}



static PF_Err
MySimpleGainFunc8(
	void* refcon,
	A_long		xL,
	A_long		yL,
	PF_Pixel8* inP,
	PF_Pixel8* outP)
{
	PF_Err		err = PF_Err_NONE;

	RenderResult* renderResult = reinterpret_cast<RenderResult*>(refcon);

	if (renderResult) {
		//BilinearSampling(xL, yL, renderResult, outP);
		DownSampling(xL, yL, renderResult, outP);
	}

	return err;
}

static PF_Err
GetStreamValue(
	AEGP_SuiteHandler& suites, /*  in  */
	AEGP_StreamRefH& streamH,  /*  in  */
	AEGP_EffectRefH& effectPH, /*  in  */
	A_Time& timeT,			   /*  in  */
	KIRIParamIdx idx,		   /*  in  */
	AEGP_StreamValue& val	   /*  out */
)
{
	PF_Err		err = PF_Err_NONE;

	ERR(suites.StreamSuite2()->AEGP_GetNewEffectStreamByIndex(
		g_pluginID,
		effectPH,
		idx,
		&streamH
	));

	ERR(suites.StreamSuite2()->AEGP_GetNewStreamValue(
		g_pluginID,
		streamH,
		AEGP_LTimeMode_LayerTime,
		&timeT,
		FALSE,
		&val
	));

	return err;
}

static PF_Err
ReleaseStreamValue(
	AEGP_SuiteHandler& suites, /*  in  */
	AEGP_StreamRefH& streamH,  /*  out  */
	AEGP_StreamValue& val     /*  out  */
)
{
	PF_Err		err = PF_Err_NONE;
	ERR(suites.StreamSuite2()->AEGP_DisposeStreamValue(&val));
	ERR(suites.StreamSuite2()->AEGP_DisposeStream(streamH));

	return err;
}

template <typename Fn>
static PF_Err WithStreamValue(
	AEGP_SuiteHandler& suites,
	AEGP_EffectRefH effectPH,
	A_Time& timeT,
	KIRIParamIdx idx,
	Fn&& useValue)
{
	PF_Err err = PF_Err_NONE;

	AEGP_StreamRefH streamH = nullptr;
	AEGP_StreamValue val;
	AEFX_CLR_STRUCT(val);

	ERR(GetStreamValue(suites, streamH, effectPH, timeT, idx, val));

	useValue(val);

	ERR(ReleaseStreamValue(suites, streamH, val));

	return err;
}


static PF_Err
GetAEStreamValueInfo(
	PF_InData* in_data,
	PF_ParamDef* params[],
	AEStreamValueInfo& info
) {
	PF_Err		err = PF_Err_NONE;

	PLOGI << "GetSelectedPlyFile ";

	ERR(GetSelectedPlyFile(in_data, info.filePath));
	AEGP_SuiteHandler suites(in_data->pica_basicP);
	AEGP_StreamRefH streamH = nullptr;

	AEGP_LayerH currentLayerH = nullptr;
	ERR(suites.PFInterfaceSuite1()->AEGP_GetEffectLayer(
		in_data->effect_ref,
		&currentLayerH
	));

	PLOGI << "GetCurrentLayer ";

	if (currentLayerH == nullptr) {
		PLOGI << "currentLayerH is nullptr";
		return err;

	}

	AEGP_EffectRefH effectPH = nullptr;
	ERR(suites.EffectSuite5()->AEGP_GetLayerEffectByIndex(
		g_pluginID,
		currentLayerH,
		0,
		&effectPH));

	AEGP_StreamValue val;
	AEFX_CLR_STRUCT(val);

	A_Time timeT;
	timeT.value = in_data->current_time;
	timeT.scale = in_data->time_scale;

	// ====== layer ======
	info.splatEnable = params[KIRI_SPLAT_ENABLE]->u.bd.value;

	// ====== anchor ======
	ERR(WithStreamValue(suites, effectPH, timeT, KIRI_ALIGN_ANCHOR_POSITION,
		[&](const AEGP_StreamValue& val) {
			info.anchorPosition[0] = (float)val.val.three_d.x;
			info.anchorPosition[1] = (float)val.val.three_d.y;
			info.anchorPosition[2] = (float)val.val.three_d.z;
		}));

	ERR(WithStreamValue(suites, effectPH, timeT, KIRI_ALIGN_SCALE,
		[&](const AEGP_StreamValue& val) {
			info.anchorScale.r = (float)val.val.one_d / 100.0;
			info.anchorScale.g = (float)val.val.one_d / 100.0;
			info.anchorScale.b = (float)val.val.one_d / 100.0;
		}));

	ERR(WithStreamValue(suites, effectPH, timeT, KIRI_ALIGN_ROTATION_X,
		[&](const AEGP_StreamValue& val) {
			info.anchorRotation.x = (float)val.val.one_d;
		}));

	ERR(WithStreamValue(suites, effectPH, timeT, KIRI_ALIGN_ROTATION_Y,
		[&](const AEGP_StreamValue& val) {
			info.anchorRotation.y = (float)val.val.one_d;
		}));

	ERR(WithStreamValue(suites, effectPH, timeT, KIRI_ALIGN_ROTATION_Z,
		[&](const AEGP_StreamValue& val) {
			info.anchorRotation.z = -(float)val.val.one_d;
		}));

	// ====== anchor ======


	// ====== transform======
	ERR(WithStreamValue(suites, effectPH, timeT, KIRI_TRANSFORM_POSITION,
		[&](const AEGP_StreamValue& val) {
			info.transformPosition[0] = (float)val.val.three_d.x;
			info.transformPosition[1] = (float)val.val.three_d.y;
			info.transformPosition[2] = (float)val.val.three_d.z;
		}));

	ERR(WithStreamValue(suites, effectPH, timeT, KIRI_TRANSFORM_SCALE,
		[&](const AEGP_StreamValue& val) {
			info.transformScale.r = (float)val.val.one_d / 100.0;
			info.transformScale.g = (float)val.val.one_d / 100.0;
			info.transformScale.b = (float)val.val.one_d / 100.0;
		}));

	ERR(WithStreamValue(suites, effectPH, timeT, KIRI_TRANSFORM_ROTATION_X,
		[&](const AEGP_StreamValue& val) {
			info.trasnformRotation.x = (float)val.val.one_d;
		}));
	ERR(WithStreamValue(suites, effectPH, timeT, KIRI_TRANSFORM_ROTATION_Y,
		[&](const AEGP_StreamValue& val) {
			info.trasnformRotation.y = (float)val.val.one_d;
		}));
	ERR(WithStreamValue(suites, effectPH, timeT, KIRI_TRANSFORM_ROTATION_Z,
		[&](const AEGP_StreamValue& val) {
			info.trasnformRotation.z = -(float)val.val.one_d;
		}));

	// ====== transform======

	// ====== render =======

	ERR(WithStreamValue(suites, effectPH, timeT, KIRI_RENDER_COLOR_SHAPE_CENTER,
		[&](const AEGP_StreamValue& val) {
			info.colorShapeCenter.x = (float)val.val.three_d.x;
			info.colorShapeCenter.y = (float)val.val.three_d.y;
			info.colorShapeCenter.z = (float)val.val.three_d.z;
		}));

	ERR(WithStreamValue(suites, effectPH, timeT, KIRI_RENDER_COLOR_SHAPE_FEATHER,
		[&](const AEGP_StreamValue& val) {
			info.colorShapeFeather = (float)val.val.one_d / 100.0;
		}));
	ERR(WithStreamValue(suites, effectPH, timeT, KIRI_RENDER_COLOR_SHAPE_SCALE_X,
		[&](const AEGP_StreamValue& val) {
			info.colorShapeScaleXYZ.x = (float)val.val.one_d;
		}));
	ERR(WithStreamValue(suites, effectPH, timeT, KIRI_RENDER_COLOR_SHAPE_SCALE_Y,
		[&](const AEGP_StreamValue& val) {
			info.colorShapeScaleXYZ.y = (float)val.val.one_d;
		}));
	ERR(WithStreamValue(suites, effectPH, timeT, KIRI_RENDER_COLOR_SHAPE_SCALE_Z,
		[&](const AEGP_StreamValue& val) {
			info.colorShapeScaleXYZ.z = (float)val.val.one_d;
		}));

	info.shDegree = params[KIRI_RENDER_SH_DEGREE]->u.bd.value - 1;
	info.colorEnable = params[KIRI_RENDER_COLOR_ENBALE]->u.bd.value;
	ERR(WithStreamValue(suites, effectPH, timeT, KIRI_RENDER_COLOR_SHAPE_SIZE,
		[&](const AEGP_StreamValue& val) {
			info.colorShapeSize = (float)val.val.one_d;
		}));

	// ====== render =======


	// ====== crop =======
	info.cropEnable = params[KIRI_CROP_ENBALE]->u.bd.value;

	ERR(WithStreamValue(suites, effectPH, timeT, KIRI_CROP_SHAPE_CENTER,
		[&](const AEGP_StreamValue& val) {
			info.cropShapeCenter.x = (float)val.val.three_d.x;
			info.cropShapeCenter.y = (float)val.val.three_d.y;
			info.cropShapeCenter.z = (float)val.val.three_d.z;
		}));

	ERR(WithStreamValue(suites, effectPH, timeT, KIRI_CROP_SHAPE_FEATHER,
		[&](const AEGP_StreamValue& val) {
			info.cropShapeFeather = (float)val.val.one_d / 100.0;
		}));

	ERR(WithStreamValue(suites, effectPH, timeT, KIRI_CROP_SHAPE_SCALE_X,
		[&](const AEGP_StreamValue& val) {
			info.cropShapeScaleXYZ.x = (float)val.val.one_d;
		}));

	ERR(WithStreamValue(suites, effectPH, timeT, KIRI_CROP_SHAPE_SCALE_Y,
		[&](const AEGP_StreamValue& val) {
			info.cropShapeScaleXYZ.y = (float)val.val.one_d;
		}));

	ERR(WithStreamValue(suites, effectPH, timeT, KIRI_CROP_SHAPE_SCALE_Z,
		[&](const AEGP_StreamValue& val) {
			info.cropShapeScaleXYZ.z = (float)val.val.one_d;
		}));

	info.cropInvert = params[KIRI_CROP_INVERT]->u.bd.value;

	ERR(WithStreamValue(suites, effectPH, timeT, KIRI_CROP_SHAPE_SIZE,
		[&](const AEGP_StreamValue& val) {
			info.cropShapeSize = (float)val.val.one_d;
		}));

	// ====== crop =======

	// ====== Splat Scale =======
	info.splatScaleEnable = params[KIRI_SPLAT_SCALE_ENBALE]->u.bd.value;

	ERR(WithStreamValue(suites, effectPH, timeT, KIRI_SPLAT_SCALE_SIZE,
		[&](const AEGP_StreamValue& val) {
			info.splatScaleSize = (float)val.val.one_d / 100.0;
		}));

	ERR(WithStreamValue(suites, effectPH, timeT, KIRI_SPLAT_SCALE_SHAPE_CENTER,
		[&](const AEGP_StreamValue& val) {
			info.splatScaleShapeCenter.x = (float)val.val.three_d.x;
			info.splatScaleShapeCenter.y = (float)val.val.three_d.y;
			info.splatScaleShapeCenter.z = (float)val.val.three_d.z;
		}));

	ERR(WithStreamValue(suites, effectPH, timeT, KIRI_SPLAT_SCALE_SHAPE_FEATHER,
		[&](const AEGP_StreamValue& val) {
			info.splatScaleShapeFeather = (float)val.val.one_d / 100.0;
		}));

	ERR(WithStreamValue(suites, effectPH, timeT, KIRI_SPLAT_SCALE_SHAPE_SCALE_X,
		[&](const AEGP_StreamValue& val) {
			info.splatScaleShapeScaleXYZ.x = (float)val.val.one_d;
		}));

	ERR(WithStreamValue(suites, effectPH, timeT, KIRI_SPLAT_SCALE_SHAPE_SCALE_Y,
		[&](const AEGP_StreamValue& val) {
			info.splatScaleShapeScaleXYZ.y = (float)val.val.one_d;
		}));

	ERR(WithStreamValue(suites, effectPH, timeT, KIRI_SPLAT_SCALE_SHAPE_SCALE_Z,
		[&](const AEGP_StreamValue& val) {
			info.splatScaleShapeScaleXYZ.z = (float)val.val.one_d;
		}));

	ERR(WithStreamValue(suites, effectPH, timeT, KIRI_SPLAT_SCALE_SHAPE_SIZE,
		[&](const AEGP_StreamValue& val) {
			info.splatScaleShapeSize = (float)val.val.one_d;
		}));

	// ====== Splat Scale =======


	// ====== Splat Noise  =======
	info.splatNoiseEnable = params[KIRI_SPLAT_NOISE_ENABLE]->u.bd.value;
	ERR(WithStreamValue(suites, effectPH, timeT, KIRI_SPLAT_NOISE_SHAPE_SIZE,
		[&](const AEGP_StreamValue& val) {
			info.splatNoiseShapeSize = (float)val.val.one_d;
		}));

	ERR(WithStreamValue(suites, effectPH, timeT, KIRI_SPLAT_NOISE_OCTAVES,
		[&](const AEGP_StreamValue& val) {
			info.splatNoiseOctaves = (float)val.val.one_d;
		}));

	ERR(WithStreamValue(suites, effectPH, timeT, KIRI_SPLAT_NOISE_SHAPE_CENTER,
		[&](const AEGP_StreamValue& val) {
			info.splatNoiseShapeCenter.x = (float)val.val.three_d.x;
			info.splatNoiseShapeCenter.y = (float)val.val.three_d.y;
			info.splatNoiseShapeCenter.z = (float)val.val.three_d.z;
		}));

	ERR(WithStreamValue(suites, effectPH, timeT, KIRI_SPLAT_NOISE_SHAPE_FEATHER,
		[&](const AEGP_StreamValue& val) {
			info.splatNoiseShapeFeather = (float)val.val.one_d / 100.0;
		}));

	ERR(WithStreamValue(suites, effectPH, timeT, KIRI_SPLAT_NOISE_SHAPE_SCALE_X,
		[&](const AEGP_StreamValue& val) {
			info.splatNoiseShapeScaleXYZ.x = (float)val.val.one_d;
		}));

	ERR(WithStreamValue(suites, effectPH, timeT, KIRI_SPLAT_NOISE_SHAPE_SCALE_Y,
		[&](const AEGP_StreamValue& val) {
			info.splatNoiseShapeScaleXYZ.y = (float)val.val.one_d;
		}));

	ERR(WithStreamValue(suites, effectPH, timeT, KIRI_SPLAT_NOISE_SHAPE_SCALE_Z,
		[&](const AEGP_StreamValue& val) {
			info.splatNoiseShapeScaleXYZ.z = (float)val.val.one_d;
		}));

	ERR(WithStreamValue(suites, effectPH, timeT, KIRI_SPLAT_NOISE_PERSISTENCE,
		[&](const AEGP_StreamValue& val) {
			info.splatNoisePersistence = (float)val.val.one_d;
		}));

	ERR(WithStreamValue(suites, effectPH, timeT, KIRI_SPLAT_NOISE_LACUNARITY,
		[&](const AEGP_StreamValue& val) {
			info.splatNoiseLacunarity = (float)val.val.one_d;
		}));

	ERR(WithStreamValue(suites, effectPH, timeT, KIRI_SPLAT_NOISE_STRENGTH,
		[&](const AEGP_StreamValue& val) {
			info.splatNoiseStrength = (float)val.val.one_d;
		}));

	ERR(WithStreamValue(suites, effectPH, timeT, KIRI_SPLAT_NOISE_STRENGTH_X,
		[&](const AEGP_StreamValue& val) {
			info.splatNoiseStrengthX = (float)val.val.one_d;
		}));

	ERR(WithStreamValue(suites, effectPH, timeT, KIRI_SPLAT_NOISE_STRENGTH_Y,
		[&](const AEGP_StreamValue& val) {
			info.splatNoiseStrengthY = (float)val.val.one_d;
		}));

	ERR(WithStreamValue(suites, effectPH, timeT, KIRI_SPLAT_NOISE_STRENGTH_Z,
		[&](const AEGP_StreamValue& val) {
			info.splatNoiseStrengthZ = (float)val.val.one_d;
		}));


	// ====== Splat Noise  =======

	// ====== Splat Opacity  =======
	info.splatOpacityEnable = params[KIRI_SPLAT_OPACITY_ENABLE]->u.bd.value;

	ERR(WithStreamValue(suites, effectPH, timeT, KIRI_SPLAT_OPACITY_SHAPE_CENTER,
		[&](const AEGP_StreamValue& val) {
			info.splatOpacityShapeCenter.x = (float)val.val.three_d.x;
			info.splatOpacityShapeCenter.y = (float)val.val.three_d.y;
			info.splatOpacityShapeCenter.z = (float)val.val.three_d.z;
		}));

	ERR(WithStreamValue(suites, effectPH, timeT, KIRI_SPLAT_OPACITY_SHAPE_FEATHER,
		[&](const AEGP_StreamValue& val) {
			info.splatOpacityShapeFeather = (float)val.val.one_d / 100.0;
		}));

	ERR(WithStreamValue(suites, effectPH, timeT, KIRI_SPLAT_OPACITY_SHAPE_SCALE_X,
		[&](const AEGP_StreamValue& val) {
			info.splatOpacityShapeScaleXYZ.x = (float)val.val.one_d;
		}));

	ERR(WithStreamValue(suites, effectPH, timeT, KIRI_SPLAT_OPACITY_SHAPE_SCALE_Y,
		[&](const AEGP_StreamValue& val) {
			info.splatOpacityShapeScaleXYZ.y = (float)val.val.one_d;
		}));

	ERR(WithStreamValue(suites, effectPH, timeT, KIRI_SPLAT_OPACITY_SHAPE_SCALE_Z,
		[&](const AEGP_StreamValue& val) {
			info.splatOpacityShapeScaleXYZ.z = (float)val.val.one_d;
		}));

	ERR(WithStreamValue(suites, effectPH, timeT, KIRI_SPLAT_OPACITY_SHAPE_SIZE,
		[&](const AEGP_StreamValue& val) {
			info.splatOpacityShapeSize = (float)val.val.one_d;
		}));

	ERR(WithStreamValue(suites, effectPH, timeT, KIRI_SPLAT_MAX_OPACITY,
		[&](const AEGP_StreamValue& val) {
			info.splatMaxOpacity = (float)val.val.one_d;
		}));

	ERR(WithStreamValue(suites, effectPH, timeT, KIRI_SPLAT_MIN_OPACITY,
		[&](const AEGP_StreamValue& val) {
			info.splatMinOpacity = (float)val.val.one_d;
		}));

	// ====== Splat Opacity  =======

	// ====== Splat Displacement  =======
	info.splatDisplacementEnable = params[KIRI_SPLAT_DISPLACEMENT_ENABLE]->u.bd.value;

	ERR(WithStreamValue(suites, effectPH, timeT, KIRI_SPLAT_DISPLACEMENT_SCALE,
		[&](const AEGP_StreamValue& val) {
			info.splatDisplacementScale = (float)val.val.one_d;
		}));

	ERR(WithStreamValue(suites, effectPH, timeT, KIRI_SPLAT_DISPLACEMENT_OFFSET,
		[&](const AEGP_StreamValue& val) {
			info.splatDisplacementOffset.x = (float)val.val.three_d.x;
			info.splatDisplacementOffset.y = (float)val.val.three_d.y;
			info.splatDisplacementOffset.z = (float)val.val.three_d.z;
		}));

	ERR(WithStreamValue(suites, effectPH, timeT, KIRI_SPLAT_DISPLACEMENT_SHAPE_FEATHER,
		[&](const AEGP_StreamValue& val) {
			info.splatDisplacementShapeFeather = (float)val.val.one_d / 100.0;
		}));

	ERR(WithStreamValue(suites, effectPH, timeT, KIRI_SPLAT_DISPLACEMENT_ROTATION_X,
		[&](const AEGP_StreamValue& val) {
			info.splatDisplacementRotation.x = (float)val.val.one_d;
		}));

	ERR(WithStreamValue(suites, effectPH, timeT, KIRI_SPLAT_DISPLACEMENT_ROTATION_Y,
		[&](const AEGP_StreamValue& val) {
			info.splatDisplacementRotation.y = (float)val.val.one_d;
		}));

	ERR(WithStreamValue(suites, effectPH, timeT, KIRI_SPLAT_DISPLACEMENT_ROTATION_Z,
		[&](const AEGP_StreamValue& val) {
			info.splatDisplacementRotation.z = (float)val.val.one_d;
		}));

	ERR(WithStreamValue(suites, effectPH, timeT, KIRI_SPLAT_DISPLACEMENT_SHAPE_SCALE_X,
		[&](const AEGP_StreamValue& val) {
			info.splatDisplacementShapeScale.x = (float)val.val.one_d;
		}));

	ERR(WithStreamValue(suites, effectPH, timeT, KIRI_SPLAT_DISPLACEMENT_SHAPE_SCALE_Y,
		[&](const AEGP_StreamValue& val) {
			info.splatDisplacementShapeScale.y = (float)val.val.one_d;
		}));

	ERR(WithStreamValue(suites, effectPH, timeT, KIRI_SPLAT_DISPLACEMENT_SHAPE_SCALE_Z,
		[&](const AEGP_StreamValue& val) {
			info.splatDisplacementShapeScale.z = (float)val.val.one_d;
		}));

	ERR(WithStreamValue(suites, effectPH, timeT, KIRI_SPLAT_DISPLACEMENT_SHAPE_CENTER,
		[&](const AEGP_StreamValue& val) {
			info.splatDisplacementShapeCenter.x = (float)val.val.three_d.x;
			info.splatDisplacementShapeCenter.y = (float)val.val.three_d.y;
			info.splatDisplacementShapeCenter.z = (float)val.val.three_d.z;
		}));

	ERR(WithStreamValue(suites, effectPH, timeT, KIRI_SPLAT_DISPLACEMENT_SHAPE_SIZE,
		[&](const AEGP_StreamValue& val) {
			info.splatDisplacementShapeSize = (float)val.val.one_d;
		}));


	// ====== Splat Displacement  =======

	// ====== Splat Dense  =======
	info.splatDenseEnable = params[KIRI_SPLAT_DENSE_ENABLE]->u.bd.value;

	ERR(WithStreamValue(suites, effectPH, timeT, KIRI_SPLAT_DENSE_SHAPE_CENTER,
		[&](const AEGP_StreamValue& val) {
			info.splatDenseShapeCenter.x = (float)val.val.three_d.x;
			info.splatDenseShapeCenter.y = (float)val.val.three_d.y;
			info.splatDenseShapeCenter.z = (float)val.val.three_d.z;
		}));

	ERR(WithStreamValue(suites, effectPH, timeT, KIRI_SPLAT_DENSE_SHAPE_SCALE_X,
		[&](const AEGP_StreamValue& val) {
			info.splatDenseShapeScaleXYZ.x = (float)val.val.one_d;
		}));

	ERR(WithStreamValue(suites, effectPH, timeT, KIRI_SPLAT_DENSE_SHAPE_SCALE_Y,
		[&](const AEGP_StreamValue& val) {
			info.splatDenseShapeScaleXYZ.y = (float)val.val.one_d;
		}));

	ERR(WithStreamValue(suites, effectPH, timeT, KIRI_SPLAT_DENSE_SHAPE_SCALE_Z,
		[&](const AEGP_StreamValue& val) {
			info.splatDenseShapeScaleXYZ.z = (float)val.val.one_d;
		}));

	ERR(WithStreamValue(suites, effectPH, timeT, KIRI_SPLAT_DENSE_SHAPE_SIZE,
		[&](const AEGP_StreamValue& val) {
			info.splatDenseShapeSize = (float)val.val.one_d;
		}));

	ERR(WithStreamValue(suites, effectPH, timeT, KIRI_SPLAT_DENSE_DENSITY,
		[&](const AEGP_StreamValue& val) {
			info.splatDenseDensity = (float)val.val.one_d;
		}));

	ERR(WithStreamValue(suites, effectPH, timeT, KIRI_SPLAT_DENSE_SHAPE_FEATHER,
		[&](const AEGP_StreamValue& val) {
			info.splatDenseShapeFeather = (float)val.val.one_d / 100.0;
		}));
	// ====== Splat Dense  =======

	// ====== Advanced  =======
	ERR(WithStreamValue(suites, effectPH, timeT, KIRI_ADVANCED_CAMERA_FOCAL_LENGTH,
		[&](const AEGP_StreamValue& val) {
			info.advancedCameraFocalLength = (float)val.val.one_d;
		}));

	ERR(WithStreamValue(suites, effectPH, timeT, KIRI_ADVANCED_SPLAT_CROP_NEAR,
		[&](const AEGP_StreamValue& val) {
			info.advancedSplatCropNear = (float)val.val.one_d;
		}));

	ERR(WithStreamValue(suites, effectPH, timeT, KIRI_ADVANCED_SPLAT_CROP_FAR,
		[&](const AEGP_StreamValue& val) {
			info.advancedSplatCropFar = (float)val.val.one_d;
		}));

	ERR(WithStreamValue(suites, effectPH, timeT, KIRI_ADVANCED_SPLAT_CROP_MAX_SCALE,
		[&](const AEGP_StreamValue& val) {
			info.advancedSplatCropMaxScale = (float)val.val.one_d;
		}));

	ERR(WithStreamValue(suites, effectPH, timeT, KIRI_ADVANCED_SPLAT_CROP_MIN_SCALE,
		[&](const AEGP_StreamValue& val) {
			info.advancedSplatCropMinScale = (float)val.val.one_d;
		}));

	// ====== Dof  =======
	ERR(WithStreamValue(suites, effectPH, timeT, KIRI_ADVANCED_DOF_ENABLE,
		[&](const AEGP_StreamValue& val) {
			info.advancedDofEnable = (float)val.val.one_d;
		}));

	ERR(WithStreamValue(suites, effectPH, timeT, KIRI_ADVANCED_DOF_FOCUS_DISTANCE,
		[&](const AEGP_StreamValue& val) {
			info.advancedDofFocusDistance = (float)val.val.one_d;
		}));

	ERR(WithStreamValue(suites, effectPH, timeT, KIRI_ADVANCED_DOF_APERTURE,
		[&](const AEGP_StreamValue& val) {
			info.advancedDofAperture = (float)val.val.one_d;
		}));

	ERR(WithStreamValue(suites, effectPH, timeT, KIRI_ADVANCED_DOF_BLUR_LEVEL,
		[&](const AEGP_StreamValue& val) {
			info.advancedDofBlurLevel = (float)val.val.one_d;
		}));
	// ====== Dof  =======
	
	// ====== Glow =======
	ERR(WithStreamValue(suites, effectPH, timeT, KIRI_ADVANCED_GLOW_ENABLE,
		[&](const AEGP_StreamValue& val) {
			info.advancedGlowEnable = (float)val.val.one_d;
		}));

	info.advancedGlowBlendMode = params[KIRI_ADVANCED_GLOW_BLEND_MODE]->u.bd.value - 1;

	ERR(WithStreamValue(suites, effectPH, timeT, KIRI_ADVANCED_GLOW_RADIUS,
		[&](const AEGP_StreamValue& val) {
			info.advancedGlowRadius = (float)val.val.one_d;
		}));

	ERR(WithStreamValue(suites, effectPH, timeT, KIRI_ADVANCED_GLOW_THRESHOLD,
		[&](const AEGP_StreamValue& val) {
			info.advancedGlowThreshold = (float)val.val.one_d;
		}));

	ERR(WithStreamValue(suites, effectPH, timeT, KIRI_ADVANCED_GLOW_SMOOTH,
		[&](const AEGP_StreamValue& val) {
			info.advancedGlowSmooth = (float)val.val.one_d;
		}));
	// ====== Glow =======
	
	// ====== Advanced  =======


	// release

	return err;
}

static PF_Err
Render(
	PF_InData* in_data,
	PF_OutData* out_data,
	PF_ParamDef* params[],
	PF_LayerDef* output)
{
	PF_Err err = PF_Err_NONE;
	std::ostringstream oss;
	PLOGI << "PF_Cmd_RENDER";

	//return err;
	AEGP_SuiteHandler	suites(in_data->pica_basicP);

	g_openGLManager.BindContext();

	auto start = std::chrono::high_resolution_clock::now();

	AEStreamValueInfo streamValueInfo;
	ERR(GetAEStreamValueInfo(in_data, params, streamValueInfo));
	auto afterGetAEStreamValueInfo = std::chrono::high_resolution_clock::now();

	std::shared_ptr<GaussianModel> sharedGaussianModel = g_assetManager.GetGaussianModel(streamValueInfo.filePath);
	if (sharedGaussianModel == nullptr) {
		return err;
	}
	GaussianModel& modelRef = *sharedGaussianModel;

	PLOGI << "get streamValueinfo " + streamValueInfo.filePath;

	AEGP_EffectRefH  effect_handle = NULL;

	A_long				linesL = 0;
	linesL = output->extent_hint.bottom - output->extent_hint.top;

	//GaussianRenderInfo gaussianRenderInfo;
	ShaderInput shaderInput;

	ERR(CalcShaderInput(in_data, params, streamValueInfo, modelRef, shaderInput));
	auto afterComputeShaderInput = std::chrono::high_resolution_clock::now();

	g_gaussianRenderer.Render(modelRef, shaderInput, streamValueInfo);
	auto afterRender = std::chrono::high_resolution_clock::now();

	RenderResult renderResult;
	renderResult = g_gaussianRenderer.GetRenderResult(shaderInput);
	auto afterGetRenderResult = std::chrono::high_resolution_clock::now();

	renderResult.ratio_x = float(in_data->downsample_x.den) / float(in_data->downsample_x.num);
	renderResult.ratio_y = float(in_data->downsample_y.den) / float(in_data->downsample_y.num);

	// copy to layer output
	ERR(suites.Iterate8Suite2()->iterate(in_data,
		0,								// progress base
		linesL,							// progress final
		&params[KIRI_INPUT]->u.ld,		// src
		NULL,							// area - null for all pixels
		(void*)&renderResult,			// refcon - your custom data pointer
		MySimpleGainFunc8,				// pixel function pointer
		output));


	auto end = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double, std::milli> elapsed = end - start;
	auto getAEStreamValueInfoElapsed = std::chrono::duration<double, std::milli>(afterGetAEStreamValueInfo - start);
	auto computeShaderInputElapsed = std::chrono::duration<double, std::milli>(afterComputeShaderInput - afterGetAEStreamValueInfo);
	auto renderElapsed = std::chrono::duration<double, std::milli>(afterRender - afterComputeShaderInput);
	auto getRenderResultElapsed = std::chrono::duration<double, std::milli>(afterGetRenderResult - afterRender);
	auto copyToLayerOutputElapsed = std::chrono::duration<double, std::milli>(end - afterGetRenderResult);

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
	oss << "Render Stage Timing" << std::endl;
	appendStageTiming("GetAEStreamValueInfo", getAEStreamValueInfoElapsed.count());
	appendStageTiming("ComputeShaderInput", computeShaderInputElapsed.count());
	appendStageTiming("Render", renderElapsed.count());
	appendStageTiming("GetRenderResult", getRenderResultElapsed.count());
	appendStageTiming("CopyToLayerOutput", copyToLayerOutputElapsed.count());
	oss << "Total"
		<< " : " << elapsed.count() << " ms"
		<< " | 100%"
		<< " | " << (elapsed.count() > 0.0 ? 1000.0 / elapsed.count() : 0.0) << " fps"
		<< std::endl;
	PLOGI << oss.str();

	g_openGLManager.UnbindContext();
	PLOGI << "PF_Cmd_RENDER END";

	return err;
}

static PF_Err
CmdEvent(
	PF_InData* in_data,
	PF_OutData* out_data,
	PF_ParamDef* params[],
	PF_LayerDef* output,
	void* extra) {

	PF_Err		err = PF_Err_NONE;

	PF_EventExtra* event_extra = reinterpret_cast<PF_EventExtra*>(extra);

	//      PLOGI <<"effect_win.index" + std::to_string(event_extra->effect_win.index));
	switch (event_extra->effect_win.index) {
	case KIRI_RENDER_COLOR_GRADIENT: {
		err = ColorGradientUIHandleEvent(in_data, out_data, params, output, event_extra);
		break;
	}
	case KIRI_RENDER_COLOR_RAMP:
	case KIRI_SPLAT_SCALE_RAMP:
	case KIRI_SPLAT_OPACITY_RAMP:
	case KIRI_SPLAT_DISPLACEMENT_OFFSET_RAMP:
	case KIRI_SPLAT_DISPLACEMENT_SCALE_RAMP:
	case KIRI_SPLAT_DISPLACEMENT_ROTATION_RAMP:
	case KIRI_SPLAT_DENSE_SHAPE_RAMP: {
		err = BezierCurveUIHandleEvent(in_data, out_data, params, output, event_extra);
		break;
	}
	}

	return err;
}

static PF_Err
CmdArbitraryCallback(
	PF_InData* in_data,
	PF_OutData* out_data,
	PF_ParamDef* params[],
	PF_LayerDef* output,
	void* extra) {

	PF_Err		err = PF_Err_NONE;

	PF_ArbParamsExtra* arb_param_extra = reinterpret_cast<PF_ArbParamsExtra*>(extra);

	//param_def->u.uu.id


	//PLOGI <<"CmdArbitraryCallback  arb_param_extra.id " + std::to_string(arb_param_extra->id));

	switch (arb_param_extra->id) {
	case DISK_ID(KIRI_RENDER_COLOR_GRADIENT): {
		err = ColorGradientUIHandleArbitrary(in_data, out_data, params, output, arb_param_extra);
		break;
	}
	case DISK_ID(KIRI_RENDER_COLOR_RAMP):
	case DISK_ID(KIRI_SPLAT_SCALE_RAMP):
	case DISK_ID(KIRI_SPLAT_OPACITY_RAMP):
	case DISK_ID(KIRI_SPLAT_DISPLACEMENT_OFFSET_RAMP):
	case DISK_ID(KIRI_SPLAT_DISPLACEMENT_SCALE_RAMP):
	case DISK_ID(KIRI_SPLAT_DISPLACEMENT_ROTATION_RAMP):
	case DISK_ID(KIRI_SPLAT_DENSE_SHAPE_RAMP): {
		err = BezierCurveUIHandleArbitrary(in_data, out_data, params, output, arb_param_extra);
		break;
	}
	}


	return err;
}


extern "C" DllExport
PF_Err PluginDataEntryFunction2(
	PF_PluginDataPtr inPtr,
	PF_PluginDataCB2 inPluginDataCallBackPtr,
	SPBasicSuite * inSPBasicSuitePtr,
	const char* inHostName,
	const char* inHostVersion)
{
	PF_Err result = PF_Err_INVALID_CALLBACK;

	result = PF_REGISTER_EFFECT_EXT2(
		inPtr,
		inPluginDataCallBackPtr,
		"KIRI GaussianSplatting", // Name
		"ADBE Skeleton", // Match Name
		"Sample Plug-ins", // Category
		AE_RESERVED_INFO, // Reserved Info
		"EffectMain",	// Entry point
		"https://www.adobe.com");	// support URL

	return result;
}


extern "C" DllExport
PF_Err
EffectMain(
	PF_Cmd			cmd,
	PF_InData * in_data,
	PF_OutData * out_data,
	PF_ParamDef * params[],
	PF_LayerDef * output,
	void* extra)
{
	PF_Err		err = PF_Err_NONE;
	try {
		switch (cmd) {
		case PF_Cmd_ABOUT: {
			err = About(in_data, out_data, params, output);
			break;
		}

		case PF_Cmd_GLOBAL_SETUP: {
			err = GlobalSetup(in_data, out_data, params, output);
			Log("PF_Cmd_GLOBAL_SETUP");
			// init here is ok , not global 
#if defined(__APPLE__)
			g_metalManager.Init();
#endif
			break;
		}

		case PF_Cmd_PARAMS_SETUP: {
			err = ParamsSetup(in_data, out_data, params, output);
			break;
		}

		case PF_Cmd_RENDER: {

			err = Render(in_data, out_data, params, output);
			break;
		}

		case PF_Cmd_UPDATE_PARAMS_UI: {

			PLOGI << "PF_Cmd_UPDATE_PARAMS_UI";

			err = InitUIWhileFirstLoad(in_data, out_data);
			out_data->out_flags |= PF_OutFlag_FORCE_RERENDER |
				PF_OutFlag_REFRESH_UI;
			break;
		}

		case PF_Cmd_USER_CHANGED_PARAM: {
			PLOGI << "PF_Cmd_USER_CHANGED_PARAM";
			out_data->out_flags |= PF_OutFlag_FORCE_RERENDER |
				PF_OutFlag_REFRESH_UI;
			break;
		}

		case PF_Cmd_FRAME_SETUP: {
			PLOGI << "PF_Cmd_FRAME_SETUP";
			break;
		}

		case PF_Cmd_EVENT: {
			//PLOGI <<"PF_Cmd_EVENT ");
			err = CmdEvent(in_data, out_data, params, output, extra);
			break;
		}

		case PF_Cmd_ARBITRARY_CALLBACK: {
			err = CmdArbitraryCallback(in_data, out_data, params, output, extra);
			break;
		}

		case PF_Cmd_SEQUENCE_SETUP: {
			PLOGI << "PF_Cmd_SEQUENCE_SETUP";
			err = InitUISeqData(in_data, out_data);

			Log("PF_Cmd_SEQUENCE_SETUP");

			break;
		}

		case PF_Cmd_SEQUENCE_SETDOWN: {
			PLOGI << "PF_Cmd_SEQUENCE_DOWN";
			if (in_data->sequence_data) {
				PF_DISPOSE_HANDLE(in_data->sequence_data);
			}
			break;
		}

		}
	}

	catch (PF_Err& thrown_err) {
		err = thrown_err;
		PLOGI << "render err {}" << err;
	}
	return err;
}
