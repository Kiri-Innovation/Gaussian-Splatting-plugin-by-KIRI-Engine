#include "UI/BezierCurveUI.h"
#include "UI/CmdArbitraryCallBackHandler.h"
#include "UI/CmdEventHandler.h"
#include "AEFX_SuiteHelper.h"
#include <cmath>
#include "Common/GlobalLogger.h"

#include "Vector"
#include "algorithm"
#include "Plugin/3DGS_Strings.h"
#include "Plugin/3DGS_PF.h"

#include "AE_Macros.h"
#include "UI/UIDataType.h"

static DRAWBOT_ImageRef bezierPointIconRef = NULL;

static PF_Err
CreateDefaultArb(
	PF_InData* in_data,
	PF_OutData* out_data,
	PF_ArbitraryH* dephault)
{
	PF_Err			err = PF_Err_NONE;
	PF_Handle		arbH = NULL;

	AEGP_SuiteHandler	suites(in_data->pica_basicP);

	arbH = suites.HandleSuite1()->host_new_handle(sizeof(BezierCurveInfo));

	if (arbH) {
		BezierCurveInfo* arbP = reinterpret_cast<BezierCurveInfo*>(PF_LOCK_HANDLE(arbH));
		if (!arbP) {
			err = PF_Err_OUT_OF_MEMORY;
		}
		else {
			AEFX_CLR_STRUCT(*arbP);
#ifdef AE_OS_WIN
#pragma warning (disable : 4305)
#endif
            //PLOGI <<"CreateDefaultArb BezierCurveUI");
			arbP->currentPointCount = 2;

			arbP->bezierPointInfo[0].enable = true;
			arbP->bezierPointInfo[0].point.x = 0.0;
			arbP->bezierPointInfo[0].point.y = 0.0;

			arbP->bezierPointInfo[1].enable = true;
			arbP->bezierPointInfo[1].point.x = 1.0;
			arbP->bezierPointInfo[1].point.y = 1.0;

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
		BezierCurveInfo* src_arbP = reinterpret_cast<BezierCurveInfo*>(suites.HandleSuite1()->host_lock_handle(sourceH));
		if (!src_arbP) {
			err = PF_Err_OUT_OF_MEMORY;
		}
		else {
			PF_Handle	destH = *dstP;
			if (destH) {
				BezierCurveInfo* dst_arbP = reinterpret_cast<BezierCurveInfo*>(suites.HandleSuite1()->host_lock_handle(destH));
				if (!dst_arbP) {
					err = PF_Err_OUT_OF_MEMORY;
				}
				else {
					memcpy(dst_arbP, src_arbP, sizeof(BezierCurveInfo));
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

	return err;
}


PF_Err
BezierCurveUIHandleArbitrary(
	PF_InData* in_data,
	PF_OutData* out_data,
	PF_ParamDef* params[],
	PF_LayerDef* output,
	PF_ArbParamsExtra* extra
) {

	PF_Err 	err = PF_Err_NONE;
	
	BezierCurveInfo* srcP = NULL;
	BezierCurveInfo* dstP = NULL;

	switch (extra->which_function) {

	case PF_Arbitrary_NEW_FUNC:

		err = CreateDefaultArb(in_data,
			out_data,
			extra->u.new_func_params.arbPH);
		break;

	case PF_Arbitrary_DISPOSE_FUNC:
		//PLOGI <<"PF_Arbitrary_DISPOSE_FUNC");
		PF_DISPOSE_HANDLE(extra->u.dispose_func_params.arbH);
		
		break;

	case PF_Arbitrary_COPY_FUNC:
		ERR(CreateDefaultArb(in_data,
			out_data,
			extra->u.copy_func_params.dst_arbPH));

		ERR(Arb_Copy(in_data,
			out_data,
			&extra->u.copy_func_params.src_arbH,
			extra->u.copy_func_params.dst_arbPH));

		break;
	case PF_Arbitrary_FLAT_SIZE_FUNC:

		//PLOGI <<"PF_Arbitrary_FLAT_SIZE_FUNC");
		*(extra->u.flat_size_func_params.flat_data_sizePLu) = sizeof(BezierCurveInfo);
		break;

	case PF_Arbitrary_FLATTEN_FUNC:
		if (extra->u.flatten_func_params.buf_sizeLu == sizeof(BezierCurveInfo)) {
			srcP = (BezierCurveInfo*)PF_LOCK_HANDLE(extra->u.flatten_func_params.arbH);
			dstP = (BezierCurveInfo*)extra->u.flatten_func_params.flat_dataPV;
			//srcP->id++;
			//PLOGI <<std::to_string(srcP->id));
			if (srcP) {
				memcpy(dstP, srcP, sizeof(BezierCurveInfo));
			}
			PF_UNLOCK_HANDLE(extra->u.flatten_func_params.arbH);
		}
		break;

	case PF_Arbitrary_UNFLATTEN_FUNC:
		if (extra->u.unflatten_func_params.buf_sizeLu == sizeof(BezierCurveInfo)) {
			PF_Handle	handle = PF_NEW_HANDLE(sizeof(BezierCurveInfo));
			dstP = (BezierCurveInfo*)PF_LOCK_HANDLE(handle);
			srcP = (BezierCurveInfo*)extra->u.unflatten_func_params.flat_dataPV;
			if (srcP) {
				memcpy(dstP, srcP, sizeof(BezierCurveInfo));
			}
			*(extra->u.unflatten_func_params.arbPH) = handle;
			*(extra->u.unflatten_func_params.arbPH) = handle;
			PF_UNLOCK_HANDLE(handle);
		}
		break;

	case PF_Arbitrary_INTERP_FUNC:
		ERR(CreateDefaultArb(in_data,
			out_data,
			extra->u.interp_func_params.interpPH));

		ERR(Arb_Interpolate(in_data,
			out_data,
			extra->u.interp_func_params.tF,
			&extra->u.interp_func_params.left_arbH,
			&extra->u.interp_func_params.right_arbH,
			extra->u.interp_func_params.interpPH));
		break;

	case PF_Arbitrary_COMPARE_FUNC:
		break;

	}
	
	return err;
}

static PF_Err
DrawCursorIcon(
	DRAWBOT_PointF32& bezierPoint,      /* in */
	DRAWBOT_Suites  drawbotSuites,		/* in */
	DRAWBOT_SurfaceRef& surfaceRef,		/* in */
	DRAWBOT_SupplierRef& supplierRef,   /* in */
	PF_EventExtra* event_extra 		    /* in */
)
{
	PF_Err err = PF_Err_NONE;

	//PLOGI <<"DrawCursorIcon ");

	if (bezierPointIconRef == NULL){

		static std::vector<uint8_t> bezierPointIconData(UI_BEZIER_ICON_WIDTH * UI_BEZIER_ICON_WIDTH * 4, 255);
		for (int x = UI_BEZIER_ICON_MARGIN; x < UI_BEZIER_ICON_WIDTH - UI_BEZIER_ICON_MARGIN; ++x) {
			for (int y = UI_BEZIER_ICON_MARGIN; y < UI_BEZIER_ICON_WIDTH - UI_BEZIER_ICON_MARGIN; ++y) {
				int idx = (y * UI_BEZIER_ICON_WIDTH + x) * 4;
#if defined(__APPLE__)
				bezierPointIconData[idx + 0] = static_cast<uint8_t>(0);
				bezierPointIconData[idx + 1] = static_cast<uint8_t>(0);
				bezierPointIconData[idx + 2] = static_cast<uint8_t>(0);
				bezierPointIconData[idx + 3] = static_cast<uint8_t>(0);
#endif

#if defined(_WIN32)
				bezierPointIconData[idx + 0] = static_cast<uint8_t>(0);
				bezierPointIconData[idx + 1] = static_cast<uint8_t>(0);
				bezierPointIconData[idx + 2] = static_cast<uint8_t>(0);
				bezierPointIconData[idx + 3] = static_cast<uint8_t>(0);
#endif
				ERR(drawbotSuites.supplier_suiteP->NewImageFromBuffer(
					supplierRef,
					UI_BEZIER_ICON_WIDTH,
					UI_BEZIER_ICON_HEIGHT,
					UI_BEZIER_ICON_WIDTH * 4,
					kDRAWBOT_PixelLayout_32ARGB_Straight,
					(void*)bezierPointIconData.data(),
					&bezierPointIconRef
				));
			}
		}
	}
	
	DRAWBOT_PointF32 pointRef = bezierPoint;
	pointRef.x -= UI_BEZIER_ICON_WIDTH / 2;
	pointRef.y -= UI_BEZIER_ICON_HEIGHT / 2;
	ERR(drawbotSuites.surface_suiteP->DrawImage(surfaceRef, bezierPointIconRef, &pointRef, 1.0f));
	//ERR(drawbotSuites.supplier_suiteP->ReleaseObject(reinterpret_cast<DRAWBOT_ObjectRef>(bezierPointIconRef)));

	return err;
}

static void 
GetSortedEnableBezierPoint(
	BezierCurveInfo* arbP, /*  in  */
	KIRIParamIdx paramIdx ,   /*  in  */
	std::vector<BezierPoint>& sortedEnableBezierPoint /*  out */
)
{
	if (arbP == NULL) {
		return;
	}

	std::vector<BezierPoint> sortedBezierPoint(MAX_BEZIER_POINT_COUNT);
	memcpy(sortedBezierPoint.data(), &arbP->bezierPointInfo[0], sizeof(BezierPoint) * MAX_BEZIER_POINT_COUNT);

	std::sort(sortedBezierPoint.begin(), sortedBezierPoint.end(),
		[](const BezierPoint& a, const BezierPoint& b) {
			return a.point.x < b.point.x;
		});

	for (int i = 0; i < sortedBezierPoint.size(); i++) {
		if (sortedBezierPoint[i].enable) {
			sortedEnableBezierPoint.push_back(sortedBezierPoint[i]);
		}
	}

}

/// <summary>
/// return -1 , if click empty region
/// return -2 , if click out of clickable region
/// return bezier point idx , if click bezier point icon
/// </summary>
/// <param name="event_extra"></param>
/// <param name="sortedEnableBezierPoint"></param>
/// <returns></returns>
static int GetClickBezierPointIndex(
	const PF_EventExtra* event_extra,     /* in */
	const std::vector<BezierPoint>& sortedEnableBezierPoint/* in */
) {


	float left = event_extra->effect_win.current_frame.left + UI_BEZIER_FRAME_WIDTH_MARGIN;
	float top = event_extra->effect_win.current_frame.top + UI_BEZIER_FRAME_HEIGHT_MARGIN;
	float right = event_extra->effect_win.current_frame.right - UI_BEZIER_FRAME_WIDTH_MARGIN;
	float bottom = event_extra->effect_win.current_frame.bottom - UI_BEZIER_FRAME_HEIGHT_MARGIN;

	float clickPosX = event_extra->u.do_click.screen_point.x;
	float clickPosY = event_extra->u.do_click.screen_point.y;

	float xRange = right - left;
	float yRange = (bottom - top);

	float iconHalfWidth = UI_BEZIER_ICON_WIDTH / 2;
	float iconHalfHeight = UI_BEZIER_ICON_HEIGHT / 2;

	if (!(clickPosX > left - iconHalfWidth && clickPosX < right + iconHalfWidth &&
		  clickPosY > top - iconHalfHeight && clickPosY < bottom + iconHalfHeight)) {
		return -2;
	}

	int clickIndex = -1;
	for (int i = 0; i < sortedEnableBezierPoint.size() ; i++) {

		float cursorIconCenterX = sortedEnableBezierPoint[i].point.x * xRange + left;
		float cursorIconCenterY = sortedEnableBezierPoint[i].point.y * yRange + top;
		if (std::abs(clickPosX - cursorIconCenterX) < iconHalfWidth &&
			std::abs(clickPosY - cursorIconCenterY) < iconHalfHeight)
		{
			clickIndex = i;
			break;
		}

	}

	return clickIndex;
}


static PF_Err
DrawEvent(
	PF_InData* in_data,
	PF_OutData* out_data,
	PF_ParamDef* params[],
	PF_LayerDef* output,
	PF_EventExtra* event_extra)
{
	PF_Err					err = PF_Err_NONE, err2 = PF_Err_NONE;

	DRAWBOT_DrawRef			drawing_ref = NULL;
	DRAWBOT_SurfaceRef		surface_ref = NULL;
	DRAWBOT_SupplierRef		supplier_ref = NULL;
	DRAWBOT_BrushRef		brush_ref = NULL;
	DRAWBOT_BrushRef		string_brush_ref = NULL;
	DRAWBOT_PathRef			path_ref = NULL;
	DRAWBOT_PenRef			pen_ref = NULL;
	DRAWBOT_FontRef			font_ref = NULL;
	DRAWBOT_ImageRef		image_ref = NULL;

	DRAWBOT_Suites			drawbotSuites;
	DRAWBOT_ColorRGBA		drawbot_color;
	DRAWBOT_RectF32			rectR;
	float					default_font_sizeF = 0.0;


	PF_ParamDef* arb_param = params[event_extra->effect_win.index];

	PF_ArbitraryH arbH = NULL;
	BezierCurveInfo* arbP = NULL;
	GetArbData(in_data, params, event_extra->effect_win.index, &arbP, &arbH);

	// Acquire all the Drawbot suites in one go; it should be matched with release routine.
	// You can also use C++ style AEFX_DrawbotSuitesScoper which doesn't need release routine.
	ERR(AEFX_AcquireDrawbotSuites(in_data, out_data, &drawbotSuites));

	PF_EffectCustomUISuite1* effectCustomUISuiteP;

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


	if (event_extra->effect_win.area == PF_EA_CONTROL) {

		std::ostringstream oss;
		
		DRAWBOT_ColorRGBA color = { 1.0f, 1.0f, 1.0, 1.0f };

		ERR(drawbotSuites.supplier_suiteP->NewPath(supplier_ref, &path_ref));
		ERR(drawbotSuites.supplier_suiteP->NewPen(supplier_ref, &color, 2.0f, &pen_ref));
		ERR(drawbotSuites.supplier_suiteP->NewBrush(supplier_ref, &color, &brush_ref));

		// === draw bezier path ===

		float left = event_extra->effect_win.current_frame.left + UI_BEZIER_FRAME_WIDTH_MARGIN;
		float top = event_extra->effect_win.current_frame.top + UI_BEZIER_FRAME_HEIGHT_MARGIN;
		float right = event_extra->effect_win.current_frame.right - UI_BEZIER_FRAME_WIDTH_MARGIN;
		float bottom = event_extra->effect_win.current_frame.bottom - UI_BEZIER_FRAME_HEIGHT_MARGIN;
		//0~1
		/*
		DRAWBOT_PointF32 controlPoint1 = { 0.2, 0.0};
		DRAWBOT_PointF32 controlPoint2 = { 0.8, 1.0};

        BezierPoint bezierPoint1;
        bezierPoint1.enable = true;
        bezierPoint1.point = controlPoint1;

        BezierPoint bezierPoint2;
        bezierPoint2.enable = true;
        bezierPoint2.point = controlPoint2;

        sortedEnableBezierPoint.push_back(bezierPoint1);
        sortedEnableBezierPoint.push_back(bezierPoint2);

		std::sort(sortedEnableBezierPoint.begin(), sortedEnableBezierPoint.end(),
			[](const BezierPoint& a, const BezierPoint& b) {
				return a.point.x < b.point.x;
			});
		*/

		std::vector<BezierPoint> sortedEnableBezierPoint = {};
		GetSortedEnableBezierPoint(arbP, (KIRIParamIdx)event_extra->effect_win.index , sortedEnableBezierPoint);

		//std::vector<DRAWBOT_PointF32> sortedBezierPoint = { controlPoint1 ,controlPoint2 };

		float xRange = right - left;
		float yRange = ( bottom - top) ;
		if (xRange < 0 || yRange < 0) {
			return err;
		}

		for ( int i = 0; i < sortedEnableBezierPoint.size() ;i++ ){
			sortedEnableBezierPoint[i].point.x = sortedEnableBezierPoint[i].point.x * xRange + left;
			sortedEnableBezierPoint[i].point.y = sortedEnableBezierPoint[i].point.y * yRange + top;
			// === draw bezier cursor ===
			DrawCursorIcon(sortedEnableBezierPoint[i].point, drawbotSuites, surface_ref, supplier_ref, event_extra);
		}

		DRAWBOT_PointF32 start = { left, sortedEnableBezierPoint[0].point.y };
		DRAWBOT_PointF32 end   = { right , sortedEnableBezierPoint[sortedEnableBezierPoint.size() - 1].point.y };
		BezierPoint endPoint;
		endPoint.point = end;
		sortedEnableBezierPoint.push_back(endPoint);
		DRAWBOT_PointF32 curveStart = start;
		DRAWBOT_PointF32 curveEnd = sortedEnableBezierPoint[0].point;
		DRAWBOT_PointF32 mid1 ;
		DRAWBOT_PointF32 mid2 ;
		for(int i = -1; i + 1 < sortedEnableBezierPoint.size(); i++ ){

			if (i == -1) {
				curveStart = start;
				curveEnd = sortedEnableBezierPoint[0].point;
			}
			else {
				curveStart = sortedEnableBezierPoint[i].point;
				curveEnd   = sortedEnableBezierPoint[i + 1].point;
			}

			mid1 = { (curveStart.x + curveEnd.x) / 2 , curveStart.y };
			mid2 = { (curveStart.x + curveEnd.x) / 2 , curveEnd.y };

			ERR(drawbotSuites.path_suiteP->MoveTo(path_ref, curveStart.x, curveStart.y));
			ERR(drawbotSuites.path_suiteP->BezierTo(path_ref, &mid1, &mid2, &curveEnd));
		} 
		ERR(drawbotSuites.surface_suiteP->StrokePath(surface_ref, pen_ref, path_ref));


		// === draw bezier path ===

		ReleaseDrawbotObject(drawbotSuites.supplier_suiteP, reinterpret_cast<DRAWBOT_ObjectRef>(string_brush_ref));
		ReleaseDrawbotObject(drawbotSuites.supplier_suiteP, reinterpret_cast<DRAWBOT_ObjectRef>(font_ref));
		ReleaseDrawbotObject(drawbotSuites.supplier_suiteP, reinterpret_cast<DRAWBOT_ObjectRef>(brush_ref));
		ReleaseDrawbotObject(drawbotSuites.supplier_suiteP, reinterpret_cast<DRAWBOT_ObjectRef>(path_ref));
		ReleaseDrawbotObject(drawbotSuites.supplier_suiteP, reinterpret_cast<DRAWBOT_ObjectRef>(image_ref));

		oss.str("");
		//PLOGI <<"finish BezierCurve UI draw event " + std::to_string(err));
	}


	// Release the earlier acquired Drawbot suites
	ERR2(AEFX_ReleaseDrawbotSuites(in_data, out_data));

	if (!err) {
		event_extra->evt_out_flags = PF_EO_HANDLED_EVENT;

	}

	PF_UNLOCK_HANDLE(arbH);

	return err;
}

static PF_Err
DoDrag(
	PF_InData* in_data,
	PF_OutData* out_data,
	PF_ParamDef* params[],
	PF_LayerDef* output,
	PF_EventExtra* event_extra)
{
	PF_Err 			err = PF_Err_NONE;
	PF_ContextH		contextH = event_extra->contextH;
	PF_Point		mouse_down;

	if (PF_Window_EFFECT == (*contextH)->w_type) {
		if (PF_EA_CONTROL == event_extra->effect_win.area) {
			mouse_down = event_extra->u.do_click.screen_point;
			PLOGI << "mouse_down.x " << std::to_string(mouse_down.x);


			float left = event_extra->effect_win.current_frame.left + UI_BEZIER_FRAME_WIDTH_MARGIN;
			float top = event_extra->effect_win.current_frame.top + UI_BEZIER_FRAME_HEIGHT_MARGIN;
			float right = event_extra->effect_win.current_frame.right - UI_BEZIER_FRAME_WIDTH_MARGIN;
			float bottom = event_extra->effect_win.current_frame.bottom - UI_BEZIER_FRAME_HEIGHT_MARGIN;

			float xRange = right - left;
			float yRange = (bottom - top);
			if (xRange < 0 || yRange < 0) {
				return err;
			}

			float lerpFactorX = (mouse_down.x - left) / xRange;
			float lerpFactorY = (mouse_down.y - top) / yRange;
			lerpFactorX = std::clamp(lerpFactorX , 0.0f, 1.0f);
			lerpFactorY = std::clamp(lerpFactorY , 0.0f, 1.0f);

			PF_ArbitraryH arbH = NULL;
			BezierCurveInfo* arbP = NULL;
			GetArbData(in_data, params, event_extra->effect_win.index, &arbP, &arbH);

			// cache in DoClick
			int clickIndex = event_extra->u.do_click.continue_refcon[0];
			arbP->bezierPointInfo[clickIndex].point.x = lerpFactorX;
			arbP->bezierPointInfo[clickIndex].point.y = lerpFactorY;

			event_extra->evt_out_flags |= PF_EO_HANDLED_EVENT | PF_EO_UPDATE_NOW;
			params[event_extra->effect_win.index]->uu.change_flags |= PF_ChangeFlag_CHANGED_VALUE;
			PF_UNLOCK_HANDLE(arbH);
		}
	}

	return err;
}

static PF_Err
DoClick(
	PF_InData* in_data,
	PF_OutData* out_data,
	PF_ParamDef* params[],
	PF_LayerDef* output,
	PF_EventExtra* event_extra)
{
	PF_Err	err = PF_Err_NONE;

	AEGP_SuiteHandler		suites(in_data->pica_basicP);
	PF_ExtendedSuiteTool	tool = PF_ExtendedSuiteTool_MAGNIFY;

	auto screenPoint = event_extra->u.do_click.screen_point;
    PLOGI << "click at screen point: " << std::to_string(screenPoint.x) << " , " << std::to_string(screenPoint.y);


	PF_ArbitraryH arbH = NULL;
	BezierCurveInfo* arbP = NULL;
	GetArbData(in_data, params, event_extra->effect_win.index, &arbP, &arbH);

	float left = event_extra->effect_win.current_frame.left + UI_BEZIER_FRAME_WIDTH_MARGIN;
	float top = event_extra->effect_win.current_frame.top + UI_BEZIER_FRAME_HEIGHT_MARGIN;
	float right = event_extra->effect_win.current_frame.right - UI_BEZIER_FRAME_WIDTH_MARGIN;
	float bottom = event_extra->effect_win.current_frame.bottom - UI_BEZIER_FRAME_HEIGHT_MARGIN;

	float xRange = right - left;
	float yRange = (bottom - top);

	std::vector<BezierPoint> sortedEnableBezierPoint = {};
	GetSortedEnableBezierPoint(arbP, (KIRIParamIdx)event_extra->effect_win.index, sortedEnableBezierPoint);
	int clickIndex = GetClickBezierPointIndex(event_extra , sortedEnableBezierPoint);
	PLOGI << "bezier Click index " + std::to_string(clickIndex);

	// &&
	if (event_extra->u.do_click.num_clicks == 2 && clickIndex == -1) {
		//add bezier point
		PLOGI << "add bezier point ";
		if (sortedEnableBezierPoint.size() < MAX_BEZIER_POINT_COUNT - 1) {

			BezierPoint bezierPoint;
			bezierPoint.enable = true;
			bezierPoint.point.x = (screenPoint.x  - left) / xRange;
			bezierPoint.point.y = (screenPoint.y  - top) / yRange;
			sortedEnableBezierPoint.push_back(bezierPoint);
		}
	}
	if (event_extra->u.do_click.modifiers & PF_Mod_CMD_CTRL_KEY &&
		event_extra->u.do_click.num_clicks == 1 && clickIndex >= 0) {
		//remove bezier point
		if (sortedEnableBezierPoint.size() > 2) {
			sortedEnableBezierPoint[clickIndex].enable = false;
		}
	}

	// drag
	if (event_extra->u.do_click.num_clicks == 1 && clickIndex >= 0) {

		event_extra->u.do_click.continue_refcon[0] = clickIndex;
		event_extra->u.do_click.send_drag = TRUE;
		event_extra->evt_out_flags = PF_EO_HANDLED_EVENT;
	}

	//rewrite
	for (int i = 0; i < MAX_BEZIER_POINT_COUNT; i++) {
		arbP->bezierPointInfo[i].enable = false;
	}
	memcpy(&arbP->bezierPointInfo[0], sortedEnableBezierPoint.data(), sortedEnableBezierPoint.size() * sizeof(BezierPoint));
	arbP->currentPointCount = sortedEnableBezierPoint.size();

	event_extra->evt_out_flags |= PF_EO_HANDLED_EVENT | PF_EO_UPDATE_NOW;
	params[event_extra->effect_win.index]->uu.change_flags |= PF_ChangeFlag_CHANGED_VALUE;
	PF_UNLOCK_HANDLE(arbH);
	
	return err;
}

static PF_Err
ChangeCursor(
	PF_InData* in_data,
	PF_OutData* out_data,
	PF_ParamDef* params[],
	PF_LayerDef* output,
	PF_EventExtra* event_extra)
{
	return PF_Err_NONE;
}

PF_Err
BezierCurveUIHandleEvent(
	PF_InData* in_data,
	PF_OutData* out_data,
	PF_ParamDef* params[],
	PF_LayerDef* output,
	PF_EventExtra* extra)
{
	PF_Err		err = PF_Err_NONE;
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


PF_Err
SetupBezierUI(
	PF_InData* in_data, /* in */
	uint32_t diskID,  /* in */
	StrIDType   strId,	  /* in */
	PF_OutData* out_data /* out */
) {
	
	PF_Err err = PF_Err_NONE;

	PF_ParamDef	def;

	AEFX_CLR_STRUCT(def);

	ERR(CreateDefaultArb(in_data,
		out_data,
		&def.u.arb_d.dephault));

	PF_ADD_ARBITRARY2(
		STR(strId),
		UI_BEZIER_TOPIC_WIDTH,
		UI_BEZIER_TOPIC_HEIGHT,
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

