; by qWord @ www.masmforum.com

;-------------------------------------------------------------
; creates an unicode string in current segment
; 	lbl:	 [optional] name of string 
; 	args:	 one or more string literals enclosed
;			 either by double or by single quotes.
;			 Numeric values are also accepted, blank 
;			 arguments forced an error. The maximal number
;			 of characters is something about 200.
;
;	remarks: an termination zero must add by user!
;			 Named strings can be used with the SIZEOF operator.
;
;			 This macro won't work with angle brackets and
;			 exclamation marks. If these char. are needed use the
;			 CWSZ-macro. 
;   example:
;	   WSTR wszXYZ,'"double quote"',10,13,"'single quote'",0
;-------------------------------------------------------------
WSTR macro lbl,args:VARARG
	IFNDEF wsz_lbl_cntr
		wsz_lbl_cntr = 0
	ENDIF
	IFB <&lbl>
		wsz_lbl TEXTEQU <anonym_WSTR_>,%wsz_lbl_cntr
		wsz_lbl_cntr = wsz_lbl_cntr + 1
	ELSE
		wsz_lbl TEXTEQU <&lbl>
	ENDIF
	wsz_line = 1
	wsz_line_1 TEXTEQU <>
	wsz_line_size = 0
	wsz_size = 0
	wsz_flg = 0
	wsz_iarg = 1
	
%	FOR _arg,<&args>
		wsz_flg = 0
		FORC char,<&_arg>
			IF wsz_flg NE 0
				wsz_pos INSTR 1,<"'>,<&char>
				IF wsz_pos EQ wsz_flg
					EXITM
				ELSE
					@CatStr(<wsz_line_>,%wsz_line) TEXTEQU @CatStr(%@CatStr(<wsz_line_>,%wsz_line)),<,>,wsz_quote,<&char>,wsz_quote
					IF wsz_line_size GE 160
						wsz_line = wsz_line + 1
						wsz_line_size = 0
						@CatStr(<wsz_line_>,%wsz_line) TEXTEQU <>
					ELSE
						wsz_line_size = wsz_line_size + 4
					ENDIF
					wsz_size = wsz_size + 1
				ENDIF
			ELSE
				wsz_flg INSTR 1,<"'0123456789>,<&char>
				IF wsz_flg NE 1 AND wsz_flg NE 2
					EXITM
				ENDIF
				wsz_quote TEXTEQU <&char>
				wsz_arg_size SIZESTR <&_arg>
				wsz_char SUBSTR <&_arg>,wsz_arg_size,1
				IFDIF wsz_char,wsz_quote
					wsz_flg=0
					EXITM
				ENDIF
			ENDIF
		ENDM
		IFE wsz_flg
			EXITM
		ELSEIF wsz_flg GT 2
			@CatStr(<wsz_line_>,%wsz_line) TEXTEQU @CatStr(%@CatStr(<wsz_line_>,%wsz_line)),<,>,<&_arg>
			IF wsz_line_size GE 160
				wsz_line = wsz_line + 1
				wsz_line_size = 0
				@CatStr(<wsz_line_>,%wsz_line) TEXTEQU <>
			ELSE
				wsz_line_size = wsz_line_size + @SizeStr(<&_arg>)
			ENDIF
			wsz_size = wsz_size + 1
		ENDIF		
		wsz_iarg = wsz_iarg + 1
	ENDM
	IFE wsz_flg
	%	.err <invalid string specifier : argument : @CatStr(%wsz_iarg)>
		EXITM
	ELSEIFE wsz_line_size
		wsz_line = wsz_line - 1
	ENDIF
	
	align 2
	IF wsz_line EQ 1
		wsz_lbl WORD @SubStr(%wsz_line_1,2);,0
	ELSE
		IFNB <&lbl>
		%	wsz_tmpS_&wsz_lbl struct
				wsz_lbl WORD wsz_size dup (?)
		%	wsz_tmpS_&wsz_lbl ends
		%	&wsz_lbl LABEL wsz_tmpS_&wsz_lbl
		ELSE
			wsz_lbl LABEL WORD
		ENDIF
		wsz_cntr = 1
		REPEAT wsz_line
			WORD @SubStr(%@CatStr(<wsz_line_>,%wsz_cntr),2)
			wsz_cntr = wsz_cntr + 1
		ENDM
		;WORD 0
	ENDIF
endm



