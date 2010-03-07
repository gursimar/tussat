.include "c:\programme\atmel\avr studio\appnotes\2313def.inc"
.dseg
string1: .byte 30
string2: .byte 30
.cseg
.org 0x0000
rjmp reset

reset:
ldi r16, low(RAMEND)
out SPL, r16
ldi XL, low(string1)
ldi XH, high(string1)
ldi YL, low(string2)
ldi YH, high(string2)
nop ; type in strings in memory window
rcall str_comp_nocase
brsh ramp
brlo lower

ramp:
breq equal
rjmp higher
lower:
nop
rjmp lower
equal:
nop
rjmp equal
higher:
nop
rjmp higher


;string compare routines
; X-> first string in SRAM, 0-terminated
; Y-> second string in SRAM, 0-terminated
; cp X, Y
; these routines are not case-sensitive (harder to write this code)


; the first thing we need is a routine that changes a character to lower-case.
; these are 'A' -> 'a' to 'Z' -> 'z'. All the other characters must remain unchaged.
; In the ascii table, upper case characters are 'A' (0x41) to 'Z' (0x5A). These must
; be changed to lower case 'a' (0x61) to 'z' (0x7A) by adding 0x20 (set bit 5)
; argument: r16; character
; return: r16; character (corrected)

char_conv_lowercase:
	cpi r16, 'A'		;check for lower boundary ('A' = 0x41)
	brsh check_upper_boundary
	ret
	check_upper_boundary:
	cpi r16, 'Z' + 1	;check for upper boundary; if lower than 'Z' + 1 
	brlo char_lowercase	;then the character is in upper case range
	ret
	char_lowercase:
	subi r16, -0x20		;convert (see above) to lower case
	ret
	
; now here's a routine that compares the string lengths:
; it works like 'cp X, Y'
; valid branches are brlo, breq, brsh
;the routine searches for the terminators and updates a counter for every character
;then a normal cp is done
;arguments: X, Y; point at strings
;return: X, Y unchanged
;r16: length of X string
;r17: length of Y string
;r18: 0
str_cp_length:
	push XL
	push XH
	push YL
	push YH			;save X and Y
	clr r16
	clr r18			;clear counters
	scl_checkX:
	ld r18, X+		;while *X != 0
	tst r18
	breq scl_checkY		;when end reached, check Y string
	inc r16			;increment r16
	rjmp scl_checkX		;loop

	scl_checkY:		;see scl_checkX
	ld r18, Y+
	tst r18
	breq scl_ret
	inc r17
	rjmp scl_checkY
	
	scl_ret:		;restore X and Y
	pop YH
	pop YL
	pop XH
	pop XL
	cp r16, r17		;perform compare
	ret			;and return
	
;this is a "real" string compare. The X and Y strings are compared character by character,
;not case-sensitive. If a character in Y different from X is detected, the routine returns
;and the flags of the compare are still valid and can be used. When a zero from one string
;is detected, two cases are possible: the other one also conatins a zero (same length) or
;the strings don't have the same length.
;arguments:
; X -> first string
; Y -> second string

;while X+ = Y+
;do:
	;inc r19
	;break
;ret
str_comp_nocase:
	push XL
	push XH
	push YL
	push YH
	clr r19
	str_comp_char:
	ld r16, X+
	tst r16
	breq sc_cp_terminator
	rcall char_conv_lowercase
	mov r17, r16
	ld r16, Y+
	rcall char_conv_lowercase
	mov r18, r16
	cp r17, r18
	brne sc_ret
	inc r19
	rjmp str_comp_char
	sc_cp_terminator:
	ld r18, Y+
	cp r16, r18
	sc_ret:
	pop YH
	pop YL
	pop XH
	pop XL
	ret