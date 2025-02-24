https://software-portability-and-optimization.hashnode.dev/calculator-in-6502
; =================== ROM Routine Definitions ===================
; Original: define      SCINIT      $ff81 ; initialize/clear screen
;define      SCINIT      $ff81 ; initialize/clear screen
define      CLEAR_SCREEN $ff81   ; initialize/clear screen (changed name)

; Original: define      CHRIN       $ffcf ; input character from keyboard
;define      CHRIN       $ffcf ; input character from keyboard
define      GET_CHAR    $ffcf   ; input character (changed name)

; Original: define      CHROUT      $ffd2 ; output character to screen
;define      CHROUT      $ffd2 ; output character to screen
define      PUT_CHAR    $ffd2   ; output character (changed name)

; Original: define      SCREEN      $ffed ; get screen size
;define      SCREEN      $ffed ; get screen size
define      SCR_SIZE    $ffed   ; screen size routine (changed name)

; Original: define      PLOT        $fff0 ; get/set cursor coordinates
;define      PLOT        $fff0 ; get/set cursor coordinates
define      SET_CURSOR  $fff0   ; set/get cursor coordinates (changed name)

; =================== Zero‑Page Variable Definitions ===================
; Original: define      PRINT_PTR   $10
;define      PRINT_PTR   $10
define      PRNT_PTR    $10     ; pointer (low) for printing (renamed)

; Original: define      PRINT_PTR_H $11
;define      PRINT_PTR_H $11
define      PRNT_PTR_H  $11     ; pointer (high) for printing (renamed)

; Original: define      value       $14
;define      value       $14
define      SUM_BCD     $14     ; holds BCD result (renamed)

; Original: define      value_h     $15
;define      value_h     $15
define      SUM_H       $15     ; holds hundreds digit (renamed)

; Original: define    operation $1
;define    operation $1
define      OP_CODE     $01     ; operation code (reserved; renamed)

; Original: define    XPOS $20
;define    XPOS $20
define      CUR_X       $20     ; current X coordinate (renamed)

; Original: define    YPOS $21
;define    YPOS $21
define      CUR_Y       $21     ; current Y coordinate (renamed)

; =================== Absolute Variable Definitions ===================
; Original: define      GETNUM_1    $0080
;define      GETNUM_1    $0080
define      NUM1_BUF    $0080   ; buffer for first digit (renamed)

; Original: define      GETNUM_2    $0081
;define      GETNUM_2    $0081
define      NUM2_BUF    $0081   ; buffer for second digit (renamed)

; =================== Initialization and Setup ===================
; Set image dimensions (width and height)
LDA #$05                    
STA $12         ; set image width to 5 pixels (unchanged)
STA $13         ; set image height to 5 pixels (unchanged)

; Set starting cursor position
LDA #$01                    
; Original: STA XPOS
;STA XPOS
STA CUR_X       ; use CUR_X instead of XPOS (changed)
; Original: STA YPOS
;STA YPOS
STA CUR_Y       ; use CUR_Y instead of YPOS (changed)

; Clear screen and print banner
JSR CLEAR_SCREEN   ; changed from SCINIT
JSR GET_CHAR       ; changed from CHRIN (pause/wait for key)
JSR PRINT_STR      ; will call our modified PRINT subroutine
; Original: dcb "C","a","l","c","u","l","a","t","o","r",00
dcb "C","a","l","c","u","l","a","t","o","r",00  ; same banner

