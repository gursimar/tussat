#include <avr/io.h>
#include <avr/interrupt.h>
static volatile uint8_t cnt2step=0;
unsigned int X=0;
unsigned int y=0; 

// this function gets now called in 20Hz intervals

ISR(TIMER2_COMP_vect)
{
        cnt2step++;
        X=PORTB4;
		if (cnt2step>=10)
		{
                if (PORTC&(1<<PORTC5))
				{
                        //LEDON;
						PORTC=0b00000000;
                }
				
				else
				{
                        PORTC=0b00100000;
                }
                
				cnt2step=0;
        }
}

/* setup timer T2 as an interrupt generating time base.
* You must call once sei() in the main program */
void init_cnt2(void)
{
        cnt2step=0;
        TIMSK=(1<<OCIE2); // compare match on OCR2
        TCNT2=0;  // init counter
        OCR2=250;//195; // value to compare against 3906Hz/195=20Hz
        // do not change any output pin, clear at compare match with OCR2:
        //TCCR2=(1<<WGM21);
        // divide clock by 256: 1MHz/256=3906.25Hz
        // clock divider, start counter (or with WGM21 setting):
//        TCCR2|=(1<<CS02)|(1<<CS01)|(0<<CS00);

		TCCR2|=(1<<CS02)|(1<<CS01)|(1<<CS00); //1024
		//no prescaling
		//TCCR2|=(0<<CS02)|(0<<CS01)|(1<<CS00); //no prescaling
		
}

int main(void)
{
        //LED_INIT;
        //LEDOFF;
		
		PORTC=0;
        init_cnt2();
        sei();

        // nothing to do in the main loop:
        while (1) {
		y++;
        }
        return(0);
}

