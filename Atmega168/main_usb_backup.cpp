
// Name: Makefile
// Project: ambilight
// Tabsize: 4
// This Revision: $Id$

#define LED_PORT_DDR        DDRC
#define LED_PORT_OUTPUT     PORTC
#define LED_BIT             0
#define SPI_CLOCK_MASK 0x03
#define SPI_2XCLOCK_MASK 0x01

#define SPI_MODE_MASK 0x0C 

#define SPI_PORT PORTB
#define SPI_DDR  DDRB
#define SPI_CS   PB2

#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>  /* for sei() */
#include <util/delay.h>     /* for _delay_ms() */

#include <avr/pgmspace.h>   /* required by usbdrv.h */
#include "usbdrv.h"
#include "oddebug.h"        /* This is also an example for using debug macros */
#include "requests.h"       /* The custom request numbers we use */

/* ------------------------------------------------------------------------- */
/* ----------------------------- USB interface ----------------------------- */
/* ------------------------------------------------------------------------- */

int diodes_r[25];
int diodes_g[25];
int diodes_b[25];
int current_diode=0;
int current_diode_rgb[3];
int step = 0;
int c=0;
int bytesRemaining=0;

uchar usbFunctionWrite(uchar *data, uchar len)
{
	int i = 0;
	int t=len;
	
	while(t>0) {
		switch(step) {
			case 0:
				current_diode=data[i];
				
				break;
				
			case 1:
				current_diode_rgb[0]=data[i];
				break;
			case 2:
				current_diode_rgb[1]=data[i];
				break;
			case 3:
				current_diode_rgb[2]=data[i];
				
				diodes_r[current_diode]=current_diode_rgb[0];
				diodes_g[current_diode]=current_diode_rgb[1];
				diodes_b[current_diode]=current_diode_rgb[2];
				step=-1;
				break;
				
				
		}
		
		step++;
		
		t--;
		i++;
	}

    bytesRemaining -= len;
    return bytesRemaining == 0;             // return 1 if we have all data
}

usbMsgLen_t usbFunctionSetup(uchar data[8])
{
usbRequest_t    *rq = (void *)data;
static uchar    dataBuffer[4];  /* buffer must stay valid when usbFunctionSetup returns */

	if(rq->bRequest == 100) {//send data
	 bytesRemaining = rq->wLength.word;
		 return USB_NO_MSG;        // tell driver to use usbFunctionWrite()
	}else 

    if(rq->bRequest == CUSTOM_RQ_ECHO){ /* echo -- used for reliability tests */
        dataBuffer[0] = rq->wValue.bytes[0];
        dataBuffer[1] = rq->wValue.bytes[1];
        dataBuffer[2] = rq->wIndex.bytes[0];
        dataBuffer[3] = rq->wIndex.bytes[1];
        usbMsgPtr = dataBuffer;         /* tell the driver which data to return */
        return 4;
    }else if(rq->bRequest == CUSTOM_RQ_SET_STATUS){
        if(rq->wValue.bytes[0] & 1){    /* set LED */
            LED_PORT_OUTPUT |= _BV(LED_BIT);
        }else{                          /* clear LED */
            LED_PORT_OUTPUT &= ~_BV(LED_BIT);
        }
    }else if(rq->bRequest == CUSTOM_RQ_GET_STATUS){
        dataBuffer[0] = ((LED_PORT_OUTPUT & _BV(LED_BIT)) != 0);
        usbMsgPtr = dataBuffer;         /* tell the driver which data to return */
        return 1;                       /* tell the driver to send 1 byte */
    }
    return 0;   /* default for not implemented requests: return no data back to host */
}

/* ------------------------------------------------------------------------- */

int __attribute__((noreturn)) main(void)
{
	//usb
uchar   i;

    wdt_enable(WDTO_1S);
    /* Even if you don't use the watchdog, turn it off here. On newer devices,
     * the status of the watchdog (on/off, period) is PRESERVED OVER RESET!
     */
    /* RESET status: all port bits are inputs without pull-up.
     * That's the way we need D+ and D-. Therefore we don't need any
     * additional hardware initialization.
     */
    odDebugInit();
    DBG1(0x00, 0, 0);       /* debug output: main starts */
    usbInit();
    usbDeviceDisconnect();  /* enforce re-enumeration, do this while interrupts are disabled! */
    i = 0;
    while(--i){             /* fake USB disconnect for > 250 ms */
        wdt_reset();
        _delay_ms(1);
    }
    usbDeviceConnect();
    LED_PORT_DDR |= _BV(LED_BIT);   /* make the LED bit an output */
	
	//timer
	TIMSK0 = _BV(OCIE0A);  // Enable Interrupt TimerCounter0 Compare Match A (SIG_OUTPUT_COMPARE0A)
    TCCR0A = _BV(WGM01);  // Mode = CTC
    TCCR0B = _BV(CS01);   // Clock/1024, 0.001024 seconds per tick
    OCR0A = 244;          // 0.001024*244 ~= .25 SIG_OUTPUT_COMPARE0A will be triggered 4 times per second.
	
	//spi
	
	SPI_DDR = (1<<PB3)|(1<<PB5)|(1<<PB2);
  
  SPCR |= _BV(MSTR);
  SPCR |= _BV(SPE);

  SPCR &= ~(_BV(DORD));
  SPCR = (SPCR & ~SPI_MODE_MASK) | 0x00;
 
  SPCR = (SPCR & ~SPI_CLOCK_MASK) | (0x01 & SPI_CLOCK_MASK);
  SPSR = (SPSR & ~SPI_2XCLOCK_MASK) | ((0x01 >> 2) & SPI_2XCLOCK_MASK);
  
  SPCR &= ~(_BV(DORD));
  int  ix;
  for(ix = 0;ix<25;ix++) {
		diodes_r[ix]=0xFF;
		diodes_g[ix]=0xFF;
		diodes_b[ix]=0xFF;
	}
	
	current_diode_rgb[0]=0x00;
	current_diode_rgb[1]=0x00;
	current_diode_rgb[2]=0x00;
  
  
	
	//usb
    sei();
    DBG1(0x01, 0, 0);       /* debug output: main loop starts */
	
	
	
	
    for(;;){                /* main event loop */
        DBG1(0x02, 0, 0);   /* debug output: main loop iterates */
        wdt_reset();
        usbPoll();
    }
}

 __attribute__ ((interrupt))  ISR (SIG_OUTPUT_COMPARE0A)
{
	if(bit_is_set(PORTC,0)) {
		PORTC &= ~_BV(0);
		
	}else{
	
	PORTC |= _BV(0);
	}
	
		for(c=0;c<25;) {
		
			for(SPDR = diodes_r[c++]; !(SPSR & _BV(SPIF)); );
			for(SPDR = diodes_g[c++]; !(SPSR & _BV(SPIF)); );
			for(SPDR = diodes_b[c++]; !(SPSR & _BV(SPIF)); );
			}
	
	if(c>=25)c=0;

	
}

/* ------------------------------------------------------------------------- */
