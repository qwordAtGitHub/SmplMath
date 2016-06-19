.nolist
include masm32rt.inc
.686p
.mmx
.xmm

include gdip.inc
include gdiplus.inc
includelib gdiplus.lib

include \macros\unicode.inc
include \macros\smplmath\math.inc

include FncDrawCtrl.inc


pwsz$ macro wsz:VARARG
LOCAL pwsz?,txt,wszSize??
	txt TEXTEQU wsz$(wsz)
	.data
		wszSize?? = $-txt
	.code
	mov eax,alloc(%wszSize??)
	push eax
	invoke ucCopy,txt,eax
	pop eax
	EXITM <eax>
endm

d2d macro mem64_dest,mem64_src
	push DWORD ptr mem64_src
	pop DWORD ptr mem64_dest
	push DWORD ptr mem64_src+4
	pop DWORD ptr mem64_dest+4
endm

xchg8 macro arg1,arg2
	mov eax,DWORD ptr arg1
	mov edx,DWORD ptr arg1+4
	d2d arg1,arg2
	mov DWORD ptr arg2,eax
	mov DWORD ptr arg2,edx
endm

GdipGetClientRectF proto hWnd:HWND,pRectF:ptr RectF
draw_axes proto graphics:PVOID,pAxesInfo:ptr AXES_INFO
calc_metrics proto hWnd:HWND,pAxesInfo:ptr AXES_INFO
plot proto graphics:PVOID,pen:PVOID,nPoints:DWORD,pAxesInfo: ptr AXES_INFO,pfnCallBack:PVOID
scale_axes proto pAxesInfo: ptr AXES_INFO,xMultipleOf:REAL8,yMultipleOf:REAL8,nMarksX:DWORD,nMarksY:DWORD
ApproxEqual proto fValue1:REAL8,fValue2:REAL8,fRange:REAL8
pix2mm proto x:DWORD,y:DWORD,pPointD: ptr PointD
mm2pix proto pPointD: ptr PointD,pPoint: ptr POINT
ClientPoint2Unit proto x:DWORD,y:DWORD,pAxesInfo: ptr AXES_INFO,pPointD: ptr PointD
Unit2ClientPoint proto pPointD: ptr PointD,pAxesInfo: ptr AXES_INFO,pPoint: ptr POINT

.data?
	align 16
	gsi		GdiplusStartupInput	<?>
	gtkn	PULONG				 ?
.code
DllMain proc hInstance:HINSTANCE,fdwReason:DWORD,lpvReserved:LPVOID
LOCAL wc:WNDCLASSEX
	
	.if fdwReason == DLL_PROCESS_ATTACH
		mov gsi.GdiplusVersion,1
		mov gsi.DebugEventCallback,0
		mov gsi.SuppressBackgroundThread,0
		mov gsi.SuppressExternalCodecs,0
		invoke GdiplusStartup,ADDR gtkn,ADDR gsi,0

		m2m wc.hInstance,hInstance
		mov wc.cbSize,SIZEOF wc
		mov wc.style,CS_HREDRAW or CS_VREDRAW or CS_DBLCLKS or CS_SAVEBITS or CS_GLOBALCLASS
		mov wc.lpfnWndProc,WndProc
		mov wc.cbClsExtra,0
		mov wc.cbWndExtra,sizeof PFNCDRAW_CNTRL
		mov wc.lpszClassName,chr$("FncDrawCtrl32")
		mov wc.hIcon,0;rv(LoadIcon,NULL,IDI_APPLICATION)
		mov wc.hIconSm,0;eax
		mov wc.hCursor,rv(LoadCursor,NULL,IDC_ARROW)
		mov wc.lpszMenuName,0
		mov wc.hbrBackground,0
		invoke RegisterClassEx,ADDR wc
	.elseif fdwReason == DLL_PROCESS_DETACH
		invoke UnregisterClass,chr$("FncDrawCtrl32"),hInstance
		invoke GdiplusShutdown,gtkn
	.endif

	mov eax,TRUE
	ret
	
DllMain endp


CopyAxesInfo proc uses ebx esi pAxesInfo: ptr AXES_INFO
	
	pushcontext assumes
	assume esi: ptr AXES_INFO
	assume ebx: ptr AXES_INFO
	
	mov esi,pAxesInfo
	mov ebx,alloc(SIZEOF AXES_INFO)
	
	invoke MemCopy,esi,ebx,SIZEOF AXES_INFO
	
	invoke ucLen,[esi].labels.pwszFontFam
	lea eax,[2*eax+2]
	mov [ebx].labels.pwszFontFam,alloc(eax)
	invoke ucCopy,[esi].labels.pwszFontFam,[ebx].labels.pwszFontFam
	
	invoke ucLen,[esi].labels.pwszFormatX
	lea eax,[2*eax+2]
	mov [ebx].labels.pwszFormatX,alloc(eax)
	invoke ucCopy,[esi].labels.pwszFormatX,[ebx].labels.pwszFormatX
	
	invoke ucLen,[esi].labels.pwszFormatY
	lea eax,[2*eax+2]
	mov [ebx].labels.pwszFormatY,alloc(eax)
	invoke ucCopy,[esi].labels.pwszFormatY,[ebx].labels.pwszFormatY
	
	mov eax,ebx
		
	popcontext assumes
	ret
	
CopyAxesInfo endp

FreeAxesInfo proc uses ebx pAxesInfo: ptr AXES_INFO
	
	assume ebx: ptr AXES_INFO
	
	mov ebx,pAxesInfo
	free [ebx].labels.pwszFontFam
	free [ebx].labels.pwszFormatX
	free [ebx].labels.pwszFormatY
	
	.if [ebx].labels.flg & ( PEN__CTRL_OWNS  or PEN__CTRL_DESTROY or PEN__CLONE)
		invoke GdipDeletePen,[ebx].labels.pen
	.endif
	
	.if [ebx].labels.flg & ( TXTPEN__CTRL_OWNS  or TXTPEN__CTRL_DESTROY or TXTPEN__CLONE)
		invoke GdipDeletePen,[ebx].labels.penTxt
	.endif
	
;	.if [ebx].labels.flg & ( BRUSH__CTRL_OWNS  or BRUSH__CTRL_DESTROY or BRUSH__CLONE)
;		invoke GdipDeleteBrush,[ebx].labels.brush
;	.endif
	
	.if [ebx].labels.flg & ( TXTBRUSH__CTRL_OWNS  or TXTBRUSH__CTRL_DESTROY or TXTBRUSH__CLONE)
		invoke GdipDeleteBrush,[ebx].labels.brushTxt
	.endif

	free pAxesInfo
	
	assume ebx: nothing
	ret
	
FreeAxesInfo endp

IsValidFncHandle proc uses ebx pFncDrwCtrl:ptr FNCDRAW_CNTRL,hFnc:HFNC
	
	mov ebx,pFncDrwCtrl
	
	.if [ebx].FNCDRAW_CNTRL.nFunctions
		xor eax,eax
		ret		
	.endif
	mov edx,[ebx].FNCDRAW_CNTRL.pFncDscptr
	mov eax,hFnc 
	.while edx
		.if edx == eax
			ret
		.endif
		mov edx,[edx].FNC_DSCPTR.pNextDscptr
	.endw
	
	xor eax,eax	
	ret
	
IsValidFncHandle endp

fdcRedirectWmMouseWheel proc uses ebx hWnd:HWND,wParam:WPARAM,lParam:LPARAM
LOCAL szClassName[256]:BYTE
LOCAL pt:POINT
	movzx eax,WORD ptr lParam[0]
	movzx ecx,WORD ptr lParam[2]
	mov pt.x,eax
	mov pt.y,ecx
	
	invoke ScreenToClient,hWnd,ADDR pt
	.if rv(ChildWindowFromPoint,hWnd,pt.x,pt.y)
		mov ebx,eax
		.data
			szFncDrawCtrlClassName	db "FncDrawCtrl32",0
		.code
		.if rv(GetClassName,ebx,ADDR szClassName,SIZEOF szClassName)
			.if rv(szCmp,ADDR szClassName,ADDR szFncDrawCtrlClassName)
				invoke MapWindowPoints,hWnd,ebx,ADDR pt,1
				mov eax,pt.x
				mov ecx,pt.y
				movzx eax,ax
				shl ecx,16
				or eax,ecx
				invoke SendMessage,ebx,WM_MOUSEWHEEL,wParam,eax
			.endif
		.endif
	.endif
	ret
	
