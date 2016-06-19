fslv_glbl_int_type  = 4 	; 2=SWORD,4=SDWORD,8=SQWORD
fslv_glbl_real_type = 8 	; 4=REAL4,8=REAL8,10=REAL10

fslv_lcl_real_type = fslv_glbl_real_type	;-fSlv,fSolve
fslv_lcl_int_type  = fslv_glbl_int_type		;/

fslv4_def_int_type = 4		;-fSlv4 
fslv4_def_real_type = 4		;/
fslv8_def_int_type = 4		;-fSlv8
fslv8_def_real_type = 8		;/
fslv10_def_int_type = 4		;-fSlv10
fslv10_def_real_type = 10	;/

@fslv4_def_int_type = 4		;-@fSlv4()
@fslv4_def_real_type = 4    ;/
@fslv8_def_int_type = 4     ;-@fSlv8()
@fslv8_def_real_type = 8    ;/
@fslvI_def_int_type = 4     ;-@fSlvI()
@fslvI_def_real_type = 4    ;/

fsTSA_glbl_int_type = 4
fsTSA_glbl_real_type = 4

fcmp_def_int_type = fslv_glbl_int_type
fcmp_def_real_type = fslv_glbl_real_type

fSolve macro expr:VARARG

	fslv_nc=fslv_nc+1
	fslvm_pos INSTR 1,<&expr>,<=>
	fslvm_size SIZESTR <&expr>
	IFE fslvm_pos
		.err <missing '='>
		EXITM
	ELSEIF fslvm_size LE 1
		.err <invalid argument>
		EXITM
	ELSEIF fslvm_pos EQ 1
		fslvm_dest TEXTEQU <>
		fslvm_expr SUBSTR <&expr>,2
	ELSE
		fslvm_dest SUBSTR <&expr>,1,fslvm_pos-1
		fslvm_expr SUBSTR <&expr>,fslvm_pos+1
	ENDIF
	
	IFNB fslvm_dest
		IFIDNI @TrimStr(%fslvm_expr),<st0>
			fst fslvm_dest
			EXITM
		ENDIF
	ENDIF
	
	fslv_extract_attributes %fslvm_expr
	IF fsea_found NE 0
		fslvm_expr TEXTEQU fsea_expr
		fslv_current_attributes TEXTEQU fsea_attributes
		fslv_attributes? = 1
		fslv_local_type_from_attributes fslv_glbl_real_type,fslv_glbl_int_type,%fsea_attributes
	ELSE
		fslv_attributes? = 0
		fslv_lcl_real_type = fslv_glbl_real_type
		fslv_lcl_int_type  = fslv_glbl_int_type
	ENDIF
	ll_MathTokenize SlvPath,SubExpr,%fslvm_expr
	IF tmt_error
	%	.err <tmt_err_txt>
		EXITM
	ENDIF
	
	ll_fSlv SlvPath,SubExpr
	IFNB fslvm_dest
		IF @MatchStrI(%fslvm_dest,1,eax,ebx,ecx,edx,esi,edi,ebp,esp,ax,bx,cx,dx,si,di,bp,sp,al,ah,cl,ch,dl,dh,bl,bh,edx::eax)
			IF msi_index LE 8
				fistp DWORD ptr [esp-4]
				mov fslvm_dest,DWORD ptr [esp-4]
			ELSEIF msi_index LE 16
				fistp WORD ptr [esp-4]
				mov fslvm_dest,WORD ptr [esp-4]
			ELSEIF msi_index LE 24
				fistp WORD ptr [esp-4]
				mov fslvm_dest,BYTE ptr [esp-4]
			ELSE
				fistp QWORD ptr [esp-8]
				mov eax,DWORD ptr [esp-8]
				mov edx,DWORD ptr [esp-4]
			ENDIF
		ELSE
			IF @MatchStrI(%fslvm_dest,1,st0,st1,st2,st3,st4,st5,st6)
				fstp st(msi_index)
			ELSE
				IF @IsIntegerType(%fslvm_dest)
					fistp fslvm_dest
				ELSE
					fstp fslvm_dest
				ENDIF
			ENDIF
		ENDIF
	ENDIF
	IF @fslv_test_attribute(<pop>,<:>,0)
		IF fsta_value GT 0
			REPEAT fsta_value
				fstp st(0)
			ENDM
		ENDIF
	ENDIF
	IF @fslv_test_attribute(<free>,<:>,0)
		fslv_tmp_i INSTR 1,fsta_value,<->
		IF fslv_tmp_i EQ 0
			ffree st(fsta_value)
		ELSE
			fslv_tmp_i1 SUBSTR fsta_value,1,fslv_tmp_i-1
			fslv_tmp_i2 SUBSTR fsta_value,fslv_tmp_i+1
			IF fslv_tmp_i2 LT fslv_tmp_i1
				fslv_tmp_i = fslv_tmp_i2
				fslv_tmp_i2 = fslv_tmp_i1
				fslv_tmp_i1 = fslv_tmp_i
			ENDIF
			fslv_tmp_i = fslv_tmp_i1
			REPEAT fslv_tmp_i2 - fslv_tmp_i1 + 1
				ffree st(fslv_tmp_i)
				fslv_tmp_i = fslv_tmp_i + 1
			ENDM
		ENDIF
	ENDIF
endm
fSlv EQU fSolve

fSlv4 macro expr:VARARG

	fslv4_nc=fslv4_nc+1
	fslvm_pos INSTR 1,<&expr>,<=>
	fslvm_size SIZESTR <&expr>
	IFE fslvm_pos
		.err <missing '='>
		EXITM
	ELSEIF fslvm_size LE 1
		.err <invalid argument>
		EXITM
	ELSEIF fslvm_pos EQ 1
		fslvm_dest TEXTEQU <>
		fslvm_expr SUBSTR <&expr>,2
	ELSE
		fslvm_dest SUBSTR <&expr>,1,fslvm_pos-1
		fslvm_expr SUBSTR <&expr>,fslvm_pos+1
	ENDIF
	
	IFNB fslvm_dest
		IFIDNI @TrimStr(%fslvm_expr),<st0>
			fst fslvm_dest
			EXITM
		ENDIF
	ENDIF

	fslv_extract_attributes %fslvm_expr
	IF fsea_found NE 0
		fslvm_expr TEXTEQU fsea_expr
		fslv_current_attributes TEXTEQU fsea_attributes
		fslv_attributes? = 1
		fslv_local_type_from_attributes fslv4_def_real_type,fslv4_def_int_type,%fsea_attributes
	ELSE
		fslv_attributes? = 0
		fslv_lcl_real_type = fslv4_def_real_type
		fslv_lcl_int_type  = fslv4_def_int_type
	ENDIF

	ll_MathTokenize SlvPath,SubExpr,%fslvm_expr
	IF tmt_error
	%	.err <tmt_err_txt>
		EXITM
	ENDIF

	ll_fSlv SlvPath,SubExpr
	IFNB fslvm_dest
		IF @MatchStrI(%fslvm_dest,1,eax,ebx,ecx,edx,esi,edi,ebp,esp,ax,bx,cx,dx,si,di,bp,sp,al,ah,cl,ch,dl,dh,bl,bh,edx::eax)
			IF msi_index LE 8
				fistp DWORD ptr [esp-4]
				mov fslvm_dest,DWORD ptr [esp-4]
			ELSEIF msi_index LE 16
				fistp WORD ptr [esp-4]
				mov fslvm_dest,WORD ptr [esp-4]
			ELSEIF msi_index LE 24
				fistp WORD ptr [esp-4]
				mov fslvm_dest,BYTE ptr [esp-4]
			ELSE
				fistp QWORD ptr [esp-8]
				mov eax,DWORD ptr [esp-8]
				mov edx,DWORD ptr [esp-4]
			ENDIF
		ELSE
			IF @MatchStrI(%fslvm_dest,1,st0,st1,st2,st3,st4,st5,st6)
				fstp st(msi_index)
			ELSE
				IF @IsIntegerType(%fslvm_dest)
					fistp fslvm_dest
				ELSE
					fstp fslvm_dest
				ENDIF
			ENDIF
		ENDIF
	ENDIF	
