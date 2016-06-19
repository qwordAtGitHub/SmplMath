; by qWord, 2011

IFNDEF __SMPLMATH__UNICODE__

;/*****************************************************************/
;/*             WSTR - macro procedure                            */
;/* This macro creates a Unicode string in the current segment    */
;/* Parameters:                                                   */
;/*    lbl:    [optional] name of string                          */
;/*    args:   one ore more string literals, either enclosed by   */
;/*            single quotation marks or by double quotes.        */
;/*            Constants can also be used, blank arguments        */
;/*            force an error. The maximal number of characters   */
;/*            is something about 240.                            */
;/*                                                               */
;/* Remarks:   A termination zero must add by user!               */
;/*            Named strings can be used with the SIZEOF and      */
;/*            LENGTHOF operator. This macro wont work with       */
;/*            angle brackets and exclemation marks.              */
;/* Example:                                                      */
;/*          WSTR myStr, "my string",13,10,'"quoted"',0           */
;/*                                                  qWord, 2011  */
;/*****************************************************************/
WSTR MACRO lbl,args:VARARG
    
    IFNDEF ucstr_lbl_cntr
        ucstr_lbl_cntr = 0
    ENDIF
    
    IFB <&lbl>
        ucstr_lbl TEXTEQU <$$_WSTR_>,%ucstr_lbl_cntr
        ucstr_lbl_cntr = ucstr_lbl_cntr + 1
    ELSE
        ucstr_lbl TEXTEQU <&lbl>
    ENDIF

    ucstr_size = 0
    ucstr_flg = 0  ; 0 => invalid argument
                   ; 1 => double quotes are used
                   ; 2 => single quotation marks are used
                   ; 3 => constant 
    ucstr_iarg = 1

    FOR _arg,<&args>
        ucstr_flg = 0
        FORC char,<&_arg>
            IF ucstr_flg NE 0
                ucstr_pos INSTR 1,<"'>,<&char>
                IF ucstr_pos EQ ucstr_flg
                    EXITM
                ENDIF
                ucstr_size = ucstr_size + 1
            ELSE
                ucstr_flg INSTR 1,<"'>,<&char>
                IFE ucstr_flg
                    IF (OPATTR _arg) AND 100y
                        ucstr_flg = 3
                    ENDIF
                    EXITM
                ENDIF
            ENDIF
        ENDM
        IF ucstr_flg EQ 3
            ucstr_size = ucstr_size + 1
        ELSEIFE ucstr_flg
            EXITM
        ENDIF
        ucstr_iarg = ucstr_iarg + 1     
    ENDM
    IFE ucstr_flg
    %   .err <invalid string specifier : argument : @CatStr(%ucstr_iarg)>
        EXITM
    ENDIF

    align 2
    ucstr_lbl WORD ucstr_size dup (?)
    org $-ucstr_size*2
    
    FOR _arg,<&args>
        ucstr_flg = 0
        FORC char,<&_arg>
            IF ucstr_flg NE 0
                ucstr_pos INSTR 1,<"'>,<&char>
                IF ucstr_pos EQ ucstr_flg
                    EXITM
                ELSE
                    ucstr_char CATSTR <dw >,ucstr_quote,<&char>,ucstr_quote
                    ucstr_char
                ENDIF
            ELSE
                ucstr_flg INSTR 1,<"'>,<&char>
                IFE ucstr_flg
                    IF (OPATTR _arg) AND 100y
                        ucstr_flg = 3
                    ENDIF
                    EXITM
                ENDIF
                ucstr_quote TEXTEQU <&char>
            ENDIF
        ENDM
        IF ucstr_flg EQ 3
            dw _arg
        ELSEIFE ucstr_flg
            EXITM
        ENDIF
        ucstr_iarg = ucstr_iarg + 1     
    ENDM

endm

;/***************************************************/
;/*               w$ , macro function               */
;/* This macro is the function-version of WSTR.     */
;/* In contrast to WSTR, the created string is      */
;/* zero terminated. The macro assumes to be called */
;/* from the .code-section and places the string    */
;/* in the .data-section.                           */
;/* Example:                                        */
;/*      mov esi,w$("my string",13,10,'xxyz')       */
;/*                                    qWord, 2011  */
;/***************************************************/
w$ MACRO args:VARARG
    .data
        WSTR ,args,0
    .code
    ucsz_retval TEXTEQU <OFFSET >,ucstr_lbl
    EXITM ucsz_retval
endm

