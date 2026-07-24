#include "AEConfig.h"
#include "AE_EffectVers.h"

#ifndef AE_OS_WIN
	#include <AE_General.r>
#endif

// AEIO
resource 'PiPL' (16000) {
	{	/* array properties: 7 elements */
		/* [1] */
		Kind {
			AEGP
		},
		/* [2] */
		Name {
			"Kiri_PlyImport"
		},
		/* [3] */
		Category {
			"Kiri Innovation"
		},
		/* [4] */
		Version {
			0x00010000
		},
		/* [5] */
#ifdef AE_OS_WIN
    #if defined(AE_PROC_INTELx64)
		CodeWin64X86 {"EntryPointFunc"},
    #elif defined(AE_PROC_ARM64)
		CodeWinARM64 {"EntryPointFunc"},
    #endif
#elif defined(AE_OS_MAC)
		CodeMacIntel64 {"EntryPointFunc"},
		CodeMacARM64 {"EntryPointFunc"},
#endif
	}
};