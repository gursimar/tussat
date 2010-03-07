#include<avr/io.h>
#include<util/delay.h>
#include<compat/deprecated.h>
#include<avr/interrupt.h>
#define ECPU 12000000U
void main(void)
{int i;
PORTC=0xFF;
ADMUX=0b00100000;
SFIOR=0x00;
ADCSRA=0b11100101;
while(1)
{
i=ADCH;
i=i>>2;
i=i<<2;
PORTC=i;
}

}