;/*****************************************************************/
;/*             WCSTR - macro procedure                           */
;/* This macro creates a Unicode string in the current segment    */
;/* This macro is identically to the WSTR-macro, except, that it  */
;/* adds support for some escape sequences:                       */
;/*     \\  ->  "\"                                               */
;/*     \t  ->  tab                                               */
;/*     \n  ->  new line (13,10)                                  */
;/*     \x  ->  "!"                                               */
;/*     \a  ->  "("                                               */
;/*     \b  ->  ")"                                               */
;/*     \l  ->  "<"                                               */
;/*     \r  ->  ">"                                               */
;/*     \p  ->  "%"                                               */
;/*     \A  ->  "&"                                               */
;/*     \q  ->  double quote '"'                                  */
;/*     \0  ->  zero-word                                         */
;/*                                                               */
;/* Example:                                                      */
;/*       WCSTR myStr, "line 1\nline2\nI'm here\x",0              */
;/*                                                  qWord, 2011  */
;/*****************************************************************/
WCSTR MACRO lbl,args:VARARG

    IFNDEF uccstr_lbl_cntr
        uccstr_lbl_cntr = 0
    ENDIF
    IFB <&lbl>
        uccstr_lbl TEXTEQU <anonym_WSTR_>,%uccstr_lbl_cntr
        uccstr_lbl_cntr = uccstr_lbl_cntr + 1
    ELSE
        uccstr_lbl TEXTEQU <&lbl>
    ENDIF

    uccstr_size = 0
    uccstr_flg = 0
    uccstr_iarg = 1
    uccstr_esc = 0
    FOR _arg,<args>
        uccstr_flg = 0
        FORC char,<&_arg>
            IF uccstr_flg NE 0
                uccstr_pos INSTR 1,<"'\>,<&char>
                IF uccstr_pos EQ uccstr_flg
                    EXITM
                ELSEIF uccstr_pos EQ 3 AND uccstr_esc EQ 0
                uccstr_esc = 1
                ELSE
                    IF uccstr_esc
                        uccstr_pos INSTR 1,<\0ablrxqtpAn>,<&char>
                        
                        IFE uccstr_pos
                            uccstr_flg=0
                            EXITM
                        ENDIF
                        uccstr_size = uccstr_size + uccstr_pos/12
                        uccstr_esc = 0
                    ENDIF
                    uccstr_size = uccstr_size + 1
                ENDIF
            ELSE
                uccstr_flg INSTR 1,<"'>,<&char>
                IFE uccstr_flg
                    IF (OPATTR _arg) AND 100y
                        uccstr_flg = 3
                    ENDIF
                    EXITM
                ENDIF
            ENDIF
        ENDM
        IF uccstr_flg EQ 0 OR uccstr_esc NE 0
            EXITM
        ELSEIF uccstr_flg EQ 3
            uccstr_size = uccstr_size + 1
        ENDIF       
        uccstr_iarg = uccstr_iarg + 1
    ENDM
    IF uccstr_flg EQ 0 OR uccstr_esc NE 0
        IF uccstr_esc
        %   .err <invalid escape sequence : argument : @CatStr(%uccstr_iarg)>
        ELSE
        %   .err <invalid string specifier : argument : @CatStr(%uccstr_iarg)>
        ENDIF
        EXITM
    ENDIF
    
    align 2
    uccstr_lbl WORD uccstr_size dup (?)
    org $-uccstr_size*2
    
    uccstr_esc = 0
    FOR _arg,<&args>
        uccstr_flg = 0
        FORC char,<&_arg>
            IF uccstr_flg NE 0
                uccstr_pos INSTR 1,<"'\>,<&char>
                IF uccstr_pos EQ uccstr_flg
                    EXITM
                ELSEIF uccstr_pos EQ 3 AND uccstr_esc EQ 0
                    uccstr_esc = 1
                ELSE
                    IFE uccstr_esc
                        uccstr_char CATSTR <dw >,uccstr_quote,<&char>,uccstr_quote
                        uccstr_char
                    ELSE
                        uccstr_pos INSTR 1,<\0ablrxqtpAn>,<&char>
                        IFE uccstr_pos
                            uccstr_flg=0
                            EXITM
                        ENDIF
                        uccstr_char SUBSTR <  5ch00h28h29h3ch3eh21h22h09h25h26h0dh,0ah>,uccstr_pos*3,3+4*(uccstr_pos/12)
                        uccstr_esc = 0
                        dw uccstr_char
                    ENDIF
                ENDIF
            ELSE
                uccstr_flg INSTR 1,<"'>,<&char>
                IFE uccstr_flg
                    IF (OPATTR _arg) AND 100y
                        uccstr_flg = 3
                    ENDIF
                    EXITM
                ENDIF
                uccstr_quote TEXTEQU <&char>
            ENDIF
        ENDM
        IF uccstr_flg EQ 3
            dw _arg
        ENDIF       
    ENDM

endm

;/***************************************************/
;/*               wc$ , macro function              */
;/* This macro is the function-version of WCSTR.    */
;/* In contrast to WCSTR, the created string is     */
;/* zero terminated. The macro assumes to be called */
;/* from the .code-section and places the string    */
;/* in the .data-section.                           */
;/*     mov esi,wc$("\lHello World\r\n:-\b")        */
;/*                                    qWord, 2011  */
;/***************************************************/
wc$ MACRO args:VARARG
    .data
        WCSTR ,args,0
    .code
    uccsz_retval TEXTEQU <OFFSET >,uccstr_lbl
    EXITM uccsz_retval
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

ELSE
	__SMPLMATH__UNICODE__ TEXTEQU <>
ENDIF


