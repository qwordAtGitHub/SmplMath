comment *
  current implemented functions:
    sin(x)
    cos(x)
    tan(x)
    atan(x)
    asin(x)
    acos(x)
    exp(x)					; e^x
    ln(x)					; base=e
    log(x)					; base=10
    logbx(base,x)
    root(x,n)				; n=1,2,3,...
    sqrt(x)
    trunc(x)
    mod(a,b)
    abs(x)
    limit(x,max,min)
    deg(x)					; rad->deg
    rad(x)					; deg->rad
    sgn(x)					; returns signe of x: -1 or +1
    min(a,b)
    max(a,b)
*

IFNDEF __MODULE__
fpu_const SEGMENT PAGE READONLY

	align 16
	fpu_const_r4_one 		REAL4 1.0
	fpu_const_r4_1div2		REAL4 0.5
	fpu_const_r8_pi_div_2	REAL8 1.57079632679489661923132
	
	align 16
	fpu_const_r10_log 	LABEL REAL10
						db 099h,0F7h,0CFh,0FBh,084h,09Ah,020h,09Ah,0FDh,03Fh		; = 1/log2(10)
	align 16
	fpu_const_r10_ln	LABEL REAL10
						db 0ACh,079h,0CFh,0D1h,0F7h,017h,072h,0B1h,0FEh,03Fh		; = 1/log2(e)
	align 16
	fpu_const_r10_2pi	LABEL REAL10
						db 035h,0C2h,068h,021h,0A2h,0DAh,00Fh,0C9h,001h,040h		; = 2*pi
	align 16
	fpu_const_r10_r2d	LABEL REAL10
						db 0C3h,0BDh,00Fh,01Eh,0D3h,0E0h,02Eh,0E5h,004h,040h		; 180/pi
	align 16
	fpu_const_r10_d2r	LABEL REAL10
						db 0AEh,0C8h,0E9h,094h,012h,035h,0FAh,08Eh,0F9h,03Fh		; pi/180
	align 16
	fpu_const_r8_log 	LABEL REAL8
						db 0FFh,079h,09Fh,050h,013h,044h,0D3h,03Fh					; = 1/log2(10)
	fpu_const_r4_log 	LABEL REAL4
						db 09Bh,020h,09Ah,03Eh										; = 1/log2(10)
	align 16
	fpu_const_r8_ln		LABEL REAL8
						db 0EFh,039h,0FAh,0FEh,042h,02Eh,0E6h,03Fh					; = 1/log2(e)
	fpu_const_r4_ln		LABEL REAL4
						db 018h,072h,031h,03Fh										; = 1/log2(e)
	align 16
	fpu_const_r8_2pi	LABEL REAL8
						db 018h,02Dh,044h,054h,0FBh,021h,019h,040h					; = 2*pi
	fpu_const_r4_2pi	LABEL REAL4
						db 0DBh,00Fh,0C9h,040h										; = 2*pi
	align 16
	fpu_const_r8_r2d	LABEL REAL8
						db 0F8h,0C1h,063h,01Ah,0DCh,0A5h,04Ch,040h					; 180/pi
	fpu_const_r4_r2d	LABEL REAL4
						db 0E1h,02Eh,065h,042h										; 180/pi	
	align 16
	fpu_const_r8_d2r	LABEL REAL8
						db 039h,09Dh,052h,0A2h,046h,0DFh,091h,03Fh					; pi/180
	fpu_const_r4_d2r	LABEL REAL4
						db 035h,0FAh,08Eh,03Ch										; pi/180


fpu_const ENDS