endm

fSlv8 macro expr:VARARG

	fslv8_nc=fslv8_nc+1
	fslvm_pos INSTR 1,<&expr>,<=>
	fslvm_size SIZESTR <&expr>
	IFE fslvm_pos
		.err <missing '='>
		EXITM
	ELSEIF fslvm_size LE 1
		.err <invalid argument>
		EXITM
	ELSEIF fslvm_pos EQ 1
		fslvm_dest TEXTEQU <>
		fslvm_expr SUBSTR <&expr>,2
	ELSE
		fslvm_dest SUBSTR <&expr>,1,fslvm_pos-1
		fslvm_expr SUBSTR <&expr>,fslvm_pos+1
	ENDIF
	
	IFNB fslvm_dest
		IFIDNI @TrimStr(%fslvm_expr),<st0>
			fst fslvm_dest
			EXITM
		ENDIF
	ENDIF

	fslv_extract_attributes %fslvm_expr
	IF fsea_found NE 0
		fslvm_expr TEXTEQU fsea_expr
		fslv_current_attributes TEXTEQU fsea_attributes
		fslv_attributes? = 1
		fslv_local_type_from_attributes fslv8_def_real_type,fslv8_def_int_type,%fsea_attributes
	ELSE
		fslv_attributes? = 0
		fslv_lcl_real_type = fslv8_def_real_type
		fslv_lcl_int_type  = fslv8_def_int_type
	ENDIF
	
	ll_MathTokenize SlvPath,SubExpr,%fslvm_expr
	IF tmt_error
	%	.err <tmt_err_txt>
		EXITM
	ENDIF

	ll_fSlv SlvPath,SubExpr
	
	IFNB fslvm_dest
		IF @MatchStrI(%fslvm_dest,1,eax,ebx,ecx,edx,esi,edi,ebp,esp,ax,bx,cx,dx,si,di,bp,sp,al,ah,cl,ch,dl,dh,bl,bh,edx::eax)
			IF msi_index LE 8
				fistp DWORD ptr [esp-4]
				mov fslvm_dest,DWORD ptr [esp-4]
			ELSEIF msi_index LE 16
				fistp WORD ptr [esp-4]
				mov fslvm_dest,WORD ptr [esp-4]
			ELSEIF msi_index LE 24
				fistp WORD ptr [esp-4]
				mov fslvm_dest,BYTE ptr [esp-4]
			ELSE
				fistp QWORD ptr [esp-8]
				mov eax,DWORD ptr [esp-8]
				mov edx,DWORD ptr [esp-4]
			ENDIF
		ELSE
			IF @MatchStrI(%fslvm_dest,1,st0,st1,st2,st3,st4,st5,st6)
				fstp st(msi_index)
			ELSE
				IF @IsIntegerType(%fslvm_dest)
					fistp fslvm_dest
				ELSE
					fstp fslvm_dest
				ENDIF
			ENDIF
		ENDIF
	ENDIF
endm

fSlv10 macro expr:VARARG

	fslv10_nc=fslv10_nc+1
	fslvm_pos INSTR 1,<&expr>,<=>
	fslvm_size SIZESTR <&expr>
	IFE fslvm_pos
		.err <missing '='>
		EXITM
	ELSEIF fslvm_size LE 1
		.err <invalid argument>
		EXITM
	ELSEIF fslvm_pos EQ 1
		fslvm_dest TEXTEQU <>
		fslvm_expr SUBSTR <&expr>,2
	ELSE
		fslvm_dest SUBSTR <&expr>,1,fslvm_pos-1
		fslvm_expr SUBSTR <&expr>,fslvm_pos+1
	ENDIF
	
	IFNB fslvm_dest
		IFIDNI @TrimStr(%fslvm_expr),<st0>
			fst fslvm_dest
			EXITM
		ENDIF
	ENDIF

	fslv_extract_attributes %fslvm_expr
	IF fsea_found NE 0
		fslvm_expr TEXTEQU fsea_expr
		fslv_current_attributes TEXTEQU fsea_attributes
		fslv_attributes? = 1
		fslv_local_type_from_attributes fslv10_def_real_type,fslv10_def_int_type,%fsea_attributes
	ELSE
		fslv_attributes? = 0
		fslv_lcl_real_type = fslv10_def_real_type
		fslv_lcl_int_type  = fslv10_def_int_type
	ENDIF
	
	ll_MathTokenize SlvPath,SubExpr,%fslvm_expr
	IF tmt_error
	%	.err <tmt_err_txt>
		EXITM
	ENDIF

	ll_fSlv SlvPath,SubExpr
	IFNB fslvm_dest
		IF @MatchStrI(%fslvm_dest,1,eax,ebx,ecx,edx,esi,edi,ebp,esp,ax,bx,cx,dx,si,di,bp,sp,al,ah,cl,ch,dl,dh,bl,bh,edx::eax)
			IF msi_index LE 8
				fistp DWORD ptr [esp-4]
				mov fslvm_dest,DWORD ptr [esp-4]
			ELSEIF msi_index LE 16
				fistp WORD ptr [esp-4]
				mov fslvm_dest,WORD ptr [esp-4]
			ELSEIF msi_index LE 24
				fistp WORD ptr [esp-4]
				mov fslvm_dest,BYTE ptr [esp-4]
			ELSE
				fistp QWORD ptr [esp-8]
				mov eax,DWORD ptr [esp-8]
				mov edx,DWORD ptr [esp-4]
			ENDIF
		ELSE
			IF @MatchStrI(%fslvm_dest,1,st0,st1,st2,st3,st4,st5,st6)
				fstp st(msi_index)
			ELSE
				IF @IsIntegerType(%fslvm_dest)
					fistp fslvm_dest
				ELSE
					fstp fslvm_dest
				ENDIF
			ENDIF
		ENDIF
	ENDIF
endm

