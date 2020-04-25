/*
 * DA5T1.c
 *
 * 
 * Author : Mateo Markovic
 */ 

#define F_CPU 16000000UL
#define BAUD 9600
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <util/setbaud.h>
#include <stdio.h>
#include <stdlib.h>

#define LATCH 4 /* PD4 RCK */
#define CLOCK 7 /* PD7 SRCK */
#define DATA 0 /* PB0 SER IN */
#define LSBFIRST 0
#define MSBFIRST 1

/* Segment byte maps for numbers 0 to 9 */
const uint8_t SEGMENT_MAP[] = {0xC0,0xF9,0xA4,0xB0,0x99,0x92,0x82,0xF8,0X80,0X90};
	
/* Byte maps to select digit 1 to 4 */
const uint8_t SEGMENT_SELECT[] = {0xF1,0xF2,0xF4,0xF8};

void ADC_Setup(); // ADC_Setup function prototype
void readTemp(); // function to read temperature from LM35 on PC0
void shift_out_init(void); // function to setup shift out
void shift_out(uint8_t indata); // function to shift out data
void display(int num); // function to display provided int on seven segment displays

float temp = 0; // Global Variable for temperature value

int main(void){
	ADC_Setup(); // run ADC setup
	shift_out_init(); // run shift-out setup
	
	while (1){
		readTemp(); // read the temperature in Centigrade
		display((int)temp); // display the temperature on the 7-segment displays
	}
}

// function to setup ADC
void ADC_Setup(){
	DDRC = 0; // Make port c an input
	PORTC = 0;
		
	ADMUX |= (1<<REFS0); // 5v Voltage ref
    // select ADC4 (PC4)
    ADMUX |= (0<<MUX3);
    ADMUX |= (1<<MUX2);
    ADMUX |= (0<<MUX1);
    ADMUX |= (0<<MUX0);
		
	ADCSRA |= (1<<ADEN); // enable the ADC
	ADCSRA |= (0<<ADSC); // don't do a conversion yet
	// set pre-scaler as 128
	ADCSRA |= (1<<ADPS2);
	ADCSRA |= (1<<ADPS1);
	ADCSRA |= (1<<ADPS0);
}

// function to get temp from LM34 ADC value
void readTemp(){
	temp = 0; // reset the temperature
	ADCSRA |= (1<<ADSC); // start the conversion
	while(ADCSRA & (1<<ADSC)); // wait for conversion to finish
	temp = ADC; // set the value from the temperature sensor to the temp variable
	temp = (temp/1024) * 500; // convert adc value to Centigrade
}

// function to setup shift out
void shift_out_init(void){
	DDRB|=(1 << DATA); // PB0 = DATA
	DDRD|=(1 << CLOCK) | (1 << LATCH); //PD7 = SCLK & PD4 = LATCH
}

// function to shift out data to the shift registers
void shift_out(uint8_t indata){
	for (uint8_t i = 0; i < 8; i++){
		/* Write bit to data port. */
		if (0 == (indata & _BV(7 - i))){
			// digital_write(SHIFT_OUT_DATA, LOW);
			PORTB &= (0 << DATA);
		} else {
			// digital_write(SHIFT_OUT_DATA, HIGH);
			PORTB |= (1 << DATA);
		}
		
		/* Pulse clock to write next bit. */
		PORTD |= (1 << CLOCK);
		PORTD &= (0 << CLOCK);
	}
}

// function to display provided int on seven segment display
void display(int num){
	int hundreds = (num/100)%10; // isolate the hundreds place
	int tens = (num/10)%10; // isolate the tens place
	int ones = (num%10); // isolate the ones place
	// display each number on the proper seven-segment display
	for (int i = 1; i < 4; i++){
		if (i == 1 && num >= 100){
			PORTD &= (0<<LATCH);
			shift_out(SEGMENT_MAP[hundreds]);
			shift_out(SEGMENT_SELECT[1]);
			PORTD |= (1<<LATCH);
		}
		if (i == 2 && num >= 10){	
			PORTD &= (0<<LATCH);
			shift_out(SEGMENT_MAP[tens]);
			shift_out(SEGMENT_SELECT[2]);
			PORTD |= (1<<LATCH);
		}
		if (i == 3){
			PORTD &= (0<<LATCH);
			shift_out(SEGMENT_MAP[ones]);
			shift_out(SEGMENT_SELECT[3]);
			PORTD |= (1<<LATCH);
		}
	}
}