;--------------------------------------
;
; function version of WSZ. 
; returns an OFFSET to anonym string.
;
;--------------------------------------
wsz$ macro args:VARARG
	IFNDEF wsz_lbl_cntr
		wsz_lbl_cntr = 0
	ENDIF
	wsz_lbl TEXTEQU <anonym_WSTR_>,%wsz_lbl_cntr
	wsz_lbl_cntr = wsz_lbl_cntr + 1

	wsz_line = 1
	wsz_line_1 TEXTEQU <>
	wsz_line_size = 0
	wsz_flg = 0
	wsz_iarg = 1
	
%	FOR _arg,<&args>
		wsz_flg = 0
		FORC char,<&_arg>
			IF wsz_flg NE 0
				wsz_pos INSTR 1,<"'>,<&char>
				IF wsz_pos EQ wsz_flg
					EXITM
				ELSE
					@CatStr(<wsz_line_>,%wsz_line) TEXTEQU @CatStr(%@CatStr(<wsz_line_>,%wsz_line)),<,>,wsz_quote,<&char>,wsz_quote
					IF wsz_line_size GE 160
						wsz_line = wsz_line + 1
						wsz_line_size = 0
						@CatStr(<wsz_line_>,%wsz_line) TEXTEQU <>
					ELSE
						wsz_line_size = wsz_line_size + 4
					ENDIF
				ENDIF
			ELSE
				wsz_flg INSTR 1,<"'0123456789>,<&char>
				IF wsz_flg NE 1 AND wsz_flg NE 2
					EXITM
				ENDIF
				wsz_quote TEXTEQU <&char>
				wsz_arg_size SIZESTR <&_arg>
				wsz_char SUBSTR <&_arg>,wsz_arg_size,1
				IFDIF wsz_char,wsz_quote
					wsz_flg=0
					EXITM
				ENDIF
			ENDIF
		ENDM
		IFE wsz_flg
			EXITM
		ELSEIF wsz_flg GT 2
			@CatStr(<wsz_line_>,%wsz_line) TEXTEQU @CatStr(%@CatStr(<wsz_line_>,%wsz_line)),<,>,<&_arg>
			IF wsz_line_size GE 160
				wsz_line = wsz_line + 1
				wsz_line_size = 0
				@CatStr(<wsz_line_>,%wsz_line) TEXTEQU <>
			ELSE
				wsz_line_size = wsz_line_size + @SizeStr(<&_arg>)
			ENDIF
		ENDIF		
		wsz_iarg = wsz_iarg + 1
	ENDM
	IFE wsz_flg
	%	.err <invalid string specifier : argument : @CatStr(%wsz_iarg)>
		EXITM <0>
	ELSEIFE wsz_line_size
		wsz_line = wsz_line - 1
	ENDIF
	.data
		align 2
	%	&wsz_lbl LABEL WORD
		wsz_cntr = 1
		REPEAT wsz_line
			WORD @SubStr(%@CatStr(<wsz_line_>,%wsz_cntr),2)
			wsz_cntr = wsz_cntr + 1
		ENDM
		WORD 0
	.code
%	EXITM <OFFSET &wsz_lbl> 
endm