@fSlv4 macro expr:VARARG
LOCAL r4lbl

	@fslv4_nc=@fslv4_nc+1
	fslvm_pos INSTR 1,<&expr>,<=>
	fslvm_size SIZESTR <&expr>
	IF fslvm_pos EQ 0
		fslvm_dest TEXTEQU <>
		fslvm_expr TEXTEQU <&expr>
	ELSE
		fslvm_dest SUBSTR <&expr>,1,fslvm_pos-1
		fslvm_expr SUBSTR <&expr>,fslvm_pos+1
		IFB fslvm_dest
			.err <missing lvalue: '='>
			EXITM <0>
		ENDIF
	ENDIF

	fslv_extract_attributes %fslvm_expr
	IF fsea_found NE 0
		fslvm_expr TEXTEQU fsea_expr
		fslv_current_attributes TEXTEQU fsea_attributes
		fslv_attributes? = 1
		fslv_local_type_from_attributes @fslv4_def_real_type,@fslv4_def_int_type,%fsea_attributes
	ELSE
		fslv_attributes? = 0
		fslv_lcl_real_type = @fslv4_def_real_type
		fslv_lcl_int_type  = @fslv4_def_int_type
	ENDIF

	ll_MathTokenize SlvPath,SubExpr,%fslvm_expr
	IF tmt_error
	%	.err <tmt_err_txt>
		EXITM <0>
	ENDIF

	ll_fSlv SlvPath,SubExpr
	IFNB fslvm_dest
		IF @MatchStrI(%fslvm_dest,1,eax,ebx,ecx,edx,esi,edi,ebp,esp,ax,bx,cx,dx,si,di,bp,sp,al,ah,cl,ch,dl,dh,bl,bh,edx::eax)
			IF msi_index LE 8
				fist DWORD ptr [esp-4]
				mov fslvm_dest,DWORD ptr [esp-4]
			ELSEIF msi_index LE 16
				fist WORD ptr [esp-4]
				mov fslvm_dest,WORD ptr [esp-4]
			ELSEIF msi_index LE 24
				fist WORD ptr [esp-4]
				mov fslvm_dest,BYTE ptr [esp-4]
			ELSE
				fld st
				fistp QWORD ptr [esp-8]
				mov eax,DWORD ptr [esp-8]
				mov edx,DWORD ptr [esp-4]
			ENDIF
		ELSE
			IF @IsIntegerType(%fslvm_dest)
				fist fslvm_dest
			ELSE
			%	IF type (fslvm_dest) EQ 4
					fstp fslvm_dest
					EXITM fslvm_dest
			%	ELSEIF type (fslvm_dest) EQ 10
					fld st
					fstp fslvm_dest
				ELSE
					fst fslvm_dest
				ENDIF
			ENDIF
		ENDIF
	ENDIF
	IFE @fslv_TLS?()
		@TLS_warning
		.data?
			align 4
			r4lbl REAL4 ?
		.code
		fstp r4lbl
		EXITM <r4lbl>
	ELSE
		r4lbl TEXTEQU @fslvTLS_line_alloc(REAL4)
		IFDIFI r4lbl,<0>
			fstp r4lbl
		ENDIF
		EXITM <r4lbl>
	ENDIF
endm

@fSlv8 macro expr:VARARG
LOCAL r8lbl

	@fslv8_nc=@fslv8_nc+1
	fslvm_pos INSTR 1,<&expr>,<=>
	fslvm_size SIZESTR <&expr>
	IF fslvm_pos EQ 0
		fslvm_dest TEXTEQU <>
		fslvm_expr TEXTEQU <&expr>
	ELSE
		fslvm_dest SUBSTR <&expr>,1,fslvm_pos-1
		fslvm_expr SUBSTR <&expr>,fslvm_pos+1
		IFB fslvm_dest
			.err <missing lvalue: '='>
			EXITM <0>
		ENDIF
	ENDIF

	fslv_extract_attributes %fslvm_expr
	IF fsea_found NE 0
		fslvm_expr TEXTEQU fsea_expr
		fslv_current_attributes TEXTEQU fsea_attributes
		fslv_attributes? = 1
		fslv_local_type_from_attributes @fslv8_def_real_type,@fslv8_def_int_type,%fsea_attributes
	ELSE
		fslv_attributes? = 0
		fslv_lcl_real_type = @fslv8_def_real_type
		fslv_lcl_int_type  = @fslv8_def_int_type
	ENDIF

	ll_MathTokenize SlvPath,SubExpr,%fslvm_expr
	IF tmt_error
	%	.err <tmt_err_txt>
		EXITM <0>
	ENDIF

	ll_fSlv SlvPath,SubExpr
	IFNB fslvm_dest
		IF @MatchStrI(%fslvm_dest,1,eax,ebx,ecx,edx,esi,edi,ebp,esp,ax,bx,cx,dx,si,di,bp,sp,al,ah,cl,ch,dl,dh,bl,bh,edx::eax)
			IF msi_index LE 8
				fist DWORD ptr [esp-4]
				mov fslvm_dest,DWORD ptr [esp-4]
			ELSEIF msi_index LE 16
				fist WORD ptr [esp-4]
				mov fslvm_dest,WORD ptr [esp-4]
			ELSEIF msi_index LE 24
				fist WORD ptr [esp-4]
				mov fslvm_dest,BYTE ptr [esp-4]
			ELSE
				fld st
				fistp QWORD ptr [esp-8]
				mov eax,DWORD ptr [esp-8]
				mov edx,DWORD ptr [esp-4]
			ENDIF
		ELSE
			IF @IsIntegerType(%fslvm_dest)
				fist fslvm_dest
			ELSE
			%	IF type (fslvm_dest) EQ 8
					fstp fslvm_dest
					EXITM fslvm_dest
			%	ELSEIF type (fslvm_dest) EQ 10
					fld st
					fstp fslvm_dest
				ELSE
					fst fslvm_dest
				ENDIF
			ENDIF
		ENDIF
	ENDIF
	IFE @fslv_TLS?()
		@TLS_warning
		.data?
			align 8
			r8lbl REAL8 ?
		.code
		fstp r8lbl
		EXITM <r8lbl>
	ELSE
		r8lbl TEXTEQU @fslvTLS_line_alloc(REAL8)
		IFDIFI r8lbl,<0>
			fstp r8lbl
		ENDIF
		EXITM <r8lbl>
	ENDIF
endm