fpu_seg SEGMENT PAGE PUBLIC 'CODE'

	;---------------------------------------------
	;             st(0) = st(1)^st(0)
	;  - st(1) is poped
	;  - no additional fpu register are used
	;  - algorithm extracted from 
	;    Raymond Filiatreault fpulib
	;---------------------------------------------	
	align 16
	fpu_st1_pow_st0 proc stdcall uses eax
		LOCAL r4Tmp:REAL4
		
		fxch
	@@:	ftst
		fstsw ax
		sahf
		mov eax,0
		ja @F				; x^y = ...
		jz zero				; 0^y = 0

		fchs				;(-x)^y -> (+x)^y

		fxch				; y = {3,5,7,...} ?
		fist r4Tmp			; if y = uneven: save negativ signe
		fxch				; else: discard negativ signe -> result will always positiv
		test r4Tmp,1		;
		setnz al			;

	@@: 
		fyl2x					; ;st(0) = st(1)^st(0) = 2^(st(0)*ld(st(1)))
		fld st(0)				; 
		frndint					;
		fsub st(1),st(0)		;
		fxch					;
		f2xm1					;
		fadd fpu_const_r4_one	;
		fscale					;
		fstp st(1)
		.if eax&1			; (-x)^{3,5,7,...} => write negativ signe
			fchs			;
		.endif				;
		ret

	zero:
		fincstp
		fincstp
		ffree st(6)
		ffree st(7)
		fldz
		ret

	fpu_st1_pow_st0 endp
	
	align 16
	fpu_exp proc stdcall
		fldl2e                  ; st(0) = e^st(0) = 2^(st(0)*ld(e))
		fmulp st(1),st          ;
		fld st(0)				;
		frndint					;
		fsub st(1),st(0)		;
		fxch					;
		f2xm1					;
		fadd fpu_const_r4_one	;
		fscale					;
		fstp st(1)
		ret
	fpu_exp endp
	
	align 16
	fpu_root proc stdcall uses eax
		LOCAL iTmp:SDWORD
		LOCAL r4Tmp:REAL4
		
		xor eax,eax
		fistp iTmp
		test iTmp,1
@@:		jz @F
		fst r4Tmp
		test r4Tmp,080000000h
		setnz al
		jz @F
		fchs
@@:		fld1
		fidiv iTmp
		
		fxch
		fyl2x					;  ;st(0) = st(1)^st(0) = 2^(st(0)*ld(st(1)))
		fld st(0)				;
		frndint					;
		fsub st(1),st(0)		;
		fxch					;
		f2xm1					;
		fadd fpu_const_r4_one	;
		fscale					;
		fstp st(1)
		
		.if eax&1
			fchs
		.endif
		ret

	fpu_root endp
	
	align 16
	; preliminary version
	fpu_fit2pi proc stdcall uses eax
		LOCAL r4Tmp:REAL4

		xor eax,eax
		fst r4Tmp
		fld st
		fabs
		fld fpu_const_r10_2pi
		fcomi st,st(1)
		fstp st(1)
	@@: jae @F
		fdiv st(1),st
		fld st(1)
		test r4Tmp,080000000h
		setnz al
		.if !ZERO?
			fchs
			fxch st(2)
			fchs
			fxch st(2)
		.endif
		frndint
		fsubp st(2),st
		fxch 			; st    = 2pi/x - rndint(2pi/x)
						; st(1) = 2pi
		fst r4Tmp
		test r4Tmp,080000000h
		.if !ZERO?
			fld1
			faddp st(1),st		
		.endif
		fmulp st(1),st
		.if eax&1
			fchs
		.endif
			
		ret
	@@: fstp st
	@1:
		ret

	fpu_fit2pi endp	
	
fpu_seg ENDS

ELSE
	EXTERNDEF fpu_const_r4_one		:REAL4
	EXTERNDEF fpu_const_r4_1div2	:REAL4
	EXTERNDEF fpu_const_r8_pi_div_2	:REAL8
	EXTERNDEF fpu_const_r10_log 	:REAL10
	EXTERNDEF fpu_const_r10_ln		:REAL10
	EXTERNDEF fpu_const_r10_2pi		:REAL10
	EXTERNDEF fpu_const_r10_r2d		:REAL10
	EXTERNDEF fpu_const_r10_d2r		:REAL10
	EXTERNDEF fpu_const_r8_log 		:REAL8
	EXTERNDEF fpu_const_r4_log 		:REAL4
	EXTERNDEF fpu_const_r8_ln		:REAL8
	EXTERNDEF fpu_const_r4_ln		:REAL4
	EXTERNDEF fpu_const_r8_2pi		:REAL8
	EXTERNDEF fpu_const_r4_2pi		:REAL4
	EXTERNDEF fpu_const_r8_r2d		:REAL8
	EXTERNDEF fpu_const_r4_r2d		:REAL4
	EXTERNDEF fpu_const_r8_d2r		:REAL8
	EXTERNDEF fpu_const_r4_d2r		:REAL4
	
	fpu_st1_pow_st0 proto stdcall
	fpu_exp proto stdcall
	fpu_root proto stdcall
	fpu_fit2pi proto stdcall