;-------------------------------------------------
; creates an unicode string. This macro works 
; the same way like WSTR, but adds support for
; escape sequences:
; \\ :	"\"
; \t :	tab	
; \n :	new line
; \x :	"!"
; \a :	"("
; \b :	")"
; \l :	"<"
; \r :	">"
; \q :	double quote '"'
;-------------------------------------------------
CWSTR macro lbl,args:VARARG
	IFNDEF wsz_lbl_cntr
		wsz_lbl_cntr = 0
	ENDIF
	IFB <&lbl>
		wsz_lbl TEXTEQU <anonym_WSTR_>,%wsz_lbl_cntr
		wsz_lbl_cntr = wsz_lbl_cntr + 1
	ELSE
		wsz_lbl TEXTEQU <&lbl>
	ENDIF
	wsz_line = 1
	wsz_line_1 TEXTEQU <>
	wsz_line_size = 0
	wsz_size = 0
	wsz_flg = 0
	wsz_iarg = 1
	wsz_esc = 0
	
	FOR _arg,<&args>
		wsz_flg = 0
		FORC char,<&_arg>
			IF wsz_flg NE 0
				wsz_pos INSTR 1,<"'\>,<&char>
				IF wsz_pos EQ wsz_flg
					EXITM
				ELSEIF wsz_pos EQ 3 AND wsz_esc EQ 0
					wsz_esc = 1
				ELSE
					wsz_char TEXTEQU wsz_quote,<&char>,wsz_quote
					IF wsz_esc
						wsz_pos INSTR 1,<\ablrxqtn>,<&char>
						IFE wsz_pos
							wsz_flg=0
							EXITM
						ENDIF
						wsz_char SUBSTR <  5ch28h29h3ch3eh21h22h09h0ah,0dh>,wsz_pos*3,3+4*(wsz_pos/9)
						wsz_size = wsz_size + wsz_pos/9
						wsz_esc = 0
					ENDIF
					@CatStr(<wsz_line_>,%wsz_line) TEXTEQU @CatStr(%@CatStr(<wsz_line_>,%wsz_line)),<,>,wsz_char
					IF wsz_line_size GE 160
						wsz_line = wsz_line + 1
						wsz_line_size = 0
						@CatStr(<wsz_line_>,%wsz_line) TEXTEQU <>
					ELSE
						wsz_pos SIZESTR wsz_char
						wsz_line_size = wsz_line_size + wsz_pos + 1
					ENDIF
					wsz_size = wsz_size + 1
				ENDIF
			ELSE
				wsz_flg INSTR 1,<"'0123456789>,<&char>
				IF wsz_flg NE 1 AND wsz_flg NE 2
					EXITM
				ENDIF
				wsz_quote TEXTEQU <&char>
				wsz_arg_size SIZESTR <&_arg>
				wsz_char SUBSTR <&_arg>,wsz_arg_size,1
				IFDIF wsz_char,wsz_quote
					wsz_flg=0
					EXITM
				ENDIF
			ENDIF
		ENDM
		IF wsz_flg EQ 0 OR wsz_esc
			EXITM
		ELSEIF wsz_flg GT 2
			@CatStr(<wsz_line_>,%wsz_line) TEXTEQU @CatStr(%@CatStr(<wsz_line_>,%wsz_line)),<,>,<&_arg>
			IF wsz_line_size GE 160
				wsz_line = wsz_line + 1
				wsz_line_size = 0
				@CatStr(<wsz_line_>,%wsz_line) TEXTEQU <>
			ELSE
				wsz_line_size = wsz_line_size + @SizeStr(<&_arg>)
			ENDIF
			wsz_size = wsz_size + 1
		ENDIF		
		wsz_iarg = wsz_iarg + 1
	ENDM
	IF wsz_flg EQ 0 OR wsz_esc NE 0
		IF wsz_esc
		%	.err <invalid escape sequence : argument : @CatStr(%wsz_iarg)>
		ELSE
		%	.err <invalid string specifier : argument : @CatStr(%wsz_iarg)>
		ENDIF
		EXITM
	ELSEIFE wsz_line_size
		wsz_line = wsz_line - 1
	ENDIF
	
	align 2
	IF wsz_line EQ 1
		wsz_lbl WORD @SubStr(%wsz_line_1,2);,0
	ELSE
		IFNB <&lbl>
		%	wsz_tmpS_&wsz_lbl struct
				wsz_lbl WORD wsz_size dup (?)
		%	wsz_tmpS_&wsz_lbl ends
		%	&wsz_lbl LABEL wsz_tmpS_&wsz_lbl
		ELSE
			wsz_lbl LABEL WORD
		ENDIF
		wsz_cntr = 1
		REPEAT wsz_line
			WORD @SubStr(%@CatStr(<wsz_line_>,%wsz_cntr),2)
			wsz_cntr = wsz_cntr + 1
		ENDM
		;WORD 0
	ENDIF
endm

