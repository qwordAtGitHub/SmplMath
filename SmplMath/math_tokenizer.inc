TK_EXPR		EQU 1
TK_TXT		EQU 2
TK_OPRT		EQU 4
Tokenize macro Pos:req,Expression:=< >
	tk_pos		= 1
	tk_flag    	= 0
	tk_SqrBrkt 	= 0
	tk_error	= 0
	tk_txt 		TEXTEQU <>
	tk_exp		TEXTEQU <>
	tk_op		TEXTEQU <>
	tk_err_str  TEXTEQU <>

	FORC char,<&Expression>
		IF tk_pos LT Pos 
			GOTO Z
		ENDIF
		
		tk_switch INSTR 1,<[]()+-*/^,#>,<&char>
		IF tk_switch EQ 0
			tk_flag = tk_flag OR TK_TXT
			tk_txt CATSTR tk_txt,<&char>
			GOTO Z
		ELSEIF tk_switch GE 5
			IF tk_SqrBrkt EQ 0
				tk_flag = tk_flag OR TK_OPRT
				tk_op SUBSTR <+-*/^,#>,tk_switch-4,1
				EXITM
			ELSE
				tk_txt CATSTR tk_txt,<&char>
			ENDIF
			GOTO Z
		ELSE
			IFIDN <&char>,<!(>
				IF tk_SqrBrkt EQ 0
					tk_exp TEXTEQU tk_get_expression(1,<%@SubStr(<&Expression>,%tk_pos)>)
					IF tkge_error EQ 1
						tk_error = 1
						tk_err_str  TEXTEQU <missing brace: '(' or ')'>
					ENDIF
					tk_flag = tk_flag OR TK_EXPR
					tk_pos = tk_pos + tkge_pos
					EXITM
				ELSE
					tk_txt CATSTR tk_txt,<&char>
				ENDIF
			ELSEIFIDN <&char>,<!)>
				IF tk_SqrBrkt EQ 0
					tk_error = 1
					tk_err_str  TEXTEQU <unexpected brace ')'>
					EXITM
				ELSE
					tk_txt CATSTR tk_txt,<&char>
				ENDIF
			ELSEIFIDN <&char>,<[>
				tk_SqrBrkt = tk_SqrBrkt + 1
				tk_txt CATSTR tk_txt,<&char>
				tk_flag = tk_flag OR TK_TXT
			ELSEIFIDN <&char>,<]>
				tk_SqrBrkt = tk_SqrBrkt - 1
				tk_txt CATSTR tk_txt,<&char>
			ENDIF
		ENDIF
	:Z	
		tk_pos = tk_pos + 1
	ENDM

	IF tk_error EQ 1
		EXITM <>
	ELSEIF tk_SqrBrkt NE 0
		tk_err_str  TEXTEQU <missing square-bracket: [ or ]>
		tk_error = 1
		EXITM <>
	ENDIF

	IF ((tk_flag AND TK_TXT) NE 0) AND ((tk_flag AND TK_OPRT) NE 0)
	%	IFNB <tk_txt>
			tk_flag = TK_TXT
			EXITM tk_txt
		ELSE
			tk_flag = TK_OPRT
			tk_pos = tk_pos + 1
			EXITM tk_op
		ENDIF	
	ELSEIF ((tk_flag AND TK_EXPR) NE 0) AND ((tk_flag AND TK_TXT) NE 0)
	%	IFNB <tk_txt>
			EXITM tk_exp
		ELSE
			tk_flag = TK_EXPR
			EXITM tk_exp
		ENDIF		
	ELSEIF (tk_flag AND TK_OPRT) NE 0
		tk_flag = TK_OPRT
		tk_pos = tk_pos + 1
		EXITM tk_op
	ELSEIF (tk_flag AND TK_EXPR) NE 0
		EXITM tk_exp
	ELSE	
		EXITM tk_txt
	ENDIF
endm

