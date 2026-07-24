#pragma once

#include <AE_GeneralPlug.h>
#include <AEFX_SuiteHandlerTemplate.h>
#include "AE_Effect.h"
#include "Param_Utils.h"

static PF_Err
DoClick(
	PF_InData* in_data,
	PF_OutData* out_data,
	PF_ParamDef* params[],
	PF_LayerDef* output,
	PF_EventExtra* event_extra);

static PF_Err
DrawEvent(
	PF_InData* in_data,
	PF_OutData* out_data,
	PF_ParamDef* params[],
	PF_LayerDef* output,
	PF_EventExtra* event_extra,
	PF_Pixel			some_color);

static PF_Err
ChangeCursor(
	PF_InData* in_data,
	PF_OutData* out_data,
	PF_ParamDef* params[],
	PF_LayerDef* output,
	PF_EventExtra* event_extra);

inline PF_Err ReleaseDrawbotObject(
	DRAWBOT_SupplierSuite1* supplierSuiteP,
	DRAWBOT_ObjectRef objRef)
{
	PF_Err err = PF_Err_NONE;
	if (objRef) {
		ERR(supplierSuiteP->ReleaseObject(objRef));
	}
	return err;
}
