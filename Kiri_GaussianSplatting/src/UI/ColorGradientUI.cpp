
#include "UI/ColorGradientUI.h"
#include "UI/CmdArbitraryCallBackHandler.h"
#include "UI/CmdEventHandler.h"
#include "Common/Global.h"
#include "AEFX_SuiteHelper.h"
#include <cmath>
#include "Common/Utils.h"
#include "Vector"
#include "algorithm"
#include "Plugin/3DGS_Strings.h"
#include "Plugin/3DGS_PF.h"

#include "AE_Macros.h"
#include "UI/UIDataType.h"

static PF_Err
CreateDefaultArb(
	PF_InData* in_data,
	PF_OutData* out_data,
	PF_ArbitraryH* dephault)
{

	PF_Err			err = PF_Err_NONE;
	PF_Handle		arbH = NULL;

	AEGP_SuiteHandler	suites(in_data->pica_basicP);

	arbH = suites.HandleSuite1()->host_new_handle(sizeof(ColorGradientInfo));

	if (arbH) {
		ColorGradientInfo* arbP = reinterpret_cast<ColorGradientInfo*>(PF_LOCK_HANDLE(arbH));
		if (!arbP) {
			err = PF_Err_OUT_OF_MEMORY;
		}
		else {
			AEFX_CLR_STRUCT(*arbP);
#ifdef AE_OS_WIN
#pragma warning (disable : 4305)
#endif
			arbP->currentCursorCount = 2;

			arbP->colorGradientCursorInfo[0].lerpFactor = 0.01;
			arbP->colorGradientCursorInfo[0].enable = true;
			arbP->colorGradientCursorInfo[0].color.alpha = 1;
			arbP->colorGradientCursorInfo[0].color.red = 1;
			arbP->colorGradientCursorInfo[0].color.green = 0;
			arbP->colorGradientCursorInfo[0].color.blue = 0;

			arbP->colorGradientCursorInfo[1].lerpFactor = 1;
			arbP->colorGradientCursorInfo[1].enable = true;
			arbP->colorGradientCursorInfo[1].color.alpha = 1;
			arbP->colorGradientCursorInfo[1].color.red = 0;
			arbP->colorGradientCursorInfo[1].color.green = 1;
			arbP->colorGradientCursorInfo[1].color.blue = 0;

#ifdef AE_OS_WIN
#pragma warning (pop)
#endif
			* dephault = arbH;
		}
		suites.HandleSuite1()->host_unlock_handle(arbH);
	}
	return err;
}

static PF_Err
Arb_Copy(
	PF_InData* in_data,
	PF_OutData* out_data,
	const PF_ArbitraryH* srcP,
	PF_ArbitraryH* dstP)
{
	PF_Err err = PF_Err_NONE;

	PF_Handle	sourceH = *srcP;

	AEGP_SuiteHandler suites(in_data->pica_basicP);

	if (sourceH) {
		ColorGradientInfo* src_arbP = reinterpret_cast<ColorGradientInfo*>(suites.HandleSuite1()->host_lock_handle(sourceH));
		if (!src_arbP) {
			err = PF_Err_OUT_OF_MEMORY;
		}
		else {
			PF_Handle	destH = *dstP;
			if (destH) {
				ColorGradientInfo* dst_arbP = reinterpret_cast<ColorGradientInfo*>(suites.HandleSuite1()->host_lock_handle(destH));
				if (!dst_arbP) {
					err = PF_Err_OUT_OF_MEMORY;
				}
				else {
					memcpy(dst_arbP, src_arbP, sizeof(ColorGradientInfo));
					suites.HandleSuite1()->host_unlock_handle(destH);
				}
			}
			suites.HandleSuite1()->host_unlock_handle(sourceH);
		}
	}
	return err;
}