@fSlvI macro expr:VARARG
LOCAL dwlbl

	@fslvI_nc=@fslvI_nc+1
	fslvm_pos INSTR 1,<&expr>,<=>
	fslvm_size SIZESTR <&expr>
	IF fslvm_pos EQ 0
		fslvm_dest TEXTEQU <>
		fslvm_expr TEXTEQU <&expr>
	ELSE
		fslvm_dest SUBSTR <&expr>,1,fslvm_pos-1
		fslvm_expr SUBSTR <&expr>,fslvm_pos+1
		IFB fslvm_dest
			.err <missing lvalue: '='>
			EXITM <0>
		ENDIF
	ENDIF

	fslv_extract_attributes %fslvm_expr
	IF fsea_found NE 0
		fslvm_expr TEXTEQU fsea_expr
		fslv_current_attributes TEXTEQU fsea_attributes
		fslv_attributes? = 1
		fslv_local_type_from_attributes @fslvI_def_real_type,@fslvI_def_int_type,%fsea_attributes
	ELSE
		fslv_attributes? = 0
		fslv_lcl_real_type = @fslvI_def_real_type
		fslv_lcl_int_type  = @fslvI_def_int_type
	ENDIF

	ll_MathTokenize SlvPath,SubExpr,%fslvm_expr
	IF tmt_error
	%	.err <tmt_err_txt>
		EXITM <0>
	ENDIF

	ll_fSlv SlvPath,SubExpr
	IFNB fslvm_dest
		IF @MatchStrI(%fslvm_dest,1,eax,ebx,ecx,edx,esi,edi,ebp,esp,ax,bx,cx,dx,si,di,bp,sp,al,ah,cl,ch,dl,dh,bl,bh,edx::eax)
			IF msi_index LE 8
				fistp DWORD ptr [esp-4]
				mov fslvm_dest,DWORD ptr [esp-4]
				EXITM fslvm_dest
			ELSEIF msi_index LE 16
				fist WORD ptr [esp-4]
				mov fslvm_dest,WORD ptr [esp-4]
			ELSEIF msi_index LE 24
				fist WORD ptr [esp-4]
				mov fslvm_dest,BYTE ptr [esp-4]
			ELSE
				fld st
				fistp QWORD ptr [esp-8]
				mov eax,DWORD ptr [esp-8]
				mov edx,DWORD ptr [esp-4]
			ENDIF
		ELSE
			IF @IsRealType(%fslvm_dest)
			%	IF type (fslvm_dest) EQ 10
					fld st
					fstp fslvm_dest
				ELSE
					fst fslvm_dest
				ENDIF
			ELSE
			%	IF type (fslvm_dest) EQ 4
					fistp fslvm_dest
					EXITM fslvm_dest
			%	ELSEIF type (fslvm_dest) EQ 8
					fld st
					fistp fslvm_dest
				ELSE
					fist fslvm_dest
				ENDIF
			ENDIF
		ENDIF
	ENDIF
	IFE @fslv_TLS?()
		@TLS_warning
		.data?
			align 4
			dwlbl SDWORD ?
		.code
		fistp dwlbl
		EXITM <dwlbl>
	ELSE
		dwlbl TEXTEQU @fslvTLS_line_alloc(SDWORD)
		IFDIFI dwlbl,<0>
			fistp dwlbl
		ENDIF
		EXITM <dwlbl>
	ENDIF
endm

fSlvTLS macro name:=<fslv_tls>,cb:=<4*4>
%	IFDEF fslv_tls_name
		.err <TLS duplication>
	ELSE
		fslv_tls_alloc_line = @Line
		fslv_tls_cb = (cb+15) AND (-16)
		fslv_tls_name TEXTEQU @TrimStr(<&name>)
		EXITM @CatStr(<&name&[fslv_tls_cb]:BYTE>)
	ENDIF
endm

fslv_nc = 0
fslv4_nc = 0
fslv8_nc = 0
fslv10_nc = 0
@fslv4_nc = 0
@fslv8_nc = 0
@fslvI_nc = 0
@fEQ_nc = 0
@fNE_nc = 0
@faEQ_nc = 0
@fLT_nc = 0
@fLE_nc = 0
@fGT_nc = 0
@fGE_nc = 0

FSLV_SUPPRESS_ALL_WARNINGS 	= 0
FSLV_SUPPRESS_TS_WARNIGNS 	= 0

fslv_glb_flags = 0
fslv_current_attributes TEXTEQU <>
fslv_attributes? = 0

fslv_regc_txt TEXTEQU <>
fslv_regc_val TEXTEQU <>

fslvTLS_curr_line = -1
fslv_tls_curr_used = 0
fslv_tls_name TEXTEQU <>
fslv_tls_cb = 0
fslv_tls_last TEXTEQU <>
fslv_tls_last_size = 0
fslv_tls_byte = 0
fslv_tls_alloc_line = 0


; macroName: name of fSlv-macros, enclosed by brackets
; e.g.: fSlvSetDefaultPrecision <@fSlv4>,REAL10,SQWORD
fSlvSetPrecision macro macroName:req,realPrec,intPrec

	fssdp_int = 0
	fssdp_real = 0

	IFIDNI <&realPrec>,<REAL4>
		fssdp_real = 4
	ELSEIFIDNI <&realPrec>,<REAL8>
		fssdp_real = 8
	ELSEIFIDNI <&realPrec>,<REAL10>
		fssdp_real = 10
	ELSEIFNB <&realPrec>
		.err <invalid precision specifier: &realPrec>
		EXITM
	ENDIF
	IFIDNI <&intPrec>,<SWORD>
		fssdp_int = 2
	ELSEIFIDNI <&intPrec>,<SDWORD>
		fssdp_int = 4
	ELSEIFIDNI <&intPrec>,<QWORD>
		fssdp_int = 8
	ELSEIFIDNI <&intPrec>,<SQWORD>
		fssdp_int = 8
	ELSEIFNB <&intPrec>
		.err <invalid precision specifier: &intPrec>
		EXITM
	ENDIF
	fssdp_cmp=0
	IFIDN <&macroName>,<fSlv>
		IFNB <&realPrec>
			fslv_glbl_real_type = fssdp_real
		ENDIF
		IFNB <&intPrec>
			fslv_glbl_int_type = fssdp_int
		ENDIF
	ELSEIFIDN <&macroName>,<fSlv4>
		IFNB <&realPrec>
			fslv4_def_real_type = fssdp_real
		ENDIF
		IFNB <&intPrec>
			fslv4_def_int_type = fssdp_int
		ENDIF
	ELSEIFIDN <&macroName>,<fSlv8>
		IFNB <&realPrec>
			fslv8_def_real_type = fssdp_real
		ENDIF
		IFNB <&intPrec>
			fslv8_def_int_type = fssdp_int
		ENDIF
	ELSEIFIDN <&macroName>,<fSlv10>
		IFNB <&realPrec>
			fslv10_def_real_type = fssdp_real
		ENDIF
		IFNB <&intPrec>
			fslv10_def_int_type = fssdp_int
		ENDIF
	ELSEIFIDN <&macroName>,<@fSlv4>
		IFNB <&realPrec>
			@fslv4_def_real_type = fssdp_real
		ENDIF
		IFNB <&intPrec>
			@fslv4_def_int_type = fssdp_int
		ENDIF
	ELSEIFIDN <&macroName>,<@fSlv8>
		IFNB <&realPrec>
			@fslv8_def_real_type = fssdp_real
		ENDIF
		IFNB <&intPrec>
			@fslv8_def_int_type = fssdp_int
		ENDIF
	ELSEIFIDN <&macroName>,<@fSlvI>
		IFNB <&realPrec>
			@fslvI_def_real_type = fssdp_real
		ENDIF
		IFNB <&intPrec>
			@fslvI_def_int_type = fssdp_int
		ENDIF
	ELSEIFIDN <&macroName>,<fEQ>
		fssdp_cmp=1
	ELSEIFIDN <&macroName>,<fNE>
		fssdp_cmp=1
	ELSEIFIDN <&macroName>,<fGT>
		fssdp_cmp=1
	ELSEIFIDN <&macroName>,<fGE>
		fssdp_cmp=1
	ELSEIFIDN <&macroName>,<fLT>
		fssdp_cmp=1
	ELSEIFIDN <&macroName>,<fLE>
		fssdp_cmp=1
	ELSEIFIDN <&macroName>,<faEQ>
		fssdp_cmp=1
	ELSE
		.err <unknown macro name: macroName>
		EXITM
	ENDIF
	IF fssdp_cmp
		IFNB <&realPrec>
			fcmp_def_real_type = fssdp_real
		ENDIF
		IFNB <&intPrec>
			fcmp_def_int_type = fssdp_int
		ENDIF
	ENDIF
