/*
 * DA2T1 C.c
 *
 * 
 * Author : Mateo Markovic
 */
 
#define F_CPU 16000000UL

#include <avr/io.h>
#include <util/delay.h>


int main(void)
{
	DDRB |= (1<<3); // set PORTB.3 as an output
	PORTB &= ~(1<<3); // make sure pinb.3 starts at 0
	
	DDRB &= ~(1<<2); // make sure DDRB.2 starts at 0
	
	DDRC &= ~(1<<3); // set PORTC.3 as an input
	PORTC |= (1<<3); //Set PINC.3 active high (pull-up)
	
	while(1){
		PORTB ^= (1<<3); // turn on portb.3
		_delay_ms(412.5); // delay .4125 secs
		PORTB ^= (1<<3); // turn off portb.3
		_delay_ms(337.5); // delay .3375 secs
		
		// if PINC.3 is now LOW
		if (!(PINC & (1<<3))) {
			DDRB &= ~(1<<3); // turn off portb.3
			DDRB ^= (1<<2); // turn on portb.2
			_delay_ms(2000); // delay 2 secs
			PORTB ^= (1<<2); // turn off portb.2
			DDRB ^= (1<<3); // turn on portb.3
		}
	}
}