static PF_Err
Arb_Interpolate(
	PF_InData* in_data,
	PF_OutData* out_data,
	double					intrp_amtF,
	const PF_ArbitraryH* l_arbPH,
	const PF_ArbitraryH* r_arbPH,
	PF_ArbitraryH* intrp_arbP)
{
	PF_Err			err = PF_Err_NONE;

	/*=
	ColorGradientInfo* int_arbP = NULL,
		* l_arbP = NULL,
		* r_arbP = NULL;

	PF_PixelFloat* headP = NULL,
		* lpixP = NULL,
		* rpixP = NULL;

	A_short 		iS = 0;

	AEGP_SuiteHandler suites(in_data->pica_basicP);

	int_arbP = reinterpret_cast<ColorGradientInfo*>(suites.HandleSuite1()->host_lock_handle(*intrp_arbP));
	l_arbP = reinterpret_cast<ColorGradientInfo*>(suites.HandleSuite1()->host_lock_handle(*l_arbPH));
	r_arbP = reinterpret_cast<ColorGradientInfo*>(suites.HandleSuite1()->host_lock_handle(*r_arbPH));

	headP = reinterpret_cast<PF_PixelFloat*>(int_arbP);
	lpixP = reinterpret_cast<PF_PixelFloat*>(l_arbP);
	rpixP = reinterpret_cast<PF_PixelFloat*>(r_arbP);

	//for (iS = 0; iS <  ColorGradientInfo_ELEMENTS; ++iS) {
	//	ColorGrid_InterpPixel(intrp_amtF,
	//		lpixP,
	//		rpixP,
	//		headP);
	//	lpixP++;
	//	rpixP++;
	//	headP++;
	//}
	suites.HandleSuite1()->host_unlock_handle(*intrp_arbP);
	suites.HandleSuite1()->host_unlock_handle(*l_arbPH);
	suites.HandleSuite1()->host_unlock_handle(*r_arbPH);
	*/
	return err;
}


static PF_Err
Arb_Print_Size()
{
	// This size is actually provided directly in ColorGrid.cpp,
	// in response to PF_Arbitrary_PRINT_FUNC
	PF_Err err = PF_Err_NONE;
	return err;
}



PF_Err
ColorGradientUIHandleArbitrary(
	PF_InData* in_data,
	PF_OutData* out_data,
	PF_ParamDef* params[],
	PF_LayerDef* output,
	PF_ArbParamsExtra* extra
) {

	PF_Err 	err = PF_Err_NONE;
	ColorGradientInfo* srcP = NULL;
	ColorGradientInfo* dstP = NULL;

	switch (extra->which_function) {

		case PF_Arbitrary_NEW_FUNC:
			//PLOGI <<"PF_Arbitrary_NEW_FUNC");
			//if (extra->u.new_func_params.refconPV != ARB_REFCON) {
			//	err = PF_Err_INTERNAL_STRUCT_DAMAGED;
			//}
			//else {
			err = CreateDefaultArb(in_data,
				out_data,
				extra->u.new_func_params.arbPH);
			//}
			break;

		case PF_Arbitrary_DISPOSE_FUNC:
			//PLOGI <<"PF_Arbitrary_DISPOSE_FUNC");
			//if (extra->u.dispose_func_params.refconPV != ARB_REFCON) {
			//	err = PF_Err_INTERNAL_STRUCT_DAMAGED;
			//}
			//else {
				PF_DISPOSE_HANDLE(extra->u.dispose_func_params.arbH);
			//}
			break;

		case PF_Arbitrary_COPY_FUNC:

			//PLOGI <<"PF_Arbitrary_COPY_FUNC");
			//if (extra->u.copy_func_params.refconPV == ARB_REFCON) {
				ERR(CreateDefaultArb(in_data,
					out_data,
					extra->u.copy_func_params.dst_arbPH));

				ERR(Arb_Copy(in_data,
					out_data,
					&extra->u.copy_func_params.src_arbH,
					extra->u.copy_func_params.dst_arbPH));
			//}
			break;
		case PF_Arbitrary_FLAT_SIZE_FUNC:

			//PLOGI <<"PF_Arbitrary_FLAT_SIZE_FUNC");
			*(extra->u.flat_size_func_params.flat_data_sizePLu) = sizeof(ColorGradientInfo);
			break;

		case PF_Arbitrary_FLATTEN_FUNC:
			//PLOGI <<"PF_Arbitrary_FLATTEN_FUNC");
			if (extra->u.flatten_func_params.buf_sizeLu == sizeof(ColorGradientInfo)) {
				srcP = (ColorGradientInfo*)PF_LOCK_HANDLE(extra->u.flatten_func_params.arbH);
				dstP = (ColorGradientInfo*)extra->u.flatten_func_params.flat_dataPV;
				//srcP->id++;
				//PLOGI <<std::to_string(srcP->id));
				if (srcP) {
					memcpy(dstP, srcP, sizeof(ColorGradientInfo));
				}
				PF_UNLOCK_HANDLE(extra->u.flatten_func_params.arbH);
			}
			break;

		case PF_Arbitrary_UNFLATTEN_FUNC:
			//PLOGI <<" PF_Arbitrary_UNFLATTEN_FUNC");
			if (extra->u.unflatten_func_params.buf_sizeLu == sizeof(ColorGradientInfo)) {
				PF_Handle	handle = PF_NEW_HANDLE(sizeof(ColorGradientInfo));
				dstP = (ColorGradientInfo*)PF_LOCK_HANDLE(handle);
				srcP = (ColorGradientInfo*)extra->u.unflatten_func_params.flat_dataPV;
				if (srcP) {
					memcpy(dstP, srcP, sizeof(ColorGradientInfo));
				}
				*(extra->u.unflatten_func_params.arbPH) = handle;
				*(extra->u.unflatten_func_params.arbPH) = handle;
				PF_UNLOCK_HANDLE(handle);
			}
			break;

		case PF_Arbitrary_INTERP_FUNC:
			//if (extra->u.interp_func_params.refconPV == ARB_REFCON) {
				ERR(CreateDefaultArb(in_data,
					out_data,
					extra->u.interp_func_params.interpPH));
			
				ERR(Arb_Interpolate(in_data,
					out_data,
					extra->u.interp_func_params.tF,
					&extra->u.interp_func_params.left_arbH,
					&extra->u.interp_func_params.right_arbH,
					extra->u.interp_func_params.interpPH));
			//}
			break;

		case PF_Arbitrary_COMPARE_FUNC:
			//ERR(Arb_Compare(in_data,
			//	out_data,
			//	&extra->u.compare_func_params.a_arbH,
			//	&extra->u.compare_func_params.b_arbH,
			//	extra->u.compare_func_params.compareP));
			break;

	}
	return err;
}