;in: Pos  = points to '('
;	 Expr = expression
;return: expression without enclosing braces
;	 tkge_error: 0 = no error
;               1 = missing bracket
;    tkge_pos: points to next char. after ')' 		
tk_get_expression macro Pos:req,Expr:=<>
	tkge_pos 	= 1
	tkge_nesting = 0
	tkge_txt 	TEXTEQU <>
	tkge_error	= 0
	FORC char,<&Expr>
		IF tkge_pos LT Pos
			GOTO Z
		ENDIF
		tge_switch INSTR 1,<()>,<&char>
		IF tge_switch EQ 0
			tkge_txt CATSTR tkge_txt,<&char>
			GOTO Z
		ELSEIF tge_switch EQ 1
			IF tkge_nesting EQ 0
				tkge_nesting = tkge_nesting + 1
			ELSE
				tkge_nesting = tkge_nesting + 1
				tkge_txt CATSTR tkge_txt,<&char>
			ENDIF
		ELSE
			tkge_nesting = tkge_nesting - 1
			IF tkge_nesting EQ 0
				EXITM
			ELSE
				tkge_txt CATSTR tkge_txt,<&char>
			ENDIF			
		ENDIF
	:Z	
		tkge_pos = tkge_pos + 1
	ENDM

	IF tkge_nesting NE 0
		tkge_error = 1			;missing brace
		EXITM <>
	ENDIF
	EXITM tkge_txt
endm

TMT_unused1		EQU 1
TMT_TXT			EQU 2
TMT_EXPR 		EQU 8
TMT_SINGLE_ARG	EQU 16
TMT_FLT			EQU 32
TMT_INT			EQU 64
TMT_END_OF_EXPR	EQU 128
TMT_BEGIN_EXPR	EQU 256
TMT_OPRT		EQU 512
TMT_MINUS		EQU 1024
TMT_PLUS		EQU 2048
TMT_NEG			EQU 4096
TMT_COMMA		EQU 8192
TMT_PUSH		EQU 16384
TMT_POP			EQU 32768
TMT_STO			EQU 65536
TMT_REC			EQU 131072
TMT_FNC			EQU 262144
TMT_unused2		EQU 524288
TMT_PI			EQU 1048576
TMT_SINGLE_FLT	EQU 2097152
TMT_SINGLE_INT	EQU 4194304
TMT_ST			EQU 8388608			; fpu specific - not used by ll_MathTokenize

TMT_CONST		= TMT_FLT OR TMT_INT
TMT_OPERAND		= TMT_CONST OR TMT_TXT OR TMT_REC OR TMT_SINGLE_ARG OR TMT_SINGLE_FLT OR TMT_SINGLE_INT

