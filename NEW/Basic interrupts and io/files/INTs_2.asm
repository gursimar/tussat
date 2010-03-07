;-----------------------------------------------------------------
; Name:		ints_2.asm
; Title:	Simple AVR Interrupt Verification Program, part 2
;-----------------------------------------------------------------

.include "8515def.inc"

; Interrupt service vectors

.org $0000
	rjmp Reset	; Reset vector
.org INT0addr
	rjmp IntV0	; INT0 vector (ext. interrupt from pin D2)	
.org INT1addr
	rjmp IntV1	; INT1 vector (ext. interrupt from pin D3)

; -- new interrupt vector -
.org OVF0addr
	rjmp TimerV0	; T/C0 overflow vector
	
;-----------------------------------------------------------------
;
; Register defines for main loop

.def	TIME=r16
.def	TEMP=r17
.def	BEEP=r18

;-----------------------------------------------------------------
;
; Reset vector - just sets up interrupts and service routines and
; then loops forever.

Reset:
	ldi	TEMP,low(RAMEND)	; Set stackptr to ram end
	out	SPL,TEMP
	ldi 	TEMP, high(RAMEND)
	out 	SPH, TEMP

	ser	TEMP		; Set TEMP to $FF to...
	out	DDRB,TEMP	; ...set data direction to "out"
	out	PORTB,TEMP	; ...all lights off!
	
	out	PORTD,TEMP	; ...all high for pullup on inputs
	ldi	TEMP,(1<<DDD6)	; bit D6 only configured as output, 
	out	DDRD,TEMP	; ...output for piezo buzzer on pin D6

	; set up int0 and int1
	
	ldi	TEMP,(1<<INT0)+(1<<INT1) ; int masks 0 and 1 set
	out	GIMSK,TEMP

	ldi	TEMP,$0f	; interrupt t0 and t1 on rising edge only
	out	MCUCR,TEMP

	;-- New init code block--
	ldi	TIME,$80	; Start from 128. NB!

	out	TCNT0,TIME		; set Timer/counter also.
	ldi	TEMP,(1<<TOIE0)	; timer overflow interrupt enable 0
	out	TIMSK,TEMP

	ldi	TEMP,$02	; clock prescaler = clk/8
	out	TCCR0,TEMP
		
	sei			; enable interrupts and off we go!

loop:
	rjmp	loop		; Infinite loop - never terminates

;-----------------------------------------------------------------
;
; Int0 vector - decrease count

IntV0:
	dec	TIME
	rjmp	Int01	; jump to common code to display new count


;-----------------------------------------------------------------
;
; Int1 vector - increase count

IntV1:
	inc 	TIME	; drop to common code to display new count

Int01:
	mov	r0,TIME		; display on LEDs
	com	r0
	out	PORTB,r0
	reti

;-----------------------------------------------------------------
;
; Timer 0 vector - toggle bit PD6

TimerV0:
	out	TCNT0,TIME	; reset time

	com	BEEP
	ori	BEEP,$BF	; bit 6 only; all others high for pullup
	out	PORTD,BEEP

	reti			; important - must reti to set interrupt flag