/// <summary>
/// setup color gradient UI
/// </summary>
/// <param name="in_data"></param>
/// <returns></returns>
/// 
PF_Err
SetupColorGradientUI(
	PF_InData* in_data,   /* in */
	uint32_t diskID,     /* in */
	PF_OutData* out_data /* out */
) {
	PF_Err err = PF_Err_NONE;

	PF_ParamDef	def;

	AEFX_CLR_STRUCT(def);

	ERR(CreateDefaultArb(in_data,
		out_data,
		&def.u.arb_d.dephault));

	PF_ADD_ARBITRARY2(
		STR(STRID_RENDER_COLOR_GRADIENT),
		UI_COLOR_GRADIENT_TOPIC_WIDTH,
		UI_COLOR_GRADIENT_TOPIC_HEIGHT,
		PF_ParamFlag_SUPERVISE,
		PF_PUI_CONTROL | PF_PUI_DONT_ERASE_CONTROL,
		def.u.arb_d.dephault,
		diskID, 
		0
		//ARB_REFCON
	);

	if (!err) {

		PF_CustomUIInfo            ci;

		AEFX_CLR_STRUCT(ci);

		ci.events = PF_CustomEFlag_EFFECT | PF_CustomEFlag_LAYER;

		err = (*(in_data->inter.register_ui))(in_data->effect_ref, &ci);
	}

	return err;
}


/// <summary>
/// sort cusor according lerp factor
/// </summary>
/// <param name="arbP"></param>
/// <param name="sortedEnableCursor"></param>
static void
GetSortedEnableCursor(
	ColorGradientInfo * arbP , /*  in  */
	std::vector<ColorGradientCursor>& sortedEnableCursor/*  out */
) 
{
	if (arbP == NULL) {
		return;
	}

	std::vector<ColorGradientCursor> sortedCursor(MAX_COLOR_GRADIENT_COUNT);
	memcpy(sortedCursor.data(), &arbP->colorGradientCursorInfo[0], sizeof(ColorGradientCursor) * MAX_COLOR_GRADIENT_COUNT);
	std::sort(sortedCursor.begin(), sortedCursor.end(),
		[](const ColorGradientCursor& a, const ColorGradientCursor& b) {
			return a.lerpFactor < b.lerpFactor;
		});;
	for (int i = 0; i < sortedCursor.size(); i++) {
		if (sortedCursor[i].enable) {
			sortedEnableCursor.push_back(sortedCursor[i]);
		}
	}

}


/// <summary>
/// return cursor index , if click pos is on one of enable cursors
/// return -1 , if click pos is out of cursor range
/// return -2 , if click pos is in cursor range but here is empty
/// </summary>
/// <param name="event_extra"></param>
/// <param name=""></param>
/// <param name=""></param>
/// <returns></returns>
static int GetClickCursorIndex(
	PF_EventExtra* event_extra ,     /* in */
	std::vector<ColorGradientCursor>& sortedEnableCursor/* in */
) {
	float clickPosX = event_extra->u.do_click.screen_point.x;
	float clickPosY = event_extra->u.do_click.screen_point.y;
	float cursorIconHalfWidth = UI_CURSOR_ICON_WIDTH / 2;
	if (clickPosY < event_extra->effect_win.param_title_frame.top + UI_COLOR_GRADIENT_IMG_HEIGHT ||
		clickPosY > event_extra->effect_win.param_title_frame.top + UI_COLOR_GRADIENT_IMG_HEIGHT + UI_CURSOR_ICON_HEIGHT ||
		clickPosX < event_extra->effect_win.param_title_frame.left + UI_COLOR_GRADIENT_TOPIC_OFFSET - cursorIconHalfWidth ||
		clickPosX > event_extra->effect_win.param_title_frame.right - UI_COLOR_GRADIENT_TOPIC_OFFSET + cursorIconHalfWidth
		) {
		return -1;
	}

	int clickIndex = -2;
	for (int i = sortedEnableCursor.size()-1;  i >=0 ; i--) {
		float cursorIconCenter = std::lerp(event_extra->effect_win.param_title_frame.left + UI_COLOR_GRADIENT_TOPIC_OFFSET,
										event_extra->effect_win.param_title_frame.right - UI_COLOR_GRADIENT_TOPIC_OFFSET,
										sortedEnableCursor[i].lerpFactor);
		float cursorIconLeft = cursorIconCenter - cursorIconHalfWidth;
		float cursorIconRight = cursorIconCenter + cursorIconHalfWidth;
		if (clickPosX > cursorIconLeft && clickPosX < cursorIconRight) {
			clickIndex = i;
			break;
		}

	}
	return clickIndex;
}


