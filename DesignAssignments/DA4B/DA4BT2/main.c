/*
 * DA4BT2.c
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
void adcSetup();
void pwmSetup();
void readADC();

// global variables
volatile float potValue = 0; // global variable to hold potentiometer value

int main(void){
	// run adc and pwm setups
	adcSetup();
	pwmSetup();
	
    while (1){
		// get the potentiometer value
		readADC();
		// if we are near the max potentiometer value
		if (potValue > 563){
			OCR1A=563; // set the servo position to about 180 degrees
			} else if (potValue > 125){
			OCR1A = potValue; // set the servo position based on potentiometer value
			} else {
			OCR1A = 125; // set the servo position to about 0 degrees
		}
    }
}

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
// function to setup the PWM timer
void pwmSetup(){
	// setup Timer 1 for non-inverted PWM
	TCCR1A|=(1<<COM1A1)|(1<<COM1B1)|(1<<WGM11);
	// set the pre-scaler to 64 with fast PWM mode
	TCCR1B|=(1<<WGM13)|(1<<WGM12)|(1<<CS11)|(1<<CS10);
	// set the PWM frequency to  50Hz
	ICR1=4999;
	// set PB1 as the PWM output
	DDRB |= (1<<1);
}
// function to read ADC value from the potentiometer
void readADC(){
	int samples = 15; // number of samples
	potValue = 0; // initial potValue
	for (int i = 0; i < samples; i++){
		ADCSRA |= (1<<ADSC); // start the ADC conversion
		while(ADCSRA & (1<<ADSC)); // wait until the conversion is done
		potValue += ADC; // store the value from the conversion
	}
	potValue = potValue/15; // take the average value
	potValue = (potValue/104) + 125; // scale the range to 125-567
}


