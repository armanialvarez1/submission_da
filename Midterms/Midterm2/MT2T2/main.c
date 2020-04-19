/*
 * MT2T2.c
 *
 * 
 * Author : Mateo Markovic
 */ 

#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <avr/interrupt.h>

void pinSetup();
void adcSetup();
void PWMSetup();
void readADC();

volatile float potValue = 0; // global variable to hold potentiometer value

int main(void){
	pinSetup(); // initialize pins
	adcSetup(); // initialize the ADC
	PWMSetup();// initialize the PWM

	while (1){
		readADC(); // call Read function
		_delay_ms(100); // delay after reading
		// make sure anything above 95% of the potentiometer sets the motor at 95% speed
		if ((potValue >= 121) && (potValue < 128)){
			OCR0A = 243; // set the PWM to 95% of max
			_delay_ms(50); // wait a little bit
		}
		// scale the RPM with the potentiometer value
		else if ((potValue < 121) && (potValue >= 1)){
			OCR0A = potValue*2;
			_delay_ms(50); // wait a little bit
		}
		// for minimum values of the potentiometer set the RPM to 0
		else {
			OCR0A = 0;
			_delay_ms(50); // wait a little bit
		}
	}
}

// function to setup pins
void pinSetup(){
	DDRC &= ~(1<<0); // make portc.0 an input (potentiometer)
	PORTC |= (1<<0); // make portc.0 active high
	DDRD |= (1<<6); // make portd.6 an output (motor PWM)
	DDRD |= (1<<5); // make portd.5 an output (AIN2)
	DDRD |= (1<<4); // make portd.4 an output (AIN1)
	// CW Motor Direction
	PORTD |= (1<<4);
	PORTD &= ~(1<<5);
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

// function to setup the PWM mode
void PWMSetup(){
	TCCR0A |= (1<<COM0A1)|(0<<COM0A0); // non-inverted mode
	TCCR0A |= (1<<WGM02)|(1<<WGM01)|(1<<WGM00); // fast PWM
	TCCR0B |= (1<<CS00); // start timer with no pre-scaler
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
	potValue = potValue/512; // scale potValue from 0-128
}