/// <summary>
/// render cursor icon
/// </summary>
/// <param name="colorGradientCursor"></param>
/// <param name="drawbotSuites"></param>
/// <param name="surfaceRef"></param>
/// <param name="supplierRef"></param>
/// <param name="event_extra"></param>
/// <returns></returns>
static PF_Err
DrawCursorIcon(
	ColorGradientCursor & colorGradientCursor, /* in */
	DRAWBOT_Suites  drawbotSuites,		/* in */
	DRAWBOT_SurfaceRef&	surfaceRef,		/* in */
	DRAWBOT_SupplierRef & supplierRef , /* in */
	PF_EventExtra* event_extra 		/* in */
) 
{
	PF_Err err = PF_Err_NONE;

	if (colorGradientCursor.enable == false) {
		return err;
	}

	//PLOGI <<"DrawCursorIcon ");

	std::vector<uint8_t> tempImageData(UI_CURSOR_ICON_WIDTH * UI_CURSOR_ICON_HEIGHT * 4 , 255);
	auto color = colorGradientCursor.color;

	float gammaFactor = 1.0 / 2.2;
	color.red = pow(color.red, gammaFactor);
	color.green = pow(color.green, gammaFactor);
	color.blue = pow(color.blue, gammaFactor);

	for (int x = UI_CURSOR_ICON_MARGIN ; x < UI_CURSOR_ICON_WIDTH - UI_CURSOR_ICON_MARGIN; ++x) {
		for (int y = UI_CURSOR_ICON_MARGIN  ; y < UI_CURSOR_ICON_HEIGHT - UI_CURSOR_ICON_MARGIN ; ++y) {
			int idx = (y * UI_CURSOR_ICON_WIDTH + x) * 4;
			 #if defined(__APPLE__)
				tempImageData[idx + 0] = static_cast<uint8_t>( 255);
				tempImageData[idx + 1] = static_cast<uint8_t>( color.red   * 255); 
				tempImageData[idx + 2] = static_cast<uint8_t>( color.green * 255); 
				tempImageData[idx + 3] = static_cast<uint8_t>( color.blue  * 255);
            #endif

            #if defined(_WIN32)
				tempImageData[idx + 0] = static_cast<uint8_t>( color.red   * 255);
				tempImageData[idx + 1] = static_cast<uint8_t>( color.green * 255); 
				tempImageData[idx + 2] = static_cast<uint8_t>( color.blue  * 255); 
				tempImageData[idx + 3] = static_cast<uint8_t>( 255);
            #endif
		}
	}
	DRAWBOT_ImageRef imageRef = NULL;
	ERR(drawbotSuites.supplier_suiteP->NewImageFromBuffer(
		supplierRef,
		UI_CURSOR_ICON_WIDTH,
		UI_CURSOR_ICON_HEIGHT,
		UI_CURSOR_ICON_WIDTH * 4,
		kDRAWBOT_PixelLayout_32ARGB_Straight,
		(void*)tempImageData.data(),
		&imageRef
	));

	DRAWBOT_PointF32 pointRef = DRAWBOT_PointF32();
	pointRef.x = std::lerp(event_extra->effect_win.param_title_frame.left + UI_COLOR_GRADIENT_TOPIC_OFFSET,
						   event_extra->effect_win.param_title_frame.right - UI_COLOR_GRADIENT_TOPIC_OFFSET,
						   colorGradientCursor.lerpFactor) - (UI_CURSOR_ICON_WIDTH/2);
	pointRef.y = event_extra->effect_win.param_title_frame.top + UI_COLOR_GRADIENT_IMG_HEIGHT;

	ERR(drawbotSuites.surface_suiteP->DrawImage(surfaceRef, imageRef, &pointRef, 1.0f));
	ERR(drawbotSuites.supplier_suiteP->ReleaseObject(reinterpret_cast<DRAWBOT_ObjectRef>(imageRef)));

	return err;
}