fdcRedirectWmMouseWheel endp

fdc_OnCreate proc uses ebx hWnd:HWND,lParam:DWORD
	
	mov ebx,alloc(SIZEOF FNCDRAW_CNTRL)
	invoke SetWindowLong,hWnd,0,ebx
	mov [ebx].FNCDRAW_CNTRL.pAxesInfo,alloc(SIZEOF AXES_INFO)
	m2m [ebx].FNCDRAW_CNTRL.zoomFactor,FP4(0.1)
	mov [ebx].FNCDRAW_CNTRL.zoomFlg,ZOOM_CENTER
	mov [ebx].FNCDRAW_CNTRL.argbBkColor,-1
	mov eax,lParam
	.if [eax].CREATESTRUCT.lpCreateParams
		mov [ebx].FNCDRAW_CNTRL.pAxesInfo,rv(CopyAxesInfo,[eax].CREATESTRUCT.lpCreateParams)
		mov [ebx].FNCDRAW_CNTRL.init,1
	.else
		mov [ebx].FNCDRAW_CNTRL.init,1
		mov ebx,[ebx].FNCDRAW_CNTRL.pAxesInfo
		assume ebx: ptr AXES_INFO
		mov [ebx].labels.pwszFontFam,pwsz$("Arial")
		mov [ebx].labels.pwszFormatX,pwsz$("%.1f")
		mov [ebx].labels.pwszFormatY,pwsz$("%.1f")
		m2m [ebx].labels.emHeight,FP4(5.0)				; 5mm
		mov [ebx].labels.flgXlbl,ALP_X_DEFAULT
		mov [ebx].labels.flgYlbl,ALP_Y_DEFAULT
		mov [ebx].labels.flg,PEN__CTRL_OWNS or BRUSH__CTRL_OWNS or TXTBRUSH__CTRL_OWNS
		m2m [ebx].labels.markSize,FP4(2.0)
		d2d [ebx].labels.xMultipleOf,FP8(1.0)
		d2d [ebx].labels.yMultipleOf,FP8(1.0)
		invoke GdipCreatePen1,ColorsBlack,FP4(1.0),UnitWorld,ADDR [ebx].labels.pen
		;invoke GdipCreateSolidFill,ColorsBlack,ADDR [ebx].labels.brush
		invoke GdipCreateSolidFill,ColorsBlack,ADDR [ebx].labels.brushTxt
		d2d [ebx].view.xMax,FP8( 5.0)
		d2d [ebx].view.xMin,FP8(-5.0)
		d2d [ebx].view.yMax,FP8( 5.0)
		d2d [ebx].view.yMin,FP8(-5.0)
		m2m [ebx].labels.frame.top,FP4(10.0)
		m2m [ebx].labels.frame.left,FP4(10.0)
		m2m [ebx].labels.frame.right,FP4(10.0)
		m2m [ebx].labels.frame.bottom,FP4(10.0)
		mov [ebx].labels.nxMarks,10
		mov [ebx].labels.nyMarks,10
		assume ebx: nothing
	.endif
	
	
	ret
	
fdc_OnCreate endp


fdc_OnDestroy proc uses ebx esi hWnd:HWND
	
	mov ebx,rv(GetWindowLong,hWnd,0)
	
	;/* free AXES_INFO */
	invoke FreeAxesInfo,[ebx].FNCDRAW_CNTRL.pAxesInfo

	;/* free function descriptors */
	.if [ebx].FNCDRAW_CNTRL.nFunctions
		mov esi,[ebx].FNCDRAW_CNTRL.pFncDscptr
		.repeat
			.if [esi].FNC_DSCPTR.flg & ( PEN__CTRL_OWNS  or PEN__CTRL_DESTROY or PEN__CLONE)
				invoke GdipDeletePen,[esi].FNC_DSCPTR.pen
			.endif
			push [esi].FNC_DSCPTR.pNextDscptr
			free esi
			pop esi
		.until !esi
	.endif
	
	
	;/* free FNCDRAW_CNTRL */
	free ebx
	
	ret
	
fdc_OnDestroy endp


fdc_set_label_info proc uses ebx esi edi pAxesInfo: ptr AXES_INFO,pLabelInfo:PVOID,uMsg:UINT
	
	;/* free old struct */
	mov edi,pAxesInfo
	lea edi,[edi].AXES_INFO.labels
	mov esi,pLabelInfo
	assume edi: ptr LABEL_INFO
	assume esi: ptr LABEL_INFO

	free [edi].pwszFontFam
	free [edi].pwszFormatX
	free [edi].pwszFormatY
	
	.if [edi].flg & ( PEN__CTRL_OWNS  or PEN__CTRL_DESTROY or PEN__CLONE)
		invoke GdipDeletePen,[edi].pen
	.endif
	
	.if [edi].flg & ( TXTPEN__CTRL_OWNS  or TXTPEN__CTRL_DESTROY or TXTPEN__CLONE)
		invoke GdipDeletePen,[edi].penTxt
	.endif
	
;	.if [edi].flg & ( BRUSH__CTRL_OWNS  or BRUSH__CTRL_DESTROY or BRUSH__CLONE)
;		invoke GdipDeleteBrush,[edi].brush
;	.endif
	
	.if [edi].flg & ( TXTBRUSH__CTRL_OWNS  or TXTBRUSH__CTRL_DESTROY or TXTBRUSH__CLONE)
		invoke GdipDeleteBrush,[edi].brushTxt
	.endif

	;/* copy new struct */
	invoke MemCopy,esi,edi,SIZEOF LABEL_INFO
	
	;/* copy strings */
	.if uMsg == FDCM_SET_LABEL_INFOA
		assume esi: nothing
		lea ebx,[2*len([esi].LABEL_INFOA.pszFontFam)+2]
		mov [edi].pwszFontFam,alloc(ebx)
    	invoke MultiByteToWideChar,CP_ACP,MB_PRECOMPOSED,[esi].LABEL_INFOA.pszFontFam,ebx,[edi].pwszFontFam,ebx
		
		lea ebx,[2*len([esi].LABEL_INFOA.pszFormatX)+2]
		mov [edi].pwszFormatX,alloc(ebx)
    	invoke MultiByteToWideChar,CP_ACP,MB_PRECOMPOSED,[esi].LABEL_INFOA.pszFormatX,ebx,[edi].pwszFormatX,ebx
		
		lea ebx,[2*len([esi].LABEL_INFOA.pszFormatY)+2]
		mov [edi].pwszFormatY,alloc(ebx)
    	invoke MultiByteToWideChar,CP_ACP,MB_PRECOMPOSED,[esi].LABEL_INFOA.pszFormatY,ebx,[edi].pwszFormatY,ebx
    	assume esi: ptr LABEL_INFO
	.else	
		invoke ucLen,[esi].pwszFontFam
		lea eax,[2*eax+2]
		mov [edi].pwszFontFam,alloc(eax)
		invoke ucCopy,[esi].pwszFontFam,[edi].pwszFontFam	
		
		invoke ucLen,[esi].pwszFormatX
		lea eax,[2*eax+2]
		mov [edi].pwszFormatX,alloc(eax)
		invoke ucCopy,[esi].pwszFormatX,[edi].pwszFormatX	
		
		invoke ucLen,[esi].pwszFormatY
		lea eax,[2*eax+2]
		mov [edi].pwszFormatY,alloc(eax)
		invoke ucCopy,[esi].pwszFormatY,[edi].pwszFormatY	
	.endif
	
	assume edi: ptr LABEL_INFOW
	;/* copy brushes and pens */
	.if [edi].flg&PEN__CLONE
		invoke GdipClonePen,[edi].pen,ADDR [edi].pen
	.elseif [edi].flg&_VALUE_ARGB_ && [edi].flg&PEN__CTRL_OWNS

		invoke GdipCreatePen1,[edi].pen,[edi].LABEL_INFOW.pen_width,UnitWorld,ADDR [edi].pen
	.endif
	
	.if [edi].flg&TXTPEN__CLONE
		invoke GdipClonePen,[edi].penTxt,ADDR [edi].penTxt
	.elseif [edi].flg&_VALUE_ARGB_ && [edi].flg&TXTPEN__CTRL_OWNS
		invoke GdipCreatePen1,[edi].penTxt,[edi].LABEL_INFOW.penTxt_width,UnitWorld,ADDR [edi].penTxt
	.endif
	
