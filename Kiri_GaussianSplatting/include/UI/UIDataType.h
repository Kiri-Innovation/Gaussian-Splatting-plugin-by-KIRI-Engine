#pragma once 

#include "UI/BezierCurveUI.h"

#define MAX_COLOR_GRADIENT_COUNT				 20
#define MAX_BEZIER_POINT_COUNT					 20

struct ColorGradientCursorGpu {
	float lerpFactor = 0;
	float R = 0;
	float G = 0;
	float B = 0;
};

typedef struct ColorGradientCursor {
	bool enable = false;
	float lerpFactor = 0; //[0,1]
	DRAWBOT_ColorRGBA color;
	ColorGradientCursorGpu ToGpu() const {
		return ColorGradientCursorGpu{ lerpFactor  , color.red , color.green , color.blue };
	}
}ColorGradientCursor;

struct ColorGradientInfoGpu {
	ColorGradientCursorGpu colorGradientCursorInfo[MAX_COLOR_GRADIENT_COUNT];
	int currentCursorCount = 0;
	int pad_0;
	int pad_1;
	int pad_2;
};

typedef struct ColorGradientInfo {
	ColorGradientCursor colorGradientCursorInfo[MAX_COLOR_GRADIENT_COUNT];
	int currentCursorCount = 0;
	ColorGradientInfoGpu ToGpu() const {
		ColorGradientInfoGpu result{  };
		result.currentCursorCount = currentCursorCount;
		for (int i = 0; i < MAX_COLOR_GRADIENT_COUNT; i++) {
			result.colorGradientCursorInfo[i] = colorGradientCursorInfo[i].ToGpu();
		}
		return result;
	}

}ColorGradientInfo;
// === Color Gradient ===

// === Bezier Curve ===
// [0,1]
struct BezierPointGpu {
	float x;
	float y;
	float pad_0;
	float pad_1;
};

typedef struct BezierPoint {
	bool enable = false;
	DRAWBOT_PointF32 point; //[0,1]
	BezierPointGpu ToGpu() const {
		return BezierPointGpu{ point.x  , point.y ,0.0 , 0.0};
	}
}BezierPoint;


struct BezierCurveInfoGpu {
	BezierPointGpu bezierPointInfo[MAX_BEZIER_POINT_COUNT];
	int currentPointCount = 0;
	int pad_0;
	int pad_1;
	int pad_2;
};

typedef struct BezierCurveInfo {
	int currentPointCount = 0;
	BezierPoint bezierPointInfo[MAX_BEZIER_POINT_COUNT];
	BezierCurveInfoGpu ToGpu() {
		BezierCurveInfoGpu result;
		result.currentPointCount = currentPointCount;
		for (int i = 0; i < currentPointCount; i++) {
			result.bezierPointInfo[i] = bezierPointInfo[i].ToGpu();
		}
		return result;
	}
}BezierCurveInfo;

// === Bezier Curve ===

struct UISequenceData {
	A_Boolean isInitialized;
};