/// <summary>
/// given 0~1 t , return lerp color in sortedEnableCursor
/// </summary>
/// <param name="sortedenableCursor"></param>
/// <param name="t"></param>
/// <param name="outColor"></param>
/// <returns></returns>
static PF_Err
GetColorFromColorGradient(
	const std::vector<ColorGradientCursor>& sortedenableCursor, /* in */
	float t , /* in */  // [0,1]
	DRAWBOT_ColorRGBA& outColor /* out */
) {

	PF_Err err = PF_Err_NONE;

	if (t <= sortedenableCursor.front().lerpFactor) {
		memcpy(&outColor, &sortedenableCursor.front().color, sizeof(outColor));
		return err;
	}
	if (t >= sortedenableCursor.back().lerpFactor ) {
		memcpy(&outColor, &sortedenableCursor.back().color, sizeof(outColor));
		return err;
	}

	int i = 0;
	for (; i < sortedenableCursor.size() -1 ; i++) {
		if (t >= sortedenableCursor[i].lerpFactor && 
			t <= sortedenableCursor[i+1].lerpFactor) {
			break;
		}
	}

	float t0 = sortedenableCursor[i].lerpFactor;
	float t1 = sortedenableCursor[i + 1].lerpFactor;
	float ratio = (t1 - t0 > 1e-6f) ? (t - t0) / (t1 - t0) : 0.0f;
	ratio = std::clamp(ratio, 0.0f, 1.0f); 

	outColor.alpha =  1;
	outColor.red   = (1-ratio) * sortedenableCursor[i].color.red   + ratio* sortedenableCursor[i + 1].color.red  ;
	outColor.green = (1-ratio) * sortedenableCursor[i].color.green + ratio* sortedenableCursor[i + 1].color.green;
	outColor.blue  = (1-ratio) * sortedenableCursor[i].color.blue  + ratio* sortedenableCursor[i + 1].color.blue ;

	return err;
}

