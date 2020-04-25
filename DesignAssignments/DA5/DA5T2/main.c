/*
 * DA5T2.c
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

#define SHIFT_REGISTER DDRB
#define SHIFT_PORT PORTB
#define DATA (1<<PB3) //MOSI (SI)
#define LATCH (1<<PB2) //SS (RCK)
#define CLOCK (1<<PB5) //SCK (SCK)

/* Segment byte maps for numbers 0 to 9 */
const uint8_t SEGMENT_MAP[] = {0xC0,0xF9,0xA4,0xB0,0x99,0x92,0x82,0xF8,0X80,0X90};

/* Byte maps to select digit 1 to 4 */
const uint8_t SEGMENT_SELECT[] = {0xF1,0xF2,0xF4,0xF8};

void spi_setup(); // spi_setup function prototype
void spi_shift_out(uint8_t data, uint8_t pos); // spi_shift_out function prototype
void display(int num); // function to display provided int on seven segment displays
void ADC_Setup(); // ADC_Setup function prototype
void readTemp(); // function to read temperature from LM35 on PC0

float temp = 0; // Global Variable for temperature value

int main(void){
	spi_setup(); // run SPI setup
	ADC_Setup(); // run ADC setup
	
    while (1){
		readTemp(); // read the temperature in Centigrade
		display((int)temp); // display the temperature on the 7-segment displays
    }
}

// function to setup SPI
void spi_setup(){
	SHIFT_REGISTER |= (DATA | LATCH | CLOCK); // Set control pins as outputs
	SHIFT_PORT &= ~(DATA | LATCH | CLOCK); // Set control pins low
	SPCR0 = (1<<SPE) | (1<<MSTR);  //Start SPI as Master
	SHIFT_PORT &= ~LATCH; // Pull LATCH low to start SPI transfer
}

// function to shift out data via SPI
void spi_shift_out(uint8_t data, uint8_t pos){
	SPDR0 = data; // Shift in the data to be shown on the seven-segment
	while(!(SPSR0 & (1<<SPIF))); // Wait for SPI process to finish
	
	SPDR0 = pos; // Shift in the position data for the seven-segment
	while(!(SPSR0 & (1<<SPIF))); //Wait for SPI process to finish
	
	// Toggle latch to copy data to the storage register
	SHIFT_PORT |= LATCH;
	SHIFT_PORT &= ~LATCH;
}

// function to display provided int on seven segment display
void display(int num){
	int hundreds = (num/100)%10; // isolate the hundreds place
	int tens = (num/10)%10; // isolate the tens place
	int ones = (num%10); // isolate the ones place
	
	// display each number on the proper seven-segment display
	for (int i = 1; i < 4; i++){
		if (i == 1 && num >= 100){
			spi_shift_out(SEGMENT_MAP[hundreds], SEGMENT_SELECT[1]);
		}
		if (i == 2 && num >= 10){
			spi_shift_out(SEGMENT_MAP[tens], SEGMENT_SELECT[2]);
		}
		if (i == 3){
			spi_shift_out(SEGMENT_MAP[ones], SEGMENT_SELECT[3]);
		}
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

