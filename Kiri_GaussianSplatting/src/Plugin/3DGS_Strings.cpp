#include "Plugin/3DGS_Strings.h"
#include "A.h"
#ifdef _WIN32
#include <windows.h>
#endif // _WIN32
#include "string"
#include "Common/Utils.h"

typedef struct {
	A_u_long	index;
	A_char		str[LANGUAGE_NUMTYPES][256];
} TableString;

static LanguageType LANGAUGE_TYPE = EN;
static bool IS_AE_2026 = false;

static int LanguageIndex(LanguageType language)
{
	const int index = static_cast<int>(language);
	if (index < 0 || index >= static_cast<int>(LANGUAGE_NUMTYPES)) {
		return static_cast<int>(EN);
	}
	return index;
}

void SetLanguageType(LanguageType language)
{
	LANGAUGE_TYPE = static_cast<LanguageType>(LanguageIndex(language));
}

LanguageType GetLanguageType()
{
	return LANGAUGE_TYPE;
}

#ifdef _WIN32
static std::string GetExeDir()
{
	char path[MAX_PATH];
	GetModuleFileNameA(NULL, path, MAX_PATH);

	std::string fullPath(path);
	return fullPath.substr(0, fullPath.find_last_of("\\/"));
}
#endif


void SetLanguageTypeFromTag(const char* langTagZ)
{
#ifdef _WIN32
	auto runningPath = GetExeDir();
	PLOGD << "runningPath " << runningPath;
	IS_AE_2026 = runningPath.find("2026") != std::string::npos;

#endif

	if (langTagZ != nullptr &&
		(langTagZ[0] == 'z' || langTagZ[0] == 'Z') &&
		(langTagZ[1] == 'h' || langTagZ[1] == 'H')) {
		SetLanguageType(ZH);
		return;
	}

	SetLanguageType(EN);
}