static PF_Err
DrawEvent(	
	PF_InData		*in_data,
	PF_OutData		*out_data,
	PF_ParamDef		*params[],
	PF_LayerDef		*output,
	PF_EventExtra	*event_extra)
{
	PF_Err					err		=	PF_Err_NONE, err2 = PF_Err_NONE;

	DRAWBOT_DrawRef			drawing_ref = NULL;
	DRAWBOT_SurfaceRef		surface_ref = NULL;
	DRAWBOT_SupplierRef		supplier_ref = NULL;
	DRAWBOT_BrushRef		brush_ref = NULL;
	DRAWBOT_BrushRef		string_brush_ref = NULL;
	DRAWBOT_PathRef			path_ref = NULL;
	DRAWBOT_FontRef			font_ref = NULL;
	DRAWBOT_ImageRef		image_ref = NULL;

	DRAWBOT_Suites			drawbotSuites;
	DRAWBOT_ColorRGBA		drawbot_color;
	DRAWBOT_RectF32			rectR;
	float					default_font_sizeF = 0.0;


	ColorGradientInfo* arbP = NULL;
	PF_ArbitraryH arbH = NULL;
	GetArbData(in_data, params, event_extra->effect_win.index, &arbP, &arbH);
	

	//PF_ParamDef* arb_param = params[event_extra->effect_win.index];
	//
	//if (arb_param->param_type == PF_Param_ARBITRARY_DATA) {
	//	arbH = arb_param->u.arb_d.value; 
	//	if (!arbH) {
	//		PLOGI <<"no arbH");
	//		return err;
	//	}
	//	arbP = reinterpret_cast<ColorGradientInfo*>(PF_LOCK_HANDLE(arbH));
	//}

	// Acquire all the Drawbot suites in one go; it should be matched with release routine.
	// You can also use C++ style AEFX_DrawbotSuitesScoper which doesn't need release routine.
	ERR(AEFX_AcquireDrawbotSuites(in_data, out_data, &drawbotSuites));
	
	PF_EffectCustomUISuite1	*effectCustomUISuiteP;

	ERR(AEFX_AcquireSuite(in_data,
							out_data,
							kPFEffectCustomUISuite,
							kPFEffectCustomUISuiteVersion1,
							NULL,
							(void**)&effectCustomUISuiteP));

	if (!err && effectCustomUISuiteP) {
		// Get the drawing reference by passing context to this new api
		ERR((*effectCustomUISuiteP->PF_GetDrawingReference)(event_extra->contextH, &drawing_ref));

		AEFX_ReleaseSuite(in_data, out_data, kPFEffectCustomUISuite, kPFEffectCustomUISuiteVersion1, NULL);
	}

	// Get the Drawbot supplier from drawing reference; it shouldn't be released like pen or brush (see below)
	ERR(drawbotSuites.drawbot_suiteP->GetSupplier(drawing_ref, &supplier_ref));
	
	// Get the Drawbot surface from drawing reference; it shouldn't be released like pen or brush (see below)
	ERR(drawbotSuites.drawbot_suiteP->GetSurface(drawing_ref, &surface_ref));

	
	if (event_extra->effect_win.area == PF_EA_CONTROL ) {

		std::ostringstream oss;
		
		std::vector<ColorGradientCursor> sortedEnableCursor = {};
		GetSortedEnableCursor(arbP, sortedEnableCursor);
	
		ERR(drawbotSuites.supplier_suiteP->NewPath(supplier_ref, &path_ref));
		
		// =========== draw color gradient UI ===========
		int kHeight = UI_COLOR_GRADIENT_IMG_HEIGHT;
		int kWidth = event_extra->effect_win.param_title_frame.right - event_extra->effect_win.param_title_frame.left - (2 * UI_COLOR_GRADIENT_TOPIC_OFFSET);
		if (kWidth <0) {
			return err;
		}
		std::vector<uint8_t> tempImageData(kHeight * kWidth * 4);
		DRAWBOT_ColorRGBA color;

		for (int x = 0; x < kWidth; ++x) {
			float t = float(x) / float(kWidth);
			GetColorFromColorGradient(sortedEnableCursor, t, color);
			// linear 2 sRGB
			float gammaFactor = 1.0 / 2.2;
			color.red = pow(color.red, gammaFactor);
			color.green = pow(color.green, gammaFactor);
			color.blue = pow(color.blue, gammaFactor);
			for (int y = 0; y < kHeight; ++y) {

				int idx = (y * kWidth + x) * 4;	
            #if defined(__APPLE__)

				tempImageData[idx + 0] = static_cast<uint8_t>( 255);
				tempImageData[idx + 1] = static_cast<uint8_t>( color.red   * 255); 
				tempImageData[idx + 2] = static_cast<uint8_t>( color.green * 255); 
				tempImageData[idx + 3] = static_cast<uint8_t>( color.blue  * 255);
            #endif

            #if defined(_WIN32)
				tempImageData[idx + 0] = static_cast<uint8_t>( color.red   * 255);
				tempImageData[idx + 1] = static_cast<uint8_t>( color.green * 255); 
				tempImageData[idx + 2] = static_cast<uint8_t>( color.blue  * 255); 
				tempImageData[idx + 3] = static_cast<uint8_t>( 255);
            #endif
			}
		}

		ERR(drawbotSuites.supplier_suiteP->NewImageFromBuffer(
			supplier_ref,
			kWidth,
			kHeight,
			kWidth * 4,
			kDRAWBOT_PixelLayout_32ARGB_Straight,
			(void*)tempImageData.data(),
			&image_ref
		));

		DRAWBOT_PointF32 point_ref = DRAWBOT_PointF32();
		point_ref.x = event_extra->effect_win.param_title_frame.left + UI_COLOR_GRADIENT_TOPIC_OFFSET;
		point_ref.y = event_extra->effect_win.param_title_frame.top;
		ERR(drawbotSuites.surface_suiteP->DrawImage(surface_ref, image_ref , &point_ref, 1.0f)); // alpha=1.0
		// =========== draw color gradient UI ===========


		// =========== draw color curosr icon ===========
		for (int i = 0;i < sortedEnableCursor.size(); i++) {
			ERR(DrawCursorIcon(sortedEnableCursor[i],
				drawbotSuites,
				surface_ref,
				supplier_ref,
				event_extra));
		}
		// =========== draw color curosr icon ===========

		// === release ===
		ReleaseDrawbotObject(drawbotSuites.supplier_suiteP, reinterpret_cast<DRAWBOT_ObjectRef>(string_brush_ref));
		ReleaseDrawbotObject(drawbotSuites.supplier_suiteP, reinterpret_cast<DRAWBOT_ObjectRef>(font_ref));
		ReleaseDrawbotObject(drawbotSuites.supplier_suiteP, reinterpret_cast<DRAWBOT_ObjectRef>(brush_ref));
		ReleaseDrawbotObject(drawbotSuites.supplier_suiteP, reinterpret_cast<DRAWBOT_ObjectRef>(path_ref));
		ReleaseDrawbotObject(drawbotSuites.supplier_suiteP, reinterpret_cast<DRAWBOT_ObjectRef>(image_ref));
	}
	

	// Release the earlier acquired Drawbot suites
	ERR2(AEFX_ReleaseDrawbotSuites(in_data, out_data));
  
	if (!err){
		event_extra->evt_out_flags = PF_EO_HANDLED_EVENT;
	}

	PF_UNLOCK_HANDLE(arbH);

	return err;
}