; =================== Main Routine: ADDNUMBERS ===================
ADDNUMBERS:
    ; --- Prompt for the first number ---
    JSR PRINT_STR
    ; Original prompt lines combined below:
    ; dcb $0d,$0d,"E","n","t","e","r",32,"a",32,"n","u","m","b","e","r"
    ; dcb "(","0","-","9","9",")",":"
    ; dcb 32,32,32,32,32,32,32,32,00
    dcb $0D,$0D,"E","n","t","e","r",32,"a",32,"n","u","m","b","e","r","(","0","-","9","9",")",":",32,00  ; changed: combined prompt

    LDA #$00                    
    ; Original: STA value_h
    ;STA value_h
    STA SUM_H       ; clear hundreds digit (changed)
    JSR GET_NUMBER  ; changed from GETNUM; get first number from user
    ; Original: STA value
    ;STA value
    STA SUM_BCD     ; store first number in SUM_BCD (changed)

    ; --- Prompt for the second number ---
    JSR PRINT_STR
    dcb "E","n","t","e","r",32,"a","n","o","t","h","e","r",32,"n","u","m","b","e","r","(","0","-","9","9",")",":",32,00  ; changed prompt text
    JSR GET_NUMBER  ; get second number; result returned in accumulator

    ; --- Perform BCD Addition ---
    SED             ; set decimal mode
    CLC             ; clear carry flag
    ADC SUM_BCD     ; add second number to first number in accumulator
    CLD             ; clear decimal mode
    STA SUM_BCD     ; store sum in SUM_BCD
    BCC SKIP_HUNDREDS  ; branch if no carry (i.e. no hundreds digit)
    INC SUM_H       ; increment hundreds digit if carry occurred
SKIP_HUNDREDS:

    ; --- Display Result ---
    ; Save accumulator on stack for later restoration
    PHA
    JSR PRINT_STR
    ; Original: dcb "R","e","s","u","l","t",":",32 ... (many spaces)
    dcb "R","e","s","u","l","t",":",32,32,32,32,32,32,32,32,32,32,00  ; changed: shorter spacing but same idea

    ; --- Process Hundreds Digit (if any) ---
    LDA SUM_H
    BEQ PROCESS_TENS    ; if no hundreds digit, jump to tens
    AND #$0F
    ORA #$30
    JSR PUT_CHAR        ; output hundreds digit character

    JSR UPDATE_GFX      ; update pointer for digit graphic
    LDA #$10            
    LDX CUR_X          ; use CUR_X (changed)
    LDY CUR_Y          ; use CUR_Y (changed)
    JSR DRAW_IMAGE     ; call modified DRAW subroutine (renamed)
    LDA CUR_X
    CLC
    ADC #$06           ; move drawing cursor right by 6 pixels
    STA CUR_X
    JMP PROCESS_TENS

    ; --- Process Tens Digit ---
PROCESS_TENS:
    LDA SUM_BCD
    AND #$F0           ; extract tens digit (upper nibble)
    BEQ PROCESS_ONES  ; if tens digit is zero, skip to ones

    ; Process tens digit: shift to lower nibble and output
    LDA SUM_BCD
    LSR
    LSR
    LSR
    LSR
    ORA #$30
    JSR PUT_CHAR

    JSR UPDATE_GFX      ; update graphic pointer for tens digit
    LDA #$10
    LDX CUR_X
    LDY CUR_Y
    JSR DRAW_IMAGE     ; draw tens digit graphic
    LDA CUR_X
    CLC
    ADC #$06
    STA CUR_X

    ; --- Process Ones Digit ---
PROCESS_ONES:
    LDA SUM_BCD
    AND #$0F           ; extract ones digit (lower nibble)
    ORA #$30
    JSR PUT_CHAR

    JSR UPDATE_GFX      ; update graphic pointer for ones digit
    LDA #$10
    LDX CUR_X
    LDY CUR_Y
    JSR DRAW_IMAGE     ; draw ones digit graphic
    LDA CUR_X
    CLC
    ADC #$06
    STA CUR_X

    RTS

; =================== Subroutine: DRAW_IMAGE ===================
; Original label: DRAW
; Changed label to DRAW_IMAGE and variable names
; Entry:
;   A  - pointer to image data structure in zero page (address, width, height)
;   X  - horizontal coordinate (CUR_X)
;   Y  - vertical coordinate (CUR_Y)
; Exit: All registers undefined

