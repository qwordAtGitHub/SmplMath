.nolist
include masm32rt.inc
.686p
.mmx
.xmm

include gdip.inc
include gdiplus.inc
includelib gdiplus.lib

include \macros\smplmath\math.inc
include \macros\unicode.inc

include ..\FncDrawCtrl\FncDrawCtrl.inc
includelib FncDrawCtrl.lib

;/* register constant e */
fSlvRegConst e,2.718281828

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

CallBack1 proto x:REAL8,py: ptr REAL8
CallBack2 proto x:REAL8,py: ptr REAL8
CallBack3 proto x:REAL8,py: ptr REAL8

WND_DATA struct
	hCtrl1	HWND	?
	hCtrl2	HWND	?
WND_DATA ends

.code
main proc
LOCAL wc:WNDCLASSEX
LOCAL msg:MSG
LOCAL gsi:GdiplusStartupInput
LOCAL gtkn:ptr ULONG
LOCAL rect:RECT
LOCAL fSlvTLS()
	
	mov gsi.GdiplusVersion,1
	mov gsi.DebugEventCallback,0
	mov gsi.SuppressBackgroundThread,0
	mov gsi.SuppressExternalCodecs,0
	invoke GdiplusStartup,ADDR gtkn,ADDR gsi,0

	mov wc.hInstance,rv(GetModuleHandle,0)
	mov wc.cbSize,SIZEOF wc
	mov wc.style,CS_HREDRAW or CS_VREDRAW or CS_DBLCLKS or CS_SAVEBITS
	mov wc.lpfnWndProc,WndProc
	mov wc.cbClsExtra,0
	mov wc.cbWndExtra,SIZEOF WND_DATA
	mov wc.lpszClassName,chr$("SomeClass32")
	mov wc.hIcon,rv(LoadIcon,NULL,IDI_APPLICATION)
	mov wc.hIconSm,eax
	mov wc.hCursor,rv(LoadCursor,NULL,IDC_ARROW)
	mov wc.lpszMenuName,0
	mov wc.hbrBackground,rv(GetStockObject,WHITE_BRUSH)
	invoke RegisterClassEx,ADDR wc
	 
	fSlv rect.left   = 0.125 * rv(GetSystemMetrics,SM_CXSCREEN)
	fSlv rect.top    = 0.125 * rv(GetSystemMetrics,SM_CYSCREEN)
	fSlv rect.right  = 0.75  * rv(GetSystemMetrics,SM_CXSCREEN)
	fSlv rect.bottom = 0.75  * rv(GetSystemMetrics,SM_CYSCREEN)
	
	mov edi,rv(CreateWindowEx,0,wc.lpszClassName,chr$("SmplMathDemo"),WS_SYSMENU or WS_SIZEBOX or WS_VISIBLE or WS_MINIMIZEBOX or WS_MAXIMIZEBOX or WS_CLIPCHILDREN,rect.left,rect.top,rect.right,rect.bottom,0,0,wc.hInstance,0)
	invoke UpdateWindow,edi
	invoke ShowWindow,edi,SW_SHOWNORMAL	
	
	.while 1
		invoke GetMessage,ADDR msg,0,0,0
		.break .if !eax || eax == -1
		invoke TranslateMessage,ADDR msg
		invoke DispatchMessage,ADDR msg		
	.endw
	push eax
	invoke GdiplusShutdown,gtkn
	pop eax
	invoke ExitProcess,0
	ret
	
main endp


