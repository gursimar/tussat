#include<avr/io.h>
#include<util/delay.h>
#include<compat/deprecated.h>
#include<avr/interrupt.h>
#define ECPU 12000000UL
ISR(INT0_vect)
{
sbi(PORTC,PIN7);
sbi(PORTC,PIN6);
sbi(PORTC,PIN5);
sbi(PORTC,PIN4);
_delay_ms(255);
_delay_ms(255);
_delay_ms(255);
_delay_ms(255);
cbi(PORTC,PIN7);
cbi(PORTC,PIN6);
cbi(PORTC,PIN5);
cbi(PORTC,PIN4);
}

void main(void)
{

GICR=0XC0;
MCUCR=0X0E;
sei();
while(1)
{
_delay_ms(255);
_delay_ms(255);
_delay_ms(255);
_delay_ms(255);
DDRC=0XFF;
DDRA=0X00;
if(bit_is_set(PINA,6))
{
PORTC=0b00000011;
_delay_ms(255);
_delay_ms(255);
_delay_ms(255);
_delay_ms(255);
}
if(bit_is_clear(PINA,6))
{
PORTC=0b00001100;
_delay_ms(255);
_delay_ms(255);
_delay_ms(255);
_delay_ms(255);
}
}}