; Zero-page variables for drawing (renamed):
; Original: define IMGPTR    $A0
;define IMGPTR    $A0
define      IMG_PTR    $A0   ; changed: IMG_PTR instead of IMGPTR
; Original: define IMGPTRH   $A1
;define IMGPTRH   $A1
define      IMG_PTR_H  $A1   ; changed: IMG_PTR_H instead of IMGPTRH
; Original: define IMGWIDTH  $A2
;define IMGWIDTH  $A2
define      IMG_W      $A2   ; changed: IMG_W instead of IMGWIDTH
; Original: define IMGHEIGHT $A3
;define IMGHEIGHT $A3
define      IMG_H      $A3   ; changed: IMG_H instead of IMGHEIGHT
; Original: define SCRPTR    $A4
;define SCRPTR    $A4
define      SCR_PTR    $A4   ; changed: SCR_PTR instead of SCRPTR
; Original: define SCRPTRH   $A5
;define SCRPTRH   $A5
define      SCR_PTR_H  $A5   ; changed: SCR_PTR_H instead of SCRPTRH
; Original: define SCRX      $A6
;define SCRX      $A6
define      SCR_X      $A6   ; changed: SCR_X instead of SCRX
; Original: define SCRY      $A7
;define SCRY      $A7
define      SCR_Y      $A7   ; changed: SCR_Y instead of SCRY

DRAW_IMAGE:
    STY SCR_Y         ; save current Y coordinate
    STX SCR_X         ; save current X coordinate

    TAY
    LDA $0006,Y       ; load low byte of image pointer
    STA IMG_PTR       ; store in IMG_PTR (changed)
    LDA $0007,Y       ; load high byte of image pointer
    STA IMG_PTR_H     ; store in IMG_PTR_H (changed)
    LDA $0002,Y       ; load image width
    STA IMG_W
    LDA $0003,Y       ; load image height
    STA IMG_H

    ; Calculate screen address: $0200 + SCR_X + (SCR_Y * 32)
    LDA #$00
    STA SCR_PTR_H
    LDA SCR_Y
    STA SCR_PTR
    LDY #$05          ; 5 left shifts to multiply by 32
SHIFT_LOOP:
    ASL SCR_PTR
    ROL SCR_PTR_H
    DEY
    BNE SHIFT_LOOP

    ; Add SCR_X to the computed offset
    LDA SCR_X
    CLC
    ADC SCR_PTR
    STA SCR_PTR
    LDA #$00
    ADC SCR_PTR_H
    STA SCR_PTR_H

    ; Add base screen address $0200 (only high byte is affected)
    LDA #$02
    CLC
    ADC SCR_PTR_H
    STA SCR_PTR_H

    ; Copy image data row by row from the image pointer to screen memory
ROW_COPY:
    LDY #$00
ROW_LOOP:
    LDA (IMG_PTR),Y
    STA (SCR_PTR),Y
    INY
    CPY IMG_W
    BNE ROW_LOOP

    ; Advance image data pointer to next row
    LDA IMG_W
    CLC
    ADC IMG_PTR
    STA IMG_PTR
    LDA #$00
    ADC IMG_PTR_H
    STA IMG_PTR_H

    ; Advance screen pointer by one row (32 bytes)
    LDA #32
    CLC
    ADC SCR_PTR
    STA SCR_PTR
    LDA #$00
    ADC SCR_PTR_H
    STA SCR_PTR_H

    DEC IMG_H
    BNE ROW_COPY

    RTS

; =================== Subroutine: PRINT_STR ===================
; Renamed from PRINT; prints a null-terminated string
PRINT_STR:
    PLA
    CLC
    ADC #$01
    STA PRNT_PTR      ; changed pointer name
    PLA
    STA PRNT_PTR_H
    TYA
    PHA
    LDY #$00
PRINT_LOOP:
    LDA (PRNT_PTR),Y
    BEQ STR_DONE
    JSR PUT_CHAR     ; use PUT_CHAR instead of CHROUT
    INY
    JMP PRINT_LOOP
