#include <avr/eeprom.h> 

void main(void) 
{ 
    

	 DDRC=0XFF;


   uint8_t value=0x11;


 eeprom_write_byte ((uint8_t *)46, value);

    
	PORTC=eeprom_read_byte((uint8_t*)46);
}