endm

FSF_USE_SSE2		EQU 1
FSF_REUSE_BY_TYPE	EQU 2	; reuse contants only, if the types are equal -  this flag applies only to integer contants
FSF_CHECK_SQWORD	EQU 4	; check QWORD-constants for range (only decimal)

;;FSF_USE_VCRT		EQU 
;;FSF_USE_VCRT_SSE2	EQU 

fSlvSetFlags macro flags:=<0>
;	IF flags AND FSF_USE_VCRT_SSE2
;		IFNDEF fslv_crt_init
;			.err <vcrt must be initialized (using fSlvInitCRT-macro) befor activating this feature>
;			EXITM
;		ENDIF
;	ENDIF
	fslv_glb_flags = fslv_glb_flags OR flags	
endm

fSlvRemoveFlags macro flags:=<0>
	fslv_glb_flags = fslv_glb_flags AND (NOT(flags))
endm

fSlvResetFlags macro
	fslv_glb_flags = 0
endm

fSlvGetFlags macro
	EXITM %fslv_glb_flags
endm

fpuSetPrecision macro mem16:=<WORD ptr [esp-2]>,prec:=<REAL8>
	IFIDNI <&prec>,<REAL4>
		fstcw mem16
		and mem16,0fCffh
		fldcw mem16
	ELSEIFIDNI <&prec>,<REAL8>
		fstcw mem16
		and mem16,0fCffh
		or mem16,0200h
		fldcw mem16
	ELSEIFIDNI <&prec>,<REAL10>
		fstcw mem16
		or mem16,0300h
		fldcw mem16
	ELSE
		.err <invalid precision specifier: &prec>
	ENDIF
endm

ldl_expand macro _arg
	EXITM <&_arg>
endm

; load local
ldl macro args:VARARG
	FOR _arg,<&args>
		ldl_pos INSTR 1,<&_arg>,<=>
		IFE ldl_pos
			.err <syntax error: expect <=>>
			EXITM
		ENDIF
		ldl_var SUBSTR <&_arg>,1,ldl_pos-1
		ldl_value SUBSTR <&_arg>,ldl_pos+1
		ldl_value TEXTEQU @TrimStr(%ldl_value)

		IF @ScanForFlt(1,%ldl_value,blanks) EQ 0
			ldl_value TEXTEQU %ldl_value
		ENDIF

		IFE @ScanForFlt(1,%ldl_value,blanks)
		%	.err <not a number: ldl_value>
			EXITM
		ENDIF
		
	%	IF (TYPE ldl_var) EQ (TYPE REAL4)
			IF @InStr(1,<0 0.0>,%ldl_value) EQ 0
				push 123456789
				org $-4
				IF sff_type EQ 1
					REAL4 ldl_value
				ELSE
				%	REAL4 &ldl_value&.0
				ENDIF
				pop DWORD ptr ldl_var
			ELSE
				mov DWORD ptr ldl_var,0
			ENDIF
	%	ELSEIF @IsIntegerType(%ldl_var) EQ 4
			IF sff_type NE 2
			%	.err <integer value expected: ldl_value>
				EXITM
			ENDIF
			IF @InStr(1,<0 0.0>,%ldl_value) EQ 0
				push 123456789
				org $-4
				SDWORD ldl_value
				pop DWORD ptr ldl_var
			ELSE
				mov DWORD ptr ldl_var,0
			ENDIF
	%	ELSEIF (TYPE ldl_var) EQ (TYPE REAL8)
			IF @InStr(1,<0 0.0>,%ldl_value) EQ 0
				push 123456789
				org $-4
				IF sff_type EQ 1
					REAL8 ldl_value
				ELSE
				%	REAL8 &ldl_value&.0
				ENDIF
				org $-4
				IF sff_type EQ 1
					REAL8 ldl_value
				ELSE
				%	REAL8 &ldl_value&.0
				ENDIF
				org $-8
				db 8Dh,49h,0		; lea ecx,[ecx+0] = nop
				db 068h				; push imm32
				org $+4
				pop DWORD ptr ldl_var+4
				pop DWORD ptr ldl_var
			ELSE
				mov DWORD ptr ldl_var,0
				mov DWORD ptr ldl_var+4,0
			ENDIF
	%	ELSEIF @IsIntegerType(%ldl_var) EQ 8
			IF sff_type NE 2
			%	.err <integer value expected: ldl_value>
				EXITM
			ENDIF
			IF @InStr(1,<0 0.0>,%ldl_value) EQ 0
				push 123456789
				org $-4
				QWORD ldl_value
				org $-4
				QWORD ldl_value
				org $-8
				db 8Dh,49h,0		; lea ecx,[ecx+0] = nop
				db 068h				; push imm32
				org $+4
				pop DWORD ptr ldl_var+4
				pop DWORD ptr ldl_var
			ELSE
				mov DWORD ptr ldl_var,0
				mov DWORD ptr ldl_var+4,0
			ENDIF
		ELSE
		%	.err <invalid typed variable: ldl_var - only REAL4/8,SDWORD and QWORD are supported>
			EXITM
		ENDIF

	ENDM
endm

;; mov REAL8 to mem64 using edx and eax
;llr8 macro Var,value
;	IFE @ScanForFlt(1,<&value>,blanks)
;		.err <not a number: value>
;		EXITM
;	ENDIF
;	IF (TYPE Var) NE (TYPE REAL8)
;		.err <invalid typed variable: Var>
;		EXITM
;	ENDIF
;	IF @InStr(1,<0 0.0>,<&value>) EQ 0
;		mov eax,-1
;		org $-4
;		IF sff_type EQ 1
;			REAL8 value
;		ELSE
;			REAL8 &value&.0
;		ENDIF
;		org $-4
;		IF sff_type EQ 1
;			REAL8 value
;		ELSE
;			REAL8 &value&.0
;		ENDIF
;		org $-8
;		db 8Dh,49h,0		; lea ecx,[ecx+0]
;		db 0bah				; mov edx,imm32
;		org $+4
;		mov DWORD ptr Var,eax
;		mov DWORD ptr Var+4,edx
;	ELSE
;		mov DWORD ptr Var,0
;		mov DWORD ptr Var+4,0
;	ENDIF
;endm

fSlvRegConst macro ConstName:req,Value:req
	IFNDEF fpu_regc_&ConstName&_type
		IF @ScanForFlt(1,<&Value>,blanks)
			fpu_regc_equ_&ConstName& TEXTEQU <&Value>
			fpu_regc_&ConstName&_type = sff_type
			EXITM
		ENDIF
		.err <ERR: fSlvRegConst(ConstName,Value): invalid value <&Value>>
	ELSE
		.err <ERR: fSlvRegConst(ConstName,Value): redefinition <&ConstName>>
	ENDIF
endm


