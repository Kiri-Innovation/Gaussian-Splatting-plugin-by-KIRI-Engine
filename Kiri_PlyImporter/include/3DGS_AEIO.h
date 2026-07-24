#pragma once

#ifndef THREEDGSAEPG_H
#define THREEDGSAEPG_H

#include "AEConfig.h"

#include "entry.h"
#include "AE_Effect.h"
#include "AE_EffectCB.h"
#include "AE_Macros.h"
#include "Param_Utils.h"
#include "AE_EffectCBSuites.h"
#include "String_Utils.h"
#include "AE_GeneralPlug.h"
#include "AEGP_SuiteHandler.h"
#include "AEConfig.h"

#include "entry.h"

#ifdef AE_OS_WIN
	//#include <windows.h>
#endif

#include "AE_IO.h"
#include "AE_Macros.h"
#include "AE_EffectCBSuites.h"
#include "AEGP_SuiteHandler.h"
#include "string.h"


typedef struct {
	A_char				name[AEGP_MAX_PATH_SIZE];	// "SDK_FILE ";
	A_u_char			hasAudio;					// 	0x00 = no audio
	//	0x0f = audio uncompressed
	//	0xff = audio RLE comp. 
	//	NOTE: for demo purposes; the plug-in does NOT compress audio
	A_u_char			hasVideo;					// always 32 bit
	//	0x00 = no video
	//	0x0f = video uncompressed
	//	0xff = video RLE comp.
	A_u_long			widthLu;					// width of frame in pixels
	A_u_long			heightLu;					// height of frame in pixels
	A_u_long			rowbytesLu;					// total bytes in row, aka width * 4, we strip any row padding
	A_Time				fpsT;
	A_short				bitdepthS;
	A_FpLong			rateF;
	A_long				avg_bit_rateL;
	A_long				block_sizeL;
	A_long				frames_per_blockL;
	AEIO_SndEncoding	encoding;
	AEIO_SndSampleSize	bytes_per_sample;
	AEIO_SndChannels	num_channels;
	A_short				padS;
	A_Time				durationT;
}IO_FileHeader;

typedef struct {
	A_long				isFlat;
	A_char				name[AEGP_MAX_PATH_SIZE];	// "SDK_FILE ";
	A_u_char			hasAudio;					// 	0x00 = no audio
	//	0x0f = audio uncompressed
	//	0xff = audio RLE comp. 
	//	NOTE: for demo purposes; the plug-in does NOT compress audio
	A_u_char			hasVideo;					// always 32 bit
	//	0x00 = no video
	//	0x0f = video uncompressed
	//	0xff = video RLE comp.
	A_u_long			widthLu;					// width of frame in pixels
	A_u_long			heightLu;					// height of frame in pixels
	A_u_long			rowbytesLu;					// total bytes in row, aka width * 4, we strip any row padding
	A_u_long			numFramesLu;				// number of frames in file
	A_Time				fpsT;
	A_short				bitdepthS;
	A_FpLong			rateF;
	A_long				avg_bit_rateL;
	A_long				block_sizeL;
	A_long				frames_per_blockL;
	AEIO_SndEncoding	encoding;
	AEIO_SndSampleSize	bytes_per_sample;
	AEIO_SndChannels	num_channels;
	A_short				padS;
	A_u_long			duration_in_msLu;
	A_Time				durationT;
} IO_FlatFileOutputOptions;


extern "C" {

	DllExport
		A_Err
		EntryPointFunc(
			struct SPBasicSuite* pica_basicP,
			A_long major_versionL,
			A_long minor_versionL,
			AEGP_PluginID pluginID,
			AEGP_GlobalRefcon* global_refconP);

	
}

#endif 