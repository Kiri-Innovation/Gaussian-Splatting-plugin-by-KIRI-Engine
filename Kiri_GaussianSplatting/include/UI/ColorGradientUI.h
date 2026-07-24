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

#ifdef AE_OS_WIN
	#include <windows.h>
#endif

#define UI_COLOR_GRADIENT_TOPIC_WIDTH			 150
#define UI_COLOR_GRADIENT_TOPIC_HEIGHT			 80
#define UI_COLOR_GRADIENT_TOPIC_OFFSET			 50
#define UI_COLOR_GRADIENT_IMG_HEIGHT			 50
#define UI_CURSOR_ICON_WIDTH					 20
#define UI_CURSOR_ICON_HEIGHT					 20
#define UI_CURSOR_ICON_MARGIN					 3



PF_Err
ColorGradientUIHandleEvent(
	PF_InData		*in_data,
	PF_OutData		*out_data,
	PF_ParamDef		*params[],
	PF_LayerDef		*output,
	PF_EventExtra	*extra
);


PF_Err
SetupColorGradientUI(
	PF_InData* in_data,   /* in */
	uint32_t diskID,     /* in */
	PF_OutData* out_data /* out */
);

PF_Err 
ColorGradientUIHandleArbitrary(
	PF_InData* in_data,
	PF_OutData* out_data,
	PF_ParamDef* params[],
	PF_LayerDef* output,
	PF_ArbParamsExtra* extra
);