fSlvRegExpr macro ExprName:req,nArgs:=<0>,Expression:VARARG
	
	IFB <&Expression>
		%echo ERR: _fSolveRegExpr(<&ExprName>,<%nArgs>,<&Expression>): blank argument
		EXITM
	ENDIF
	
	fsre_cntr = 1
	REPEAT nArgs
	%	IF @InStr(1,<&Expression>,@CatStr(<arg>,%fsre_cntr)) EQ 0
			fsre_cntr = 0
			EXITM
		ENDIF
		fsre_cntr = fsre_cntr + 1
	ENDM
	
	IF fsre_cntr EQ 0 AND nArgs NE 0
		echo ERR: fSlveRegExpr(<&ExprName>,<&nArgs>,<&Expression>): missing arguments (argX) in Expression
		EXITM
	ENDIF
	
	FOR _arg,<&ExprName>
		fsre_name TEXTEQU <&_arg>
	ENDM

	@CatStr(<fSlv_Expr_>,%fsre_name) TEXTEQU <&Expression>	
	@CatStr(<fSlv_Expr_args_>,%fsre_name) = nArgs
endm


r4IsValid macro mem32:req
	push mem32
	and DWORD ptr [esp],7F800000h
	xor DWORD ptr [esp],7F800000h
	lea esp,[esp+4]
	EXITM <!ZERO?>
endm

r8IsValid macro mem64:req
	push DWORD ptr mem64+4
	and DWORD ptr [esp],7FF00000h
	xor DWORD ptr [esp],7FF00000h
	lea esp,[esp+4]
	EXITM <!ZERO?>
endm


;/*
; * fcmp a,b
; * compare two numeric values, passed as variables or as immediate values, and set the
; * eflags according to the result (use ja,jb,... and NOT jg,jl...). For comparision FCOMIP is used. 
; * The following immediate values cause the macros to create special code instead of constants:
; *                        <0>,<0.0>,<1>,<1.0>,<-1>,<-1.0>
; * Supported data types: all x86-87 specific data types (FLD/FILD) and all 32Bit-GPRs (eax,edx,...).
; * For loading GPRs the memory location [esp-64] is used! This macro will only works correct, 
; * when called in the code section (.code).
; * Example:
; *     fcmp r4Var1,1.0E-7
; *     jae xyz
; */
fcmp macro args:VARARG
	fcmpx <>,0,args
endm
fcmp_smpl=0

;/*
; * fEQ/NE/GT/GE/LT/LE(a,b,pByte)
; * compare two numeric values, passed as variables or immediate values, and return the boolean
; * result as a byte (0 or 1). For comparison FCOMIP is used. The following immediate values cause the
; * macros to create special code instead of constants: <0>,<0.0>,<1>,<1.0>,<-1>,<-1.0>
; * Supported data types: all x86-87 specific data types (FLD/FILD), but no FPU registers. Also you can
; * use all 32/16/8Bit-GPRs (eax,edx,ax,dx,ah,al,...) and the register pair edx::eax.
; * For loading GPRs the memory location 'xWORD ptr [esp-(SIZEOF xWORD)]' is used!
; * 
; * [> If local storage is initialized by the macro fSlvTLS(), the following section doesn't applies <]
; * The parameter pByte is an optional BYTE variable. If this parameter is not specified,
; * a BYTE is allocated in the BSS segment (.data?), thus the produced code is NOT thread-safe! By 
; * passing a local variable or an ESP-relative expression, thread-safe code can be created (see example)
; *
; * Also DO NOT USE these macros with .ELSEIF and .WHILE - it won't work!
; * Other notes: This macro will only work correct, when called in the code section (.code)
; *
; * Example:
; * .if fEQ(r4Var,12.34) && fGT(r4Var2,3) ; not thread-safe (except: fSlvTLS() is used)
; *    ...
; * .emdif
; *
; * .if fEQ(r4Var,12.34,[esp-1]) && fGT(r4Var2,3,[esp-2]) ; thread safe
; *    ...
; * .endif
; */
fEQ macro args:VARARG ; a:req,b:req,pByte
	@fEQ_nc=@fEQ_nc+1
	fcmpx <e>,0,args
	EXITM fcmp_bool_byte
endm
fNE macro args:VARARG ; a:req,b:req,pByte
	@fNE_nc=@fNE_nc+1
	fcmpx <ne>,0,args
	EXITM fcmp_bool_byte
endm
fGT macro args:VARARG ; a:req,b:req,pByte
	@fGT_nc=@fGT_nc+1
	fcmpx <!a>,0,args
	EXITM fcmp_bool_byte
endm
fGE macro args:VARARG ; a:req,b:req,pByte
	@fGE_nc=@fGE_nc+1
	fcmpx <ae>,0,args
	EXITM fcmp_bool_byte
endm
fLT macro args:VARARG ; a:req,b:req,pByte
	@fLT_nc=@fLT_nc+1
	fcmpx <!b>,0,args
	EXITM fcmp_bool_byte
endm
fLE macro args:VARARG ; a:req,b:req,pByte
	@fLE_nc=@fLE_nc+1
	fcmpx <be>,0,args
	EXITM fcmp_bool_byte
endm

faEQ  macro args:VARARG ;a:req,b:req,factor:=<1.0E-2>,pByte
	@faEQ_nc=@faEQ_nc+1
	fcmpx <foo>,1,args
	EXITM fcmp_bool_byte
endm

fcmpx macro cmpSuffix,approxEQ:=<0>,args:VARARG ;cmpSuffix,approxEQ:=<0>,a:req,b:req,pByte

	IFNDEF fcmp_line
		fcmp_line = @Line
	ENDIF

	fslv_lcl_real_type = fcmp_def_real_type
	fslv_lcl_int_type  = fcmp_def_int_type
	fslv_attributes? = 0
	fcmpx_args TEXTEQU <&args>
	fslv_extract_attributes <&args>
	IF fsea_found NE 0
		fcmpx_args TEXTEQU fsea_expr
		fslv_current_attributes TEXTEQU fsea_attributes
		fslv_attributes? = 1
		fslv_local_type_from_attributes fcmp_def_real_type,fcmp_def_int_type,%fsea_attributes
	ENDIF

	fcmpx_a TEXTEQU <>
	fcmpx_b TEXTEQU <>
	fcmpx_pByte TEXTEQU <>
	fcmpx_factor TEXTEQU <>
	
	fcmpx_cntr=0