;	.if [edi].flg&BRUSH__CLONE
;		invoke GdipCloneBrush,[edi].brush,ADDR [edi].brush
;	.elseif [edi].flg&_VALUE_ARGB_ && [edi].flg&BRUSH__CTRL_OWNS
;		invoke GdipCreateSolidFill,[edi].brush,ADDR [edi].brush
;	.endif
	
	.if [edi].flg&TXTBRUSH__CLONE
		invoke GdipCloneBrush,[edi].brushTxt,ADDR [edi].brushTxt
	.elseif [edi].flg&_VALUE_ARGB_ && [edi].flg&TXTBRUSH__CTRL_OWNS
		invoke GdipCreateSolidFill,[edi].brushTxt,ADDR [edi].brushTxt
	.endif
	
	assume edi: nothing
	assume esi: nothing
	
	ret
	
fdc_set_label_info endp

WndProc proc uses ebx esi edi hWnd:HWND,uMsg:UINT,wParam:WPARAM,lParam:LPARAM
LOCAL ps:PAINTSTRUCT
LOCAL graphics:PVOID	
LOCAL hBkDC:HDC
LOCAL rect:RECT
LOCAL pixpincx:REAL4,pixpincy:REAL4
LOCAL hdc:HDC
LOCAL pointf:PointF
LOCAL pointd:PointD
LOCAL fdc_nmhdr:FDC_NMHDR
LOCAL fSlvTLS()

	.if uMsg == WM_CLOSE
		invoke DestroyWindow,hWnd
		invoke PostQuitMessage,0
	.elseif uMsg == WM_DESTROY
		invoke fdc_OnDestroy,hWnd
	.elseif uMsg == WM_CREATE
		invoke fdc_OnCreate,hWnd,lParam
	.elseif uMsg == FDCM_SET_AXES_INFO
		mov ebx,rv(GetWindowLong,hWnd,0)
		.if [ebx].FNCDRAW_CNTRL.pAxesInfo
			invoke FreeAxesInfo,[ebx].FNCDRAW_CNTRL.pAxesInfo
		.endif
		mov [ebx].FNCDRAW_CNTRL.pAxesInfo,rv(CopyAxesInfo,wParam)
		mov [ebx].FNCDRAW_CNTRL.init,1
		invoke InvalidateRect,hWnd,0,0
	.elseif uMsg == FDCM_GET_STYLE
		mov ebx,rv(GetWindowLong,hWnd,0)
		mov eax,[ebx].FNCDRAW_CNTRL.dwStyle
		ret
	.elseif uMsg == FDCM_SET_STYLE
		mov ebx,rv(GetWindowLong,hWnd,0)
		.if wParam & (NOT FDCS_VALID_STYLES)
			xor eax,eax
			ret
		.endif
		m2m [ebx].FNCDRAW_CNTRL.dwStyle,wParam
		;invoke InvalidateRect,hWnd,0,TRUE
		mov eax,1
		ret
	.elseif uMsg == FDCM_SET_CURR_VIEW
		mov ebx,rv(GetWindowLong,hWnd,0)
		mov ebx,[ebx].FNCDRAW_CNTRL.pAxesInfo
		mov esi,wParam
		d2d [ebx].AXES_INFO.view.xMax,[esi].CURR_VIEW.xMax
		d2d [ebx].AXES_INFO.view.xMin,[esi].CURR_VIEW.xMin
		d2d [ebx].AXES_INFO.view.yMax,[esi].CURR_VIEW.yMax
		d2d [ebx].AXES_INFO.view.yMin,[esi].CURR_VIEW.yMin
		invoke calc_metrics,hWnd,ebx
		invoke scale_axes,ebx,[ebx].AXES_INFO.labels.xMultipleOf,[ebx].AXES_INFO.labels.yMultipleOf,[ebx].AXES_INFO.labels.nxMarks,[ebx].AXES_INFO.labels.nyMarks
		invoke InvalidateRect,hWnd,0,TRUE
		mov eax,1
		ret
	.elseif uMsg == FDCM_CLIENT2UNIT
		mov ebx,rv(GetWindowLong,hWnd,0)
		mov edx,wParam
		invoke ClientPoint2Unit,[edx].POINT.x,[edx].POINT.y,ebx,lParam
		mov eax,1
		ret
	.elseif uMsg == FDCM_UNIT2CLIENT
		invoke Unit2ClientPoint,wParam,rv(GetWindowLong,hWnd,0),lParam
		mov eax,1
		ret
	.elseif uMsg == FDCM_GET_CURR_VIEW
		mov ebx,rv(GetWindowLong,hWnd,0)
		mov ebx,[ebx].FNCDRAW_CNTRL.pAxesInfo
		mov esi,wParam
		d2d [esi].CURR_VIEW.xMax,[ebx].AXES_INFO.view.xMax
		d2d [esi].CURR_VIEW.xMin,[ebx].AXES_INFO.view.xMin
		d2d [esi].CURR_VIEW.yMax,[ebx].AXES_INFO.view.yMax
		d2d [esi].CURR_VIEW.yMin,[ebx].AXES_INFO.view.yMin
		mov eax,1
		ret
	.elseif uMsg == FDCM_SET_FNC_CB
		mov ebx,rv(GetWindowLong,hWnd,0)
		.if rv(IsValidFncHandle,ebx,wParam)
			mov eax,[ebx].FNC_DSCPTR.pCallBack
			m2m [ebx].FNC_DSCPTR.pCallBack,lParam
			invoke InvalidateRect,hWnd,0,TRUE
			mov eax,1
		.else
			xor eax,eax
		.endif
		ret
	.elseif uMsg == FDCM_SET_FNC_NPOINTS
		mov ebx,rv(GetWindowLong,hWnd,0)
		.if rv(IsValidFncHandle,ebx,wParam) && lParam >= 2
			mov eax,[ebx].FNC_DSCPTR.nPoints
			m2m [ebx].FNC_DSCPTR.nPoints,lParam
			invoke InvalidateRect,hWnd,0,TRUE
			mov eax,1
		.else
			xor eax,eax
		.endif
		ret
	.elseif uMsg == FDCM_DEL_FNC
		mov ebx,rv(GetWindowLong,hWnd,0)
		.if rv(IsValidFncHandle,ebx,wParam)
			mov edx,wParam
			.if [edx].FNC_DSCPTR.flg & ( PEN__CTRL_OWNS  or PEN__CTRL_DESTROY or PEN__CLONE)
				invoke GdipDeletePen,[edx].FNC_DSCPTR.pen
				mov edx,wParam
			.endif
			.if [edx].FNC_DSCPTR.pPrevDscptr && [edx].FNC_DSCPTR.pNextDscptr
				mov eax,[edx].FNC_DSCPTR.pPrevDscptr
				m2m [eax].FNC_DSCPTR.pNextDscptr,[edx].FNC_DSCPTR.pNextDscptr
				mov eax,[edx].FNC_DSCPTR.pNextDscptr
				m2m [eax].FNC_DSCPTR.pPrevDscptr,[edx].FNC_DSCPTR.pPrevDscptr
			.elseif [edx].FNC_DSCPTR.pPrevDscptr
				mov eax,[edx].FNC_DSCPTR.pPrevDscptr
				mov [eax].FNC_DSCPTR.pNextDscptr,0
			.else;if [edx].FNC_DSCPTR.pNextDscptr
				mov eax,[edx].FNC_DSCPTR.pNextDscptr
				mov [eax].FNC_DSCPTR.pPrevDscptr,0
				mov [ebx].FNCDRAW_CNTRL.pFncDscptr,eax
			.endif
			free wParam
			dec [ebx].FNCDRAW_CNTRL.nFunctions
			invoke InvalidateRect,hWnd,0,TRUE
			mov eax,1
		.else
			xor eax,eax
		.endif
		ret
	.elseif uMsg == FDCM_GET_FNC_DSCRPTR
		mov ebx,rv(GetWindowLong,hWnd,0)
		.if rv(IsValidFncHandle,ebx,wParam)
			mov edx,lParam
			mov eax,wParam
			m2m [edx].FNC_DSCPTR.flg,[eax].FNC_DSCPTR.flg
			m2m [edx].FNC_DSCPTR.nPoints,[eax].FNC_DSCPTR.nPoints
			m2m [edx].FNC_DSCPTR.pCallBack,[eax].FNC_DSCPTR.pCallBack
			m2m [edx].FNC_DSCPTR.pen,[eax].FNC_DSCPTR.pen
			m2m [edx].FNC_DSCPTR._width,[eax].FNC_DSCPTR._width
			mov [edx].FNC_DSCPTR.pNextDscptr,0
			mov [edx].FNC_DSCPTR.pPrevDscptr,0
			mov eax,1
		.else
			xor eax,eax
		.endif
		ret
	.elseif uMsg == FDCM_SET_FNC_PEN
		mov ebx,rv(GetWindowLong,hWnd,0)
		.if rv(IsValidFncHandle,ebx,wParam)
			mov esi,wParam
			mov edi,lParam
			.if [esi].FNC_DSCPTR.flg & ( PEN__CTRL_OWNS  or PEN__CTRL_DESTROY or PEN__CLONE)
				invoke GdipDeletePen,[esi].FNC_DSCPTR.pen
			.endif
			m2m [esi].FNC_DSCPTR.pen,[edi].FNC_DSCPTR.pen
			m2m [esi].FNC_DSCPTR.flg,[edi].FNC_DSCPTR.flg
			m2m [esi].FNC_DSCPTR._width,[edi].FNC_DSCPTR._width
			
			.if [edi].FNC_DSCPTR.flg&PEN__CLONE
				invoke GdipClonePen,[edi].FNC_DSCPTR.pen,ADDR [esi].FNC_DSCPTR.pen
			.elseif [edi].FNC_DSCPTR.flg&_VALUE_ARGB_ && [edi].FNC_DSCPTR.flg&PEN__CTRL_OWNS
				invoke GdipCreatePen1,[edi].FNC_DSCPTR.pen,[edi].FNC_DSCPTR._width,UnitWorld,ADDR [esi].FNC_DSCPTR.pen
			.endif
			invoke InvalidateRect,hWnd,0,TRUE
			mov eax,1
		.else
			xor eax,eax
		.endif
		ret
	.elseif uMsg == FDCM_SET_LABEL_INFOA || uMsg == FDCM_SET_LABEL_INFOW
		mov ebx,rv(GetWindowLong,hWnd,0)
		invoke fdc_set_label_info,[ebx].FNCDRAW_CNTRL.pAxesInfo,wParam,uMsg
		ret
	.elseif uMsg == FDCM_SET_BKCOLOR
		mov ebx,rv(GetWindowLong,hWnd,0)
		m2m [ebx].FNCDRAW_CNTRL.argbBkColor,wParam
		invoke InvalidateRect,hWnd,0,TRUE
		mov eax,1
		ret
	.elseif uMsg == FDCM_GET_BKCOLOR
		mov ebx,rv(GetWindowLong,hWnd,0)
		mov eax,[ebx].FNCDRAW_CNTRL.argbBkColor
		ret
	.elseif uMsg == FDCM_ADD_FUNCTION
		mov esi,wParam
		assume esi: ptr FNC_DSCPTR
		mov ebx,rv(GetWindowLong,hWnd,0)
		.if esi && [esi].nPoints && [esi].pCallBack
			mov edi,alloc(SIZEOF FNC_DSCPTR)
			assume edi: ptr FNC_DSCPTR
			.if [ebx].FNCDRAW_CNTRL.nFunctions
				mov edx,[ebx].FNCDRAW_CNTRL.pFncDscptr
				.while 1
					mov eax,[edx].FNC_DSCPTR.pNextDscptr
					.break .if !eax 
					mov edx,eax
				.endw
				mov [edx].FNC_DSCPTR.pNextDscptr,edi
				mov [edi].pPrevDscptr,edx
				inc [ebx].FNCDRAW_CNTRL.nFunctions
			.else
				mov [ebx].FNCDRAW_CNTRL.pFncDscptr,edi
				mov [ebx].FNCDRAW_CNTRL.nFunctions,1
			.endif
			.if ![esi].flg&PEN__VALID_
				mov [edi].flg,PEN__DEFAULT
			.else
				m2m [edi].flg,[esi].flg
			.endif
			m2m [edi].nPoints,[esi].nPoints
			m2m [edi].pCallBack,[esi].pCallBack
			m2m [edi].pen,[esi].pen
			
			.if [esi].flg&PEN__CLONE
				invoke GdipClonePen,[esi].pen,ADDR [edi].pen
			.elseif [esi].flg&_VALUE_ARGB_ && [esi].flg&PEN__CTRL_OWNS
				invoke GdipCreatePen1,[esi].pen,[esi]._width,UnitWorld,ADDR [edi].pen
			.endif
			invoke InvalidateRect,hWnd,0,TRUE
			mov eax,edi
			ret
		.else
			xor eax,eax
			ret	
		.endif
		assume esi: nothing
		assume edi: nothing
	.elseif uMsg == WM_PAINT
		mov ebx,rv(GetWindowLong,hWnd,0)
		invoke BeginPaint,hWnd,ADDR ps
		invoke GetClientRect,hWnd,ADDR rect
		mov hBkDC,rv(CreateCompatibleDC,ps.hdc)
		push rv(SelectObject,hBkDC,rv(CreateCompatibleBitmap,ps.hdc,rect.right,rect.bottom))
		
		
		invoke GdipCreateFromHDC,hBkDC,ADDR graphics
		invoke GdipSetPageUnit,graphics,UnitMillimeter
		invoke GdipSetSmoothingMode,graphics,SmoothingModeHighQuality
		
		.if [ebx].FNCDRAW_CNTRL.init
			mov esi,ebx
			mov ebx,[ebx].FNCDRAW_CNTRL.pAxesInfo
			assume ebx: ptr AXES_INFO
			assume esi: ptr FNCDRAW_CNTRL
						
			invoke calc_metrics,hWnd,ebx
			invoke scale_axes,ebx,[ebx].labels.xMultipleOf,[ebx].labels.yMultipleOf,[ebx].labels.nxMarks,[ebx].labels.nyMarks
			
			invoke GdipGraphicsClear,graphics,[esi].argbBkColor
			
			push ebx
			mov ebx,[esi].pFncDscptr
			assume ebx: ptr FNC_DSCPTR
			xor edi,edi
			.while edi < [esi].nFunctions && ebx
				invoke plot,graphics,[ebx].pen,[ebx].nPoints,[esi].pAxesInfo,[ebx].pCallBack
				mov ebx,[ebx].pNextDscptr
				inc edi
			.endw
			pop ebx

			invoke draw_axes,graphics,ebx
			
			assume ebx: nothing
			assume esi: nothing
		.endif
		
		invoke BitBlt,ps.hdc,0,0,rect.right,rect.bottom,hBkDC,0,0,SRCCOPY
		pop eax
		invoke DeleteObject,rv(SelectObject,hBkDC,eax)
		invoke DeleteDC,hBkDC

		invoke GdipDeleteGraphics,graphics
		invoke EndPaint,hWnd,ADDR ps		
	
	.elseif uMsg == WM_LBUTTONDOWN
		mov ebx,rv(GetWindowLong,hWnd,0)
		m2m [ebx].FNCDRAW_CNTRL.MousePos,lParam
		mov [ebx].FNCDRAW_CNTRL.bMouseDown,1
		invoke SetCapture,hWnd
	.elseif uMsg == WM_LBUTTONUP
		invoke ReleaseCapture
		mov ebx,rv(GetWindowLong,hWnd,0)
		mov [ebx].FNCDRAW_CNTRL.bMouseDown,0
	.elseif uMsg == WM_MOUSEMOVE
		mov ebx,rv(GetWindowLong,hWnd,0)
		mov esi,ebx

		.if wParam & MK_LBUTTON && [esi].FNCDRAW_CNTRL.bMouseDown
			mov ebx,[ebx].FNCDRAW_CNTRL.pAxesInfo
			assume ebx: ptr AXES_INFO
			
			mov hdc,rv(GetDC,0)
			mov pixpincx,rv(GetDeviceCaps,hdc,LOGPIXELSX)
			mov pixpincy,rv(GetDeviceCaps,hdc,LOGPIXELSY)
			invoke ReleaseDC,0,hdc

			fSlv [ebx].view.xMin = [ebx].view.xMin - (WORD ptr lParam[0]-[esi].FNCDRAW_CNTRL.MousePos.x) * (25.4 / pixpincx) / [ebx].metrics.mmpux * 0.01 
			fSlv [ebx].view.xMax = [ebx].view.xMax - (WORD ptr lParam[0]-[esi].FNCDRAW_CNTRL.MousePos.x) * (25.4 / pixpincx) / [ebx].metrics.mmpux * 0.01 
			fSlv [ebx].view.yMin = [ebx].view.yMin + (WORD ptr lParam[2]-[esi].FNCDRAW_CNTRL.MousePos.y) * (25.4 / pixpincy) / [ebx].metrics.mmpuy * 0.01 
			fSlv [ebx].view.yMax = [ebx].view.yMax + (WORD ptr lParam[2]-[esi].FNCDRAW_CNTRL.MousePos.y) * (25.4 / pixpincy) / [ebx].metrics.mmpuy * 0.01 
			m2m [esi].FNCDRAW_CNTRL.MousePos,lParam
			invoke calc_metrics,hWnd,ebx
			invoke InvalidateRect,hWnd,0,0
			
			assume ebx: nothing
		.endif

		.if [esi].FNCDRAW_CNTRL.dwStyle & FDCS_MOUSE_NOTIFICATION
			movzx ecx,WORD ptr lParam
			movzx edx,WORD ptr lParam+2
			mov fdc_nmhdr.point.x,ecx
			mov fdc_nmhdr.point.y,edx
			mov ebx,[esi].FNCDRAW_CNTRL.pAxesInfo
			invoke ClientPoint2Unit,ecx,edx,ebx,ADDR fdc_nmhdr.pointd
			m2m fdc_nmhdr.nmhdr.hwndFrom,hWnd
			m2m fdc_nmhdr.nmhdr.idFrom,rv(GetWindowLong,hWnd,GWL_ID)
			m2m fdc_nmhdr.fwKeys,wParam
			push eax
			m2m fdc_nmhdr.nmhdr.code,FDCNM_MOUSE_MOVE
			mov edx,rv(GetParent,hWnd)			
			pop ecx
			invoke SendMessage,edx,WM_NOTIFY,ecx,ADDR fdc_nmhdr
		.endif
	.elseif uMsg == WM_MOUSEWHEEL
		mov ebx,rv(GetWindowLong,hWnd,0)
		.if [ebx].FNCDRAW_CNTRL.init
			.if [ebx].FNCDRAW_CNTRL.zoomFlg ;& ZOOM_CENTER
				mov esi,ebx
				mov ebx,[ebx].FNCDRAW_CNTRL.pAxesInfo
				assume ebx: ptr AXES_INFO
				
				fSlv = abs([ebx].view.xMax-[ebx].view.xMin)*0.1
				fSlv = abs([ebx].view.yMax-[ebx].view.yMin)*0.1
				.if SWORD ptr wParam+2 < 0
					fSlv [ebx].view.xMin = [ebx].view.xMin - st1
					fSlv [ebx].view.xMax = [ebx].view.xMax + st1
					fSlv [ebx].view.yMin = [ebx].view.yMin - st0
					fSlv [ebx].view.yMax = [ebx].view.yMax + st0
				.else
					fSlv [ebx].view.xMin = [ebx].view.xMin + st1
					fSlv [ebx].view.xMax = [ebx].view.xMax - st1
					fSlv [ebx].view.yMin = [ebx].view.yMin + st0
					fSlv [ebx].view.yMax = [ebx].view.yMax - st0
				.endif
				fstp st
				fstp st
				
				.if [esi].FNCDRAW_CNTRL.dwStyle & FDCS_MOUSE_NOTIFICATION
					invoke calc_metrics,hWnd,ebx
					invoke GetCursorPos,ADDR fdc_nmhdr.point
					invoke ScreenToClient,hWnd,ADDR fdc_nmhdr.point
					invoke ClientPoint2Unit,fdc_nmhdr.point.x,fdc_nmhdr.point.y,ebx,ADDR fdc_nmhdr.pointd
					m2m fdc_nmhdr.nmhdr.hwndFrom,hWnd
					movzx eax,WORD ptr wParam
					m2m fdc_nmhdr.fwKeys,eax
					m2m fdc_nmhdr.nmhdr.idFrom,rv(GetWindowLong,hWnd,GWL_ID)
					push eax
					m2m fdc_nmhdr.nmhdr.code,FDCNM_MOUSE_MOVE
					mov edx,rv(GetParent,hWnd)			
					pop ecx
					invoke SendMessage,edx,WM_NOTIFY,ecx,ADDR fdc_nmhdr
				.endif
				assume ebx: nothing
			.endif
			
			invoke InvalidateRect,hWnd,0,0
			
		.endif
	.elseif uMsg == WM_LBUTTONDBLCLK
		mov ebx,rv(GetWindowLong,hWnd,0)
		.if [ebx].FNCDRAW_CNTRL.init
			mov ebx,[ebx].FNCDRAW_CNTRL.pAxesInfo
			assume ebx: ptr AXES_INFO
			movzx ecx,WORD ptr lParam
			movzx edx,WORD ptr lParam+2
			
			invoke pix2mm,ecx,edx,ADDR pointd
			.if rv(ApproxEqual,@fSlv8([ebx].metrics.xAxe+[ebx].metrics.rectf.y),pointd.y,FP8(1.0))
				.if [ebx].labels.flgXlbl & ALP_CENTER_BOTTOM
					mov [ebx].labels.flgXlbl,ALP_QDRNT_2
				.elseif [ebx].labels.flgXlbl & ALP_QDRNT_2
					mov [ebx].labels.flgXlbl,ALP_QDRNT_1
				.elseif [ebx].labels.flgXlbl & ALP_QDRNT_1
					mov [ebx].labels.flgXlbl,ALP_CENTER_TOP
				.elseif [ebx].labels.flgXlbl & ALP_CENTER_TOP
					mov [ebx].labels.flgXlbl,ALP_QDRNT_0
				.elseif [ebx].labels.flgXlbl & ALP_QDRNT_0
					mov [ebx].labels.flgXlbl,ALP_QDRNT_3
				.elseif [ebx].labels.flgXlbl & ALP_QDRNT_3
					mov [ebx].labels.flgXlbl,ALP_CENTER_BOTTOM
				.endif
				invoke InvalidateRect,hWnd,0,0
				xor eax,eax
				ret
			.endif
			.if rv(ApproxEqual,@fSlv8([ebx].metrics.yAxe+[ebx].metrics.rectf.x),pointd.x,FP8(1.0))
				.if [ebx].labels.flgYlbl & ALP_CENTER_LEFT
					mov [ebx].labels.flgYlbl,ALP_QDRNT_1
				.elseif [ebx].labels.flgYlbl & ALP_QDRNT_1
					mov [ebx].labels.flgYlbl,ALP_QDRNT_0
				.elseif [ebx].labels.flgYlbl & ALP_QDRNT_0
					mov [ebx].labels.flgYlbl,ALP_CENTER_RIGHT
				.elseif [ebx].labels.flgYlbl & ALP_CENTER_RIGHT
					mov [ebx].labels.flgYlbl,ALP_QDRNT_3
				.elseif [ebx].labels.flgYlbl & ALP_QDRNT_3
					mov [ebx].labels.flgYlbl,ALP_QDRNT_2
				.elseif [ebx].labels.flgYlbl & ALP_QDRNT_2
					mov [ebx].labels.flgYlbl,ALP_CENTER_LEFT
				.endif
				invoke InvalidateRect,hWnd,0,0
			.endif
			assume ebx: nothing
		.endif
	.else
		invoke DefWindowProc,hWnd,uMsg,wParam,lParam
		ret
	.endif

	xor eax,eax
	ret
	