WndProc proc uses ebx esi edi hWnd:HWND,uMsg:UINT,wParam:WPARAM,lParam:LPARAM
LOCAL fncdscptr:FNC_DSCPTR
LOCAL x:REAL8,y:REAL8
LOCAL cv:CURR_VIEW
LOCAL li:LABEL_INFOA
LOCAL graphics:PVOID

	.if uMsg == WM_CLOSE
		invoke PostQuitMessage,0
	.elseif uMsg == WM_CREATE
		;fn LoadLibrary,"FncDrwCtl.dll"
		
		;/*-------------------------------------*/
		;/* create a control for showing graphs */
		;/*-------------------------------------*/
		mov edi,rv(CreateWindowEx,0,"FncDrawCtrl32",0,WS_VISIBLE or WS_CHILD or WS_BORDER,10,50,500,500,hWnd,0,rv(GetWindowLong,hWnd,GWL_HINSTANCE),0)
		invoke SetWindowLong,hWnd,WND_DATA.hCtrl1,edi

		;/*-------------------------------------*/
		;/* request mouse notification          */
		;/*-------------------------------------*/
		invoke SendMessage,edi,FDCM_SET_STYLE,FDCS_MOUSE_NOTIFICATION,0

		;/*-------------------------------------*/
		;/* overwrite default control settings  */
		;/*-------------------------------------*/
		mov li.flg,PEN__ARGB or TXTBRUSH__ARGB
		mov li.pen,0ff0f0fffh
		mov li.penTxt,0
		mov li.brushTxt,0ff000000h
		m2m li.emHeight,FP4(4.0)
		mov li.pszFontFam,chr$("Arial")
		mov li.pszFormatX,chr$("%.2f")
		mov li.pszFormatY,chr$("%.1f")
		mov li.flgXlbl,ALP_X_DEFAULT
		mov li.flgYlbl,ALP_Y_DEFAULT
		m2m li.markSize,FP4(1.5)
		m2m li.pen_width,FP4(0.75)
		m2m li.penTxt_width,FP4(0.75)
		d2d li.xMultipleOf,FP8(3.141)
		d2d li.yMultipleOf,FP8(1.0)
		mov li.nxMarks,5
		mov li.nyMarks,4
		m2m li.frame.top,FP4(8.0)
		m2m li.frame.left,FP4(8.0)
		m2m li.frame.right,FP4(8.0)
		m2m li.frame.bottom,FP4(8.0)
		mov li.xLblOffX,0
		mov li.xLblOffY,0
		mov li.yLblOffX,0
		mov li.yLblOffY,0
		invoke SendMessage,edi,FDCM_SET_LABEL_INFOA,ADDR li,0

		;/*-------------------------------------*/
		;/* set view                            */
		;/*-------------------------------------*/
		d2d cv.xMax,FP8(9.42477)
		d2d cv.xMin,FP8(-3.141)
		d2d cv.yMax,FP8(3.0)
		d2d cv.yMin,FP8(-1.0)		
		invoke SendMessage,edi,FDCM_SET_CURR_VIEW,ADDR cv,0

		;/*-------------------------------------*/
		;/* add functions to draw               */
		;/*-------------------------------------*/
		mov fncdscptr.flg,PEN__ARGB
		mov fncdscptr.nPoints,500
		mov fncdscptr.pCallBack,OFFSET CallBack1
		mov fncdscptr.pen,ColorsSalmon
		m2m fncdscptr._width,FP4(0.5)
		invoke SendMessage,edi,FDCM_ADD_FUNCTION,ADDR fncdscptr,0
	
		mov fncdscptr.pCallBack,OFFSET CallBack2
		mov fncdscptr.pen,ColorsDeepSkyBlue
		invoke SendMessage,edi,FDCM_ADD_FUNCTION,ADDR fncdscptr,0

		;/*-------------------------------------*/
		;/* create a second graph-control and   */
		;/* keep default settings               */
		;/*-------------------------------------*/		
		mov edi,rv(CreateWindowEx,0,"FncDrawCtrl32",0,WS_VISIBLE or WS_CHILD or WS_BORDER ,520,50,500,500,hWnd,0,rv(GetWindowLong,hWnd,GWL_HINSTANCE),0)
		invoke SetWindowLong,hWnd,WND_DATA.hCtrl2,edi
		invoke SendMessage,edi,FDCM_SET_STYLE,FDCS_MOUSE_NOTIFICATION,0
		
		mov fncdscptr.pCallBack,OFFSET CallBack3
		mov fncdscptr.pen,ColorsMediumSpringGreen
		invoke SendMessage,edi,FDCM_ADD_FUNCTION,ADDR fncdscptr,0
		invoke SendMessage,edi,FDCM_SET_BKCOLOR,ColorsAliceBlue,0
		
	.elseif uMsg == WM_MOUSEWHEEL

		;/*-------------------------------------*/
		;/* redirect WM_MOUSEWHEEL to controls  */
		;/*-------------------------------------*/
		invoke fdcRedirectWmMouseWheel,hWnd,wParam,lParam
		
	.elseif uMsg == WM_NOTIFY
	
		;/*-------------------------------------*/
		;/* print mouse position in graphs      */
		;/* world units                         */
		;/*-------------------------------------*/
		
		mov ebx,rv(GetWindowLong,hWnd,WND_DATA.hCtrl1)
		mov edi,rv(GetWindowLong,hWnd,WND_DATA.hCtrl2)
		mov edx,lParam
		.if [edx].FDC_NMHDR.nmhdr.code == FDCNM_MOUSE_MOVE && ebx == [edx].FDC_NMHDR.nmhdr.hwndFrom
			d2d x,[edx].FDC_NMHDR.pointd.x
			d2d y,[edx].FDC_NMHDR.pointd.y
			mov edi,rv(GetDC,hWnd)
			invoke GdipCreateFromHDC,edi,ADDR graphics
			invoke GdipGraphicsClear,graphics,ColorsWhite
			invoke GdipDeleteGraphics,graphics
			invoke TextOut,edi,10,0,real8$(x),len(real8$(x))
			invoke TextOut,edi,10,20,real8$(y),len(real8$(y))
			invoke ReleaseDC,hWnd,edi
		.elseif [edx].FDC_NMHDR.nmhdr.code == FDCNM_MOUSE_MOVE && edi == [edx].FDC_NMHDR.nmhdr.hwndFrom
			d2d x,[edx].FDC_NMHDR.pointd.x
			d2d y,[edx].FDC_NMHDR.pointd.y
			mov edi,rv(GetDC,hWnd)
			invoke GdipCreateFromHDC,edi,ADDR graphics
			invoke GdipGraphicsClear,graphics,ColorsWhite
			invoke GdipDeleteGraphics,graphics
			invoke TextOut,edi,520,0,real8$(x),len(real8$(x))
			invoke TextOut,edi,520,20,real8$(y),len(real8$(y))
			invoke ReleaseDC,hWnd,edi
		.endif
	.else
		invoke DefWindowProc,hWnd,uMsg,wParam,lParam
		ret
	.endif

	xor eax,eax
	ret
	