STR_DONE:
    TYA
    CLC
    ADC PRNT_PTR
    STA PRNT_PTR
    LDA PRNT_PTR_H
    ADC #$00
    STA PRNT_PTR_H
    PLA
    TAY
    LDA PRNT_PTR_H
    PHA
    LDA PRNT_PTR
    PHA
    RTS

; =================== Subroutine: GET_NUMBER ===================
; Renamed from GETNUM; reads a 2-digit number and returns a BCD in A
GET_NUMBER:
    TXA
    PHA
    TYA
    PHA
    LDX #$00
    STX NUM1_BUF    ; use NUM1_BUF (changed)
    STX NUM2_BUF    ; use NUM2_BUF (changed)
NUM_CURSOR:
    LDA #$A0       ; display black square as cursor background
    JSR PUT_CHAR
    LDA #$83       ; display left cursor symbol
    JSR PUT_CHAR
GET_KEY:
    JSR GET_CHAR   ; wait for a key press
    CMP #$00
    BEQ GET_KEY
    CMP #$08       ; BACKSPACE?
    BEQ NUM_BACKSPACE
    CMP #$0D       ; ENTER?
    BEQ NUM_ENTER
    CMP #$30       ; valid digit starts at '0'
    BMI GET_KEY
    CMP #$3A       ; up to '9'
    BMI NUM_DIGIT
    JMP GET_KEY
NUM_ENTER:
    CPX #$00
    BEQ GET_KEY
    LDA #$20
    JSR PUT_CHAR
    LDA #$0D
    JSR PUT_CHAR
    LDA NUM1_BUF
    CPX #$01
    BEQ NUM_DONE
    ASL
    ASL
    ASL
    ASL            ; shift left 4 bits (multiply by 16)
    ORA NUM2_BUF   ; combine digits into BCD
NUM_DONE:
    STA NUM1_BUF
    PLA
    TAY
    PLA
    TAX
    LDA NUM1_BUF
    RTS
NUM_DIGIT:
    CPX #$02       ; allow only 2 digits
    BPL GET_KEY
    PHA
    JSR PUT_CHAR
    PLA
    AND #$0F
    STA NUM1_BUF,X
    INX
    JMP NUM_CURSOR
NUM_BACKSPACE:
    CPX #$00
    BEQ GET_KEY
    LDA #$20
    JSR PUT_CHAR
    LDA #$83
    JSR PUT_CHAR
    JSR PUT_CHAR
    LDA #$20
    JSR PUT_CHAR
    LDA #$83
    JSR PUT_CHAR
    DEX
    LDA #$00
    STA NUM1_BUF,X
    JMP NUM_CURSOR

; =================== Subroutine: UPDATE_GFX ===================
; Renamed from update_value; sets pointer for 5x5 graphic for a digit
UPDATE_GFX:
    CMP #$31        ; compare with ASCII '0'
    BMI SET_GFX_0
    CMP #$32
    BMI SET_GFX_1
    CMP #$33
    BMI SET_GFX_2
    CMP #$34
    BMI SET_GFX_3
    CMP #$35
    BMI SET_GFX_4
    CMP #$36
    BMI SET_GFX_5
    CMP #$37
    BMI SET_GFX_6
    CMP #$38
    BMI SET_GFX_7
    CMP #$39
    BMI SET_GFX_8
    CMP #$40
    BMI SET_GFX_9
    RTS
SET_GFX_0:
    LDA #<N_0
    STA $16
    LDA #>N_0
    STA $17
    RTS
SET_GFX_1:
    LDA #<N_1
    STA $16
    LDA #>N_1
    STA $17
    RTS
SET_GFX_2:
    LDA #<N_2
    STA $16
    LDA #>N_2
    STA $17
    RTS
SET_GFX_3:
    LDA #<N_3
    STA $16
    LDA #>N_3
    STA $17
    RTS
SET_GFX_4:
    LDA #<N_4
    STA $16
    LDA #>N_4
    STA $17
    RTS
