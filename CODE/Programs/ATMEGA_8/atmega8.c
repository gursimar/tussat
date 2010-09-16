//	include files
//#define F_CPU 1000000UL	//this statement works    1mhz is default fre
#include <salvo.h>

#include <avr/interrupt.h>
#include <avr/eeprom.h> 
//#include <util/delay.h>
#include <avr/io.h>

//prototypes
void check(void);
void snake(void);
void rtclock(void);

//variables
unsigned char i=0;
unsigned char count=0;

unsigned char d=0;	//day
unsigned char m=0;	//month means 127 days
//unsigned char y=0;	//year
unsigned char h=0;	//hour
unsigned char min=0;	//minutes
unsigned char sec=0;	//seconds

uint8_t value=0x00;
char temp=0;	//8bit temp variable



//	Board- and compiler-specific initilaization, etc.
void board_init ( void )
{
  // Timer1 init.
 	TCCR1B = 0x00;  // Stop Timer1
  	TCNT1H = 0x00;  // Clear Timer1
  	OCR1AH = 0x00;  // Set Compare A to 39
  	OCR1AL = 0x26;  //  ((4MHz/1024)/39) = 10ms (9.984ms) timer
  	TIMSK  = _BV(OCIE1A);  // Compare A Interrupt enable
  	TCCR1B = _BV(WGM12)|_BV(CS12)|_BV(CS10);  // Start Timer1 with clk/1024
}

void board_enable_interrupt ( void )
{
  	sei();
}



/************************************************************
****                                                     ****
**                                                         **
Active ISRs.

Timer1: Interrupt happens at predefined system tick rate,
          calls OSTimer(). Since this ISR calls a Salvo
          service, it (and all other ISRs that call Salvo
          services) must be disabled by Salvo's interrupt
          hooks.

**                                                         **
****                                                     ****
************************************************************/
ISR(TIMER1_COMPA_vect) 
{	
	//The whole code depends on the interrupt from the timer ........
	if (count >=40)
	{
		//every one sec ...
		rtclock();
		count=0;
	}
 	OSTimer();
	//PORTB=~PORTB;
}

ISR(TIMER1_CAPT_vect, ISR_ALIASOF(TIMER1_COMPA_vect));


int main( void )
{
	board_enable_interrupt();
	board_init();
	OSInit();
	OSCreateTask(check, OSTCBP(1), 8);	//moderate priority
	OSCreateTask(snake, OSTCBP(2), 3);		//high priority
	
//	board_init();
	
	DDRB=0xFF;
	DDRC=0xFF;
	DDRD=0xFF;
	PORTB=0b00000001;

	while (1) 
	{
		OSSched();
	}
}

//low Priority
void check()
{
	while(1)
	{
		if(PINC==0b0000001)
		{
			
			eeprom_write_byte ((uint8_t *)46, value);
			
			// only for veryfing purposes ....

			temp = min<<4;
			value = temp | sec;
			PORTB=eeprom_read_byte((uint8_t*)46);
			

			OS_Delay(25);
			//_delay_ms(5000);
		}
		else
		{
			PORTD=1<<i;
		}
		OS_Yield();
	}
}

//high priority
void snake()
{
	
	while(1)
	{
		PORTD=1<<(i);
		
		i++;
		if(i==8)
		{
			i=0;
		}

		//OS_Yield();

		 OS_Delay(50);
		//_delay_ms(1000);
		
	}
	
}

void rtclock(void)
{
	sec++;

	if(sec>=60)
	{
		sec=0;
		min++;
		
		if(min>=60)
		{
			h++;
			if(h>=24)
			{
				d++;
				if(d>=127)
				{
					m++;
				}
			}

		}
	}
}