WndProc endp

CallBack1 proc x:REAL8,py: ptr REAL8

	mov edx,py
	fSlv REAL8 ptr [edx] = limit(tan(x),10,-10)
	
	ret
	
CallBack1 endp

CallBack2 proc x:REAL8,py: ptr REAL8
	
	mov edx,py
	
	;/* register expression with one argument (=arg1) */
	fSlvRegExpr <Tri>,1,arg1^(-2)*cos(arg1*x)+(arg1+2)^-2*cos((arg1+2)*x)+(arg1+4)^-2*cos((arg1+4)*x)

	;/* fourier series: triangular pulse */
	fSlv REAL8 ptr [edx] = {i2,r4} Tri(1)+Tri(7)+Tri(13);+Tri(19)+Tri(25)\
;				+Tri(31)+Tri(37)+Tri(43)+Tri(49)+Tri(55)\
;				+Tri(61)+Tri(67)+Tri(73)+Tri(79)+Tri(85)\
;				+Tri(91)+Tri(97)+Tri(103)+Tri(109)

	ret
	
CallBack2 endp

CallBack3 proc x:REAL8,py: ptr REAL8
	
	mov edx,py
	
	fSlv x = x - 2
	
	fSlv REAL8 ptr [edx] = 3*e^(-1.7*abs(x))*sin(20*x) + 2
	
		
	ret
	
CallBack3 endp
fSlvStatistics
end main
  