ll_MathTokenize macro SlvPath:req,SubExpr:req,Expression:=<>
	
	tmt_pos = 1
	tmt_nesting = 1
	tmt_brc_pos INSTR 1,<&Expression>,<!)!(>
	IF tmt_brc_pos	; bugfix for masm - it fails on expr. with deep brace nestings
		tmt_expr TEXTEQU @RepAllStr(1,<&Expression>,<!)!(>,<!)*!(>)
	ELSE
		tmt_expr TEXTEQU <&Expression>
	ENDIF
	tmt_flag = 0
	tmt_error = 0
	tmt_err_txt TEXTEQU <>

	ll_create SlvPath,2
	lst_create SubExpr
	
	tmt_iNode = 1
	tmt_prdct = 0
	tmt_sum = 0
	tmt_exp = 0
	tmt_pps = 0
	tmt_exps = 0
	tmt_iSub = 0
	tmt_iTmp = 0
	tmt_iLast = 1
	tmt_iNext = 1
	tmt_iLastSum = 1
	tmt_iTop	 = 2
	tmt_iLastPrdct = 1
	tmt_iBeginPrdct = 3
		
	tmt_flag = TMT_BEGIN_EXPR
	WHILE 1
		tmt_txt TEXTEQU Tokenize(%tmt_pos,%tmt_expr)
		IF tk_error EQ 1
			EXITM
		ELSE
			IF (tk_flag AND TK_OPRT) NE 0
				tmt_on_oprt SlvPath,SubExpr
				GOTO Z							; avoid strange abort of WHILE-loop on e.g.: "1^(-2)*...." oO
			ELSEIF (tk_flag AND TK_EXPR) NE 0
				tmt_on_expr SlvPath,SubExpr
				GOTO Z
			ELSEIF (tk_flag AND TK_TXT) NE 0
			%	IF fslv_is_reg_const(<%tk_txt>)
					tmt_iLast=ll_insert(SlvPath,%tmt_iNext,0)
					IF fsir_flg LE 2
						IF fsir_flg EQ 1
							ll_set_data SlvPath,%tmt_iLast,2,%(TMT_FLT OR (tmt_flag AND TMT_NEG))
						ELSE
							ll_set_data SlvPath,%tmt_iLast,2,%(TMT_INT OR (tmt_flag AND TMT_NEG))
						ENDIF
						ll_set_data SlvPath,%tmt_iLast,1,%fsir_txt
					ELSE
						ll_set_data SlvPath,%tmt_iLast,2,%(TMT_TXT OR (tmt_flag AND TMT_NEG))
						ll_set_data SlvPath,%tmt_iLast,1,%fsir_txt
					ENDIF
					tmt_pos = tk_pos
			%	ELSEIF @ScanForFlt(%tmt_pos,%tmt_expr) GE 1
					tmt_iLast=ll_insert(SlvPath,%tmt_iNext,0)
					IF ll_is_defined(SlvPath,%tmt_iNext,2)					;; 070711
						IF ll_get_data(SlvPath,%tmt_iNext,2) AND TMT_OPRT	;; x-const -> x+(-const)
							IFIDNI ll_get_data(SlvPath,%tmt_iNext,1),<->	;;
								IFDIF Tokenize(%sff_pos,%tmt_expr),<^>		;; 080711
									sff_numstr CATSTR <->,sff_numstr		;; x-const^y -> keep
									ll_set_data SlvPath,%tmt_iNext,1,<+>	;;
								ENDIF										;;
							ENDIF											;;
						ENDIF                                               ;;
					ENDIF													;;
					IF sff_type EQ 1
						ll_set_data SlvPath,%tmt_iLast,2,%TMT_FLT
					ELSE
						ll_set_data SlvPath,%tmt_iLast,2,%TMT_INT
					ENDIF
					ll_set_data SlvPath,%tmt_iLast,1,%sff_numstr
					tmt_pos = sff_pos
				ELSE
					tmt_iLast=ll_insert(SlvPath,%tmt_iNext,0)
					IFIDNI @TrimStr(%tk_txt),<pi>
						ll_set_data SlvPath,%tmt_iLast,2,%(TMT_PI OR TMT_FLT OR (tmt_flag AND TMT_NEG))
						ll_set_data SlvPath,%tmt_iLast,1,%tk_txt
					ELSE
						ll_set_data SlvPath,%tmt_iLast,2,%(TMT_TXT OR (tmt_flag AND TMT_NEG))
						ll_set_data SlvPath,%tmt_iLast,1,%tk_txt
					ENDIF
					tmt_pos = tk_pos
				ENDIF
				tmt_flag = 0
			ELSE
				IF tmt_nesting EQ 1
					EXITM
				ELSE
					; restore state (return from nesting)
					tmt_nesting = tmt_nesting - 1
					tmt_expr TEXTEQU @CatStr(<tmt_as_>,%tmt_nesting,<_expr>)
				%	FOR param,<@CatStr(<tmt_as_>,%tmt_nesting,<_vars1>)>
						param
					ENDM
				%	FOR param,<@CatStr(<tmt_as_>,%tmt_nesting,<_vars2>)>
						param
					ENDM
					tmt_flag = TMT_END_OF_EXPR
				ENDIF
			ENDIF
		ENDIF
	:Z
		;ll_DebugOut SlvPath,2,8,1,width=10,<<ralign=1,hex=1>>
		IF tmt_error
			EXITM
		ENDIF
	ENDM

	IF tk_error EQ 1
		tmt_error = 1
		tmt_err_txt TEXTEQU tk_err_str
		EXITM
	ELSEIF (tmt_flag AND TMT_OPRT) NE 0
		tmt_error = 1
		tmt_err_txt TEXTEQU <missing operand: check for double operators (if used, also check predefined expressions)>
		EXITM
	ELSEIF tmt_error EQ 1
		EXITM
	ENDIF
endm

tmt_on_oprt macro SlvPath,SubExpr
	IF ((tmt_flag AND (TMT_PLUS OR TMT_MINUS)) NE 0) AND @InStr(1,<+->,%tk_op) NE 0
		tmt_error = 1
		tmt_err_txt TEXTEQU <double sign not allowed: <++>,<-->,<+->,<-+>>
		EXITM
%	ELSEIF ((tmt_flag AND (TMT_BEGIN_EXPR OR TMT_COMMA OR TMT_OPRT)) NE 0) AND @InStr(1,<+->,%tk_op) NE 0
		IF @ScanForFlt(%tmt_pos,%tmt_expr) GE 1
			tmt_iLast=ll_insert(SlvPath,%tmt_iNext,0)
			IF sff_type EQ 1
				ll_set_data SlvPath,%tmt_iLast,2,%TMT_FLT
			ELSE
				ll_set_data SlvPath,%tmt_iLast,2,%TMT_INT
			ENDIF
			ll_set_data SlvPath,%tmt_iLast,1,%sff_numstr
			tmt_pos = sff_pos
			tmt_flag = 0
		ELSE
			tmt_pos  = tk_pos
			tmt_flag = (TMT_NEG*@InStr(1,%tk_op,<->))
		ENDIF
		EXITM
	ELSEIF (tmt_flag AND (TMT_NEG OR TMT_OPRT))
		tmt_error = 1
		tmt_err_txt TEXTEQU <invalid combination of operators>
		EXITM
	ELSE
		IF @InStr(1,<+->,%tk_op) NE 0
			; optimazion for: A+B*C
			tmt_pps = 0
;			IF tmt_sum EQ 0 AND tmt_prdct EQ 0 AND tmt_exp EQ 0
;				IFIDN tk_op,<+>
;					tmt_pps = 1
;				ENDIF
;			ENDIF
			
			;alloc new top-node and prepare previous for usage 
			IF tmt_nesting EQ 1
				tmt_iNext = tmt_iTop
				tmt_iTop = ll_insert(SlvPath,%tmt_iTop,0)
			ELSE
				tmt_iNext = ll_insert(SlvPath,%tmt_iTop)
			ENDIF
			ll_set_data SlvPath,%tmt_iNext,2,%TMT_OPRT
			ll_set_data SlvPath,%tmt_iNext,1,%tk_op
			
			tmt_exp = 0
			tmt_prdct = 0
			tmt_sum = tmt_sum+1
			tmt_iLastSum = tmt_iNext
			tmt_iLastPrdct = tmt_iNext
			tmt_iBeginPrdct = tmt_iNext
			tmt_iSub = tmt_iBeginPrdct
		ELSEIF @InStr(1,<*/#^>,%tk_op) NE 0
			IFIDN tk_op,<^>
				IF tmt_prdct NE 0 OR tmt_exp NE 0 OR tmt_sum NE 0
					tmt_iTmp=ll_insert(SlvPath,%tmt_iLast,0)
					ll_set_data SlvPath,%tmt_iTmp,2,%TMT_POP
					ll_set_data SlvPath,%tmt_iTmp,1,<pop>
					IF tmt_exp NE 0
						tmt_iLast=ll_move(SlvPath,%tmt_iLast,%tmt_iBeginPrdct)
						IF tmt_prdct EQ 0 AND tmt_exp EQ 1
							tmt_iLastPrdct = tmt_iTmp
						ENDIF
						tmt_iSub = tmt_iLast
					ELSE
						tmt_iLast=ll_move(SlvPath,%tmt_iLast,%tmt_iLastPrdct)
						tmt_iLastPrdct = tmt_iTmp
						tmt_iSub = tmt_iLast
					ENDIF
					tmt_iBeginPrdct = tmt_iLast
					tmt_iTmp=ll_insert(SlvPath,%tmt_iLast,0)
					ll_set_data SlvPath,%tmt_iTmp,2,%TMT_PUSH
					ll_set_data SlvPath,%tmt_iTmp,1,<push>
				ELSE
					tmt_iSub = tmt_iLast
				ENDIF						
				tmt_iNext = ll_insert(SlvPath,%tmt_iLast,0)
				ll_set_data SlvPath,%tmt_iNext,2,%TMT_OPRT
				ll_set_data SlvPath,%tmt_iNext,1,%tk_op
				tmt_exp = tmt_exp + 1
			ELSE
				IF tmt_prdct EQ 0 AND tmt_sum NE 0 AND tmt_exp EQ 0 ;
					tmt_iTmp=ll_insert(SlvPath,%tmt_iLast,0)
					IF tmt_pps EQ 0
						ll_set_data SlvPath,%tmt_iTmp,2,%TMT_POP
						ll_set_data SlvPath,%tmt_iTmp,1,<pop>
					ENDIF
					tmt_iLast=ll_move(SlvPath,%tmt_iLast,%tmt_iLastSum)
					IF tmt_pps
						ll_set_data SlvPath,%tmt_iTmp,2,ll_get_data(SlvPath,%ll_get_previous(SlvPath,%tmt_iLast),2)
						ll_set_data SlvPath,%tmt_iTmp,1,ll_get_data(SlvPath,%ll_get_previous(SlvPath,%tmt_iLast),1)
						foo=ll_delete(SlvPath,ll_get_previous(SlvPath,%tmt_iLast))
					ENDIF
					tmt_iLastSum = tmt_iTmp
					IF tmt_pps EQ 0
						tmt_iTmp=ll_insert(SlvPath,%tmt_iLast,0)
						ll_set_data SlvPath,%tmt_iTmp,2,%TMT_PUSH
						ll_set_data SlvPath,%tmt_iTmp,1,<push>
					ENDIF
				ENDIF
				tmt_exps = 0
				IF tmt_exp
					IF tmt_prdct OR tmt_exp GT 1
						tmt_iLast = tmt_iLastPrdct
					ENDIF
					tmt_exp = 0
					tmt_exps = 1
				ENDIF

				tmt_iNext = ll_insert(SlvPath,%tmt_iLast,0)
				ll_set_data SlvPath,%tmt_iNext,2,%TMT_OPRT
				ll_set_data SlvPath,%tmt_iNext,1,%tk_op

				IF tmt_prdct EQ 0
					tmt_iBeginPrdct = tmt_iLast
					IF tmt_exps
						tmt_iLastPrdct = tmt_iNext
					ELSE
						tmt_iLastPrdct = tmt_iLast
					ENDIF
				ELSE
					tmt_iLastPrdct = tmt_iNext
				ENDIF
				tmt_iSub = tmt_iLastPrdct
				
				tmt_prdct = 1
			ENDIF
		ENDIF

		tmt_pos  = tk_pos
		tmt_flag = TMT_OPRT
		IFIDN tk_op,<+>
			tmt_flag = TMT_OPRT OR TMT_PLUS
		ELSEIFIDN tk_op,<->
			tmt_flag = TMT_OPRT OR TMT_MINUS
		ELSEIFIDNI tk_op,<,>
			tmt_iNext = ll_insert(SlvPath,%tmt_iTop)
			ll_set_data SlvPath,%tmt_iNext,2,%TMT_PUSH
			ll_set_data SlvPath,%tmt_iNext,1,<push>
			
			;tmt_iTop = tmt_iNext
			tmt_iLast = tmt_iNext
			tmt_iLastSum = tmt_iNext
			tmt_iLastPrdct = tmt_iNext
			tmt_iBeginPrdct = ll_get_next_free(SlvPath)
			tmt_iSub = 0
			tmt_pps = 0
			tmt_exp = 0
			tmt_prdct = 0
			tmt_sum = 0
			
			tmt_flag = TMT_COMMA OR TMT_BEGIN_EXPR
		ENDIF
	ENDIF
endm

tmt_on_expr macro SlvPath,SubExpr
	IF (tk_flag AND TK_TXT) NE 0
	%	IF tmt_is_reg_expr(<%tk_txt>)
			tmt_rexpr TEXTEQU tmt_get_expr(<%tk_txt>,<%tk_exp>)
			tk_exp TEXTEQU tmt_rexpr				; 22072011
			tk_flag = tk_flag XOR TK_TXT			;
;			tmt_size SIZESTR tmt_expr				;
;			IF tk_pos LE tmt_size					;
;				tmt_expr SUBSTR tmt_expr,tk_pos		;
;				tmt_expr CATSTR tmt_rexpr,tmt_expr 	;
;			ELSE									;
;				tmt_expr TEXTEQU tmt_rexpr			;
;			ENDIF									;
;			tmt_pos = 1								;
;			tmt_flag = 0							;
;			EXITM									;		
		ENDIF
	ENDIF
	tmt_iLast=ll_insert(SlvPath,%tmt_iNext,0)
	; check for singel agument or function call with one argument
	IF tmt_check_for_single_argument(SlvPath,SubExpr)
		EXITM
	ENDIF

	; check for reoccurrence of expression (this must be done after check for  singel-agument function call)
	IF tmt_chk_expr(SubExpr,<%tk_exp>) NE 0 AND (tk_flag AND TK_TXT) EQ 0
		ll_set_data SlvPath,%tmt_iLast,2,%(TMT_REC OR (tmt_flag AND TMT_NEG))
		ll_set_data SlvPath,%tmt_iLast,1,%tce_ref
		tmt_pos = tk_pos
		tmt_flag = 0
		EXITM
	ENDIF
		
	ll_set_data SlvPath,%tmt_iLast,2,%(TMT_REC OR (tmt_flag AND TMT_NEG))
	ll_set_data SlvPath,%tmt_iLast,1,%tce_ref

	IF tmt_iSub EQ 0
		tmt_iTmp=ll_insert(SlvPath,%tmt_iBeginPrdct)
		tmt_iSub = tmt_iTmp
	ELSE
		tmt_iSub=ll_insert(SlvPath,%tmt_iSub)
	ENDIF
	
	tmt_pos = tk_pos
	; store current state to 'nesting stack'
	@CatStr(<tmt_as_>,%tmt_nesting,<_vars1>) CATSTR <tmt_iLast=>,%tmt_iLast,<,tmt_iNext=>,%tmt_iNext,<,tmt_exp=>,%tmt_exp,<,tmt_prdct=>,%tmt_prdct,<,tmt_sum=>,%tmt_sum,<,tmt_iTop=>,%tmt_iTop
	@CatStr(<tmt_as_>,%tmt_nesting,<_vars2>) CATSTR <tmt_iLastSum=>,%tmt_iLastSum,<,tmt_iLastPrdct=>,%tmt_iLastPrdct,<,tmt_iBeginPrdct=>,%tmt_iBeginPrdct,<,tmt_pos=>,%tmt_pos,<,tmt_iSub=>,%tmt_iSub
	@CatStr(<tmt_as_>,%tmt_nesting,<_expr>)  TEXTEQU tmt_expr

	tmt_iLast=ll_insert(SlvPath,%tmt_iSub,0)
	ll_set_data SlvPath,%tmt_iLast,2,%TMT_STO
	ll_set_data SlvPath,%tmt_iLast,1,%tce_ref

	IF tk_flag AND TK_TXT
		tmt_iTmp=ll_insert(SlvPath,%tmt_iLast)
		ll_set_data SlvPath,%tmt_iTmp,2,%(TMT_FNC) ; OR (tmt_flag AND TMT_NEG)
		ll_set_data SlvPath,%tmt_iTmp,1,<%tk_txt>
		tmt_iTop = tmt_iTmp
	ELSE
		tmt_iTop = tmt_iLast
	ENDIF

	tmt_iLast = tmt_iSub
	tmt_iNext = tmt_iSub
	tmt_iLastSum = tmt_iSub
	tmt_iLastPrdct = tmt_iSub
	tmt_iBeginPrdct = ll_get_next_free(SlvPath)
	tmt_iSub = 0
	tmt_pps = 0
	tmt_exp = 0
	tmt_prdct = 0
	tmt_sum = 0

	tmt_nesting = tmt_nesting + 1
	tmt_pos     = 1
	tmt_expr TEXTEQU tk_exp
	tmt_flag = TMT_BEGIN_EXPR
endm

; return 1 if reuse of expression or 0
; if expression found: return reference-value through tce_ref
; else: record expression
tmt_chk_expr macro SubExpr,txt:req
	IF lst_get_cnt(SubExpr) GT 1
		tce_iLine = lst_tm_search(SubExpr,1,<&txt>)
		IF tce_iLine
			fooABC=lst_inc_value(SubExpr,%tce_iLine,2)
			tce_ref = tce_iLine-2
			EXITM %tce_iLine
		ENDIF
	ENDIF
	tce_iLine = lst_append(SubExpr)
	lst_set_data SubExpr,%tce_iLine,1,<&txt>
	lst_set_data SubExpr,%tce_iLine,2,1
	tce_ref = tce_iLine-2
	EXITM <0>
endm

tmt_check_for_single_argument macro SlvPath,SubExpr
	IF @ScanForFlt(1,<%tk_exp>,blank) NE 0
		GOTO Z
	ELSE
		IF @InStr(1,<%tk_exp>,<,>) NE 0 OR @InStr(1,<%tk_exp>,<^>) NE 0
			EXITM <0>
		ELSE
			tmt_pos1 INSTR 1,tk_exp,<[>
			tmt_pos2 INSTR 1,tk_exp,<]>
			tmt_res TEXTEQU <>
			IF tmt_pos1
				tmt_res CATSTR @SubStr(<%tk_exp>,1,%tmt_pos1-1)
				IF tmt_pos2 LT @SizeStr(<%tk_exp>)
					tmt_res CATSTR @SubStr(<%tk_exp>,%tmt_pos2+1,)
				ENDIF
			ELSE
				tmt_res CATSTR tk_exp,< >
			ENDIF
			tmt_res CATSTR tmt_res,< >
			IF @InStr(1,<%tmt_res>,<+>) EQ 0 AND @InStr(1,<%tmt_res>,<->) EQ 0 AND @InStr(1,<%tmt_res>,<*>) EQ 0 AND @InStr(1,<%tmt_res>,</>) EQ 0
			%	IF ((OPATTR [tk_exp]) AND 1011y) NE 0
					tk_flag = (tk_flag AND TK_TXT)
				%	IF fslv_is_reg_const(<%tk_exp>)
						IF fsir_flg EQ 3
							tk_exp TEXTEQU fsir_txt
						ELSE
							IF tk_flag AND TK_TXT ;/* function call ? */
								IF fsir_flg EQ 2
									ll_set_data SlvPath,%tmt_iLast,2,%(TMT_FNC OR TMT_SINGLE_INT OR (tmt_flag AND TMT_NEG))
								ELSE
									ll_set_data SlvPath,%tmt_iLast,2,%(TMT_FNC OR TMT_SINGLE_FLT OR (tmt_flag AND TMT_NEG))
								ENDIF
								tk_txt TEXTEQU fsir_txt,<,>,tk_txt
								ll_set_data SlvPath,%tmt_iLast,1,<%tk_txt>
							ELSE
								ll_set_data SlvPath,%tmt_iLast,1,%fsir_txt
								IF fsir_flg EQ 2
									ll_set_data SlvPath,%tmt_iLast,2,%(TMT_INT OR (tmt_flag AND TMT_NEG))
								ELSE
									ll_set_data SlvPath,%tmt_iLast,2,%(TMT_FLT OR (tmt_flag AND TMT_NEG))
								ENDIF
							ENDIF
							tmt_pos = tk_pos
							tmt_flag = 0
							EXITM <1>
						ENDIF
					ENDIF					
					IF tk_flag AND TK_TXT
						ll_set_data SlvPath,%tmt_iLast,2,%(TMT_FNC OR TMT_SINGLE_ARG OR (tmt_flag AND TMT_NEG))
						tk_txt TEXTEQU tk_exp,<,>,tk_txt
						ll_set_data SlvPath,%tmt_iLast,1,<%tk_txt>
					ELSE
						ll_set_data SlvPath,%tmt_iLast,1,%tk_exp
						ll_set_data SlvPath,%tmt_iLast,2,%(TMT_TXT OR (tmt_flag AND TMT_NEG))
					ENDIF
					tmt_pos = tk_pos
					tmt_flag = 0
					EXITM <1>
				ENDIF
			ENDIF
		ENDIF
	ENDIF
	EXITM <0>
:Z		
	tk_flag = (tk_flag AND TK_TXT)
	IF tk_flag AND TK_TXT ;/* function call ? */
		IF sff_type EQ 2
			ll_set_data SlvPath,%tmt_iLast,2,%(TMT_FNC OR TMT_SINGLE_INT OR (tmt_flag AND TMT_NEG))
		ELSE
			ll_set_data SlvPath,%tmt_iLast,2,%(TMT_FNC OR TMT_SINGLE_FLT OR (tmt_flag AND TMT_NEG))
		ENDIF
		tk_txt TEXTEQU sff_numstr,<,>,tk_txt
		ll_set_data SlvPath,%tmt_iLast,1,<%tk_txt>
	ELSE
		ll_set_data SlvPath,%tmt_iLast,1,%sff_numstr
		IF sff_type EQ 2
			ll_set_data SlvPath,%tmt_iLast,2,%(TMT_INT OR (tmt_flag AND TMT_NEG))
		ELSE
			ll_set_data SlvPath,%tmt_iLast,2,%(TMT_FLT OR (tmt_flag AND TMT_NEG))
		ENDIF
	ENDIF
	
	tmt_pos = tk_pos
	tmt_flag = 0
	EXITM <1>
endm

tmt_is_expr_def macro ExprName
	IFDEF fSlv_Expr_&ExprName&
		EXITM <1>
	ENDIF
	EXITM <0>
endm

tmt_is_reg_expr macro ExprName:req
	FOR _arg,<&ExprName>
		tire_name TEXTEQU <&_arg>
	ENDM
	 ;% IFDEF fSlv_Expr_&tire_name&
	EXITM tmt_is_expr_def(%tire_name)
endm

tmt_get_expr macro ExprName,List:=<>
	FOR _arg,<&ExprName>
		tge_name TEXTEQU <&_arg>
	ENDM

	tge_cnt = @CatStr(<fSlv_Expr_args_>,%tge_name) 
	tge_expr TEXTEQU @CatStr(<fSlv_Expr_>,<%tge_name>)
	
	tge_cntr = 0
	FOR _arg,<&List>
		tge_cntr = tge_cntr + 1
	ENDM

	IF tge_cntr NE tge_cnt
		echo math parser: invalid number of arguments for reg. expression
		EXITM <0.0>
	ENDIF

	tge_cntr = 1
	FOR param,<&List>
		tge_arg TEXTEQU <arg>,%tge_cntr
		tge_expr TEXTEQU @RepAllStr(1,<%tge_expr>,<%tge_arg>,<&param>)
		tge_cntr = tge_cntr + 1
	ENDM	
	EXITM tge_expr
endm

; fsir_flg: 1 = flt , 2 = int , 3 = variable
fslv_is_reg_const macro name:=<>
	fsir_flg = 0
	fsir_txt TEXTEQU <>	
	IFNB <&name>
		FOR _ctxt,<&name>
			fsir_txt TEXTEQU <&_ctxt>
		ENDM
		IF @InStr(1,<%fsir_txt>,< >) EQ 0 AND @InStr(1,<%fsir_txt>,<	>) EQ 0 AND @InStr(1,<%fsir_txt>,<.>) EQ 0
		%	IFNDEF fpu_regc_&fsir_txt&_type
				EXITM <0>
			ELSE
			%	IF fpu_regc_&fsir_txt&_type EQ 0
					fsir_txt TEXTEQU <fpu_regc_>,fsir_txt
					fsir_flg = 3
					EXITM <1>
			%	ELSEIF fpu_regc_&fsir_txt&_type EQ 1
					fsir_txt TEXTEQU <fpu_regc_equ_>,fsir_txt
					fsir_flg = 1
					EXITM <1>
			%	ELSEIF fpu_regc_&fsir_txt&_type EQ 2
					fsir_txt TEXTEQU <fpu_regc_equ_>,fsir_txt
					fsir_flg = 2
					EXITM <1>
				ENDIF
			ENDIF			
		ELSE
			EXITM <0>
		ENDIF
	ENDIF
	EXITM <0>	
endm








