WndProc endp


Unit2ClientPoint proc uses ebx pPointD: ptr PointD,pAxesInfo: ptr AXES_INFO,pPoint: ptr POINT
	
	mov ebx,pAxesInfo
	assume ebx: ptr AXES_INFO
	mov edx,pPointD
	
	fSlv [edx].PointD.x = ([edx].PointD.x - [ebx].view.xMin)*[ebx].metrics.mmpux + [ebx].metrics.rectf.x
	fSlv [edx].PointD.y = ([edx].PointD.y + [ebx].view.yMax)*[ebx].metrics.mmpuy + [ebx].metrics.rectf.y

	invoke mm2pix,edx,pPoint
	
	assume ebx: nothing
	ret
	
Unit2ClientPoint endp



ClientPoint2Unit proc uses ebx x:DWORD,y:DWORD,pAxesInfo: ptr AXES_INFO,pPointD: ptr PointD

	mov ebx,pAxesInfo
	assume ebx: ptr AXES_INFO
	
	;/* pointf  to millimeters */
	invoke pix2mm,x,y,pPointD
	mov edx,pPointD
	
	fSlv [edx].PointD.x = [ebx].view.xMin + ([edx].PointD.x - [ebx].metrics.rectf.x)/[ebx].metrics.mmpux
	fSlv [edx].PointD.y = [ebx].view.yMax - ([edx].PointD.y - [ebx].metrics.rectf.y)/[ebx].metrics.mmpuy
	
	mov eax,1
	assume ebx: nothing	
	ret
	
