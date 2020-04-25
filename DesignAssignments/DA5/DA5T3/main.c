/*
 * DA3BT3.c
 *
 * 
 * Author : Mateo Markovic
 */ 

// clock speeds/baud rates
#define F_CPU 16000000UL
#define BAUD 9600
// ds18b20 Connections
#define DS18B20_PORT PORTC
#define DS18B20_DDR DDRC
#define DS18B20_PIN PINC
#define DS18B20_DQ PC0
// ds18b20 Commands
#define DS18B20_CMD_CONVERTTEMP 0x44
#define DS18B20_CMD_RSCRATCHPAD 0xbe
#define DS18B20_CMD_WSCRATCHPAD 0x4e
#define DS18B20_CMD_CPYSCRATCHPAD 0x48
#define DS18B20_CMD_RECEEPROM 0xb8
#define DS18B20_CMD_RPWRSUPPLY 0xb4
#define DS18B20_CMD_SEARCHROM 0xf0
#define DS18B20_CMD_READROM 0x33
#define DS18B20_CMD_MATCHROM 0x55
#define DS18B20_CMD_SKIPROM 0xcc
#define DS18B20_CMD_ALARMSEARCH 0xec
#define DS18B20_STOPINTERRUPTONREAD 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <util/setbaud.h>

void UART_Setup(); // UART_Setup function prototype
void UART_tx_string(char *data); // UART_tx_string function prototype

// ds18b20 functions
uint8_t ds18b20_reset(); // function to reset ds18b20
void ds18b20_writebit(uint8_t bit); // function to write bit to ds18b20
uint8_t ds18b20_readbit(void); // function to read bit from ds18b20
void ds18b20_writebyte(uint8_t byte); // function to write byte to ds18b20
uint8_t ds18b20_readbyte(void); // function to read byte from ds18b20
double ds18b20_gettemp(); // function to get the temperature from ds18b20

int main(void) {
	UART_Setup(); // setup UART
	char tempChar[100]; // buffer for temperature variable
	double temp = 0; // variable to hold the temperature
	sei(); // enable global interrupts
	while (1){
		temp = ds18b20_gettemp(); // get the temperature using the ds18b20 library
		snprintf(tempChar,sizeof(tempChar),"%f\r\n", temp); // convert temp to a char * (tempChar)
		UART_tx_string("Temperature: "); // print Temperature: to the terminal
		UART_tx_string(tempChar); // print the temperature to the terminal
		UART_tx_string("\r\n"); // end the line
		_delay_ms(500); // wait half a second
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

// ds18b20 library functions

//ds18b20 initialize function
uint8_t ds18b20_reset() {
	uint8_t i;

	//low for 480us
	DS18B20_PORT &= ~ (1<<DS18B20_DQ); //low
	DS18B20_DDR |= (1<<DS18B20_DQ); //output
	_delay_us(480);

	//release line and wait for 60uS
	DS18B20_DDR &= ~(1<<DS18B20_DQ); //input
	_delay_us(60);

	//get value and wait 420us
	i = (DS18B20_PIN & (1<<DS18B20_DQ));
	_delay_us(420);

	//return the read value, 0=ok, 1=error
	return i;
}

// ds18b20 write one bit function
void ds18b20_writebit(uint8_t bit){
	//low for 1uS
	DS18B20_PORT &= ~ (1<<DS18B20_DQ); //low
	DS18B20_DDR |= (1<<DS18B20_DQ); //output
	_delay_us(1);

	//if we want to write 1, release the line (if not will keep low)
	if(bit)
		DS18B20_DDR &= ~(1<<DS18B20_DQ); //input

	//wait 60uS and release the line
	_delay_us(60);
	DS18B20_DDR &= ~(1<<DS18B20_DQ); //input
}

// ds18b20 read one bit function
uint8_t ds18b20_readbit(void){
	uint8_t bit=0;

	//low for 1uS
	DS18B20_PORT &= ~ (1<<DS18B20_DQ); //low
	DS18B20_DDR |= (1<<DS18B20_DQ); //output
	_delay_us(1);

	//release line and wait for 14uS
	DS18B20_DDR &= ~(1<<DS18B20_DQ); //input
	_delay_us(14);

	//read the value
	if(DS18B20_PIN & (1<<DS18B20_DQ))
		bit=1;

	//wait 45uS and return read value
	_delay_us(45);
	return bit;
}

// ds18b20 write one byte function
void ds18b20_writebyte(uint8_t byte){
	uint8_t i=8;
	while(i--){
		ds18b20_writebit(byte&1);
		byte >>= 1;
	}
}

// ds18b20 read one byte function
uint8_t ds18b20_readbyte(void){
	uint8_t i=8, n=0;
	while(i--){
		n >>= 1;
		n |= (ds18b20_readbit()<<7);
	}
	return n;
}

// ds18b20 get temperature function
double ds18b20_gettemp(){
	uint8_t temperature_l;
	uint8_t temperature_h;
	double retd = 0;

	#if DS18B20_STOPINTERRUPTONREAD == 1
	cli();
	#endif

	ds18b20_reset(); //reset
	ds18b20_writebyte(DS18B20_CMD_SKIPROM); //skip ROM
	ds18b20_writebyte(DS18B20_CMD_CONVERTTEMP); //start temperature conversion

	while(!ds18b20_readbit()); //wait until conversion is complete

	ds18b20_reset(); //reset
	ds18b20_writebyte(DS18B20_CMD_SKIPROM); //skip ROM
	ds18b20_writebyte(DS18B20_CMD_RSCRATCHPAD); //read scratchpad

	//read 2 byte from scratchpad
	temperature_l = ds18b20_readbyte();
	temperature_h = ds18b20_readbyte();

	#if DS18B20_STOPINTERRUPTONREAD == 1
	sei();
	#endif

	//convert the 12 bit value obtained
	retd = ( ( temperature_h << 8 ) + temperature_l ) * 0.0625;

	return retd;
}

