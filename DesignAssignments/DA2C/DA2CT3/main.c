/*
 * DA2CT3.c
 *
 * 
 * Author : Mateo Markovic
 */ 

#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

int Compares = 0; // Global Variable to track compares
int pressed = 0; // Global Variable to track button pressed state

int main()
{
	OCR0A = 39; // Set compare register to 39
	TCCR0A = 2; // Enable CTC Mode
	TCCR0B = 5; // set the pre-scaler to 1024 and start timer
	TIMSK0 = (1<<OCIE0A); // Enable the timer interrupt
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

// CTC Interrupt Function
ISR (TIMER0_COMPA_vect){
	Compares++; // increment the compare count each time
	
	if (pressed == 1) {
		Compares = 0; // reset the compare count
		PORTB &= ~(1<<3); // turn off portb.3
		PORTB |= (1<<2); // turn on portb.2
		pressed = 2; // move to next state
		return; // exit the interrupt
	}
	if (pressed == 2) {
		if (Compares == 800){
			PORTB &= ~(1<<2); // turn off portb.2
			PORTB |= (1<<3); // turn on portb.3
			Compares = 0; // reset the compare count
			pressed = 0; // reset the button state
		}
		return; // exit the interrupt
	}
	
	if(Compares == 165) // delay until we compare 165 times
		PORTB &= ~(1<<3); // turn off portb.3
	if (Compares == 300){ // delay until we compare another 135 times (300 total)
		PORTB |= (1<<3); // turn on portb.3
		Compares = 0; // reset the compare count
	}
}

