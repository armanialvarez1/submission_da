/*
 * DA2CT1.c
 *
 * 
 * Author : Mateo Markovic
 */ 

#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>

// function that delays 2500us x provided input using timer 0 in normal mode
void delay2500us(int cycles){
	cycles++;
	TCCR0A = 0; // Normal Operation
	TCNT0 = 0; // initialize timer 0 count to zero
	
	TCNT0=217; // start the count at 217
	TCCR0B = 5; // set the pre-scaler to 1024 and start timer
		
		// loop for the number of times user specified
		for (int i = 0; i < cycles; i++){
			while ((TIFR0 & 0x01) == 0); // check for overflow
				TCNT0=217; // make sure to set the counter back to 217
				TIFR0=0x01; // reset the overflow flag
		}
		
		TCCR0B = 0x00; // turn off the timer
}

int main()
{
	DDRB |= (1<<3); // set PORTB.3 as an output
	PORTB &= ~(1<<3); // make sure pinb.3 starts at 0

	DDRB |= (1<<2); // set PORTB.2 as an output
	PORTB &= ~(1<<2); // make sure pinb.2 starts at 0
	
	DDRC &= ~(1<<3); // set PORTC.3 as an input
	PORTC |= (1<<3); //Set PINC.3 active high (pull-up)
	
    while (1)
    {
		PORTB |= (1<<3); // turn on portb.3
		delay2500us(165); // delay .4125 secs
		PORTB &= ~(1<<3); // turn off portb.3
		delay2500us(135); // delay .3375 secs
		
		// if PINC.3 is now LOW
		if (!(PINC & (1<<3))) {
			PORTB &= ~(1<<3); // turn off portb.3
			PORTB |= (1<<2); // turn on portb.2
			delay2500us(800); // delay 2 secs
			PORTB &= ~(1<<2); // turn off portb.2
			PORTB |= (1<<3); // turn on portb.3
		}
    }
}