ClientPoint2Unit endp

;/* returns client size in millimerters */
GdipGetClientRectF proc hWnd:HWND,pRectF:ptr RectF
LOCAL pixpincHorz:SDWORD,pixpincVert:SDWORD
LOCAL hdc:HDC

	mov hdc,rv(GetDC,0)
	invoke GetClientRect,hWnd,pRectF
	mov pixpincHorz,rv(GetDeviceCaps,hdc,LOGPIXELSX)
	mov pixpincVert,rv(GetDeviceCaps,hdc,LOGPIXELSY)
	invoke ReleaseDC,0,hdc
	
	mov edx,pRectF
	fSlv [edx].RectF._Width = SDWORD ptr [edx].RectF._Width * 25.4 / pixpincHorz
	fSlv [edx].RectF.Height = SDWORD ptr [edx].RectF.Height * 25.4 / pixpincVert 
	ret
	
GdipGetClientRectF endp

pix2mm proc x:DWORD,y:DWORD,pPointD: ptr PointD
LOCAL pixpincHorz:SDWORD,pixpincVert:SDWORD
LOCAL hdc:HDC

	mov hdc,rv(GetDC,0)
	mov pixpincHorz,rv(GetDeviceCaps,hdc,LOGPIXELSX)
	mov pixpincVert,rv(GetDeviceCaps,hdc,LOGPIXELSY)
	invoke ReleaseDC,0,hdc
	
	mov edx,pPointD
	
	fSlv [edx].PointD.x = x * 25.4 / pixpincHorz
	fSlv [edx].PointD.y = y * 25.4 / pixpincVert
	
	ret
	
