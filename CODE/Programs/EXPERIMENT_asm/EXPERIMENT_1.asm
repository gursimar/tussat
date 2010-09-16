.org 0x0000
rjmp main

main:
ldi r16,0xFF
out DDRB,r16

loop:
Rjmp loop
