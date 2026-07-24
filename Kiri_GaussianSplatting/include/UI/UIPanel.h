#pragma once

#include "Plugin/3DGS_PF.h"
#include "Common/Utils.h"
#include <chrono>
#include <AE_GeneralPlug.h>
#include <AEFX_SuiteHandlerTemplate.h>
#include "AE_Effect.h"
#include "Param_Utils.h"
#include "UI/BezierCurveUI.h"

 

//#define ARB_REFCON			(void*)0xDEADBEEFDEADBEEF


PF_Err SetupLayerInputUI(PF_InData* in_data , PF_OutData* out_data);
PF_Err SetupAlignUI(PF_InData* in_data, PF_OutData* out_data);
PF_Err SetupTransformUI(PF_InData* in_data, PF_OutData* out_data);
PF_Err SetupEffectUI(PF_InData* in_data, PF_OutData* out_data);
PF_Err SetupRenderUI(PF_InData* in_data, PF_OutData* out_data);
PF_Err SetupCropUI(PF_InData* in_data, PF_OutData* out_data);
PF_Err SetupSplatScaleUI(PF_InData* in_data, PF_OutData* out_data);
PF_Err SetupSplatNoiseUI(PF_InData* in_data, PF_OutData* out_data);
PF_Err SetupSplatOpacityUI(PF_InData* in_data, PF_OutData* out_data);
PF_Err SetupSplatDisplacementUI(PF_InData* in_data, PF_OutData* out_data);
PF_Err SetupSplatDenseUI(PF_InData* in_data, PF_OutData* out_data);
PF_Err SetupAdvancedUI(PF_InData* in_data, PF_OutData* out_data);

PF_Err InitUISeqData(PF_InData* in_data, PF_OutData* out_data);
PF_Err InitUIWhileFirstLoad(PF_InData* in_data, PF_OutData* out_data);