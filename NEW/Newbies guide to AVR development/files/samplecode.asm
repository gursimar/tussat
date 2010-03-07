;My Very First AVR Project

.include "8515def.inc"  ;Includes the 8515 definitions file

.def Temp = R16          ;Gives "Defines" Register R16 the name Temp

.org 0x0000             ;Places the following code from address 0x0000
   rjmp  RESET          ;Take a Relative Jump to the RESET Label
	
RESET:                  ;Reset Label
   ldi Temp, 0xFF       ;Store 255 in R16 (Since we have defined R16 = Temp)
   out DDRB, Temp       ;Store this value in The PORTB Data direction Register

Loop:                   ;Loop Label
   out PORTB, Temp      ;Write all highs (255 decimal) to PORTB
   dec Temp             ;Decrement R16 (Temp)
   rjmp Loop            ;Take a relative jump to the Loop label
