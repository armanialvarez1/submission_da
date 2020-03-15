/*
 * DA3BT1.c
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


void UART_Setup(); // UART_Setup function prototype
void UART_tx_string(char *data); // UART_tx_string function prototype
void readTemp(); // function to read temperature from LM35 on PC0

int OF = 0; // Global Variable to track overflow

float temp = 0; // Global Variable for temperature value
float Ftemp = 0; // Global Variable for Fahrenheit temperature value

int main(void) {
	// timer setup
    TCCR1A = 0; // Normal Operation
    TCCR1B = 4; // set the pre-scaler to 256 and start timer
    TCNT1=59285; // start the count at 59285
    TIMSK1 = (1<<TOIE1); // Enable the timer interrupt
    sei(); // Enable global interrupts
	
	UART_Setup(); // run UART setup
	
	// ADC setup
	DDRC = 0; // Make port c an input
	PORTC = 0;
	
	ADMUX |= (1<<REFS0); // 5v Voltage ref
	// select ADC0 (PC5)
	ADMUX |= (0<<MUX3);
	ADMUX |= (1<<MUX2);
	ADMUX |= (0<<MUX1);
	ADMUX |= (1<<MUX0);
	
	ADCSRA |= (1<<ADEN); // enable the ADC
	ADCSRA |= (0<<ADSC); // don't do a conversion yet
	// set pre-scaler as 128
	ADCSRA |= (1<<ADPS2);
	ADCSRA |= (1<<ADPS1);
	ADCSRA |= (1<<ADPS0);
	
    while (1);
}

// function to setup UART and Baud rate
void UART_Setup(){
	UBRR0H = UBRRH_VALUE;
	UBRR0L = UBRRL_VALUE;
	UCSR0C = _BV(UCSZ01) | _BV(UCSZ00); // 8-bit data
	UCSR0B = _BV(TXEN0) | _BV(RXEN0); // Enable TX and RX
}

// function to send data to serial port
void UART_tx_string(char *data){
	while ((*data != '\0')){
		while (!(UCSR0A & (1<<UDRE0)));
		UDR0 = *data;
		data++;
	}
}

void readTemp(){
	temp = 0; // reset the temperature
	ADCSRA |= (1<<ADSC); // start the conversion
	while(ADCSRA & (1<<ADSC)); // wait for conversion to finish
	temp = ADC; // set the value from the temperature sensor to the temp variable
	temp = (temp/1024) * 500; // convert adc value to Centigrade
	Ftemp = (temp * 9/5) + 32; // convert C to F temp
}

// OF increments once every 0.1 seconds
ISR (TIMER1_OVF_vect){
	OF++; // increment overflow
	TCNT1 = 59285; // make sure to set the counter back to 59285
	
	char tempChar [30]; // char array from temp float value
	char fTempChar [30]; // char array from Fahrenheit temp float value
	
	if (OF == 5){ // every 5 seconds (5 * 0.1)
		
		readTemp(); // get the temperature
		UART_tx_string("\nCentigrade: ");
		snprintf(tempChar,sizeof(tempChar),"%f\r\n", temp); // convert temp to a char * (tempChar)
		UART_tx_string(tempChar); // output the Centigrade temp
		
		snprintf(fTempChar,sizeof(fTempChar),"%f\r\n", Ftemp); // convert temp to a char * (tempChar)
		UART_tx_string("Fahrenheit: ");
		UART_tx_string(fTempChar); // output the Fahrenheit temp
		UART_tx_string("\n");
		
		OF = 0; // reset the overflow
	}
}