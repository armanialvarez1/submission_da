/*
 * DA4BT1.c
 *
 * 
 * Author : Mateo Markovic
 */ 

#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <avr/interrupt.h>

// function prototypes
void pinSetup();
void ctcSetup();
void adcSetup();
void readADC();
void rotate();

// global variables
volatile float potValue = 0; // global variable to hold potentiometer value
volatile int step = 0; // global variable to track steps
volatile int compares = 0; // global variable to track number of compares
volatile int delay = 0; // global variable to control delay

int main(void){
	// setup the pins
    pinSetup();
	// setup the CTC Timer
	ctcSetup();
	// setup the ADC
	adcSetup();
    while (1){
		// read the potentiometer value
		readADC();
		if (potValue > 100){
			delay = 1; // set max speed of stepper motor (2ms delay)
		} else if (potValue > 1){
			delay = (100 - potValue); // scale motor speed with potentiometer value
		} else {
			delay = 10000; // basically turn motor off at very low potentiometer values
		}
    }
}

// function the sets up the pins
void pinSetup(){
	// set PortB 1-4 as outputs
	DDRB |= (1<<1);
	DDRB |= (1<<2);
	DDRB |= (1<<3);
	DDRB |= (1<<4);
	// start PinB 1-4 at low
	PORTB &= ~(1<<1);
	PORTB &= ~(1<<2);
	PORTB &= ~(1<<3);
	PORTB &= ~(1<<4);
}
// function the sets up the CTC Timer
void ctcSetup(){
	OCR0A = 125; // Set compare register to 125
	TCCR0A = 2; // Enable CTC Mode
	TCCR0B = 4; // set the pre-scaler to 256 and start timer
	TIMSK0 = (1<<OCIE0A); // Enable the timer interrupt
	sei(); // Enable global interrupts
}
// function the sets up the ADC
void adcSetup(){
	// set PC0 as an input and active high (potentiometer)
	DDRC &= ~(1<<0);
	PORTC |= (1<<0);
	
	// use AVCC
	ADMUX |= (0<<REFS1);
	ADMUX |= (1<<REFS0);
		
	// select ADC0
	ADMUX |= (0<<MUX2);
	ADMUX |= (0<<MUX1);
	ADMUX |= (0<<MUX0);
		
	// left align
	ADMUX |= (1<<ADLAR);
		
	// enable ADC
	ADCSRA |= (1<<ADEN);
		
	// set pre-scaler to 128
	ADCSRA |= (1<<ADPS2);
	ADCSRA |= (1<<ADPS1);
	ADCSRA |= (1<<ADPS0);
}
// function the reads the potentiometer value using ADC
void readADC(){
	int samples = 15; // number of samples
	potValue = 0; // initial potValue
	for (int i = 0; i < samples; i++){
		ADCSRA |= (1<<ADSC); // start the ADC conversion
		while(ADCSRA & (1<<ADSC)); // wait until the conversion is done
		potValue += ADC; // store the value from the conversion
	}
	potValue = potValue/15; // take the average value
	potValue = potValue/600; // scale the value down by 600 (range of 0-110)
}
// function to rotate the motor
void rotate(){
	// alternate which input is high based on which step we are on
	switch(step){
		case 1: PORTB |= (1<<1);
				PORTB &= ~(1<<2);
				PORTB &= ~(1<<3);
				PORTB &= ~(1<<4);
				break;
				
		case 2:	PORTB &= ~(1<<1);
				PORTB |= (1<<2);
				PORTB &= ~(1<<3);
				PORTB &= ~(1<<4);
				break;
				
		case 3:	PORTB &= ~(1<<1);
				PORTB &= ~(1<<2);
				PORTB |= (1<<3);
				PORTB &= ~(1<<4);
				break;
				
		case 4:	PORTB &= ~(1<<1);
				PORTB &= ~(1<<2);
				PORTB &= ~(1<<3);
				PORTB |= (1<<4);
				break;
	}
	// if we reached stepped 4
	if (step == 4){
		// make the next step 1
		step = 1;
	} else {
		// increment the step for the next time
		step++;
	}
}

// CTC Interrupt Function
ISR (TIMER0_COMPA_vect){
	compares++; // increment compares
	if (compares >= delay){ // if we delayed long enough (based on potentiometer value)
		rotate(); // call the rotate function to turn a step
		compares = 0; // reset compares
	}
}

