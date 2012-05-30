#include <avr/io.h>
#include <util/delay.h>


#define SPI_PORT PORTB
#define SPI_DDR  DDRB
#define SPI_CS   PB2

#define SPI_CLOCK_MASK 0x03
#define SPI_2XCLOCK_MASK 0x01

#define SPI_MODE_MASK 0x0C 

void delay_ms(uint16_t x);

int main (void)
{
  SPI_DDR = (1<<PB3)|(1<<PB5)|(1<<PB2);
  
  SPCR |= _BV(MSTR);
  SPCR |= _BV(SPE);

  SPCR &= ~(_BV(DORD));
  SPCR = (SPCR & ~SPI_MODE_MASK) | 0x00;
 
  SPCR = (SPCR & ~SPI_CLOCK_MASK) | (0x01 & SPI_CLOCK_MASK);
  SPSR = (SPSR & ~SPI_2XCLOCK_MASK) | ((0x01 >> 2) & SPI_2XCLOCK_MASK);
  
  SPCR &= ~(_BV(DORD));
   
  int selected=0;
  int t=0;
  _delay_ms(100);
  while(1) {
  
    for(int c=0; c<25; c++) {
		/*if(!(c==selected)) {
		 for(SPDR = 255; !(SPSR & _BV(SPIF)); );
			for(SPDR = 0; !(SPSR & _BV(SPIF)); );
			for(SPDR = 0; !(SPSR & _BV(SPIF)); );
		}else{
			for(SPDR = 0; !(SPSR & _BV(SPIF)); );
			for(SPDR = 0; !(SPSR & _BV(SPIF)); );
			for(SPDR = 255; !(SPSR & _BV(SPIF)); );
		}*/
		
		if(c<13 && t || !t && c>=13) {
			for(SPDR = 255; !(SPSR & _BV(SPIF)); );
			for(SPDR = 0; !(SPSR & _BV(SPIF)); );
			for(SPDR = 0; !(SPSR & _BV(SPIF)); );
			
			
		}else{
			for(SPDR = 0; !(SPSR & _BV(SPIF)); );
			for(SPDR = 0; !(SPSR & _BV(SPIF)); );
			for(SPDR = 255; !(SPSR & _BV(SPIF)); );
			}
	}
	
    _delay_ms(100);
	
	selected++;
	t=(t)?0:1;
	if(selected==25){
		selected=0;
	}
  
  }
   
   return(0);
}