pix2mm endp

mm2pix proc pPointD: ptr PointD,pPoint: ptr POINT
LOCAL pixpincHorz:SDWORD,pixpincVert:SDWORD
LOCAL hdc:HDC

	mov hdc,rv(GetDC,0)
	mov pixpincHorz,rv(GetDeviceCaps,hdc,LOGPIXELSX)
	mov pixpincVert,rv(GetDeviceCaps,hdc,LOGPIXELSY)
	invoke ReleaseDC,0,hdc
	
	mov edx,pPointD
	mov eax,pPoint
	
	fSlv [edx].POINT.x = [eax].PointD.x * 0.03937 * pixpincHorz
	fSlv [edx].POINT.y = [eax].PointD.y * 0.03937 * pixpincVert
	
	ret
	
mm2pix endp

ApproxEqual proc fValue1:REAL8,fValue2:REAL8,fRange:REAL8
	
	xor eax,eax
	fSlv = abs( fValue1 - fValue2 )
	fSlv = fRange
	fcomip st,st(1)
	fstp st
	setae al	
	ret
	
ApproxEqual endp

calc_metrics proc uses ebx hWnd:HWND,pAxesInfo:ptr AXES_INFO

	mov ebx,pAxesInfo
	assume ebx: ptr AXES_INFO

	.if fGT([ebx].view.xMin,[ebx].view.xMax,[esp-1])
		xchg8 [ebx].view.xMin,[ebx].view.xMax
	.endif
	
	.if fGT([ebx].view.yMin,[ebx].view.yMax,[esp-1])
		xchg8 [ebx].view.yMin,[ebx].view.yMax
	.endif
	
	invoke GdipGetClientRectF,hWnd,ADDR [ebx].metrics.rectf
	
	fSlv [ebx].metrics.rectf.x = [ebx].metrics.rectf.x + [ebx].labels.frame.left
	fSlv [ebx].metrics.rectf.y = [ebx].metrics.rectf.y + [ebx].labels.frame.top
	fSlv [ebx].metrics.rectf._Width = [ebx].metrics.rectf._Width - ([ebx].labels.frame.left+[ebx].labels.frame.right)
	fSlv [ebx].metrics.rectf.Height = [ebx].metrics.rectf.Height - ([ebx].labels.frame.top + [ebx].labels.frame.bottom)

	fSlv [ebx].metrics.mmpux = [ebx].metrics.rectf._Width/([ebx].view.xMax-[ebx].view.xMin)
	fSlv [ebx].metrics.mmpuy = [ebx].metrics.rectf.Height/([ebx].view.yMax-[ebx].view.yMin)
	fSlv [ebx].metrics.xOrgin = [ebx].metrics.rectf.x - [ebx].view.xMin * [ebx].metrics.mmpux
	fSlv [ebx].metrics.yOrgin = [ebx].view.yMax * [ebx].metrics.mmpuy + [ebx].metrics.rectf.y

	.if fGE([ebx].view.xMax,0,[esp-1]) && fLE([ebx].view.xMin,0,[esp-2])
		fSlv [ebx].metrics.yAxe = abs([ebx].view.xMin)*[ebx].metrics.mmpux
	.elseif DWORD ptr [ebx].view.xMax+4 & 80000000h
		fSlv [ebx].metrics.yAxe = [ebx].metrics.rectf._Width
	.else
		fSlv [ebx].metrics.yAxe = 0
	.endif
	
	.if fGE([ebx].view.yMax,0,[esp-1]) && fLE([ebx].view.yMin,0,[esp-2])
		fSlv [ebx].metrics.xAxe = ([ebx].view.yMax-[ebx].view.yMin)*[ebx].metrics.mmpuy - abs([ebx].view.yMin)*[ebx].metrics.mmpuy
	.elseif DWORD ptr [ebx].view.yMax+4 & 80000000h
		fSlv [ebx].metrics.xAxe = 0
	.else
		fSlv [ebx].metrics.xAxe = [ebx].metrics.rectf.Height
	.endif
	
	assume ebx: ptr nothing
	
	ret
	