static PF_Err 
DoDrag(	
	PF_InData		*in_data,
	PF_OutData		*out_data,
	PF_ParamDef		*params[],
	PF_LayerDef		*output,
	PF_EventExtra	*event_extra)
{
	PF_Err 			err			= PF_Err_NONE;
	PF_ContextH		contextH	= event_extra->contextH;
	PF_Point		mouse_down;
	
	if (PF_Window_EFFECT == (*contextH)->w_type){
		if (PF_EA_CONTROL == event_extra->effect_win.area) {
			mouse_down = event_extra->u.do_click.screen_point;
			PLOGI <<"mouse_down.x " + std::to_string(mouse_down.x);

			float frameLeft = event_extra->effect_win.param_title_frame.left + UI_COLOR_GRADIENT_TOPIC_OFFSET;
			float frameRight = event_extra->effect_win.param_title_frame.right - UI_COLOR_GRADIENT_TOPIC_OFFSET;
			float lerpFactor = (mouse_down.x - frameLeft) / (frameRight - frameLeft);
			lerpFactor = std::clamp(lerpFactor, 0.0f, 1.0f);
			PLOGI <<"lerpFactor " + std::to_string(lerpFactor);

			PF_ArbitraryH arbH = NULL;
			ColorGradientInfo* arbP = NULL;
			GetArbData(in_data, params, event_extra->effect_win.index, &arbP, &arbH);

			//std::vector<ColorGradientCursor> sortedEnableCursor;
			//GetSortedEnableCursor(arbP, sortedEnableCursor);
			// cache in DoClick
			int clickIndex = event_extra->u.do_click.continue_refcon[0];

			// rewrite
			//std::sort(sortedEnableCursor.begin(), sortedEnableCursor.end(),
			//	[](const ColorGradientCursor& a, const ColorGradientCursor& b) {
			//		return a.lerpFactor < b.lerpFactor;
			//	});
			//
			//for (int i = 0; i < MAX_COLOR_GRADIENT_COUNT; i++) {
			//	arbP->colorGradientCursorInfo[i].enable = false;
			//}
			//
			//
			//memcpy(&arbP->colorGradientCursorInfo[0], sortedEnableCursor.data(), sortedEnableCursor.size() * sizeof(ColorGradientCursor));
			//arbP->currentCursorCount = sortedEnableCursor.size();
			arbP->colorGradientCursorInfo[clickIndex].lerpFactor = lerpFactor;


			event_extra->evt_out_flags |= PF_EO_HANDLED_EVENT | PF_EO_UPDATE_NOW;
			params[event_extra->effect_win.index]->uu.change_flags |= PF_ChangeFlag_CHANGED_VALUE;
			PF_UNLOCK_HANDLE(arbH);
		}
	}
	return err;
}

