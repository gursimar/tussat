.include "c:\programme\atmel\avr studio\appnotes\2313def.inc"

rjmp reset

.dseg
string:				;just a label we can use (first SRAM address)
.cseg

reset:
ldi r16, low(RAMEND)		;stack setup
out SPL, r16
ldi YL, low(string)		;set Y to "string"
ldi YH, high(string)

rcall load_string		;call the routine
.db "Hello!", 0			;which shall store this string in SRAM

loop: rjmp loop			;and loop forever

load_string:			;the routine:
pop ZH				;pop return address
pop ZL
lsl ZL				;multiply by two
rol ZH
read_string:
lpm				;load character
mov r16, r0			;move to r16 (for cpi)
adiw ZL, 1			;inc Z
rcall process_string		;process the character in r16
cpi r16, 0			;compare character in r16 with zero
brne read_string		;if end of string not reached, loop
lsr ZH				;divide the return address by two
ror ZL
push ZL				;and push it onto the stack
push ZH
ret				;return

process_string:			;the characters are processed by
st Y+, r16			;storing them in SRAM, one after the other
ret