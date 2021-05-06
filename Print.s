; Print.s
; Student names: change this to your names or look very silly
; Last modification date: change this to the last modification date or look very silly
; Runs on TM4C123
; EE319K lab 7 device driver for any LCD
;
; As part of Lab 7, students need to implement these LCD_OutDec and LCD_OutFix
; This driver assumes two low-level LCD functions
; SSD1306_OutChar   outputs a single 8-bit ASCII character
; SSD1306_OutString outputs a null-terminated string 

    IMPORT   SSD1306_OutChar
    IMPORT   SSD1306_OutString
    EXPORT   LCD_OutDec
    EXPORT   LCD_OutFix
    PRESERVE8
    AREA    |.text|, CODE, READONLY, ALIGN=2
    THUMB

size EQU 4 ;Binding


;-----------------------LCD_OutDec-----------------------
; Output a 32-bit number in unsigned decimal format
; Input: R0 (call by value) 32-bit unsigned number
; Output: none
; Invariables: This function must not permanently modify registers R4 to R11
LCD_OutDec

	 SUB SP, #8 ;create offset for stack pointer (Allocation)
	 MOV R12, SP ;save SP in R12
	 MOV R1, #0 ;intialize pointer
	 
	 MOV R2, #10 
	 STR R2, [R12,#size] ;Store R2 (local variable) onto stack
	 
back1
	 MOV R3, R0 ;Save input value in R3
	 LDR R2, [R12, #size] ;Read R2 (local variable) from stack
	 ; n%10 (Mod operator --> DIV, MUL, SUB)
	 UDIV R0, R2 ;R0/10
	 MUL R2, R0, R2 ;R0*10
	 SUB R2, R3, R2 
	 
	 PUSH {R2, LR} ;Save R2
	 ADD R1, #1 ;increment pointer
	 CMP R0, #0 ;keep checking until R0 is 0
	 BNE back1
	 
back2
	 POP {R0, LR} ;Restore R0
	 ADD R0, #0x30 ;ASCII offset
	 PUSH {R1, LR} ;Save pointer and link register
	 BL SSD1306_OutChar ;Print character
	 POP {R1, LR}
	 SUB R1, #1 ;Decrement pointer
	 CMP R1, #0 ;iterate again until pointer is at 0
	 BNE back2
	 ADD SP, #8 ;Deallocation

     BX  LR
;* * * * * * * * End of LCD_OutDec * * * * * * * *

; -----------------------LCD _OutFix----------------------
; Output characters to LCD display in fixed-point format
; unsigned decimal, resolution 0.01, range 0.00 to 9.99
; Inputs:  R0 is an unsigned 32-bit number
; Outputs: none
; E.g., R0=0,    then output "0.00 "
;       R0=3,    then output "0.03 "
;       R0=89,   then output "0.89 "
;       R0=123,  then output "1.23 "
;       R0=999,  then output "9.99 "
;       R0>999,  then output "*.** "
; Invariables: This function must not permanently modify registers R4 to R11
LCD_OutFix
	 
	 MOV R1, #999 ;range is 999
	 CMP R0, R1 ;if greater than 999, print *.***
	 BHI higher 
	 
	 MOV R1, #100 
	 MOV R2, R0 ;save input in R2
	 ;n%100
	 UDIV R0, R0, R1 ;R0/100
	 MUL R1, R1, R0 ;100*R0
	 SUB R2, R2, R1 ;R2 -> remaining three digits
	 
	 ADD R0, R0, #0x30 ;offset for ASCII value
	 
	 ;First digit
	 PUSH {R2, LR}
	 BL SSD1306_OutChar
	 
	 ;Period (.)
	 MOV R0, #0x2E
	 BL SSD1306_OutChar
	 POP {R2, LR}
	 
	 ;n&10
	 MOV R1, #10 
	 UDIV R0, R2, R1 ;R2/10
	 MUL R1, R1, R0 ;10*R0
	 SUB R2, R2, R1 ;remaining two digits
	 
	 ADD R0, R0, #0x30 ;offset for ASCII value
	 
	 ;Second digit
	 PUSH {R2, LR}
	 BL SSD1306_OutChar
	 POP {R2, LR}
	 
	 MOV R0, R2
	 
	 ADD R0, #0x30 ;offset for ASCII value
	 
	 ;Third digit
	 PUSH {R2, LR}
	 BL SSD1306_OutChar
	 POP {R2, LR}
	 
	 B done
	 
higher
	;print *.***
	MOV R0, #0x2A ;Ascii value for *
	PUSH {R0, LR}
	BL SSD1306_OutChar
	POP {R0, LR}
	
	MOV R0, #0x2E ;Ascii value for .
	PUSH {R0, LR}
	BL SSD1306_OutChar
	POP {R0, LR}
	
	MOV R0, #0x2A ;Ascii value for *
	PUSH {R0, LR}
	BL SSD1306_OutChar
	POP {R0, LR}
	
	MOV R0, #0x2A ;Ascii value for *
	PUSH {R0, LR}
	BL SSD1306_OutChar
	POP {R0, LR}
	
	
done
     BX   LR
 
     ALIGN
;* * * * * * * * End of LCD_OutFix * * * * * * * *

     ALIGN          ; make sure the end of this section is aligned
     END            ; end of file
