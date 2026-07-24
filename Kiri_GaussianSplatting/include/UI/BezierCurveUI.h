#pragma once

#include "AEConfig.h"
#include "entry.h"
#include "AE_EffectUI.h"
#include "AE_EffectCBSuites.h"
#include "AE_AdvEffectSuites.h"
#include "AE_EffectSuitesHelper.h"
#include "String_Utils.h"
#include "AEGP_SuiteHandler.h"
#include "Param_Utils.h"
#include "AE_Macros.h"
#include "Plugin/3DGS_PF.h"


#define UI_BEZIER_TOPIC_WIDTH			 150
#define UI_BEZIER_TOPIC_HEIGHT			 150
#define UI_BEZIER_TOPIC_OFFSET			 50
#define UI_BEZIER_IMG_HEIGHT			 50
#define UI_BEZIER_ICON_WIDTH			 20
#define UI_BEZIER_ICON_HEIGHT			 20
#define UI_BEZIER_ICON_MARGIN			 3
#define UI_BEZIER_FRAME_WIDTH_MARGIN	 20
#define UI_BEZIER_FRAME_HEIGHT_MARGIN    20



PF_Err
SetupBezierUI(
	PF_InData* in_data, /* in */
	uint32_t diskID,  /* in */
	StrIDType   strId,	  /* in */
	PF_OutData* out_data /* out */
);

PF_Err
BezierCurveUIHandleEvent(
	PF_InData* in_data,
	PF_OutData* out_data,
	PF_ParamDef* params[],
	PF_LayerDef* output,
	PF_EventExtra* extra
);

PF_Err
BezierCurveUIHandleArbitrary(
	PF_InData* in_data,
	PF_OutData* out_data,
	PF_ParamDef* params[],
	PF_LayerDef* output,
	PF_ArbParamsExtra* extra
);