%	FOR _arg,<fcmpx_args>
		IF fcmpx_cntr EQ 0
			fcmpx_a TEXTEQU <&_arg>
		ELSEIF fcmpx_cntr EQ 1
			fcmpx_b TEXTEQU <&_arg>
		ELSEIF fcmpx_cntr EQ 2
			IF approxEQ EQ 0
				fcmpx_pByte TEXTEQU <&_arg>
			ELSE
				fcmpx_factor TEXTEQU <&_arg>
			ENDIF
		ELSEIF approxEQ NE 0 AND fcmpx_cntr EQ 3
			fcmpx_pByte TEXTEQU <&_arg>
		ELSE
			.err <to many arguments>
			EXITM
		ENDIF
		fcmpx_cntr=fcmpx_cntr+1
	ENDM
	IF fcmpx_cntr LT 2
		.err <missing arguments>
		EXITM
	ELSEIFB <&cmpSuffix>
		IF fcmpx_cntr NE 2 AND approxEQ EQ 0
			.err <invalid number of arguments>
			EXITM
		ENDIF
	ENDIF
	
	IF approxEQ NE 0
		IFB fcmpx_factor
			fcmpx_factor TEXTEQU <1.0E-2>
		ENDIF
	ENDIF	
	
	IFB fcmpx_pByte
		IFNB <&cmpSuffix>
			IFE @fslv_TLS?()
				@TLS_warning
				fcmp_bool_byte TEXTEQU get_unique_lbl(<fcmp_bool_byte_>)
				IF fcmp_line NE @Line
					fcmp_line = @Line
					.data?
						align 4	
					.code
				ENDIF
				.data?
					fcmp_bool_byte db ?
				.code
			ELSE
				fcmp_bool_byte TEXTEQU @fslvTLS_line_alloc(BYTE)
			ENDIF
		ENDIF
	ENDIF
	
	IF @InStr(1,<0.0>,%fcmpx_b) EQ 1
		fldz
	ELSEIF @InStr(1,<1.0>,%fcmpx_b) EQ 1
		fld1
	ELSEIF @InStr(1,<-1.0>,%fcmpx_b) EQ 1
		fld1
		fchs
	ELSE ; bug fix for masm
		IF @MatchStrI(%fcmpx_b,1,eax,ebx,ecx,edx,esi,edi,ebp,esp,ax,bx,cx,dx,si,di,bp,sp,al,ah,cl,ch,dl,dh,bl,bh,edx::eax)
			IF msi_index LE 8
				mov DWORD ptr [esp-4],fcmpx_b
				fild DWORD ptr [esp-4]
			ELSEIF msi_index LE 16
				mov WORD ptr [esp-4],fcmpx_b
				fild WORD ptr [esp-4]
			ELSEIF msi_index LE 24
				.if fcmpx_b & 80h
					mov DWORD ptr [esp-4],-1
				.else
					mov DWORD ptr [esp-4],0
				.endif
				mov BYTE ptr [esp-4],fcmpx_b
				fild DWORD ptr [esp-4]
			ELSE
				mov DWORD ptr [esp-8+4],edx
				mov DWORD ptr [esp-8],eax
				fild QWORD ptr [esp-8]
			ENDIF
		ELSE ; bug fix for masm
			IF @ScanForFlt(1,%fcmpx_b,blanks) NE 0
				IF sff_type EQ 1
					fld @fSlvCreateConstant(get_unique_lbl(<fcmp_lbl1_>),%fcmpx_b,@CatStr(<REAL>,%fslv_lcl_real_type))
				ELSE
					fild @fSlvCreateConstant(get_unique_lbl(<fcmp_lbl1_>),%fcmpx_b,@SubStr(<SWORD SDWORD SQWORD>,%@SubStr(< 0107  14>,%fslv_lcl_int_type,2),%5+((fslv_lcl_int_type GT 2) AND 1)))
				ENDIF
			ELSE
				IF @IsRealType(%fcmpx_b)
					fld fcmpx_b
				ELSE
					fild fcmpx_b
				ENDIF
			ENDIF
		ENDIF		
	ENDIF

	IF @InStr(1,<0.0>,%fcmpx_a) EQ 1 
		fldz
	ELSEIF @InStr(1,<1.0>,%fcmpx_a) EQ 1
		fld1
	ELSEIF @InStr(1,<-1.0>,%fcmpx_a) EQ 1
		fld1
		fchs
	ELSE ; bug fix
		IF @MatchStrI(%fcmpx_a,1,eax,ebx,ecx,edx,esi,edi,ebp,esp,ax,bx,cx,dx,si,di,bp,sp,al,ah,cl,ch,dl,dh,bl,bh,edx::eax)
			IF msi_index LE 8
				mov DWORD ptr [esp-4],fcmpx_a
				fild DWORD ptr [esp-4]
			ELSEIF msi_index LE 16
				mov WORD ptr [esp-4],fcmpx_a
				fild WORD ptr [esp-4]
			ELSEIF msi_index LE 24
				.if fcmpx_a & 80h
					mov DWORD ptr [esp-4],-1
				.else
					mov DWORD ptr [esp-4],0
				.endif
				mov BYTE ptr [esp-4],fcmpx_a
				fild DWORD ptr [esp-4]
			ELSE
				mov DWORD ptr [esp-8+4],edx
				mov DWORD ptr [esp-8],eax
				fild QWORD ptr [esp-8]
			ENDIF
		ELSE
			IF @ScanForFlt(1,%fcmpx_a,blanks) NE 0
				IF sff_type EQ 1
					fld @fSlvCreateConstant(get_unique_lbl(<fcmp_lbl2_>),%fcmpx_a,@CatStr(<REAL>,%fslv_lcl_real_type))
				ELSE
					fild @fSlvCreateConstant(get_unique_lbl(<fcmp_lbl2_>),%fcmpx_a,@SubStr(<SWORD SDWORD SQWORD>,%@SubStr(< 0107  14>,%fslv_lcl_int_type,2),%5+((fslv_lcl_int_type GT 2) AND 1)))
				ENDIF
			ELSE
				IF @IsRealType(%fcmpx_a)
					fld fcmpx_a
				ELSE
					fild fcmpx_a
				ENDIF
			ENDIF
		ENDIF
	ENDIF
	
	IF approxEQ EQ 0
		fcomip st,st(1)
		fstp st
		IFB fcmpx_pByte
			IFNB <&cmpSuffix>
				set&cmpSuffix& fcmp_bool_byte
			ENDIF
		ELSE
			set&cmpSuffix& BYTE ptr (fcmpx_pByte)
			fcmp_bool_byte TEXTEQU <BYTE ptr !(>,fcmpx_pByte,<!)>
		ENDIF
	ELSE
		feqa_txt TEXTEQU <>
		IF @ScanForFlt(1,%fcmpx_factor,blanks) NE 0
			IF sff_type EQ 1
				feqa_txt TEXTEQU @fSlvCreateConstant(get_unique_lbl(<fcmp_lbl2_>),%fcmpx_factor,@CatStr(<REAL>,@SubStr(<   4   8 8>,%fslv_lcl_real_type,1)))
			ELSE
				.err <fEQ_approx: invalid !factor : expect REAL constant or variable>
				EXITM
			ENDIF
		ELSE
			IF @IsRealType(%fcmpx_factor) XOR 2 ; match 4 or 8
				feqa_txt TEXTEQU fcmpx_factor
			ELSE
				.err <fEQ_approx: invalid !factor : expect REAL constant or variable>
				EXITM
			ENDIF
		ENDIF

		fld st
		fmul feqa_txt
		fabs
		fxch st(2)
		fsubp st(1),st
		fabs
		fcomip st,st(1)
		fstp st
		IFB fcmpx_pByte
			setbe fcmp_bool_byte
		ELSE
			setbe BYTE ptr (fcmpx_pByte)
		ENDIF

; 18032012		
;		fld st
;		fmul feqa_txt
;		fabs
;		fld st
;		fadd st,st(2)
;		fcomip st,st(3)
;		.if !CARRY?
;			fchs
;			faddp st(1),st
;			fcomip st,st(1)
;			fstp st
;			IFB fcmpx_pByte
;				setbe fcmp_bool_byte
;			ELSE
;				setbe BYTE ptr (fcmpx_pByte)
;			ENDIF
;		.else
;			fstp st
;			fstp st
;			fstp st
;			IFB fcmpx_pByte
;				mov fcmp_bool_byte,0
;			ELSE
;				mov BYTE ptr (fcmpx_pByte),0
;			ENDIF
;		.endif

		IFNB fcmpx_pByte
			fcmp_bool_byte TEXTEQU <BYTE ptr !(>,fcmpx_pByte,<!)>
		ENDIF	
	ENDIF