cwsz$ macro args:VARARG
	IFNDEF wsz_lbl_cntr
		wsz_lbl_cntr = 0
	ENDIF
	wsz_lbl TEXTEQU <anonym_WSTR_>,%wsz_lbl_cntr
	wsz_lbl_cntr = wsz_lbl_cntr + 1
	
	wsz_line = 1
	wsz_line_1 TEXTEQU <>
	wsz_line_size = 0
	wsz_flg = 0
	wsz_iarg = 1
	wsz_esc = 0
	
	FOR _arg,<&args>
		wsz_flg = 0
		FORC char,<&_arg>
			IF wsz_flg NE 0
				wsz_pos INSTR 1,<"'\>,<&char>
				IF wsz_pos EQ wsz_flg
					EXITM
				ELSEIF wsz_pos EQ 3 AND wsz_esc EQ 0
					wsz_esc = 1
				ELSE
					wsz_char TEXTEQU wsz_quote,<&char>,wsz_quote
					IF wsz_esc
						wsz_pos INSTR 1,<\ablrxqtn>,<&char>
						IFE wsz_pos
							wsz_flg=0
							EXITM
						ENDIF
						wsz_char SUBSTR <  5ch28h29h3ch3eh21h22h09h0ah,0dh>,wsz_pos*3,3+4*(wsz_pos/9)
						wsz_esc = 0
					ENDIF
					@CatStr(<wsz_line_>,%wsz_line) TEXTEQU @CatStr(%@CatStr(<wsz_line_>,%wsz_line)),<,>,wsz_char
					IF wsz_line_size GE 160
						wsz_line = wsz_line + 1
						wsz_line_size = 0
						@CatStr(<wsz_line_>,%wsz_line) TEXTEQU <>
					ELSE
						wsz_pos SIZESTR wsz_char
						wsz_line_size = wsz_line_size + wsz_pos + 1
					ENDIF
				ENDIF
			ELSE
				wsz_flg INSTR 1,<"'0123456789>,<&char>
				IF wsz_flg NE 1 AND wsz_flg NE 2
					EXITM
				ENDIF
				wsz_quote TEXTEQU <&char>
				wsz_arg_size SIZESTR <&_arg>
				wsz_char SUBSTR <&_arg>,wsz_arg_size,1
				IFDIF wsz_char,wsz_quote
					wsz_flg=0
					EXITM
				ENDIF
			ENDIF
		ENDM
		IF wsz_flg EQ 0 OR wsz_esc
			EXITM
		ELSEIF wsz_flg GT 2
			@CatStr(<wsz_line_>,%wsz_line) TEXTEQU @CatStr(%@CatStr(<wsz_line_>,%wsz_line)),<,>,<&_arg>
			IF wsz_line_size GE 160
				wsz_line = wsz_line + 1
				wsz_line_size = 0
				@CatStr(<wsz_line_>,%wsz_line) TEXTEQU <>
			ELSE
				wsz_line_size = wsz_line_size + @SizeStr(<&_arg>)
			ENDIF
		ENDIF		
		wsz_iarg = wsz_iarg + 1
	ENDM
	IF wsz_flg EQ 0 OR wsz_esc NE 0
		IF wsz_esc
		%	.err <invalid escape sequence : argument : @CatStr(%wsz_iarg)>
		ELSE
		%	.err <invalid string specifier : argument : @CatStr(%wsz_iarg)>
		ENDIF
		EXITM <0>
	ELSEIFE wsz_line_size
		wsz_line = wsz_line - 1
	ENDIF
	.data
		align 2
	%	&wsz_lbl LABEL WORD
		wsz_cntr = 1
		REPEAT wsz_line
			WORD @SubStr(%@CatStr(<wsz_line_>,%wsz_cntr),2)
			wsz_cntr = wsz_cntr + 1
		ENDM
		WORD 0
	.code
%	EXITM <OFFSET &wsz_lbl> 
endm

; greek capital letters
CAP_ALPHA   EQU <0391h>
CAP_BETA    EQU <0392h>
CAP_GAMMA   EQU <0393h>
CAP_DELTA   EQU <0394h>
CAP_EPSILON EQU <0395h>
CAP_ZETA    EQU <0396h>
CAP_ETA     EQU <0397h>
CAP_THETA   EQU <0398h>
CAP_IOTA    EQU <0399h>
CAP_KAPPA   EQU <039Ah>
CAP_LAMDA   EQU <039Bh>
CAP_MU      EQU <039Ch>
CAP_NU      EQU <039Dh>
CAP_XI      EQU <039Eh>
CAP_OMICRON EQU <039Fh>
CAP_PI      EQU <03A0h>
CAP_RHO     EQU <03A1h>
CAP_SIGMA   EQU <03A3h>
CAP_TAU     EQU <03A4h>
CAP_UPSILON EQU <03A5h>
CAP_PHI     EQU <03A6h>
CAP_CHI     EQU <03A7h>
CAP_PSI     EQU <03A8h>
CAP_OMEGA   EQU <03A9h>

; greek small letters
SML_ALPHA   EQU <03B1h>
SML_BETA    EQU <03B2h>
SML_GAMMA   EQU <03B3h>
SML_DELTA   EQU <03B4h>
SML_EPSILON EQU <03B5h>
SML_ZETA    EQU <03B6h>
SML_ETA     EQU <03B7h>
SML_THETA   EQU <03B8h>
SML_IOTA    EQU <03B9h>
SML_KAPPA   EQU <03BAh>
SML_LAMDA   EQU <03BBh>
SML_MU      EQU <03BCh>
SML_NU      EQU <03BDh>
SML_XI      EQU <03BEh>
SML_OMICRON EQU <03BFh>
SML_PI      EQU <03C0h>
SML_RHO     EQU <03C1h>
SML_SIGMA   EQU <03C3h>
SML_TAU     EQU <03C4h>
SML_UPSILON EQU <03C5h>
SML_PHI     EQU <03C6h>
SML_CHI     EQU <03C7h>
SML_PSI     EQU <03C8h>
SML_OMEGA   EQU <03C9h>




