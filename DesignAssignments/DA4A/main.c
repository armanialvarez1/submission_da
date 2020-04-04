/*
 * DA4AT1.c
 *
 * 
 * Author : Mateo Markovic
 */ 

#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <avr/interrupt.h>

void adcSetup();
void intSetup();
void PWMSetup();
void readADC();

volatile float potValue = 0; // global variable to hold potentiometer value

int main(void){
	DDRC &= ~(1<<0); // make portc.0 an input (potentiometer)
	DDRC |= (1<<3); // make portc.3 an output (motor STBY)
	DDRC &= ~(1<<1); // make portc.1 as an input (button)
	DDRB |= (1<<1); // make portb.1 an output (motor PWM)
	PORTC |= (1<<3); // set portc.3 high
	PORTC |= (1<<0); // make portc.0 active high
	PORTC |= (1<<1); // make portc.1 active high
	adcSetup(); // initialize the ADC
	intSetup(); // initialize the interrupt
	PWMSetup();// initialize the PWM

    while (1){
		readADC(); // call Read function
		_delay_ms(100); // delay after reading
		// make sure anything above 95% of the potentiometer sets the motor at 95% speed
		if ((potValue >= 62260) && (potValue < 65535)){
			OCR1A = 62260; // set the PWM to 95% of max
			_delay_ms(50); // wait a little bit
		}
		// scale the RPM with the potentiometer value
		else if ((potValue < 62257) && (potValue >= 3000)){
			OCR1A = potValue;
			_delay_ms(50); // wait a little bit
		}
		// for minimum values of the potentiometer set the RPM to 0
		else {
			OCR1A = 0;
		}
    }
}

// function to setup ADC
void adcSetup(){
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

// function to setup the pin change interrupt
void intSetup(){
	PCICR = (1<<PCIE1); // enable pin change interrupt 1
	PCMSK1 = (1<<PCINT9); // Mask for PortC.1
	sei(); // enable global interrupts
}

// function to setup the PWM mode
void PWMSetup(){
	ICR1 = 0XFFFF; // ICR1 as top
	TCCR1A |= (1<<COM1A1)|(1<<COM1B1); // non-inverted mode
	TCCR1A |= (1<<WGM11); // fast PWM
	TCCR1B |= (1<<WGM12)|(1<<WGM13); // fast PWM
	TCCR1B |= (1<<CS10); // start timer
}

// function to readADC value
void readADC(){
	int samples = 15; // number of samples
	potValue = 0; // initial potValue
	for (int i = 0; i < samples; i++){
		ADCSRA |= (1<<ADSC); // start the ADC conversion
		while(ADCSRA & (1<<ADSC)); // wait until the conversion is done
		potValue += ADC; // store the value from the conversion
	}
	potValue = potValue/15; // take the average value
}

// pin change ISR
ISR(PCINT1_vect){
	// only on the falling edge
	if( (PINC & (1<< PINC1)) == 0 ){
		// toggle the motor (STBY pin)
		PORTC ^= (1<<3);
	}
}