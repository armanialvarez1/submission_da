/*
 * MT1.c
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
#include <util/delay.h>

void UART_Setup(); // function to setup UART
void UART_tx_string(char *data); // function to transmit a string
unsigned char UART_rx(); //function to receive a char via UART
void readTemp(); // function to read temperature from LM35 on PC0
void displayMenu(); // function to display the help menu

void displayTempC(); // function to display temp in Celsius
void displayTempF(); // function to display temp in Fahrenheit
void ledOn(); // function to turn LED on PB5 on
void ledOff(); // function to turn LED on PB5 off
void sendString(); // function to send string to terminal
void sendInt(); // function to send string to terminal and blink led with that delay


float temp = 0; // Global Variable for temperature value
float Ftemp = 0; // Global Variable for Fahrenheit temperature value
volatile unsigned char rx_data[8]; // char array to hold rx data
volatile unsigned int count = 0; // variable to hold data position for rx
volatile unsigned char data_recieved = 0; // variable to hold whether or not all the data has been received
volatile unsigned char blink = 0; // variable to determine whether or not to keep blinking the LED
int delay = 1; // delay for LED blink


int main(void){
	DDRB |= (1<<5); // set PORTB.5 as an output
	PORTB |= (1<<5); // make sure the LED starts off
	DDRB |= (1<<2); // set PORTB.2 as an output
	PORTB |= (1<<2); // make sure the LED starts off
	
    UART_Setup(); // run UART setup
    	
    // ADC setup
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
	
	UCSR0B |= (1 << RXCIE0 ); // Enable the USART Receive interrupt
	sei(); // Globally enable interrupts
	
	displayMenu(); // display the help menu
	
    while (1) {
		// if we got data
		if (data_recieved == 1){
			// depending on the command, call the specific function
			switch(rx_data[0]){
				case 'h': displayMenu();
						break;
				case 't': displayTempC();
						break;
				case 'T': displayTempF();
						break;
				case 'o': ledOn();
						break;
				case 'O': ledOff();
						break;
				case 's': sendString();
						break;
				case 'i': sendInt();
						break;
		}	
		data_recieved = 0; // reset data-received
		}
    }
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

//function to display the help menu via serial
void displayMenu(){
	// transmit the help menu
	UART_tx_string("\n\n----------Help Menu----------");
	UART_tx_string("\nt - display temperature in C");
	UART_tx_string("\nT - display temperature in F");
	UART_tx_string("\no - turn on LED (PB5)");
	UART_tx_string("\nO - turn off LED (PB5)");
	UART_tx_string("\ns - sends a string to the terminal");
	UART_tx_string("\ni - sends a random integer to the terminal and blinks LED (PB2) with that delay in seconds");
}

// function to display temp in Celsius
void displayTempC(){
	readTemp(); // call readTemp function to obtain temperature
	
	char tempChar [30]; // char array from temp float value
	snprintf(tempChar,sizeof(tempChar),"%f\r\n", temp); // convert temp to a char * (tempChar)
	
	UART_tx_string("\n\nTemperature in C: ");
	UART_tx_string(tempChar); // output the Centigrade temp
}

// function to display temp in Fahrenheit
void displayTempF(){
	readTemp(); // call readTemp function to obtain temperature
	
	char fTempChar [30]; // char array from Fahrenheit temp float value
	snprintf(fTempChar,sizeof(fTempChar),"%f\r\n", Ftemp); // convert temp to a char * (tempChar)
	
	UART_tx_string("\n\nTemperature in F: ");
	UART_tx_string(fTempChar); // output the Fahrenheit temp
}

// function to turn LED on PB5 on
void ledOn(){
	PORTB &= ~(1<<5); // turn on portb.5
	UART_tx_string("\n\nLED on PB5 turned on");
}

// function to turn LED on PB5 off
void ledOff(){
	PORTB |= (1<<5); // turn off portb.5
	UART_tx_string("\n\nLED on PB5 turned off");
}

// function to send string to terminal
void sendString(){
	char* str = "Hello CPE 301"; // the string to send
	UART_tx_string("\n\n"); // send a blank line
	UART_tx_string(str); // send the string
}

// function to send string to terminal and blink LED on PB2 with that delay
void sendInt(){
	char charX[30]; // buffer for the int
	delay = rand() % 2000; // set x to a random int
	blink = 1; // enable blinking
	
	itoa(delay, charX, 10); // convert x to a char * (charX)
	UART_tx_string("\n\n");
	UART_tx_string(charX); // transmit charX
	
	UART_tx_string("\n\nLED on PB2 is blinking with a ");
	UART_tx_string(charX); // transmit charX
	UART_tx_string(" millisecond delay");
	
	while (blink == 1){
		PORTB &= ~(1<<2); // turn on portb.2
		for (int i = 0; i < delay; i++){
			_delay_ms(1); // delay based on command i
			i++;
		}
		PORTB |= (1<<2); // turn off portb.2
		for (int i = 0; i < delay; i++){
			_delay_ms(1); // delay based on command i
			i++;
		}
	}
}

// interrupt for RX
ISR(USART0_RX_vect){
	// store the received data
	rx_data[count] = UDR0;
	// stop blinking if not selected
	if (rx_data[count] != 'i'){
		blink = 0;
	}
	// if we hit the end of the line then reset the count and mark that the data has been received
	if (rx_data[count] == '\n'){
		data_recieved = 1;
		count = 0;
		} else {
		// otherwise increment the data count
		count++;
	}
}