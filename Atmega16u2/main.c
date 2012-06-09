#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>  

#define SPI_PORT PORTB
#define SPI_DDR  DDRB
#define SPI_CS   PB2

#define SPI_CLOCK_MASK 0x03
#define SPI_2XCLOCK_MASK 0x01

#define SPI_MODE_MASK 0x0C 
#define output_low(port,pin) port &= ~(1<<pin)
#define output_high(port,pin) port |= (1<<pin)
#define set_input(portdir,pin) portdir &= ~(1<<pin)
#define set_output(portdir,pin) portdir |= (1<<pin)

volatile int diodes[25*3];
volatile int current=0;
volatile int time=0;

int main (void)
{
	//spi
	/*int bittimer = ( F_CPU / 128000 / 16 ) - 1;
	SPI_DDR = (1<<PB3)|(1<<PB5)|(1<<PB2);

	SPCR |= _BV(MSTR);
	SPCR |= _BV(SPE);

	SPCR &= ~(_BV(DORD));
	SPCR = (SPCR & ~SPI_MODE_MASK) | 0x00;

	SPCR = (SPCR & ~SPI_CLOCK_MASK) | (0x01 & SPI_CLOCK_MASK);
	SPSR = (SPSR & ~SPI_2XCLOCK_MASK) | ((0x01 >> 2) & SPI_2XCLOCK_MASK);
	SPCR &= ~(_BV(DORD));
  
	//serial
	/* Set the baud rate 
	UBRR0H = (unsigned char) (bittimer >> 8);
	UBRR0L = (unsigned char) bittimer;
	/* set the framing to 8N1 
	UCSR0C = (3 << UCSZ00);
	/* Engage! 
	UCSR0B = (1 << RXEN0) | (1 << TXEN0) | (1 << RXCIE0 );
	
  
	//timer
	TIMSK0 = _BV(OCIE0A);  // Enable Interrupt TimerCounter0 Compare Match A (SIG_OUTPUT_COMPARE0A)
	TCCR0A = _BV(WGM01);  // Mode = CTC
	TCCR0B = _BV(CS02) | _BV(CS00);   // Clock/1024, 0.001024 seconds per tick
	OCR0A = 244;          // 0.001024*244 ~= .25 SIG_OUTPUT_COMPARE0A will be triggered 4 times per second.

	sei();*/
	set_output(DDRB, PB0);
	
	while(1) {
		_delay_ms(500);
		output_high(PORTB, PB0);
		_delay_ms(500);
		output_low(PORTB, PB0);
		
	}
	
   return 0;
}

ISR (SIG_OUTPUT_COMPARE0A)
{
	int i=0;
	time++;
	
	if(time > 4*30) {
		for(i = 0;i<25;i++) {
			for(SPDR = 0x00; !(SPSR & _BV(SPIF)); );
			for(SPDR = 0x00; !(SPSR & _BV(SPIF)); );
			for(SPDR = 0x00; !(SPSR & _BV(SPIF)); );
		}
		
		current=0;
		time=0;
	}
}
/*
ISR(SIG_USART_RECV) 
{ 
	char ReceivedByte;
	int i;
	
	ReceivedByte = UDR0;

	diodes[current++]=ReceivedByte;
	time=0;

	if(current>=25*3) {
		current=0;
		for(i = 0;i<25*3;i++) {
			for(SPDR = diodes[i]; !(SPSR & _BV(SPIF)); );
		}
	}
}
*/