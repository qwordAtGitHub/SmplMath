;
; this macro creates fpu code
;

ll_fSlv macro SlvPath,SubExpr
	
	IFNDEF lfs_lbl
		lfs_lbl = 0
	ENDIF

	lfs_stack_cntr = 0
	lfs_flg = 0
	
	lfs_st TEXTEQU <st!(>
	
	iml_create StkSlot
	iml_create MemSlot
	lfs_nss = 0

	;----------------------------
	; go through list and:
	; - search for st0,st1,...
	; - delete empty nodes 
	;----------------------------
	lfs_st_max = -1
	lfs_iNode = ll_get_first(SlvPath)
	WHILE lfs_iNode
		lfs_txt TEXTEQU ll_get_data(SlvPath,%lfs_iNode,1)
		IFB lfs_txt
			lfs_iNode = ll_delete(SlvPath,%lfs_iNode)		
		ELSE
		%	FOR _arg,<lfs_txt>
				lfs_txt TEXTEQU <&_arg>
			ENDM
			lfs_pos INSTR 1,<    st0  st1  st2  st3  st4  st5  st6>,lfs_txt
			IF lfs_pos NE 0 AND (lfs_pos MOD 5) EQ 0
				ll_or_value SlvPath,%lfs_iNode,2,%TMT_ST
				lfs_txt SUBSTR lfs_txt,3
				ll_set_data SlvPath,%lfs_iNode,1,%lfs_txt
				IF lfs_txt GT lfs_st_max
					lfs_st_max = lfs_txt
				ENDIF
			ENDIF	
			lfs_iNode = ll_get_next(SlvPath,%lfs_iNode)	
		ENDIF
	ENDM
	IF lfs_st_max NE -1
		lfs_stack_cntr = lfs_st_max + 1
	ENDIF
	IF @fslv_test_attribute(<stck>,<:>,0)
		lfs_stack_cntr = lfs_stack_cntr + fsta_value
	ENDIF
	
	;-----------------------------------------
	; - create 'streaming' buffer serving
	;   tokens from linked list.
	; - each token has a textmacro: lfs_txtX 
	;   and a constant lfs_flgX
	; - 4 tokens are stocked
	; - lfs_txt2/flg2 is the current token 
	;-----------------------------------------
	lfs_iNode = ll_stream_tm(SlvPath,1,,lfs_txt2,lfs_txt3,lfs_txt4,lfs_txt5)
	lfs_iNode = ll_stream_const(SlvPath,2,,lfs_flg2,lfs_flg3,lfs_flg4,lfs_flg5)
	
	;-----------------------------------------------
	; - process token list
	; - lfs_skip is used to discard tokens
	;   prior lfs_txt/flg2
	; - lfs_stack_cntr holds the number of currently
	;   used fpu-register
	;-----------------------------------------------
	lfs_skip = 0
	WHILE lfs_flg2
		IF lfs_skip
			lfs_skip = lfs_skip - 1
			GOTO Z
		ENDIF
		
		IF lfs_flg2 AND TMT_TXT
			IF lfs_flg2 AND TMT_ST
			%	fld st(lfs_stack_cntr-(lfs_st_max-lfs_txt2)-1)
				IF lfs_flg2 AND TMT_NEG
					fchs
				ENDIF
			ELSE
				IF @MatchStrI(%lfs_txt2,1,eax,ebx,ecx,edx,esi,edi,ebp,esp,ax,bx,cx,dx,si,di,bp,sp,al,ah,cl,ch,dl,dh,bl,bh)
					IF msi_index LE 8
						mov DWORD ptr [esp-4],lfs_txt2
						lfs_txt2 TEXTEQU <DWORD ptr [esp-4]>
					ELSEIF msi_index LE 16
						mov WORD ptr [esp-4],lfs_txt2
						lfs_txt2 TEXTEQU <WORD ptr [esp-4]>
					ELSE
						mov BYTE ptr [esp-4],lfs_txt2
						.if lfs_txt2 & 80h
							mov BYTE ptr [esp-3],0ffh
						.else
							mov BYTE ptr [esp-3],0
						.endif
						lfs_txt2 TEXTEQU <WORD ptr [esp-4]>
					ENDIF
				ENDIF
				IF @IsIntegerType(%lfs_txt2)
					fild lfs_txt2
				ELSE
					fld lfs_txt2
				ENDIF
				IF lfs_flg2 AND TMT_NEG
					fchs
				ENDIF
			ENDIF
			lfs_stack_cntr = lfs_stack_cntr + 1
		ELSEIF lfs_flg2 AND TMT_CONST
			lfs_txt TEXTEQU lfs_txt2
			lfs_flg = lfs_flg2
			llfs_load_const
			lfs_stack_cntr = lfs_stack_cntr + 1
		ELSEIF lfs_flg2 AND TMT_PUSH
			IF (lfs_flg3 AND TMT_OPRT) NE 0 AND (lfs_flg4 AND TMT_POP) NE 0
				;; st(1) <+-*/^> st(0)
				fslv_st1_oprt_st0
				lfs_stack_cntr = lfs_stack_cntr - 1
				lfs_skip = 2
			ELSEIF (lfs_flg3 AND TMT_OPERAND) NE 0 AND (lfs_flg4 AND TMT_OPRT) NE 0 AND (lfs_flg5 AND TMT_POP) NE 0
				;; ? <+-*/^> st(0)
				fslv_load_larg
				IFNB lfs_txt
					fslv_mem_oprt_st0
				ELSE
					fslv_stck_chk 1
					fslv_st0_oprt_st1
				ENDIF
				lfs_skip = 3
			ELSEIF (lfs_flg3 AND TMT_OPRT) NE 0
				%echo debug: push,<+-*/^>,? - unknowen situation - report to author
			ELSEIF lfs_flg3 AND TMT_POP
				;; push,pop
				;; hold st(0)
				lfs_skip = 1
			ELSE
				;; hold st(0)
				lfs_stack_cntr = lfs_stack_cntr + 1
			ENDIF			
		ELSEIF lfs_flg2 AND TMT_POP
			%echo debug: pop - unknowen situation - report to author			
		ELSEIF lfs_flg2 AND TMT_STO
		%	IF (lfs_flg3 AND TMT_OPRT) NE 0 AND (lfs_flg4 AND TMT_REC) NE 0 AND llfs_is_idn(<%lfs_txt2>,<%lfs_txt4>) NE 0
				;; st(1) <+-*/^> st(0)
				fslv_sto SubExpr,<%lfs_txt2>,1,%lfs_stack_cntr
				IF lfs_flg4 AND TMT_NEG
					fchs
				ENDIF
				fslv_st1_oprt_st0
				lfs_stack_cntr = lfs_stack_cntr - 1
				lfs_skip = 2
		%	ELSEIF (lfs_flg3 AND TMT_OPERAND) NE 0 AND (lfs_flg4 AND TMT_OPRT) NE 0 AND (lfs_flg5 AND TMT_REC) NE 0 AND llfs_is_idn(<%lfs_txt2>,<%lfs_txt5>) NE 0
				;; ? <+-*/^> st(0)
				fslv_sto SubExpr,<%lfs_txt2>,1,%lfs_stack_cntr
				IF lfs_flg5 AND TMT_NEG
					fchs
				ENDIF
				fslv_load_larg
				IFNB lfs_txt					
					fslv_mem_oprt_st0
				ELSE
					fslv_stck_chk 1
					fslv_st0_oprt_st1
				ENDIF
				lfs_skip = 3
		%	ELSEIF (lfs_flg3 AND TMT_REC) NE 0 AND llfs_is_idn(<%lfs_txt2>,<%lfs_txt3>) NE 0 AND (lfs_flg4 AND TMT_REC) NE 0 AND llfs_is_idn(<%lfs_txt2>,<%lfs_txt4>) NE 0
				;; sto x , rec x , rec x
				fslv_sto SubExpr,<%lfs_txt2>,2,%lfs_stack_cntr
				IF (lfs_flg3 AND TMT_NEG) NE 0 AND (lfs_flg4 AND TMT_NEG) NE 0
					fchs
					fld st
				ELSEIF lfs_flg3 AND TMT_NEG
					fld st
					fchs
					fxch
				ELSEIF lfs_flg4 AND TMT_NEG
					fld st
					fchs
				ELSE
					fld st
				ENDIF
				lfs_stack_cntr = lfs_stack_cntr + 1
				lfs_skip = 2
		%	ELSEIF (lfs_flg3 AND TMT_REC) NE 0 AND llfs_is_idn(<%lfs_txt2>,<%lfs_txt3>) NE 0
				;; sto x, rec x
				;; hold
				fslv_sto SubExpr,<%lfs_txt2>,1,%lfs_stack_cntr
				IF lfs_flg3 AND TMT_NEG
					fchs
				ENDIF
				lfs_skip = 1
			ELSE
				fslv_sto SubExpr,<%lfs_txt2>,0,%lfs_stack_cntr
				lfs_stack_cntr = lfs_stack_cntr - 1
			ENDIF
		ELSEIF lfs_flg2 AND TMT_REC
			lfs_txt TEXTEQU fslv_rec(SubExpr,<%lfs_txt2>,%lfs_stack_cntr)
			IFNB lfs_txt
				fld lfs_txt
			ENDIF
			IF lfs_flg2 AND TMT_NEG
				fchs
				lfs_txt TEXTEQU <>
			ENDIF
			lfs_stack_cntr = lfs_stack_cntr + 1
		ELSEIF lfs_flg2 AND TMT_OPRT
			;; st(0) <+-*/^> ?
			lfs_powflg = 0
			IF lfs_flg3 AND TMT_OPERAND
				fslv_load_rarg
			ELSE
				.err <missing operand>
				EXITM
			ENDIF
			IFNB lfs_txt
				fslv_st0_oprt_mem
			ELSE
				IFE ((lfs_powflg EQ 2 OR lfs_powflg EQ 4) OR (lfs_powflg EQ 8 OR lfs_powflg EQ 9))
					fslv_stck_chk 1
				ENDIF
				IFIDN lfs_txt2,<+>
					faddp st(1),st(0)
				ELSEIFIDN lfs_txt2,<->
					fsubp st(1),st(0)
				ELSEIFIDN lfs_txt2,<*>
					fmulp st(1),st(0)
				ELSEIFIDN lfs_txt2,</>
					fdivp st(1),st(0)
				ELSEIFIDN lfs_txt2,<^>
					IF lfs_powflg EQ 0
						call fpu_st1_pow_st0
					ELSEIF lfs_powflg EQ 2 OR lfs_powflg EQ 4
						fmul st(0),st(0)
					ELSEIF lfs_powflg EQ 3 OR lfs_powflg EQ 5
						fslv_stck_chk 2
						fld st
						fmul st,st
						fmulp st(1),st(0)
					ELSEIF lfs_powflg EQ 8 OR lfs_powflg EQ 9
						fsqrt
					ENDIF
					IF lfs_powflg EQ 4 OR lfs_powflg EQ 5 OR lfs_powflg EQ 1 OR lfs_powflg EQ 9
						fslv_stck_chk 2
						fld1
						fdivrp st(1),st(0)
					ENDIF
				ENDIF
			ENDIF
			lfs_skip = 1
		ELSEIF lfs_flg2 AND TMT_FNC
			lfs_flg = lfs_flg2
			lfs_txt TEXTEQU lfs_txt2
			fslv_call_fnc
		ELSEIF lfs_flg2 AND TMT_NEG
			fchs
		ELSE
			.err <unknowen>
		ENDIF
	:Z
		fslv_stck_chk 0
		;---------------------------------------
		; - make one step forward in 
		;   token-buffer
		;---------------------------------------
		lfs_flg2 = lfs_flg3
		lfs_flg3 = lfs_flg4
		lfs_flg4 = lfs_flg5
		lfs_txt2 TEXTEQU lfs_txt3
		lfs_txt3 TEXTEQU lfs_txt4
		lfs_txt4 TEXTEQU lfs_txt5
		IF lfs_iNode
			lfs_txt5 TEXTEQU ll_get_data(SlvPath,%lfs_iNode,1)
			lfs_flg5 = ll_get_data(SlvPath,%lfs_iNode,2)
			lfs_iNode = ll_get_next(SlvPath,%lfs_iNode)
		ELSE
			lfs_txt5 TEXTEQU <>
			lfs_flg5 = 0
		ENDIF
		
	ENDM

	IF iml_get_max(StkSlot) GE 0
		lfs_cntr=0
		REPEAT iml_get_max(StkSlot)+1
		%	ffree st(lfs_stack_cntr+lfs_cntr)
			lfs_cntr = lfs_cntr + 1
		ENDM
	ENDIF
endm

fslv_stck_chk macro nstck:req
	IF lfs_stack_cntr + nstck GT 8
		.err <ERROR: FPU stack overflow - reformulate expression>
	ENDIF	
endm

; st(0) <+-*/^> real4/8/10/int32/64
fslv_st0_oprt_mem macro
	
	IF @IsRealType(<%lfs_txt>) EQ 10 OR @IsIntegerType(<%lfs_txt>) EQ 8
		IF iit_type EQ 8
			fild lfs_txt
		ELSE
			fld lfs_txt
		ENDIF
		fslv_stck_chk 1
		IFIDN lfs_txt2,<+>
			faddp st(1),st(0)
		ELSEIFIDN lfs_txt2,<->
			fsubp st(1),st(0)
		ELSEIFIDN lfs_txt2,<*>
			fmulp st(1),st(0)
		ELSEIFIDN lfs_txt2,</>
			fdivp st(1),st(0)
		ELSEIFIDN lfs_txt2,<^>
			call fpu_st1_pow_st0
		ENDIF
	ELSE	
		IF @InStr(1,<%lfs_txt>,<%lfs_st>) NE 1
			IFIDN lfs_txt2,<+>
				@CatStr(<f>,@SubStr(<i>,1,%(@IsIntegerType(<%lfs_txt>) NE 0) AND 1),<add >,<%lfs_txt>)
				;fadd lfs_txt
			ELSEIFIDN lfs_txt2,<->
				@CatStr(<f>,@SubStr(<i>,1,%(@IsIntegerType(<%lfs_txt>) NE 0) AND 1),<sub >,<%lfs_txt>)
				;fsub lfs_txt
			ELSEIFIDN lfs_txt2,<*>
				@CatStr(<f>,@SubStr(<i>,1,%(@IsIntegerType(<%lfs_txt>) NE 0) AND 1),<mul >,<%lfs_txt>)
				;fmul lfs_txt
			ELSEIFIDN lfs_txt2,</>
				@CatStr(<f>,@SubStr(<i>,1,%(@IsIntegerType(<%lfs_txt>) NE 0) AND 1),<div >,<%lfs_txt>)
				;fdiv lfs_txt
			ELSEIFIDN lfs_txt2,<^>
				fslv_stck_chk 1
				@CatStr(<f>,@SubStr(<i>,1,%(@IsIntegerType(<%lfs_txt>) NE 0) AND 1),<ld >,<%lfs_txt>)
				;fld lfs_txt
				call fpu_st1_pow_st0
			ENDIF
			EXITM
		ELSE
			IFIDN lfs_txt2,<+>
				fadd st(0),lfs_txt
			ELSEIFIDN lfs_txt2,<->
				fsub st(0),lfs_txt
			ELSEIFIDN lfs_txt2,<*>
				fmul st(0),lfs_txt
			ELSEIFIDN lfs_txt2,</>
				fdiv st(0),lfs_txt
			ELSEIFIDN lfs_txt2,<^>
				fslv_stck_chk 1
				fld lfs_txt
				fxch
				call fpu_st1_pow_st0
			ENDIF
			EXITM
		ENDIF
	ENDIF
endm

; real4/8/10/int32/64 <+-*/^> st(0)
fslv_mem_oprt_st0 macro
	
	IF @IsRealType(<%lfs_txt>) EQ 10 OR @IsIntegerType(<%lfs_txt>) EQ 8
		IF iit_type EQ 8
			fild lfs_txt
		ELSE
			fld lfs_txt
		ENDIF
		fslv_stck_chk 1
		IFIDN lfs_txt4,<+>
			faddp st(1),st(0)
		ELSEIFIDN lfs_txt4,<->
			fsubrp st(1),st(0)
		ELSEIFIDN lfs_txt4,<*>
			fmulp st(1),st(0)
		ELSEIFIDN lfs_txt4,</>
			fdivrp st(1),st(0)
		ELSEIFIDN lfs_txt4,<^>
			fxch
			call fpu_st1_pow_st0
		ENDIF
	ELSE	
		IF @InStr(1,<%lfs_txt>,<%lfs_st>) NE 1
			IFIDN lfs_txt4,<+>
				@CatStr(<f>,@SubStr(<i>,1,%(@IsIntegerType(<%lfs_txt>) NE 0) AND 1),<add >,<%lfs_txt>)
				;fadd lfs_txt
			ELSEIFIDN lfs_txt4,<->
				@CatStr(<f>,@SubStr(<i>,1,%(@IsIntegerType(<%lfs_txt>) NE 0) AND 1),<subr >,<%lfs_txt>)
				;fsubr lfs_txt
			ELSEIFIDN lfs_txt4,<*>
				@CatStr(<f>,@SubStr(<i>,1,%(@IsIntegerType(<%lfs_txt>) NE 0) AND 1),<mul >,<%lfs_txt>)
				;fmul lfs_txt
			ELSEIFIDN lfs_txt4,</>
				@CatStr(<f>,@SubStr(<i>,1,%(@IsIntegerType(<%lfs_txt>) NE 0) AND 1),<divr >,<%lfs_txt>)
				;fdivr lfs_txt
			ELSEIFIDN lfs_txt4,<^>
				fslv_stck_chk 1
				@CatStr(<f>,@SubStr(<i>,1,%(@IsIntegerType(<%lfs_txt>) NE 0) AND 1),<ld >,<%lfs_txt>)
				;fld lfs_txt
				fxch
				call fpu_st1_pow_st0
			ENDIF
			EXITM
		ELSE
			IFIDN lfs_txt4,<+>
				fadd st(0),lfs_txt
			ELSEIFIDN lfs_txt4,<->
				fsubr st(0),lfs_txt
			ELSEIFIDN lfs_txt4,<*>
				fmul st(0),lfs_txt
			ELSEIFIDN lfs_txt4,</>
				fdivr st(0),lfs_txt
			ELSEIFIDN lfs_txt4,<^>
				fslv_stck_chk 1
				fld lfs_txt
				fxch
				call fpu_st1_pow_st0
			ENDIF
			EXITM
		ENDIF
	ENDIF
endm

fslv_st1_oprt_st0 macro
	IFIDN lfs_txt3,<+>
		faddp st(1),st(0)
	ELSEIFIDN lfs_txt3,<->
		fsubp st(1),st(0)
	ELSEIFIDN lfs_txt3,<*>
		fmulp st(1),st(0)
	ELSEIFIDN lfs_txt3,</>
		fdivp st(1),st(0)
	ELSEIFIDN lfs_txt3,<^>
		call fpu_st1_pow_st0
	ENDIF
endm

fslv_st0_oprt_st1 macro
	IFIDN lfs_txt4,<+>
		faddp st(1),st(0)
	ELSEIFIDN lfs_txt4,<->
		fsubrp st(1),st(0)
	ELSEIFIDN lfs_txt4,<*>
		fmulp st(1),st(0)
	ELSEIFIDN lfs_txt4,</>
		fdivrp st(1),st(0)
	ELSEIFIDN lfs_txt4,<^>
		fxch
		call fpu_st1_pow_st0
	ENDIF
endm

fslv_call_fnc macro
	IF lfs_flg AND (TMT_SINGLE_FLT OR TMT_SINGLE_INT)
		lfs_fncName SUBSTR lfs_txt,@InStr(1,<%lfs_txt>,<,>)+1
	ELSE
		lfs_fncName TEXTEQU lfs_txt
	ENDIF
	lfs_fncName TEXTEQU @TrimStr(<%lfs_fncName>)
	IF llfs_is_fnc_def(<%lfs_fncName>)
		lfs_fnc_nargs = llfs_get_fnc_nargs(<%lfs_fncName>)
		IF (lfs_flg AND (TMT_SINGLE_INT OR TMT_SINGLE_FLT)) NE 0 AND lfs_fnc_nargs NE 1
		%	.err <function <lfs_fncName> require 1 argument>
		ELSE
			fslv_stck_chk llfs_get_fnc_nregs(<%lfs_fncName>)
			IF lfs_flg AND (TMT_SINGLE_INT OR TMT_SINGLE_FLT)
				lfs_txt TEXTEQU @SubStr(<%lfs_txt>,1,%(@InStr(1,<%lfs_txt>,<,>)-1))
				IF lfs_flg AND TMT_SINGLE_INT
					lfs_flg = lfs_flg OR TMT_INT
				ELSE
					lfs_flg = lfs_flg OR TMT_FLT
				ENDIF
				llfs_load_const
				@CatStr(<fslv_fnc_>,<%lfs_fncName>)
				lfs_stack_cntr = lfs_stack_cntr + 1
			ELSEIF lfs_flg AND TMT_SINGLE_ARG
				lfs_arg TEXTEQU @SubStr(<%lfs_txt>,1,%(@InStr(1,<%lfs_txt>,<,>)-1))
				lfs_reg = @MatchStrI(%lfs_arg,1,eax,ebx,ecx,edx,esi,edi,ebp,esp,ax,bx,cx,dx,si,di,bp,sp,al,ah,cl,ch,dl,dh,bl,bh)
				IF lfs_reg
					IF lfs_reg LE 8
						mov DWORD ptr [esp-4],lfs_arg
						lfs_arg TEXTEQU <DWORD ptr [esp-4]>
					ELSEIF lfs_reg LE 16
						mov WORD ptr [esp-4],lfs_arg
						lfs_arg TEXTEQU <WORD ptr [esp-4]>
					ELSE
						mov BYTE ptr [esp-4],lfs_arg
						.if lfs_arg & 80h
							mov BYTE ptr [esp-3],0ffh
						.else
							mov BYTE ptr [esp-3],0
						.endif
						lfs_arg TEXTEQU <WORD ptr [esp-4]>
					ENDIF
				ENDIF
				IF @IsIntegerType(%lfs_arg)
					fild lfs_arg
				ELSE
					fld lfs_arg
				ENDIF				
				@CatStr(<fslv_fnc_>,<%lfs_fncName>)
				lfs_stack_cntr = lfs_stack_cntr + 1
			ELSE
				@CatStr(<fslv_fnc_>,<%lfs_fncName>)
				lfs_stack_cntr = lfs_stack_cntr - (lfs_fnc_nargs - 1)
			ENDIF
			IF lfs_flg AND TMT_NEG
				fchs
			ENDIF
		ENDIF
	ELSE
	%	.err <undefined function used: lfs_fncName>
	ENDIF
endm

fslv_load_rarg macro
	IF lfs_flg3 AND TMT_TXT
		IF lfs_flg3 AND TMT_ST
			flra_tmp = lfs_txt3		; for jwasm 
			lfs_txt TEXTEQU <st(>,%lfs_stack_cntr-(lfs_st_max-flra_tmp)-1,<)>
		ELSE
			flra_reg = @MatchStrI(%lfs_txt3,1,eax,ebx,ecx,edx,esi,edi,ebp,esp,ax,bx,cx,dx,si,di,bp,sp,al,ah,cl,ch,dl,dh,bl,bh)
			IF flra_reg
				IF flra_reg LE 8
					mov DWORD ptr [esp-4],lfs_txt3
					lfs_txt3 TEXTEQU <DWORD ptr [esp-4]>
				ELSEIF flra_reg LE 16
					mov WORD ptr [esp-4],lfs_txt3
					lfs_txt3 TEXTEQU <WORD ptr [esp-4]>
				ELSE
					mov BYTE ptr [esp-4],lfs_txt3
					.if lfs_txt3 & 80h
						mov BYTE ptr [esp-3],0ffh
					.else
						mov BYTE ptr [esp-3],0
					.endif
					lfs_txt3 TEXTEQU <WORD ptr [esp-4]>
				ENDIF
			ENDIF
			IF @IsIntegerType(<%lfs_txt3>) EQ 8
				fild lfs_txt
				lfs_txt TEXTEQU <>
			ELSE
				lfs_txt TEXTEQU lfs_txt3
			ENDIF
		ENDIF
		IF lfs_flg3 AND TMT_NEG
			IFNB lfs_txt
				IF @IsIntegerType(<%lfs_txt>)
					fild lfs_txt
				ELSE
					fld lfs_txt
				ENDIF
			ENDIF
			fchs
			lfs_txt TEXTEQU <>
		ENDIF
	ELSEIF lfs_flg3 AND TMT_CONST
		IFIDN lfs_txt2,<^>
			lfs_txt TEXTEQU <>
			lfs_pos INSTR 1,<2.0 3.0-1.0-2.0-3.0 0.5-0.5>,lfs_txt3
			IF lfs_pos
				lfs_powflg = @SubStr(<200030010004000500008009000>,%lfs_pos,1)
			ENDIF
			IF lfs_powflg EQ 0
				lfs_txt TEXTEQU lfs_txt3
				lfs_flg = lfs_flg3
				lfs_txt TEXTEQU llfs_make_const()
			ENDIF
		ELSE
			lfs_txt TEXTEQU lfs_txt3
			lfs_flg = lfs_flg3
			lfs_txt TEXTEQU llfs_make_const()
		ENDIF
	ELSEIF lfs_flg3 AND TMT_REC
		lfs_txt TEXTEQU fslv_rec(SubExpr,<%lfs_txt3>,%lfs_stack_cntr)
		IF lfs_flg3 AND TMT_NEG
			IFNB lfs_txt
				fld lfs_txt
			ENDIF
			fchs
			lfs_txt TEXTEQU <>
		ENDIF
	ELSEIF lfs_flg3 AND (TMT_SINGLE_FLT OR TMT_SINGLE_INT OR TMT_SINGLE_ARG)
		lfs_flg = lfs_flg3
		lfs_txt TEXTEQU lfs_txt3
		fslv_call_fnc
		lfs_txt TEXTEQU <>
		lfs_stack_cntr = lfs_stack_cntr - 1	;; correction only
	ENDIF
	IFNB lfs_txt
		IF fslv_lcl_real_type EQ 10
			IF @InStr(1,<%lfs_txt>,<%lfs_st>) NE 1 AND @IsIntegerType(<%lfs_txt>) EQ 0
				fld lfs_txt
				lfs_txt TEXTEQU <>
			ENDIF
		ENDIF
	ENDIF
endm

fslv_load_larg macro
	IF lfs_flg3 AND TMT_TXT
		IF lfs_flg3 AND TMT_ST
			flla_tmp = lfs_txt3		; for jwasm
			lfs_txt TEXTEQU <st(>,%lfs_stack_cntr-(lfs_st_max-flla_tmp)-1,<)>
		ELSE
			flla_reg = @MatchStrI(%lfs_txt3,1,eax,ebx,ecx,edx,esi,edi,ebp,esp,ax,bx,cx,dx,si,di,bp,sp,al,ah,cl,ch,dl,dh,bl,bh)
			IF flla_reg
				IF flla_reg LE 8
					mov DWORD ptr [esp-4],lfs_txt3
					lfs_txt3 TEXTEQU <DWORD ptr [esp-4]>
				ELSEIF flla_reg LE 16
					mov WORD ptr [esp-4],lfs_txt3
					lfs_txt3 TEXTEQU <WORD ptr [esp-4]>
				ELSE
					mov BYTE ptr [esp-4],lfs_txt3
					.if lfs_txt3 & 80h
						mov BYTE ptr [esp-3],0ffh
					.else
						mov BYTE ptr [esp-3],0
					.endif
					lfs_txt3 TEXTEQU <WORD ptr [esp-4]>
				ENDIF
			ENDIF
			IF @IsIntegerType(%lfs_txt3) EQ 8
				fild lfs_txt
				lfs_txt TEXTEQU <>
			ELSE
				lfs_txt TEXTEQU lfs_txt3
			ENDIF
		ENDIF
		IF lfs_flg3 AND TMT_NEG
			IFNB lfs_txt
				IF @IsIntegerType(<%lfs_txt>)
					fild lfs_txt
				ELSE
					fld lfs_txt
				ENDIF
			ENDIF
			fchs
			lfs_txt TEXTEQU <>
		ENDIF	
	ELSEIF lfs_flg3 AND TMT_CONST
		lfs_txt TEXTEQU lfs_txt3
		lfs_flg = lfs_flg3
		lfs_txt TEXTEQU llfs_make_const()
	ELSEIF lfs_flg3 AND TMT_REC
		lfs_txt TEXTEQU fslv_rec(SubExpr,<%lfs_txt3>,%lfs_stack_cntr)
		IF lfs_flg3 AND TMT_NEG
			IFNB lfs_txt
				fld lfs_txt
			ENDIF
			fchs
			lfs_txt TEXTEQU <>
		ENDIF
	ELSEIF lfs_flg3 AND (TMT_SINGLE_FLT OR TMT_SINGLE_INT OR TMT_SINGLE_ARG)
		lfs_flg = lfs_flg3
		lfs_txt TEXTEQU lfs_txt3
		fslv_call_fnc
		lfs_txt TEXTEQU <>
		lfs_stack_cntr = lfs_stack_cntr - 1	;; correction only
	ENDIF
	IFNB lfs_txt
		IF fslv_lcl_real_type EQ 10
			IF @InStr(1,<%lfs_txt>,<%lfs_st>) NE 1 AND @IsIntegerType(<%lfs_txt>) EQ 0
				fld lfs_txt
				lfs_txt TEXTEQU <>
			ENDIF
		ENDIF
	ENDIF
endm

fslv_sto macro SubExpr,refValue:req,RecCnt:req,nStack
	fss_nUse = lst_get_data(SubExpr,%(refValue+2),2)
	IF fss_nUse GT RecCnt AND RecCnt NE 0
		IF lfs_nss LT 8
			lfs_nss = lfs_nss + 1
			fss_index=iml_request_index(StkSlot)
			lst_set_data SubExpr,%(refValue+2),2,%(fss_nUse-RecCnt)
			lst_set_data SubExpr,%(refValue+2),3,%fss_index
			lst_set_data SubExpr,%(refValue+2),4,<1>					; type = stack
		%	fst st(nStack+fss_index)
		ELSE
			.err <internal error: slot-type = memory>
;			fss_index = iml_request_index(MemSlot)
;			lst_set_data SubExpr,%(refValue+2),2,%(fss_nUse-RecCnt)
;			lst_set_data SubExpr,%(refValue+2),3,%fss_index
;			lst_set_data SubExpr,%(refValue+2),4,<0>					; type = memory
;			IF fslv_lcl_real_type EQ 10
;				fld st
;				fstp @CatStr(<REAL10 ptr [esp->,%(16+(fss_index+1)*16),<]>)
;			ELSE 
;				fst @CatStr(<REAL>,%fslv_lcl_real_type,< ptr [esp->,%(16+(fss_index+1)*@InStr(1,<   r4  r8>,<%fslv_oprnd_size>)),<]>)
;			ENDIF
		ENDIF
	ELSEIF RecCnt EQ fss_nUse
		; hold
		lst_set_data SubExpr,%(refValue+2),2,0
	ELSEIF RecCnt EQ 0
		IF lfs_nss LT 8
			lfs_nss = lfs_nss + 1
			fss_index=iml_request_index(StkSlot)
			lst_set_data SubExpr,%(refValue+2),3,%fss_index
			lst_set_data SubExpr,%(refValue+2),4,<1>					; type = stack
			IF nStack+fss_index NE 1
			%	fstp st(nStack+fss_index)
			ENDIF
		ELSE
			.err <internal error: slot-type = memory>
;			fss_index = iml_request_index(MemSlot)
;			lst_set_data SubExpr,%(refValue+2),3,%fss_index
;			lst_set_data SubExpr,%(refValue+2),4,<0>					; type = memory
;		%	fstp @CatStr(<REAL>,@SubStr(<%fslv_oprnd_size>,2),< ptr [esp->,%(16+(fss_index+1)*@InStr(1,<   r4  r8      r10>,<%fslv_oprnd_size>)),<]>)
		ENDIF
	ENDIF
endm

fslv_rec macro SubExpr,refValue:req,nStack
	fsr_index = lst_get_data(SubExpr,%(refValue+2),3)
	fsr_type = lst_get_data(SubExpr,%(refValue+2),4)
	fsr_val=lst_dec_value(SubExpr,%(refValue+2),2)
	IF fsr_type
		IF fsr_val EQ 0
			lfs_nss = lfs_nss - 1
			iml_free_index StkSlot,%fsr_index
		ENDIF
		EXITM @CatStr(<st!(>,%(nStack+fsr_index),<!)>)
	ELSE
		.err <internal error: slot-type = memory>
;		IF fsr_val EQ 0
;			iml_free_index MemSlot,%fsr_index
;		ENDIF
;		EXITM @CatStr(<REAL>,%fslv_lcl_real_type,< ptr [esp->,(fsr_index+1)*@InStr(1,<   r4  r8r10>,<%fslv_oprnd_size>),<]>)
	ENDIF
endm

llfs_is_idn macro param1,param2
	IFIDNI <param1>,<param2>
		EXITM <1>
	ENDIF
	EXITM <0>	
endm

llfs_load_const macro
	
	IF lfs_flg AND TMT_PI
		fldpi
		IF lfs_flg AND TMT_NEG
			fchs
		ENDIF
	ELSEIF @InStr(1,<1 1.0>,<%lfs_txt>) LE 3 AND @InStr(1,<1 1.0>,<%lfs_txt>) NE 0
		fld1
		IF lfs_flg AND TMT_NEG
			fchs
		ENDIF
	ELSEIF @InStr(1,<-1 -1.0>,<%lfs_txt>) LE 5 AND @InStr(1,<-1 -1.0>,<%lfs_txt>) NE 0
		fld1
		IFE lfs_flg AND TMT_NEG
			fchs
		ENDIF
	ELSEIF @InStr(1,<0 0.0>,<%lfs_txt>) LE 3 AND @InStr(1,<0 0.0>,<%lfs_txt>) NE 0
		fldz
	ELSE
		IF lfs_flg AND TMT_INT
			IF fslv_lcl_int_type EQ 2
				llfsmc_var TEXTEQU @CatStr(<fslv_int2_>,%lfs_lbl)
				llfsmc_type TEXTEQU <SWORD>
			ELSEIF fslv_lcl_int_type EQ 4
				llfsmc_var TEXTEQU @CatStr(<fslv_int4_>,%lfs_lbl)
				llfsmc_type TEXTEQU <SDWORD>
			ELSE
				llfsmc_var TEXTEQU @CatStr(<fslv_int8_>,%lfs_lbl)
				llfsmc_type TEXTEQU <SQWORD>
			ENDIF
		ELSE
			llfsmc_var TEXTEQU @CatStr(<fslv_real>,%fslv_lcl_real_type,<_>,%lfs_lbl)
			llfsmc_type TEXTEQU @CatStr(<REAL>,%fslv_lcl_real_type)
		ENDIF

		llfsmc_var TEXTEQU @fSlvCreateConstant(%llfsmc_var,%lfs_txt,%llfsmc_type)

		IF lfs_flg AND TMT_INT
			fild llfsmc_var
		ELSE
			fld llfsmc_var
		ENDIF
		IF lfs_flg AND TMT_NEG
			fchs
		ENDIF
		lfs_lbl = lfs_lbl + 1
	ENDIF
endm

llfs_make_const macro
	IF lfs_flg AND TMT_PI
		fldpi
		IF lfs_flg AND TMT_NEG
			fchs
		ENDIF	
	ELSEIF @InStr(1,<1 1.0>,<%lfs_txt>) LE 3 AND @InStr(1,<1 1.0>,<%lfs_txt>) NE 0
		fld1
		IF lfs_flg AND TMT_NEG
			fchs
		ENDIF
	ELSEIF @InStr(1,<-1 -1.0>,<%lfs_txt>)  LE 5 AND @InStr(1,<-1 -1.0>,<%lfs_txt>) NE 0
		fld1
		IFE lfs_flg AND TMT_NEG
			fchs
		ENDIF
	ELSEIF @InStr(1,<0 0.0>,<%lfs_txt>) LE 3 AND @InStr(1,<0 0.0>,<%lfs_txt>) NE 0
		fldz
	ELSE
		IF lfs_flg AND TMT_INT
			IF fslv_lcl_int_type EQ 2
				llfsmc_var TEXTEQU @CatStr(<fslv_int2_>,%lfs_lbl)
				llfsmc_type TEXTEQU <SWORD>
			ELSEIF fslv_lcl_int_type EQ 4
				llfsmc_var TEXTEQU @CatStr(<fslv_int4_>,%lfs_lbl)
				llfsmc_type TEXTEQU <SDWORD>
			ELSE
				llfsmc_var TEXTEQU @CatStr(<fslv_int8_>,%lfs_lbl)
				llfsmc_type TEXTEQU <SQWORD>
			ENDIF
		ELSE
			llfsmc_var TEXTEQU @CatStr(<fslv_real>,%fslv_lcl_real_type,<_>,%lfs_lbl)
			llfsmc_type TEXTEQU @CatStr(<REAL>,%fslv_lcl_real_type)
		ENDIF		
		
		llfsmc_var TEXTEQU @fSlvCreateConstant(%llfsmc_var,%lfs_txt,%llfsmc_type)
		
		lfs_lbl = lfs_lbl + 1
		IF lfs_flg AND TMT_NEG
			IF lfs_flg AND TMT_INT
				fild llfsmc_var
			ELSE
				fld llfsmc_var
			ENDIF
			fchs
		ELSE
			EXITM llfsmc_var
		ENDIF		
	ENDIF
	EXITM <>
endm

llfs_is_fnc_def macro FncName:req
	IFDEF fslv_fnc_&FncName&_dscptr
		EXITM <1>
	ENDIF
	EXITM <0>
endm

llfs_get_fnc_nargs macro FncName:req
	EXITM fslv_fnc_&FncName&_dscptr(%FNCD_NARGS)
endm

llfs_get_fnc_nregs macro FncName:req
	EXITM fslv_fnc_&FncName&_dscptr(%FNCD_NREGS)
endm


fscc_nconsts = 0
fscc_nreused   = 0 ; for statistic purpose
fscc_nr_SWORD  = 0 ;
fscc_nr_SDWORD = 0 ;
fscc_nr_SQWORD = 0 ;
fscc_nr_REAL4  = 0 ;
fscc_nr_REAL8  = 0 ;
fscc_nr_REAL10 = 0 ;
fscc_nc_SWORD  = 0 ;
fscc_nc_SDWORD = 0 ;
fscc_nc_SQWORD = 0 ;
fscc_nc_REAL4  = 0 ;
fscc_nc_REAL8  = 0 ;
fscc_nc_REAL10 = 0 ;

; use ONLY!: SWORD DWORD QWORD REAL4 REAl8 REAL10
; REAL4/8/10 must be written as FP-values
@fSlvCreateConstant macro name:req,value:req,_type:req
	
	fscc_value TEXTEQU @TrimStr(<&value>)
	fscc_cntr = 0
	fscc_match = 0
	IFE @fslv_test_attribute(<no const reuse>) OR @fslv_test_attribute(<no reuse>)
		IFE (fslv_glb_flags AND FSF_REUSE_BY_TYPE)
			WHILE fscc_cntr LT fscc_nconsts
			%	IFIDNI <fscc_value>,<@CatStr(<fscc_const_value_>,%fscc_cntr)>
					fscc_txt TEXTEQU @CatStr(<fscc_const_type_>,%fscc_cntr)
					fscc_tpos1 INSTR 1,<SWORD SDWORD SQWORD REAL4 REAL8 REAL10>,fscc_txt
					fscc_tpos2 INSTR 1,<SWORD SDWORD SQWORD REAL4 REAL8 REAL10>,<&_type>
					IF fscc_tpos1 GE 21 AND fscc_tpos2 GE 21 AND fscc_tpos2 EQ fscc_tpos1
						fscc_match=1
						EXITM
					ELSEIF fscc_tpos1 NE 0 AND fscc_tpos2 NE 0 AND fscc_tpos1 LE 14 AND fscc_tpos2 LE 14 AND fscc_tpos2 GE fscc_tpos1
						fscc_match=1
						EXITM
					ENDIF
				ENDIF
				fscc_cntr=fscc_cntr+1
			ENDM
		ELSE
			WHILE fscc_cntr LT fscc_nconsts
			%	IFIDNI <&_type>,<@CatStr(<fscc_const_type_>,%fscc_cntr)>
				%	IFIDNI <fscc_value>,<@CatStr(<fscc_const_value_>,%fscc_cntr)>
						fscc_match=1
						EXITM
					ENDIF
				ENDIF
				fscc_cntr=fscc_cntr+1
			ENDM
		ENDIF
	ENDIF
	IF fscc_match
		fscc_nreused=fscc_nreused+1
		@CatStr(<fscc_nr_>,%@CatStr(<fscc_const_type_>,%fscc_cntr)) = @CatStr(<fscc_nr_>,%@CatStr(<fscc_const_type_>,%fscc_cntr)) + 1
		EXITM @CatStr(<fscc_const_name_>,%fscc_cntr)
	ELSE
		.const
			fscc_align = @InStr(1,<SWORD SDWORD SQWORD REAL4 REAL8 REAL10>,<&_type>)
			IF fscc_align
				fscc_align = @SubStr(<50000010000002000000100000200000400000>,%fscc_align,1)
			ENDIF
			IFE fscc_align
				.err <invalid type qualifier: &_type>
			ELSE
				IF fscc_align NE 5
					align fscc_align*4
				ELSE
					align 2
				ENDIF
			ENDIF

			IFIDNI <&_type>,<SWORD>
				IF value LT -32768 OR value GT 32767
					.err <constant !value out of range (SWORD): &value : change local or global precision to SDWORD>
				ENDIF
				name _type value
			ELSEIFIDNI <&_type>,<SDWORD>
				IF value LT -2147483648 OR value GT 2147483647
					.err <constant !value out of range (SDWORD): &value : change local or global precision to SQWORD>
				ENDIF
				name _type value
			ELSEIFIDNI <&_type>,<SQWORD>
				IF fslv_glb_flags AND FSF_CHECK_SQWORD
					IFE chk_sqword(&value)
						.err <constant !value out of range (SQWORD): &value : switch to floating point constant>
					ENDIF
				ENDIF
				name QWORD value
			ELSE
				name _type value
			ENDIF
		.code
		@CatStr(<fscc_nc_>,<&_type>) = @CatStr(<fscc_nc_>,<&_type>) + 1
		@CatStr(<fscc_const_value_>,%fscc_nconsts) TEXTEQU fscc_value
		@CatStr(<fscc_const_type_>,%fscc_nconsts) TEXTEQU <&_type>
		@CatStr(<fscc_const_name_>,%fscc_nconsts) TEXTEQU <&name>
		fscc_nconsts=fscc_nconsts+1
		EXITM <&name>
	ENDIF
endm

chk_sqword macro num:req
	FOR numtxt,<&num>
		chksq_num TEXTEQU <&numtxt>
	ENDM
	chksq_char SUBSTR chksq_num,1,1
	chksq_max TEXTEQU <9223372036854775807>
	IFIDN chksq_char,<+>
		chksq_num SUBSTR chksq_num,2
	ELSEIFIDN chksq_char,<->
		chksq_num SUBSTR chksq_num,2
		chksq_max TEXTEQU <9223372036854775808>
	ENDIF
	
	chksq_size1 SIZESTR chksq_num
	chksq_size2 SIZESTR <9223372036854775807>
	IF chksq_size1 LT chksq_size2
		EXITM <1>
	ELSEIF chksq_size1 GT chksq_size2
		EXITM <0>
	ENDIF
	chksq_result = 1
	chksq_pos = 1
%	FORC char,<chksq_num>
		chksq_char SUBSTR chksq_max,chksq_pos,1
		IFE @InStr(1,<0123456789>,%chksq_char) ;; probably hexa-decimal number
			chksq_result=0 
			EXITM
		ENDIF
		IF char GT chksq_char
			chksq_result=0
			EXITM
		ENDIF
		chksq_pos=chksq_pos+1
	ENDM
	EXITM @CatStr(%chksq_result)
endm

fslv_extract_attributes macro args:VARARG
	fsea_size SIZESTR <&args>
	fsea_found = 0
	IFE fsea_size
		EXITM
	ENDIF
	fsea_pos INSTR 1,<&args>,<{>
	IFE fsea_pos
		EXITM
	ELSE
		fsea_pos2 INSTR fsea_pos,<&args>,<}>
		fsea_attributes SUBSTR <&args>,fsea_pos+1,fsea_pos2-fsea_pos-1
		IF fsea_pos EQ 1
			fsea_expr SUBSTR <&args>,fsea_pos2+1
		ELSEIF fsea_pos2 GE fsea_size
			fsea_expr SUBSTR <&args>,1,fsea_pos-1
		ELSE
			fsea_expr CATSTR @SubStr(<&args>,1,%fsea_pos-1),@SubStr(<&args>,%fsea_pos2+1)
		ENDIF
		fsea_found = 1
	ENDIF
endm

fslv_local_type_from_attributes macro def_real_type:req,def_int_type:req,attributes:VARARG

fslv_lcl_real_type = def_real_type
fslv_lcl_int_type  = def_int_type

flta_int = 0
flta_real = 0
	FOR attr,<attributes>
		flta_size SIZESTR <&attr>
		IF @InStr(1,<i2 i4 i8>,<&attr>) NE 0 AND flta_size EQ 2
			IFE flta_int
				flta_int = 1
				fslv_lcl_int_type = @SubStr(<&attr>,2,1)
			ELSE
				.err <integer-type-attribute found two or more times>
			ENDIF
		ELSEIF @InStr(1,<r4 r8 r10>,<&attr>) NE 0 AND (flta_size EQ 3 OR flta_size EQ 2)
			IFE flta_real
				flta_real = 1
				fslv_lcl_real_type = @SubStr(<&attr>,2)
			ELSE
				.err <real-type-attribute found two or more times>
			ENDIF
		ENDIF
	ENDM
endm

; sepeartor = optional char. (exact 1) seperating attribute and value e.g. <name:value>
@fslv_test_attribute macro name:req,separator,default_value
	fsta_found = 0
	fsta_attr TEXTEQU <&name>
	fsta_value TEXTEQU <&default_value>
	IF fslv_attributes?
	%	FOR attr,<fslv_current_attributes>
			IFB <&separator>
				IFIDNI <&name>,<&attr>
					fsta_found=1
					EXITM
				ENDIF
			ELSE
				IFNB <&attr>
					fsta_pos INSTR 1,<&attr>,<&separator>
					IF fsta_pos GT 1
						fsta_attr SUBSTR <&attr>,1,fsta_pos-1
						IFIDNI fsta_attr,<&name>
							IF fsta_pos+1 LE @SizeStr(<&attr>)
								fsta_value SUBSTR <&attr>,fsta_pos+1
							ENDIF
							fsta_found=1
							EXITM
						ENDIF
					ENDIF
				ENDIF
			ENDIF
		ENDM
	ENDIF
	EXITM @CatStr(%fsta_found)	
endm

@fslv_TLS? macro
	IFB fslv_tls_name
		EXITM <0>
%	ELSEIFDEF fslv_tls_name
		EXITM <1>
	ENDIF
	EXITM <0>
endm

@fslvTLS_line_alloc macro _type:req
	IFB fslv_tls_name
		.err <thread local storage not initialized>
		EXITM <0>
%	ELSEIFNDEF fslv_tls_name
		.err <thread local storage not initialized>
		EXITM <0>
	ENDIF
	
	IF fslvTLS_curr_line NE @Line
		fslvTLS_curr_line = @Line
		fslv_tls_curr_used = 0
		fslv_tls_byte = 0
	ENDIF
	
	IFIDNI <&_type>,<BYTE>
		fsta_size = 1
	ELSEIFIDNI <&_type>,<SDWORD>
		fsta_size = 4
	ELSEIFIDNI <&_type>,<REAL4>
		fsta_size = 4
	ELSEIFIDNI <&_type>,<REAL8>
		fsta_size = 8
	ELSEIFIDNI <&_type>,<REAL10>
		fsta_size = 12
	ELSE
		.err <invalid type qualifier>
		EXITM <0>
	ENDIF
	
	IF fslv_tls_byte NE 0 AND fsta_size NE 1
		IF ((fslv_tls_curr_used+3)AND(-4))+fsta_size GT fslv_tls_cb
		%	.err <TLS: buffer to small - increase size at line @CatStr(%fslv_tls_alloc_line) : fSlvTLS[,@CatStr(%((fslv_tls_curr_used+3)AND(-4))+fsta_size)]>
			fslv_tls_curr_used = ((fslv_tls_curr_used+3)AND(-4)) + fsta_size
			EXITM <0>
		ENDIF
		fsta_var TEXTEQU <&_type ptr >,fslv_tls_name,<[>,%((fslv_tls_curr_used+3)AND(-4)),<]>
		fslv_tls_curr_used = ((fslv_tls_curr_used+3)AND(-4)) + fsta_size
		fslv_tls_byte = 0
	ELSE
		IF fslv_tls_curr_used+fsta_size GT fslv_tls_cb
		%	.err <TLS: buffer to small - increase size at line @CatStr(%fslv_tls_alloc_line) : fSlvTLS[,@CatStr(%fslv_tls_curr_used+fsta_size)]>
			fslv_tls_curr_used = fslv_tls_curr_used + fsta_size
			EXITM <0>
		ENDIF
		fsta_var TEXTEQU <&_type ptr >,fslv_tls_name,<[>,%fslv_tls_curr_used,<]>
		fslv_tls_curr_used = fslv_tls_curr_used + fsta_size
		IF fsta_size NE 1
			fslv_tls_byte = 0
		ELSE
			fslv_tls_byte = 1
		ENDIF
	ENDIF
	EXITM fsta_var
endm

fslv_tls_warning_line = -1
@TLS_warning macro
	IF FSLV_SUPPRESS_ALL_WARNINGS EQ 0 AND FSLV_SUPPRESS_TS_WARNIGNS EQ 0 AND fslv_tls_warning_line NE @Line
		%echo @FileCur: WARNING: code not thread save: line @CatStr(%@Line)
	ENDIF
	fslv_tls_warning_line = @Line
endm