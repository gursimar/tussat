.include "c:\programme\atmel\avr studio\appnotes\2313def.inc"
;LED (with current limiting resistor) connected to AT90S2313 (0C1 / PortB.3)
;anode to Vcc, cathode to pin. Result: The LED is active when the port pin is low.
;we'll need inverted pwm output. 

;The AVR is running at 7.3728 MHz. For other frequencies,
;alter the TCNT0 reload value to your_freq / 256 / 256 (256 prescaler, 256 ISRs per second)
;this will go through the whole pwm range (8 bit) in one second. You can also use
;other resolution settings.
.org 0x0000
rjmp reset
.org 0x0006
rjmp Tim0_ovf			;Timer 0 overflow interrupt

reset:
ldi r16, low(RAMEND)		;stack setup
out SPL, r16

sbi DDRB, 3			;PortB.3 is the Output Compare 1 pin (for PWM)

ldi r16, 143
out TCNT0, r16			;set TCNT0 to 143 for overflow after (7.3228MHz / 256/ 256)
ldi r16, 0
out TCNT1H, r16			;clear TCNT1 and OCR1A
out TCNT1L, r16
out OCR1AH, r16
out OCR1AL, r16

ldi r16, 0b00000100		;prescaler: 256
out TCCR0, r16
ldi r16, 0b00000010		;allow Timer 0 overflow interrupt
out TIMSK, r16

ldi r16, 0b11000001		;Timer 1 PWM mode, inverted (LED is active low)
out TCCR1A, r16			;8-bit PWM
ldi r16, 0b00000001		;enable Timer 1 with no prescaler
out TCCR1B, r16

ldi r16, 0xFF			;set flag register to 0xFF (increase PWM value in
mov r2, r16			;ISR)

sei				;enable interrupts

loop:
rjmp loop			;loop forever

tim0_ovf:			;The ISR (T0 overflow)
tst r2				;flag register = 0 (decreasing PWM value?)
breq led_down			;yes? go to led_down
in r17, OCR1AL			;else get pwm value
inc r17				;increment it
out OCR1AL, r17			;and write back
cpi r17, 0xFF			;max value for 8-bit pwm reached?
breq switch_down		;yes? go to switch_down (dec in next ISR)
re_led_up:
ldi r16, 143			;reload TCNT0 with value for 7.3728 MHz / 256 / 256)
out TCNT0, r16
reti				;and that's it
switch_down:			;switch to dec pwm:
clr r2				;set flag register to 0
rjmp re_led_up			;and return

led_down:			;we're decreasing the pwm value...
in r17, OCR1AL			;get pwm value
dec r17				;decrement it
out OCR1AL, r17			;and write back
tst r17				;0 reached?
breq switch_up			;inc in next ISR
re_led_down:
ldi r16, 143			;reload TCNT0 with value for 7.3728 MHz / 256
out TCNT0, r16
reti				;and return
switch_up:
ldi r16, 0xFF			;load flag register with 0xFF
mov r2, r16
rjmp re_led_down		;and return