TableString g_strs[STRID_NUMTYPES] = {
	{ STRID_NONE,                        { "", "" } },
	{ STRID_NAME,                        { "KIRI INNOVATIONS", "KIRI INNOVATIONS" } },
	{ STRID_DESCRIPTION,                 { "https://www.kiriengine.app/", "https://www.kiriengine.app/" } },

	{ STRID_SPLAT_ENABLE,                { "Splat Enable", "启用" } },
	{ STRID_SELECT_FOOTAGE_LAYER,       { "Select Footage", "选择素材" } },

	// ================= Align =================
	{ STRID_ALIGN_TOPIC,                { "Align", "对齐" } },
	{ STRID_ALIGN_ANCHOR_POSITION,      { "Align Anchor Point", "锚点对齐" } },
	{ STRID_ALIGN_SCALE,                { "Align Scale", "缩放对齐" } },
	{ STRID_ALIGN_ROTATION_X,           { "Align Rotation X", "旋转 X" } },
	{ STRID_ALIGN_ROTATION_Y,           { "Align Rotation Y", "旋转 Y" } },
	{ STRID_ALIGN_ROTATION_Z,           { "Align Rotation Z", "旋转 Z" } },

	// ================= Transform =================
	{ STRID_TRANSFORM_TOPIC,            { "Transform", "变换" } },
	{ STRID_TRANSFORM_POSITION,         { "Position", "位置" } },
	{ STRID_TRANSFORM_SCALE,            { "Scale", "缩放" } },
	{ STRID_TRANSFORM_ROTATION_X,       { "Rotation X", "旋转 X" } },
	{ STRID_TRANSFORM_ROTATION_Y,       { "Rotation Y", "旋转 Y" } },
	{ STRID_TRANSFORM_ROTATION_Z,       { "Rotation Z", "旋转 Z" } },

	// ================= Effect =================
	{ STRID_EFFECT_TOPIC,               { "Effect", "效果" } },

	// ================= Render =================
	{ STRID_RENDER_TOPIC,               { "Render", "渲染" } },
	{ STRID_RENDER_COLOR_ENABLE,        { "Color Enable", "颜色启用" } },
	{ STRID_RENDER_COLOR_GRADIENT,      { "Color Gradient", "渐变" } },
	{ STRID_RENDER_COLOR_RAMP,          { "Color Ramp", "曲线" } },
	{ STRID_RENDER_SHDEGREE,            { "SH Degree", "SH 阶数" } },

	{ STRID_RENDER_COLOR_SHAPE_SIZE,    { "Color Shape Size", "区域大小" } },
	{ STRID_RENDER_COLOR_SHAPE_SCALE_X, { "Color Shape Scale X", "X 轴" } },
	{ STRID_RENDER_COLOR_SHAPE_SCALE_Y, { "Color Shape Scale Y", "Y 轴" } },
	{ STRID_RENDER_COLOR_SHAPE_SCALE_Z, { "Color Shape Scale Z", "Z 轴" } },
	{ STRID_RENDER_COLOR_SHAPE_CENTER,  { "Color Shape Center", "中心" } },
	{ STRID_RENDER_COLOR_SHAPE_FEATHER, { "Color Shape Feather", "羽化" } },

	// ================= Crop =================
	{ STRID_CROP_TOPIC,                 { "Crop", "裁剪" } },
	{ STRID_CROP_ENABLE,                { "Crop Enable", "启用" } },
	{ STRID_CROP_INVERT,                { "Crop Invert", "反转" } },

	{ STRID_CROP_SHAPE_SIZE,            { "Crop Shape Size", "区域大小" } },
	{ STRID_CROP_SHAPE_SCALE_X,         { "Crop Shape Scale X", "X 轴" } },
	{ STRID_CROP_SHAPE_SCALE_Y,         { "Crop Shape Scale Y", "Y 轴" } },
	{ STRID_CROP_SHAPE_SCALE_Z,         { "Crop Shape Scale Z", "Z 轴" } },
	{ STRID_CROP_SHAPE_CENTER,          { "Crop Shape Center", "中心" } },
	{ STRID_CROP_SHAPE_FEATHER,         { "Crop Shape Feather", "羽化" } },

	// ================= Splat Scale =================
	{ STRID_SPLAT_SCALE_TOPIC,          { "Splat Scale", "缩放" } },
	{ STRID_SPLAT_SCALE_ENABLE,         { "Scale Enable", "启用" } },
	{ STRID_SPLAT_SCALE_SIZE,           { "Scale Size", "大小" } },
	{ STRID_SPLAT_SCALE_RAMP,           { "Scale Ramp", "曲线" } },

	{ STRID_SPLAT_SCALE_SHAPE_SIZE,     { "Scale Shape Size", "区域大小" } },
	{ STRID_SPLAT_SCALE_SHAPE_SCALE_X,  { "Scale Shape Scale X", "X 轴" } },
	{ STRID_SPLAT_SCALE_SHAPE_SCALE_Y,  { "Scale Shape Scale Y", "Y 轴" } },
	{ STRID_SPLAT_SCALE_SHAPE_SCALE_Z,  { "Scale Shape Scale Z", "Z 轴" } },
	{ STRID_SPLAT_SCALE_SHAPE_CENTER,   { "Scale Shape Center", "中心" } },
	{ STRID_SPLAT_SCALE_SHAPE_FEATHER,  { "Scale Shape Feather", "羽化" } },

	// ================= Noise =================
	{ STRID_SPLAT_NOISE_TOPIC,          { "Noise", "噪声" } },
	{ STRID_SPLAT_NOISE_ENABLE,         { "Noise Enable", "启用" } },

	{ STRID_SPLAT_NOISE_SHAPE_SIZE,     { "Noise Shape Size", "区域大小" } },
	{ STRID_SPLAT_NOISE_SHAPE_SCALE_X,  { "Noise Shape Scale X", "X 轴" } },
	{ STRID_SPLAT_NOISE_SHAPE_SCALE_Y,  { "Noise Shape Scale Y", "Y 轴" } },
	{ STRID_SPLAT_NOISE_SHAPE_SCALE_Z,  { "Noise Shape Scale Z", "Z 轴" } },
	{ STRID_SPLAT_NOISE_SHAPE_CENTER,   { "Noise Shape Center", "中心" } },
	{ STRID_SPLAT_NOISE_SHAPE_FEATHER,  { "Noise Shape Feather", "羽化" } },

	{ STRID_SPLAT_NOISE_STRENGTH,       { "Noise Strength", "强度" } },
	{ STRID_SPLAT_NOISE_STRENGTH_X,     { "Noise Strength X", "X 强度" } },
	{ STRID_SPLAT_NOISE_STRENGTH_Y,     { "Noise Strength Y", "Y 强度" } },
	{ STRID_SPLAT_NOISE_STRENGTH_Z,     { "Noise Strength Z", "Z 强度" } },
	{ STRID_SPLAT_NOISE_OCTAVES,        { "Noise Octaves", "层级" } },
	{ STRID_SPLAT_NOISE_PERSISTENCE,    { "Noise Persistence", "衰减" } },
	{ STRID_SPLAT_NOISE_LACUNARITY,     { "Noise Lacunarity", "频率" } },

	// ================= Opacity =================
	{ STRID_SPLAT_OPACITY_TOPIC,        { "Opacity", "不透明度" } },
	{ STRID_SPLAT_OPACITY_ENABLE,       { "Opacity Enable", "启用" } },
	{ STRID_SPLAT_MIN_OPACITY,          { "Min Opacity", "最小值" } },
	{ STRID_SPLAT_MAX_OPACITY,          { "Max Opacity", "最大值" } },
	{ STRID_SPLAT_OPACITY_RAMP,         { "Opacity Ramp", "曲线" } },

	{ STRID_SPLAT_OPACITY_SHAPE_SIZE,   { "Opacity Shape Size", "区域大小" } },
	{ STRID_SPLAT_OPACITY_SHAPE_SCALE_X,{ "Opacity Shape Scale X", "X 轴" } },
	{ STRID_SPLAT_OPACITY_SHAPE_SCALE_Y,{ "Opacity Shape Scale Y", "Y 轴" } },
	{ STRID_SPLAT_OPACITY_SHAPE_SCALE_Z,{ "Opacity Shape Scale Z", "Z 轴" } },
	{ STRID_SPLAT_OPACITY_SHAPE_CENTER, { "Opacity Shape Center", "中心" } },
	{ STRID_SPLAT_OPACITY_SHAPE_FEATHER,{ "Opacity Shape Feather", "羽化" } },

	// ================= Displacement =================
	{ STRID_SPLAT_DISPLACEMENT_TOPIC,   { "Displacement", "位移" } },
	{ STRID_SPLAT_DISPLACEMENT_ENABLE,  { "Displacement Enable", "启用" } },

	{ STRID_SPLAT_DISPLACEMENT_OFFSET,  { "Displacement Offset", "偏移" } },
	{ STRID_SPLAT_DISPLACEMENT_OFFSET_RAMP,{ "Displacement Offset Ramp", "曲线" } },

	{ STRID_SPLAT_DISPLACEMENT_SCALE,   { "Displacement Scale", "缩放" } },
	{ STRID_SPLAT_DISPLACEMENT_SCALE_RAMP,{ "Displacement Scale Ramp", "曲线" } },

	{ STRID_SPLAT_DISPLACEMENT_ROTATION_X,{ "Displacement Rotation X" , "旋转 X" } },
	{ STRID_SPLAT_DISPLACEMENT_ROTATION_Y,{ "Displacement Rotation Y" , "旋转 Y" } },
	{ STRID_SPLAT_DISPLACEMENT_ROTATION_Z,{ "Displacement Rotation Z" , "旋转 Z" } },
	{ STRID_SPLAT_DISPLACEMENT_ROTATION_RAMP,{ "Displacement Rotation Ramp", "曲线" } },

	{ STRID_SPLAT_DISPLACEMENT_SHAPE_SIZE, { "Displacement Shape Size", "区域大小" } },
	{ STRID_SPLAT_DISPLACEMENT_SHAPE_SCALE_X,{ "Displacement Shape Scale X" , "X 轴" } },
	{ STRID_SPLAT_DISPLACEMENT_SHAPE_SCALE_Y,{ "Displacement Shape Scale Y" , "Y 轴" } },
	{ STRID_SPLAT_DISPLACEMENT_SHAPE_SCALE_Z,{ "Displacement Shape Scale Z" , "Z 轴" } },
	{ STRID_SPLAT_DISPLACEMENT_SHAPE_CENTER,{  "Displacement Shape Center" , "中心" } },
	{ STRID_SPLAT_DISPLACEMENT_SHAPE_FEATHER,{ "Displacement Shape Center" , "羽化" } },

	// ================= Dense =================
	{ STRID_SPLAT_DENSE_TOPIC,          { "Density", "密度" } },
	{ STRID_SPLAT_DENSE_ENABLE,         { "Dense Enable", "启用" } },
	{ STRID_SPLAT_DENSE_DENSITY,        { "Density", "密度" } },
	{ STRID_SPLAT_DENSE_DENSITY_RAMP,   { "Density Ramp", "曲线" } },

	{ STRID_SPLAT_DENSE_SHAPE_SIZE,     { "Dense Shape Size", "区域大小" } },
	{ STRID_SPLAT_DENSE_SHAPE_SCALE_X,  { "Dense Shape Scale X" , "X 轴" } },
	{ STRID_SPLAT_DENSE_SHAPE_SCALE_Y,  { "Dense Shape Scale Y" , "Y 轴" } },
	{ STRID_SPLAT_DENSE_SHAPE_SCALE_Z,  { "Dense Shape Scale Z" , "Z 轴" } },
	{ STRID_SPLAT_DENSE_SHAPE_CENTER,   { "Dense Shape Center" , "中心" } },
	{ STRID_SPLAT_DENSE_SHAPE_FEATHER,  { "Dense Shape Center" , "羽化" } },


	{ STRID_INVERT_SPHERE_TOPIC,		{ "Invert Sphere" ,"球面反转"  }},
	{ STRID_INVERT_SPHERE_ENABLE,		{ "Enable"  ,		"启动"  }}, 
	{ STRID_INVERT_SPHERE_CENTER,		{ "Center"  ,		"中心"  }}, 
	{ STRID_INVERT_SPHERE_RADIUS,		{ "Radius"  ,		"半径"  }}, 
	{ STRID_INVERT_SPHERE_INTENSITY,	{ "Intensity" ,	"反转强度"  }}, 
	{ STRID_INVERT_SPHERE_DISTANCE,		{ "Distance"  ,	"临界距离"  }},
	{ STRID_INVERT_SPHERE_COMPRESSION,	{ "Compression" , "压缩密度"  }},

	// ================= Advanced =================
	{ STRID_ADVANCED_TOPIC,               { "Advanced", "高级" } },
	{ STRID_ADVANCED_CAMERA_FOCAL_LENGTH, { "Camera Focal Length", "焦距" } },
	{ STRID_ADVANCED_SPLAT_CROP_NEAR,     { "Near Crop", "近裁剪" } },
	{ STRID_ADVANCED_SPLAT_CROP_FAR,      { "Far Crop", "远裁剪" } },
	{ STRID_ADVANCED_SPLAT_CROP_MAX_SCALE,{ "Max Scale", "最大缩放" } },
	{ STRID_ADVANCED_SPLAT_CROP_MIN_SCALE,{ "Min Scale", "最小缩放" } },

	// ================= DOF =================
	{ STRID_KIRI_ADVANCED_DOF_TOPIC,		 { "Depth of Field", "景深" } },
	{ STRID_KIRI_ADVANCED_DOF_ENABLE,		 { "Enable", "启用" } },
	{ STRID_KIRI_ADVANCED_DOF_FOCUS_DISTANCE,{ "Focus Distance", "对焦距离" } },
	{ STRID_KIRI_ADVANCED_DOF_APERTURE,		 { "Aperture", "光圈" } },
	{ STRID_KIRI_ADVANCED_DOF_BLUR_LEVEL,	 { "Blur", "模糊" } },

	// ================ GLOW =================

	{  STRID_KIRI_ADVANCED_GLOW_TOPIC,		{ "Glow", "辉光" } },
	{  STRID_KIRI_ADVANCED_GLOW_ENABLE,		{ "Enable", "启用" } },
	{  STRID_KIRI_ADVANCED_GLOW_BLEND_MODE,	{ "Blend Mode", "混合模式" } },
	{  STRID_KIRI_ADVANCED_GLOW_RADIUS,		{ "Radius", "半径" } },
	{  STRID_KIRI_ADVANCED_GLOW_THRESHOLD,	{ "Threshold", "阈值" } },
	{  STRID_KIRI_ADVANCED_GLOW_SMOOTH,		{ "Smooth", "平滑" } }

	// ================ GLOW =================

};

static_assert((sizeof(g_strs) / sizeof(g_strs[0])) == STRID_NUMTYPES, "Missing string table entries");

 extern "C" std::string GetStringPtr(int strNum)
{
	const int languageIndex = LanguageIndex(LANGAUGE_TYPE);

	A_char* gbk = g_strs[strNum].str[languageIndex];
#ifdef _WIN32
		if (IS_AE_2026) {
			std::string result = GBKToUTF8(gbk);
			return result;
		}
#endif // _WIN32
		return gbk;
}