static PF_Err 
DoClick(
	PF_InData		*in_data,
	PF_OutData		*out_data,
	PF_ParamDef		*params[],
	PF_LayerDef		*output,
	PF_EventExtra	*event_extra)
{
	PF_Err	err		=	PF_Err_NONE;

	AEGP_SuiteHandler		suites(in_data->pica_basicP);
	PF_ExtendedSuiteTool	tool = PF_ExtendedSuiteTool_MAGNIFY;
	
	auto screenPoint = event_extra->u.do_click.screen_point;

	float frameLeft = event_extra->effect_win.param_title_frame.left + UI_COLOR_GRADIENT_TOPIC_OFFSET;
	float frameRight = event_extra->effect_win.param_title_frame.right - UI_COLOR_GRADIENT_TOPIC_OFFSET;
	float lerpFactor = (screenPoint.x - frameLeft) / (frameRight - frameLeft);

	PF_ArbitraryH arbH = NULL;
	ColorGradientInfo* arbP = NULL;
	GetArbData(in_data, params, event_extra->effect_win.index, &arbP, &arbH);
	PLOGI <<"lerp Factor " << lerpFactor;

	std::vector<ColorGradientCursor> sortedEnableCursor;
	GetSortedEnableCursor(arbP ,sortedEnableCursor);
	int clickIndex = GetClickCursorIndex(event_extra, sortedEnableCursor);
	if (clickIndex == -1) {
		PLOGI <<"click out of range";
		return err;
	}
	else {
		PLOGI <<"click in range";
	}

	if (//event_extra->u.do_click.modifiers == PF_Mod_NONE &&
		event_extra->u.do_click.num_clicks == 2 ) {

		DRAWBOT_ColorRGBA color;
		GetColorFromColorGradient(sortedEnableCursor, lerpFactor, color);

		//add cursor
		if (clickIndex == -2) {
			if (sortedEnableCursor.size() < MAX_COLOR_GRADIENT_COUNT - 1) {

				ColorGradientCursor newCursor;
				newCursor.enable = true;
				newCursor.lerpFactor = lerpFactor;
				newCursor.color.red = color.red;
				newCursor.color.green = color.green;
				newCursor.color.blue = color.blue;
				sortedEnableCursor.push_back(newCursor);
			}
		}
		else{

			// edit color from cusor 
			PF_PixelFloat box_colorP = PF_PixelFloat();
			box_colorP.red   = color.red;
			box_colorP.green = color.green;
			box_colorP.blue  = color.blue ;
			ERR(suites.AppSuite4()->PF_AppColorPickerDialog(
				"Select Color",
				&box_colorP,
				TRUE,
				&box_colorP));
			sortedEnableCursor[clickIndex].color.red = box_colorP.red;
			sortedEnableCursor[clickIndex].color.green = box_colorP.green;
			sortedEnableCursor[clickIndex].color.blue = box_colorP.blue;
		}
	}
	if (event_extra->u.do_click.modifiers & PF_Mod_CMD_CTRL_KEY &&
		event_extra->u.do_click.num_clicks == 1 && clickIndex != -2) {
		//remove cursor
		if (sortedEnableCursor.size() > 2) {
			sortedEnableCursor[clickIndex].enable = false;
		}
		
	}
	if (event_extra->u.do_click.num_clicks == 1 && clickIndex != -2) {
		event_extra->u.do_click.continue_refcon[0] = clickIndex;
		event_extra->u.do_click.send_drag = TRUE;
		event_extra->evt_out_flags = PF_EO_HANDLED_EVENT;
	}
	
	//rewrite
	std::sort(sortedEnableCursor.begin(), sortedEnableCursor.end(),
		[](const ColorGradientCursor& a, const ColorGradientCursor& b) {
			return a.lerpFactor < b.lerpFactor;
	});

	for (int i = 0; i < MAX_COLOR_GRADIENT_COUNT ; i++ ) {
		arbP->colorGradientCursorInfo[i].enable = false;
	}


	memcpy(&arbP->colorGradientCursorInfo[0], sortedEnableCursor.data(), sortedEnableCursor.size() * sizeof(ColorGradientCursor));
	arbP->currentCursorCount = sortedEnableCursor.size();

	event_extra->evt_out_flags |= PF_EO_HANDLED_EVENT | PF_EO_UPDATE_NOW;
	params[event_extra->effect_win.index]->uu.change_flags |= PF_ChangeFlag_CHANGED_VALUE;
	PF_UNLOCK_HANDLE(arbH);

	return err;
}

static PF_Err 
ChangeCursor(	
	PF_InData		*in_data,
	PF_OutData		*out_data,
	PF_ParamDef		*params[],
	PF_LayerDef		*output,
	PF_EventExtra	*event_extra)
{
	//if (PF_Mod_SHIFT_KEY & event_extra->u.adjust_cursor.modifiers)	{
	//	event_extra->u.adjust_cursor.set_cursor = PF_Cursor_EYEDROPPER;
	//} else {
	//	if (PF_Mod_CMD_CTRL_KEY & event_extra->u.adjust_cursor.modifiers) {
	//		event_extra->u.adjust_cursor.set_cursor = PF_Cursor_CROSSHAIRS;
	//	}
	//}
	return PF_Err_NONE;
}

PF_Err 
ColorGradientUIHandleEvent(
				PF_InData		*in_data,
				PF_OutData		*out_data,
				PF_ParamDef		*params[],
				PF_LayerDef		*output,
				PF_EventExtra	*extra)
{
	PF_Err		err		= PF_Err_NONE;
	//PLOGI <<std::to_string(extra->e_type));

	switch (extra->e_type) {
		case PF_Event_DO_CLICK: {
			//PLOGI <<"PF_Event_DO_CLICK ");
			err = DoClick(in_data, out_data, params, output, extra);
			break;
		}
		case PF_Event_DRAG: {
			//PLOGI <<"PF_Event_DRAG ");
			err = DoDrag(in_data, out_data, params, output, extra);
			break;
		}
		case PF_Event_DRAW: {
			//PLOGI <<"PF_Event_DRAW ");

			err = DrawEvent(in_data, out_data, params, output, extra);
			break;
		}
		case PF_Event_ADJUST_CURSOR: {
			//PLOGI <<"PF_Event_ADJUST_CURSOR ");
			//err = ChangeCursor(in_data, out_data, params, output, extra);
			//break;
		}
		default:
			break;
	}
	return err;
}


