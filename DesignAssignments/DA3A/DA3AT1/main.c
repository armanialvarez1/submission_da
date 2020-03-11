/*
 * DA3AT1.c
 *
 *
 * Author : Mateo Markovic
 */ 

#define F_CPU 16000000UL
#define BAUD 9600
#include <avr/io.h>
#include <util/delay.h>
#include <util/setbaud.h>
#include <stdio.h>
#include <stdlib.h>


void UART_Setup(); // UART_Setup function prototype
void UART_tx_string(char *data); // UART_tx_string function prototype

int main() {
	srand(8139); // seed the random number generator
	UART_Setup(); // run UART setup
	
	int x; // variable to hold the int
	float y; // variable to hold the float
	char charX[30]; // buffer for x
	char charY[30]; // bufer for y

    while (1){
		_delay_ms(1000); // wait 1s
		UART_tx_string("\nRandom int and float: "); // transmit the string
		x = rand(); // set x to a random int
		itoa(x, charX, 10); // convert x to a char * (charX)
		UART_tx_string(charX); // transmit charX
		UART_tx_string(", "); // transmit a comma
		
		y = (float)rand() / (float)rand(); // set y to a random float
		snprintf(charY,sizeof(charY),"%f\r\n", y); // convery y to a char * (charY)
		UART_tx_string(charY); // transmit charY
	}
}

// function to setup UART and Baud rate
void UART_Setup(){
	UBRR0H = UBRRH_VALUE;
	UBRR0L = UBRRL_VALUE;
	UCSR0C = _BV(UCSZ01) | _BV(UCSZ00); // 8-bit data
	UCSR0B = _BV(TXEN0); // Enable TX
}

// function to send data to serial port
void UART_tx_string(char *data){
	while ((*data != '\0')){
		while (!(UCSR0A & (1<<UDRE0)));
		UDR0 = *data;
		data++;
	}
}

