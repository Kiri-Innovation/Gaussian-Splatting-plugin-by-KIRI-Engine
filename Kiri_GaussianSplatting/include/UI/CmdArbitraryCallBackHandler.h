#pragma once

#include <AE_GeneralPlug.h>
#include <AEFX_SuiteHandlerTemplate.h>
#include "AE_Effect.h"
#include "Param_Utils.h"
#include "Common/Utils.h"


static PF_Err
CreateDefaultArb(
	PF_InData* in_data,
	PF_OutData* out_data,
	PF_ArbitraryH* dephault);

static PF_Err
Arb_Copy(
	PF_InData* in_data,
	PF_OutData* out_data,
	const PF_ArbitraryH* srcP,
	PF_ArbitraryH* dstP);

static PF_Err
Arb_Interpolate(
	PF_InData* in_data,
	PF_OutData* out_data,
	double					itrp_amtF,
	const PF_ArbitraryH* l_arbP,
	const PF_ArbitraryH* r_arbP,
	PF_ArbitraryH* result_arbP);

static PF_Err
Arb_Compare(
	PF_InData* in_data,
	PF_OutData* out_data,
	const PF_ArbitraryH* a_arbP,
	const PF_ArbitraryH* b_arbP,
	PF_ArbCompareResult* resultP);


// remember to release arbHP outside this function
template<typename T>
inline PF_Err GetArbData(PF_InData* in_data,     /* in */
	PF_ParamDef* params[],	 /* in */
	PF_ParamIndex paramIdx, /* in */
	T** arbPP,				 /* out */
	PF_ArbitraryH* arbHP)	 /* out */
{
	PF_Err err = PF_Err_NONE;

	PF_ParamDef* arb_param = params[paramIdx];

	if (arb_param->param_type == PF_Param_ARBITRARY_DATA) {
		*arbHP = arb_param->u.arb_d.value;
		if (!*arbHP) {
			PLOGI <<"no arbH for " + std::to_string(paramIdx);
			return err;
		}
		*arbPP = reinterpret_cast<T*>(PF_LOCK_HANDLE(*arbHP));
	}
	return err;
}
