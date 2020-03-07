/*
 * DA2CT2.c
 *
 * 
 * Author : Mateo Markovic
 */ 

#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

int OF = 0; // Global Variable to track overflow
int pressed = 0; // Global Variable to track button pressed state

int main()
{
	TCCR0A = 0; // Normal Operation
	TCCR0B = 5; // set the pre-scaler to 1024 and start timer
	TCNT0=217; // start the count at 217
	TIMSK0 = (1<<TOIE0); // Enable the timer interrupt
	sei(); // Enable global interrupts
	
	DDRB |= (1<<3); // set PORTB.3 as an output
	PORTB |= (1<<3); // make sure pinb.3 starts at 1

	DDRB |= (1<<2); // set PORTB.2 as an output
	PORTB &= ~(1<<2); // make sure pinb.2 starts at 0
	
	DDRC &= ~(1<<3); // set PORTC.3 as an input
	PORTC |= (1<<3); //Set PINC.3 active high (pull-up)
	
    while (1) {
		// if PINC.3 is now LOW
		if (!(PINC & (1<<3)))
			pressed = 1; // set the button to pressed state
    }
}

// Overflow interrupt function
ISR (TIMER0_OVF_vect){
	OF++; // increment the overflow each time
	TCNT0=217; // make sure to set the counter back to 217
	
	if (pressed == 1) {
		OF = 0; // reset the overflow
		PORTB &= ~(1<<3); // turn off portb.3
		PORTB |= (1<<2); // turn on portb.2
		pressed = 2; // move to next state
		return; // exit the interrupt
	}
	if (pressed == 2) {
		if (OF == 800){
			PORTB &= ~(1<<2); // turn off portb.2
			PORTB |= (1<<3); // turn on portb.3
			OF = 0; // rest the overflow
			pressed = 0; // reset the button state
		}
		return; // exit the interrupt
	}
	
	if(OF == 165) // delay until we overflew 165 times
		PORTB &= ~(1<<3); // turn off portb.3
	if (OF == 300){ // delay until we overflew another 135 times (300 total)
		PORTB |= (1<<3); // turn on portb.3
		OF = 0; // reset the overflow
	}
}