endm

IF @WordSize NE 8
	R4 macro value:req
		EXITM R??(4,value)
	endm
	
	R8 macro value:req
		EXITM R??(8,value)
	endm
	
	R10 macro value:req
		EXITM R??(10,value)
	endm
ELSE
	_R4 macro value:req
		EXITM R??(4,value)
	endm
	
	_R8 macro value:req
		EXITM R??(8,value)
	endm
	
	_R10 macro value:req
		EXITM R??(10,value)
	endm
ENDIF

R?? macro num:req,value:req
	??R&num&_value TEXTEQU <&value>
	IFE @ScanForFlt(1,<&value>,blanks)
		.err <R&num&: invalid constant: value>
	ELSE
		IF sff_type NE 1
			IF (sff_flag AND SFF_HEX_SUFFIX) NE 0
				.err <R&num&: floating pointer initializer expected>
				EXITM <0>
			ENDIF
			??R&num&_value CATSTR ??R&num&_value,<.0>
		ENDIF
		EXITM @fSlvCreateConstant(get_unique_lbl(<??R&num&_CONST_>),%??R&num&_value,REAL&num&)
	ENDIF
	EXITM <0>
endm

fSlvStatistics macro
	%echo
	%echo fslv-Statistics @ @FileCur::line(@CatStr(%@Line))
	%echo ------ macro calls ------
	%echo calls total: @CatStr(%fslv_nc+fslv4_nc+fslv8_nc+fslv10_nc+@fslv4_nc+@fslv8_nc+@fslvI_nc+@fEQ_nc+@faEQ_nc+@fNE_nc+@fLT_nc+@fLE_nc+@fGT_nc+@fGE_nc)
	%echo 
	%echo fSolve:  @CatStr(%fslv_nc)
	%echo fSlv4:   @CatStr(%fslv4_nc)
	%echo fSlv8:   @CatStr(%fslv8_nc)
	%echo fSlv10:  @CatStr(%fslv10_nc)
	%echo @fSlv4:  @CatStr(%@fslv4_nc)
	%echo @fSlv8:  @CatStr(%@fslv8_nc)
	%echo @fSlvI:  @CatStr(%@fslvI_nc)
	%echo fEQ:     @CatStr(%@fEQ_nc)
	%echo faEQ:    @CatStr(%@faEQ_nc)
	%echo fNE:     @CatStr(%@fNE_nc)
	%echo fLT:     @CatStr(%@fLT_nc)
	%echo fLE:     @CatStr(%@fLE_nc)
	%echo fGT:     @CatStr(%@fGT_nc)
	%echo fGE:     @CatStr(%@fGE_nc)
	%echo 
	%echo ------ constants --------
	%echo created constants: @CatStr(%fscc_nconsts)
	%echo reused constants:  @CatStr(%fscc_nreused)
	%echo 
	%echo created SWORD:  @CatStr(%fscc_nc_SWORD)
	%echo created SDWORD: @CatStr(%fscc_nc_SDWORD)
	%echo created SQWORD: @CatStr(%fscc_nc_SQWORD)
	%echo created REAL4:  @CatStr(%fscc_nc_REAL4)
	%echo created REAL8:  @CatStr(%fscc_nc_REAL8)
	%echo created REAL10: @CatStr(%fscc_nc_REAL10)
	%echo
	%echo reused SWORD:   @CatStr(%fscc_nr_SWORD)
	%echo reused SDWORD:  @CatStr(%fscc_nr_SDWORD)
	%echo reused SQWORD:  @CatStr(%fscc_nr_SQWORD)
	%echo reused REAL4:   @CatStr(%fscc_nr_REAL4)
	%echo reused REAL8:   @CatStr(%fscc_nr_REAL8)
	%echo reused REAL10:  @CatStr(%fscc_nr_REAL10)
	%echo ------ end statistics ---
	%echo
endm

@fSlvTSA macro arg
	IFB <&arg>
		EXITM <&arg>
	ENDIF
	fstsa_pos1 INSTR 1,<&arg>,<">
	fstsa_pos2 INSTR 1,<&arg>,<'>
	IF fstsa_pos1 NE 0 OR fstsa_pos2 NE 0
		EXITM <&arg>
	ENDIF
	fstsa_arg TEXTEQU @TrimStr(<&arg>)
	fstsa_pos1 INSTR 1,fstsa_arg,<=>
	fstsa_pos2 INSTR 1,fstsa_arg,<:>
	IF fstsa_pos1 EQ 0 AND fstsa_pos2 EQ 0
		EXITM <&arg>
	ENDIF
	IF fstsa_pos1 NE 0 AND fstsa_pos2 NE 0
		.err <syntax error in argument: invalid formula>
		EXITM <0>
	ENDIF
	IF fstsa_pos1 NE 0
		fstsa_pos = fstsa_pos1
		fstsa_real = 1
	ELSE
		fstsa_pos = fstsa_pos2
		fstsa_real = 0
	ENDIF
	fstsa_size SIZESTR fstsa_arg
	IF fstsa_size EQ fstsa_pos
		.err <invalid expression as argument>
		EXITM <0>
	ENDIF
	IF fstsa_pos NE 1
		fstsa_expr SUBSTR fstsa_arg,fstsa_pos+1
		fstsa_dest SUBSTR fstsa_arg,1,fstsa_pos-1
		IF @MatchStrI(%fstsa_dest,1,r4,r8,i4)
			IF msi_index LE 2
				fstsa_real = 1
				fsTSA_lcl_real_type = @GetArgByIndex(%msi_index-1,,4,8)
			ELSE
				fstsa_real = 0
				fsTSA_lcl_int_type = 4
			ENDIF
			fstsa_dest TEXTEQU <>
		ELSE
			fsTSA_lcl_int_type = fsTSA_glbl_int_type
			fsTSA_lcl_real_type = fsTSA_glbl_int_type
		ENDIF
	ELSE
		fstsa_expr SUBSTR fstsa_arg,fstsa_pos+1
		fstsa_dest TEXTEQU <>
		fsTSA_lcl_int_type = fsTSA_glbl_int_type
		fsTSA_lcl_real_type = fsTSA_glbl_int_type
	ENDIF
	
	IFNB fstsa_dest
		fstsa_expr CATSTR fstsa_dest,<=>,fstsa_expr
	ENDIF
	IF fstsa_real
		IF fsTSA_lcl_real_type EQ 4
			EXITM @fSlv4(%fstsa_expr)
		ELSEIF fsTSA_lcl_real_type EQ 8
			EXITM @fSlv8(%fstsa_expr)
		ELSE
			.err <@fSlvTSA: data type not supported>
			EXITM <0>
		ENDIF
	ELSE
		IF fsTSA_lcl_int_type EQ 4
			EXITM @fSlvI(%fstsa_expr)
		ELSE
			.err <@fSlvTSA: data type not supported>
			EXITM <0>
		ENDIF
	ENDIF	
endm