calc_metrics endp

scale_axes proc uses ebx pAxesInfo: ptr AXES_INFO,xMultipleOf:REAL8,yMultipleOf:REAL8,nMarksX:DWORD,nMarksY:DWORD
	
	mov ebx,pAxesInfo
	assume ebx: ptr AXES_INFO
	
	.if !nMarksX || !nMarksY
		xor eax,eax
		ret
	.endif
	
	fSlv [ebx].labels.xMarkDist = trunc(([ebx].view.xMax - [ebx].view.xMin)/xMultipleOf/nMarksX)*xMultipleOf
	fSlv [ebx].labels.yMarkDist = trunc(([ebx].view.yMax - [ebx].view.yMin)/yMultipleOf/nMarksY)*yMultipleOf
	
	.if !DWORD ptr [ebx].labels.xMarkDist && !DWORD ptr [ebx].labels.xMarkDist+4
		fSlv [ebx].labels.xMarkDist = xMultipleOf
	.endif
	
	.if !DWORD ptr [ebx].labels.yMarkDist && !DWORD ptr [ebx].labels.yMarkDist+4
		fSlv [ebx].labels.yMarkDist = yMultipleOf
	.endif	
	
	assume ebx: nothing
	
	ret
		
scale_axes endp

plot proc uses ebx esi edi graphics:PVOID,pen:PVOID,nPoints:DWORD,pAxesInfo: ptr AXES_INFO,pfnCallBack:PVOID
LOCAL pitch:REAL8
LOCAL x:REAL8,y:REAL8

	mov ebx,pAxesInfo
	assume ebx: ptr AXES_INFO
	
	.if !nPoints
		xor eax,eax
		ret
	.elseif nPoints < 2
		mov nPoints,2
	.endif

	fSlv [ebx].metrics.mmpux = [ebx].metrics.rectf._Width/([ebx].view.xMax-[ebx].view.xMin)
	fSlv [ebx].metrics.mmpuy = [ebx].metrics.rectf.Height/([ebx].view.yMax-[ebx].view.yMin)
	fSlv [ebx].metrics.xOrgin = [ebx].metrics.rectf.x - [ebx].view.xMin * [ebx].metrics.mmpux
	fSlv [ebx].metrics.yOrgin = [ebx].view.yMax * [ebx].metrics.mmpuy + [ebx].metrics.rectf.y

	fSlv pitch =  ([ebx].view.xMax - [ebx].view.xMin) / (nPoints-1)
	d2d x,[ebx].view.xMin
	mov edx,nPoints
	lea edx,[edx*SIZEOF PointF]
	mov edi,alloc(edx)
	assume edi: ptr PointF
	xor esi,esi
	.while esi < nPoints
		fSlv [edi+esi*SIZEOF PointF].x = [ebx].metrics.xOrgin + x*[ebx].metrics.mmpux
		
		mov edx,pfnCallBack
		assume edx: ptr CB_FNC_VALUE
		invoke edx,x,ADDR y
		fSlv [edi+esi*SIZEOF PointF].y = [ebx].metrics.yOrgin - y * [ebx].metrics.mmpuy 
		
		fSlv x = x + pitch
		inc esi
	.endw
	
	invoke GdipDrawLines,graphics,pen,edi,nPoints
	
	free edi
	
	assume edx: nothing
	assume ebx: nothing
	mov eax,1
	ret
	
plot endp

draw_label proc uses ebx IsYAxe:DWORD,path:PVOID,fontFam:PVOID,strFormat:PVOID,xPos:REAL4,yPos:REAL4,Value:REAL8,pAxesInfo: ptr AXES_INFO
LOCAL wsz[64]:WORD 
LOCAL rectf:RectF
LOCAL pathTmp:PVOID
LOCAL cc:DWORD
LOCAL chrsWidth:REAL4
LOCAL qdrnt:DWORD
LOCAL flg:DWORD
LOCAL fSlvTLS()

	mov ebx,pAxesInfo
	assume ebx: ptr AXES_INFO

	mov rectf.x,0
	mov rectf.y,0
	mov rectf._Width,0
	mov rectf.Height,0
	mov qdrnt,0
	
	invoke GdipCreatePath,FillModeAlternate,ADDR pathTmp
	.if !IsYAxe
		mov edx,[ebx].labels.pwszFormatX
	.else
		mov edx,[ebx].labels.pwszFormatY
	.endif
	mov cc,rv(crt_swprintf,ADDR wsz,edx,@fSlv8(Value))
	invoke GdipAddPathString,pathTmp,ADDR wsz, cc,fontFam,0,[ebx].labels.emHeight,ADDR rectf,strFormat
	invoke GdipGetPathWorldBounds,pathTmp,ADDR rectf,0,0
	invoke GdipDeletePath,pathTmp
	fSlv chrsWidth = rectf._Width
	
	;/* draw label for x-axes */
	.if !IsYAxe
		m2m flg,[ebx].labels.flgXlbl
		.if [ebx].labels.flgXlbl&ALP_CENTER_BOTTOM
	@@:		fSlv rectf.x = xPos - chrsWidth*0.5 - rectf.x
			fSlv rectf.y = yPos + [ebx].labels.markSize*1.5 
		.elseif [ebx].labels.flgXlbl&ALP_CENTER_TOP
			fSlv rectf.x = xPos - chrsWidth*0.5 - rectf.x
			fSlv rectf.y = yPos - [ebx].labels.markSize*1.5 - [ebx].labels.emHeight
		.elseif [ebx].labels.flgXlbl&(ALP_QDRNT_0 or  ALP_QDRNT_1 or ALP_QDRNT_2 or ALP_QDRNT_3)
			mov qdrnt,1
		.else
			jmp @B
	@@:	.endif
		
		.if [ebx].labels.flgXlbl&ALP_USE_X_OFFSETS
			fSlv rectf.x = rectf.x + [ebx].labels.xLblOffX
			fSlv rectf.y = rectf.y + [ebx].labels.xLblOffY
		.endif
	.else
		m2m flg,[ebx].labels.flgYlbl
		.if [ebx].labels.flgYlbl&ALP_CENTER_RIGHT
			fSlv rectf.x = xPos + [ebx].labels.markSize*1.5 - rectf.x
			fSlv rectf.y = yPos - [ebx].labels.emHeight*0.5 - rectf.y*0.5
		.elseif [ebx].labels.flgYlbl&ALP_CENTER_LEFT
	@@:		fSlv rectf.x = xPos - [ebx].labels.markSize*1.5 - chrsWidth - rectf.x
			fSlv rectf.y = yPos - [ebx].labels.emHeight*0.5 - rectf.y*0.5
		.elseif [ebx].labels.flgYlbl&(ALP_QDRNT_0 or  ALP_QDRNT_1 or ALP_QDRNT_2 or ALP_QDRNT_3)
			mov qdrnt,1
		.else
			jmp @B	
	@@:	.endif
		
		.if [ebx].labels.flgYlbl&ALP_USE_Y_OFFSETS
			fSlv rectf.x = rectf.x + [ebx].labels.yLblOffX
			fSlv rectf.y = rectf.y + [ebx].labels.yLblOffY
		.endif
	.endif

	.if qdrnt
		.if flg&ALP_QDRNT_0
			fSlv rectf.x = xPos + [ebx].labels.markSize
			fSlv rectf.y = yPos - [ebx].labels.markSize - [ebx].labels.emHeight
		.elseif flg&ALP_QDRNT_1
			fSlv rectf.x = xPos - chrsWidth - rectf.x - [ebx].labels.markSize
			fSlv rectf.y = yPos - [ebx].labels.markSize - [ebx].labels.emHeight
		.elseif flg&ALP_QDRNT_2
			fSlv rectf.x = xPos - chrsWidth - rectf.x - [ebx].labels.markSize
			fSlv rectf.y = yPos + [ebx].labels.markSize 
		.elseif flg&ALP_QDRNT_3
			fSlv rectf.x = xPos + [ebx].labels.markSize
			fSlv rectf.y = yPos + [ebx].labels.markSize
		.endif
	.endif
	
	mov rectf._Width,0
	mov rectf.Height,0
	fn GdipAddPathString,path,ADDR wsz,cc,fontFam,0,[ebx].labels.emHeight,ADDR rectf,strFormat
	
	assume ebx: nothing
	mov eax,1
	ret
	
