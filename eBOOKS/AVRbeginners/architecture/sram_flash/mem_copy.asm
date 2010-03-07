.include "c:\programme\atmel\avr studio\appnotes\2313def.inc"

.org 0x0000
rjmp reset

reset:
	ldi r16, low(RAMEND)
	out SPL, r16
	
	ldi XL, low(0x65)
	ldi XH, high(0x65)
	ldi YL, low(0x60)
	ldi YH, high(0x60)
	ldi r16, 10
	rcall copy_mem
	ldi XL, low(0x60)
	ldi XH, high(0x60)
	ldi YL, low(0x65)
	ldi YH, high(0x65)
	rcall copy_mem
	
	loop: rjmp loop
	
	copy_mem:
	mov r17, r16
	copy_mem_loop:
	tst r17
	breq end_copy_mem
 
	ld r18, X+
	st Y+, r18
	dec r17
	rjmp copy_mem_loop
 
	end_copy_mem:
	sub XL, r16
	sbci XH, 0
	sub YL, r16
	sbci YH, 0
	ret