ENDIF	
FNCD_VERSION	EQU 1
FNCD_NARGS		EQU 2
FNCD_NREGS		EQU 4
FNCD_USES_SSE	EQU 8

default_fnc_dscptr macro FncName:req,nArgs:=<1>,nRegs:=<0>
	fslv_fnc_&FncName&_dscptr macro flags:req
		IF flags AND FNCD_VERSION
			EXITM <1>					; compatible with version 1 ;-)
		ELSEIF flags AND FNCD_NARGS
			EXITM <&nArgs>				; number of arguments
		ELSEIF flags AND FNCD_NREGS
			EXITM <&nRegs>				; number of registers used additional to the ones used for agument passing
		ELSE
			EXITM <0>
		ENDIF
	endm
endm

type_depending_fnc_dscptr macro FncName:req,nArgs:=<1>,nRegs_r4:=<0>,nRegs_r8:=<0>,nRegs_r10:=<0>
	fslv_fnc_&FncName&_dscptr macro flags:req
		IF flags AND FNCD_VERSION
			EXITM <1>
		ELSEIF flags AND FNCD_NARGS
			EXITM <&nArgs>
		ELSEIF flags AND FNCD_NREGS
			IF fslv_lcl_real_type EQ 4
				EXITM <&nRegs_r4>
			ELSEIF fslv_lcl_real_type EQ 8
				EXITM <&nRegs_r8>
			ELSEIF fslv_lcl_real_type EQ 10
				EXITM <&nRegs_r10>
			ENDIF				
		ELSE
			EXITM <0>
		ENDIF	
	endm
endm

; root(x,n)
; n = 2,3,4,...
; e.g. root(-27,3) = -3
fslv_fnc_root macro
	call fpu_root
endm
default_fnc_dscptr <root>,2

fslv_fnc_sqrt macro
	fsqrt
endm
default_fnc_dscptr <sqrt>,1

; exp(...) = e^(...)
fslv_fnc_exp macro
	call fpu_exp
endm
default_fnc_dscptr <exp>,1

fslv_fnc_sin macro
	fsin
endm
default_fnc_dscptr <sin>,1

fslv_fnc_cos macro
	fcos
endm
default_fnc_dscptr <cos>,1

fslv_fnc_tan macro	
	fptan
	fstp st
endm
default_fnc_dscptr <tan>,1,2

fslv_fnc_abs macro
	fabs
endm
default_fnc_dscptr <abs>,1

fslv_fnc_rad macro
	IF fslv_lcl_real_type LE 8
		fmul @CatStr(<fpu_const_r>,%fslv_lcl_real_type,<_d2r>)
	ELSE
		fld @CatStr(<fpu_const_r>,%fslv_lcl_real_type,<_d2r>)
		fmulp
	ENDIF
endm
type_depending_fnc_dscptr <rad>,1,0,0,1
	
fslv_fnc_deg macro
	IF fslv_lcl_real_type LE 8
		fmul @CatStr(<fpu_const_r>,%fslv_lcl_real_type,<_r2d>)
	ELSE
		fld @CatStr(<fpu_const_r>,%fslv_lcl_real_type,<_r2d>)
		fmulp
	ENDIF	
endm
type_depending_fnc_dscptr <deg>,1,0,0,1

fslv_fnc_log macro
	fld1
	fxch
	fyl2x
	IF fslv_lcl_real_type LE 8
		fmul @CatStr(<fpu_const_r>,%fslv_lcl_real_type,<_log>)
	ELSE
		fld @CatStr(<fpu_const_r>,%fslv_lcl_real_type,<_log>)
		fmulp
	ENDIF
endm
default_fnc_dscptr <log>,1,1

fslv_fnc_ln macro
	fld1
	fxch
	fyl2x
	IF fslv_lcl_real_type LE 8
		fmul @CatStr(<fpu_const_r>,%fslv_lcl_real_type,<_ln>)
	ELSE
		fld @CatStr(<fpu_const_r>,%fslv_lcl_real_type,<_ln>)
		fmulp
	ENDIF
endm		
default_fnc_dscptr <ln>,1,1