draw_label endp

draw_axes proc uses ebx graphics:PVOID,pAxesInfo: ptr AXES_INFO
LOCAL mmpux:REAL8,mmpuy:REAL8
LOCAL xCenter:REAL8,yCenter:REAL8
LOCAL xPos:REAL4,xVal:REAL8
LOCAL yPos:REAL4,yVal:REAL8
LOCAL matrix:REAL4
LOCAL fontFam:PVOID
LOCAL strFormat:PVOID
LOCAL path:PVOID
LOCAL rectf:RectF
LOCAL wsz[64]:WORD
LOCAL fSlvTLS()
	
	mov ebx,pAxesInfo
	assume ebx: ptr AXES_INFO
	
	invoke GdipCreateMatrix,ADDR matrix
	invoke GdipGetWorldTransform,graphics,matrix
	invoke GdipTranslateWorldTransform,graphics,[ebx].metrics.rectf.x,[ebx].metrics.rectf.y,MatrixOrderAppend
	
	fn GdipCreateFontFamilyFromName,[ebx].labels.pwszFontFam,0,ADDR fontFam
	invoke GdipCreateStringFormat,0,LANG_NEUTRAL,ADDR strFormat
	invoke GdipCreatePath,FillModeAlternate,ADDR path

	mov rectf._Width,0
	mov rectf.Height,0
	
	;/* draw y-axis */
	invoke GdipDrawLine,graphics,[ebx].labels.pen,[ebx].metrics.yAxe,0,[ebx].metrics.yAxe,[ebx].metrics.rectf.Height
	
	;/* draw x-axis */
	invoke GdipDrawLine,graphics,[ebx].labels.pen,0,[ebx].metrics.xAxe,[ebx].metrics.rectf._Width,[ebx].metrics.xAxe
	
	;/* draw x-axis scale */
	.if DWORD ptr [ebx].labels.xMarkDist || DWORD ptr [ebx].labels.xMarkDist+4
		.if  rv(ApproxEqual,@fSlv8(abs(mod([ebx].view.xMin,[ebx].labels.xMarkDist))),[ebx].labels.xMarkDist,@fSlv8(1.E-3*[ebx].labels.xMarkDist))
			fSlv xVal = [ebx].view.xMin
			fSlv xPos = 0
		.else
			.if DWORD ptr [ebx].view.xMin+4 & 80000000h
				fSlv xVal = [ebx].view.xMin - mod([ebx].view.xMin,[ebx].labels.xMarkDist)
				fSlv xPos = -mod([ebx].view.xMin,[ebx].labels.xMarkDist)*[ebx].metrics.mmpux
			.else
				fSlv xVal = [ebx].view.xMin + ([ebx].labels.xMarkDist-mod([ebx].view.xMin,[ebx].labels.xMarkDist))
				fSlv xPos = ([ebx].labels.xMarkDist-mod([ebx].view.xMin,[ebx].labels.xMarkDist))*[ebx].metrics.mmpux
			.endif
		.endif
		
		.while 1
			.break .if fGT(xVal,@fSlv8([ebx].view.xMax+[ebx].labels.xMarkDist*1.E-3))

			;/* do not draw at point of origin */
			.if !rv(ApproxEqual,xVal,FP8(0.0),@fSlv8([ebx].labels.xMarkDist*1.E-3))
				invoke GdipDrawLine,graphics,[ebx].labels.pen,xPos,@fSlv4([ebx].metrics.xAxe-[ebx].labels.markSize),xPos,@fSlv4([ebx].metrics.xAxe+[ebx].labels.markSize)
				invoke draw_label,0,path,fontFam,strFormat,xPos,[ebx].metrics.xAxe,xVal,ebx				
			.endif
			fSlv xVal = xVal + [ebx].labels.xMarkDist
			fSlv xPos = xPos + [ebx].labels.xMarkDist * [ebx].metrics.mmpux
		.endw
	.endif
	
	;/* draw y-axis scale */
	.if DWORD ptr [ebx].labels.yMarkDist || DWORD ptr [ebx].labels.yMarkDist+4
		.if rv(ApproxEqual,@fSlv8(abs(mod([ebx].view.yMin,[ebx].labels.yMarkDist))),[ebx].labels.yMarkDist,@fSlv8(1.E-3*[ebx].labels.yMarkDist))
			fSlv yVal = [ebx].view.yMin
			fSlv yPos = ([ebx].view.yMax - [ebx].view.yMin) * [ebx].metrics.mmpuy
		.else 
			.if DWORD ptr [ebx].view.yMin+4 & 80000000h
				fSlv yVal = [ebx].view.yMin - mod([ebx].view.yMin,[ebx].labels.yMarkDist)
				fSlv yPos = ([ebx].view.yMax-[ebx].view.yMin)*[ebx].metrics.mmpuy + mod([ebx].view.yMin,[ebx].labels.yMarkDist)*[ebx].metrics.mmpuy
			.else
				fSlv yVal = [ebx].view.yMin + ([ebx].labels.yMarkDist-mod([ebx].view.yMin,[ebx].labels.yMarkDist))
				fSlv yPos = ([ebx].view.yMax-[ebx].view.yMin)*[ebx].metrics.mmpuy - ([ebx].labels.yMarkDist-mod([ebx].view.yMin,[ebx].labels.yMarkDist))*[ebx].metrics.mmpuy
			.endif
		.endif

		.while 1
			.break .if fGT(yVal,@fSlv8([ebx].view.yMax+[ebx].labels.yMarkDist*1.E-3),[esp-1])
			
			;/* do not draw at point of origin */
			.if !rv(ApproxEqual,yVal,FP8(0.0),@fSlv8([ebx].labels.yMarkDist*1.E-3))
				invoke GdipDrawLine,graphics,[ebx].labels.pen,@fSlv4([ebx].metrics.yAxe-[ebx].labels.markSize),yPos,@fSlv4([ebx].metrics.yAxe+[ebx].labels.markSize),yPos
				invoke draw_label,1,path,fontFam,strFormat,[ebx].metrics.yAxe,yPos,yVal,ebx
			.endif
			
			fSlv yVal = yVal + [ebx].labels.yMarkDist
			fSlv yPos = yPos - [ebx].labels.yMarkDist  * [ebx].metrics.mmpuy
		.endw
	.endif
	
	.if [ebx].labels.penTxt
		invoke GdipDrawPath,graphics,[ebx].labels.penTxt,path
	.endif
	
	.if [ebx].labels.brushTxt
		invoke GdipFillPath,graphics,[ebx].labels.brushTxt,path
	.endif
	
	invoke GdipDeleteFontFamily,fontFam
	invoke GdipDeleteStringFormat,strFormat
	invoke GdipDeletePath,path	
	
	invoke GdipSetWorldTransform,graphics,matrix
	invoke GdipDeleteMatrix,matrix
	
	assume ebx: nothing
	mov eax,1
	ret
	
draw_axes endp
fSlvStatistics
end DllMain