SET_GFX_5:
    LDA #<N_5
    STA $16
    LDA #>N_5
    STA $17
    RTS
SET_GFX_6:
    LDA #<N_6
    STA $16
    LDA #>N_6
    STA $17
    RTS
SET_GFX_7:
    LDA #<N_7
    STA $16
    LDA #>N_7
    STA $17
    RTS
SET_GFX_8:
    LDA #<N_8
    STA $16
    LDA #>N_8
    STA $17
    RTS
SET_GFX_9:
    LDA #<N_9
    STA $16
    LDA #>N_9
    STA $17
    RTS

; =================== Image Data Definitions ===================
; The following blocks define 5x5 pixel images for digits and other symbols.

G_O:
    DCB $11,$11,$11,$11,$11
    DCB $11,$00,$00,$00,$11
    DCB $11,$11,$11,$11,$11
    DCB $00,$00,$00,$00,$11
    DCB $11,$11,$11,$11,$11

N_0:
    DCB $00,$11,$11,$11,$00
    DCB $11,$00,$00,$00,$11
    DCB $11,$00,$00,$00,$11
    DCB $11,$00,$00,$00,$11
    DCB $00,$11,$11,$11,$00

N_1:
    DCB $00,$11,$11,$00,$00
    DCB $11,$00,$11,$00,$00
    DCB $11,$00,$11,$00,$00
    DCB $00,$00,$11,$00,$00
    DCB $11,$11,$11,$11,$11

N_2:
    DCB $00,$11,$11,$11,$11
    DCB $11,$00,$00,$11,$00
    DCB $00,$00,$11,$00,$00
    DCB $00,$11,$00,$00,$11
    DCB $11,$11,$11,$11,$00

N_3:
    DCB $11,$11,$11,$11,$11
    DCB $00,$00,$00,$00,$11
    DCB $00,$00,$11,$11,$11
    DCB $00,$00,$00,$00,$11
    DCB $11,$11,$11,$11,$11

N_4:
    DCB $00,$00,$00,$11,$00
    DCB $00,$00,$11,$11,$00
    DCB $00,$11,$00,$11,$00
    DCB $11,$11,$11,$11,$11
    DCB $00,$00,$00,$11,$00

N_5:
    DCB $11,$11,$11,$11,$11
    DCB $11,$00,$00,$00,$00
    DCB $11,$11,$11,$11,$11
    DCB $00,$00,$00,$00,$11
    DCB $11,$11,$11,$11,$11

N_6:
    DCB $11,$11,$11,$11,$11
    DCB $11,$00,$00,$00,$00
    DCB $11,$11,$11,$11,$11
    DCB $11,$00,$00,$00,$11
    DCB $11,$11,$11,$11,$11

N_7:
    DCB $11,$11,$11,$11,$00
    DCB $00,$00,$00,$11,$00
    DCB $00,$00,$11,$11,$11
    DCB $00,$00,$00,$11,$00
    DCB $00,$00,$00,$11,$00

N_8:
    DCB $11,$11,$11,$11,$11
    DCB $11,$00,$00,$00,$11
    DCB $11,$11,$11,$11,$11
    DCB $11,$00,$00,$00,$11
    DCB $11,$11,$11,$11,$11

N_9:
    DCB $11,$11,$11,$11,$11
    DCB $11,$00,$00,$00,$11
    DCB $11,$11,$11,$11,$11
    DCB $00,$00,$00,$00,$11
    DCB $11,$11,$11,$11,$11

G_X:
    DCB $07,$00,$00,$00,$07
    DCB $00,$07,$00,$07,$00
    DCB $00,$00,$07,$00,$00
    DCB $00,$07,$00,$07,$00
    DCB $07,$00,$00,$00,$07

G_BLANK:
    DCB $00,$00,$00,$00,$00
    DCB $00,$00,$00,$00,$00
    DCB $00,$00,$00,$00,$00
    DCB $00,$00,$00,$00,$00
    DCB $00,$00,$00,$00,$00