; logbx(base,num)
fslv_fnc_logbx macro
	fld1
	fxch
	fyl2x
	fxch	 ;st(1)=log2(x), st(0) = b
	fld1
	fxch
	fyl2x    ;st(1)=log2(x), st(0) = log2(b)
	fdivp st(1),st(0)
endm
default_fnc_dscptr <logbx>,2,1

; limit(val,maximum,minimum)
; if (val>maximum) {return maximum}
; else if (val < minimum){return minimum} 
; else {return val}
fslv_fnc_limit macro
LOCAL below,above,fin
	fcomi st,st(2)
	ja below
	fstp st
	fcomi st,st(1)
	jb above
	fstp st
	jmp fin
below:
	fstp st(1)
	fstp st(1)
	jmp fin	
above:
	fstp st(1)
fin:
endm
default_fnc_dscptr <limit>,3

fslv_fnc_min macro
	fcomi st,st(1)
	.if !CARRY?
		fstp st
	.else
		fstp st(1)
	.endif
endm
default_fnc_dscptr <min>,2

fslv_fnc_max macro
	fcomi st,st(1)
	.if !CARRY?
		fstp st(1)
	.else
		fstp st
	.endif
endm
default_fnc_dscptr <max>,2

; fit2pi(x)
; transform an multiple of 2*PI to u*2*PI, 
; whereas 1 >= u >= 0 for positiv values and
; 0 >= u >= -1 for negativ values.
fslv_fnc_fit2pi macro
	call fpu_fit2pi
endm
default_fnc_dscptr <fit2pi>,1,2

; sign function
fslv_fnc_sgn macro
	fstp REAL4 ptr [esp-4]
	fld1
	.if (DWORD ptr [esp-4])&80000000h
		fchs
	.endif	
endm
default_fnc_dscptr <sgn>,1,0

; if zero: ifz(x,trueVal,falseVal)
;fslv_fnc_ifz macro
;	fstp fslv_fns_sto_r10
;	.if !(DWORD ptr fslv_fns_sto_r10) && !(DWORD ptr fslv_fns_sto_r10+4) && !((WORD ptr fslv_fns_sto_r10)&7fffh)
;		fstp st(1)
;	.else
;		fstp st
;	.endif
;endm
;default_fnc_dscptr <ifz>,3,0

fslv_fnc_atan macro
	fld1
	fpatan
endm
default_fnc_dscptr <atan>,1,0

fslv_fnc_asin macro
  	fst REAL4 ptr [esp-4]
  	and DWORD ptr [esp-4],80000000h
  	or  DWORD ptr [esp-4],3F800000h
  	fmul st,st
  	fld st  	
  	fld1
  	fsubrp st(1),st
  	fdivp st(1),st
  	fsqrt
  	fld1
  	fpatan
  	fmul REAL4 ptr [esp-4]
	
endm
default_fnc_dscptr <asin>,1,2

fslv_fnc_acos macro

  	fst REAL4 ptr [esp-4]
  	test REAL4 ptr [esp-4],80000000h
  	fmul st,st
  	fld st  	
  	fld1
  	fsubrp st(1),st
  	fdivp st(1),st
  	fsqrt
  	fld1
  	fpatan
  	fld fpu_const_r8_pi_div_2
  	.if ZERO?
  		fsubrp st(1),st 
  	.else	
  		faddp st(1),st
  	.endif
endm
default_fnc_dscptr <acos>,1,2

fslv_fnc_trunc macro
	push eax
	fstcw WORD ptr [esp-2]
	movzx eax,WORD ptr [esp-2]
	or eax,0c00h
	mov [esp-4],ax
	fldcw [esp-4]
	frndint	
	fldcw WORD ptr [esp-2]
	pop eax
endm
default_fnc_dscptr <trunc>,1,0

;mod(x,y) = x mod y
fslv_fnc_mod macro
	fdiv st(1),st
	fxch
	fld st
	push eax
	fstcw WORD ptr [esp-2]
	movzx eax,WORD ptr [esp-2]
	or eax,0c00h
	mov [esp-4],ax
	fldcw [esp-4]
	frndint	
	fldcw WORD ptr [esp-2]
	pop eax
	fsubp st(1),st
	fmulp st(1),st
endm
default_fnc_dscptr <mod>,2,1

