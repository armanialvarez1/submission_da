/*
 * MT2T1.c
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
void PWMSetup();

int main(void){
	pinSetup(); // initialize pins
	PWMSetup();// initialize the PWM
	
	OCR0A = 255; //Set motor speed to 50% (50% DC)
	
	while (1){
		// run motor for 5 secs CW
		PORTD |= (1<<4); // AIN1 High
		PORTD &= ~(1<<5); // AIN2 Low
		_delay_ms(5000);
		
		// stop motor for 1 sec
		PORTD &= ~(1<<4); // AIN1 Low
		_delay_ms(1000);
		
		// run motor for 5 secs CCW
		PORTD |= (1<<5); // AIN2 High
		_delay_ms(5000);
		
		// stop motor for 1 sec
		PORTD &= ~(1<<5); // AIN2 Low
		_delay_ms(1000);
	}
}

// function to setup pins
void pinSetup(){
	DDRD |= (1<<6); // make portd.6 an output (motor PWM)
	DDRD |= (1<<5); // make portd.5 an output (AIN2)
	DDRD |= (1<<4); // make portd.4 an output (AIN1)
}

// function to setup the PWM mode
void PWMSetup(){
	TCCR0A |= (1<<COM0A1)|(0<<COM0A0); // non-inverted mode
	TCCR0A |= (1<<WGM02)|(1<<WGM01)|(1<<WGM00); // fast PWM
	TCCR0B |= (1<<CS00); // start timer with no pre-scaler
}

