#include "AEConfig.h"
#include "AE_EffectVers.h"

#ifndef AE_OS_WIN
	#include <AE_General.r>
#endif

// PF 
resource 'PiPL' (16000) {
	{	/* array properties: 12 elements */
		/* [1] */
		Kind {
			AEEffect
		},
		/* [2] */
		Name {
			"Kiri_GaussianSplatting"
		},
		/* [3] */
		Category {
			"Kiri Innovation"
		},
#ifdef AE_OS_WIN
   #if defined(AE_PROC_INTELx64)
		CodeWin64X86 {"EffectMain"},
   #elif defined(AE_PROC_ARM64)
		CodeWinARM64 {"EffectMain"},
   #endif
#elif defined(AE_OS_MAC)
		CodeMacIntel64 {"EffectMain"},
		CodeMacARM64 {"EffectMain"},
#endif
		/* [6] */
		AE_PiPL_Version {
			2,
			0
		},
		/* [7] */
		AE_Effect_Spec_Version {
			PF_PLUG_IN_VERSION,
			PF_PLUG_IN_SUBVERS
		},
		/* [8] */
		AE_Effect_Version {
			557057	/* 1.1 */
		},
		/* [9] */
		AE_Effect_Info_Flags {
			0
		},
		/* [10] */
		AE_Effect_Global_OutFlags {
			//0x800000 // PF_OutFlag_PiPL_OVERRIDES_OUTDATA_OUTFLAGS
			//0x408000
			//0x408040
			0x408050
		},
		AE_Effect_Global_OutFlags_2 {
			0x00000006 // PF_OutFlag2_I_USE_3D_CAMERA | PF_OutFlag2_I_USE_3D_LIGHTS 
		},
		/* [11] */
		AE_Effect_Match_Name {
			"ADBE Kiri_GaussianSplatting"
		},
		/* [12] */
		AE_Reserved_Info {
			0
		},
		/* [13] */
		AE_Effect_Support_URL {
			"https://www.adobe.com"
		}
	}
};