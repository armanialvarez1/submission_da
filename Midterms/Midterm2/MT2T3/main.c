/*
 * MT2T3.c
 *
 * 
 * Author : Mateo Markovic
 */ 

#define F_CPU 16000000UL
#define BAUD 9600
#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <stdlib.h>
#include <avr/interrupt.h>
#include <util/setbaud.h>

// definitions for R/F modes
#define RISING 0
#define FALLING 1

void pinSetup();
void adcSetup();
void PWMSetup();
void captureSetup();
void UART_Setup();
void UART_tx_string(char *data);
void readADC();
void calcRPM1x();
void calcRPM2x();
void printRPM();

volatile float potValue = 0; // global variable to hold potentiometer value
char mode = FALLING; // global variable to track fall/rise edge

volatile uint32_t revTick1x = 0; // global variable to count revTicks in 1x mode
volatile uint32_t OF1x = 0;  // global variable to track overflows for small rotations in 1x mode
volatile uint32_t period1x = 0; // global variable to store period in 1x mode
volatile double avgRPM1x = 0; // global variable to store RPM in 1x mode

volatile uint32_t revTick2x = 0; // global variable to count revTicks in 2x mode
volatile uint32_t OF2x = 0;  // global variable to track overflows for small rotations in 2x mode
volatile uint32_t period2x = 0; // global variable to store period in 2x mode
volatile double avgRPM2x = 0; // global variable to store RPM in 2x mode

int main(void){
	pinSetup(); // initialize pins
	adcSetup(); // initialize the ADC
	PWMSetup();// initialize the PWM
	captureSetup(); // setup capture
	UART_Setup(); // setup UART
	OCR0A = 128; // initial motor thrust
	
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
		
		
		calcRPM1x(); // calculate the rpm
		calcRPM2x(); // calculate the rpm
		printRPM(); // display the RPM on the terminal
		_delay_ms(1500); // wait 1.5 secs between readings
		
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
	// setup PB0 as an active high input
	DDRB &= ~(1 << DDB0);
	PORTB |= (1 << DDB0);
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

// function to setup 1x and 2x capture
void captureSetup(){
	TCNT1 = 0; // initial timer value
	// capture on rising edge
	TCCR1A = 0;
	TCCR1B = (0 << ICNC1) | (1 << ICES1);
	TCCR1C = 0;
	// setup interrupt
	TIFR1 = (1 << ICF1) | (1 << TOV1);
	TIMSK1 = (1 << ICIE1) | (1 << TOIE1);
	// start timer with no pre-scaler
	TCCR1B |= (1 << CS10);
	// enable global interrupts
	sei();
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

// function to calculate rpm in 1x mode
void calcRPM1x(){
	volatile double rpm = 0; // temp rpm variable
	
	for (int i = 0; i < 20; i++){
		rpm = ((double)period1x/16000000); // convert period to seconds
		rpm = rpm * 8; // Multiply by 8 because there is 8 rising edges in 1 revolution
		rpm = rpm * 120; // multiply 120 to get period of shaft
		rpm = 1/rpm; // get the frequency of the shaft (rps)
		rpm = rpm * 60; // convert to rpm
		avgRPM1x += rpm; // add to rpm total
	}
	avgRPM1x = avgRPM1x/20; // take the average of the rpm
	// for super low RPMs, set the RPM to zero
	if (avgRPM1x < 2.5)
		avgRPM1x = 0;
}

// function to calculate rpm in 2x mode
void calcRPM2x(){
	volatile double rpm = 0; // temp rpm variable
	
	for (int i = 0; i < 20; i++){
		rpm = (double)period2x / 2; // take the average period of rising and falling edges
		rpm = rpm/16000000; // convert period to seconds
		rpm = rpm * 8; // Multiply by 8 because there is 8 rising edges in 1 revolution
		rpm = rpm * 120; // multiply 120 to get period of shaft
		rpm = 1/rpm; // get the frequency of the shaft (rps)
		rpm = rpm * 60; // convert to rpm
		avgRPM2x += rpm; // add to rpm total
	}
	avgRPM2x = avgRPM2x/20; // take the average of the rpm
	// for super low RPMs, set the RPM to zero
	if (avgRPM2x < 2.5)
		avgRPM2x = 0;
}

// function to send the RPM to the terminal
void printRPM(){
	UART_tx_string("1x Mode RPM: ");
	char rpmChar1x[50];
	sprintf(rpmChar1x, "%f", avgRPM1x);
	UART_tx_string(rpmChar1x);
	UART_tx_string("\n");
	
	UART_tx_string("2x Mode RPM: ");
	char rpmChar2x[50];
	sprintf(rpmChar2x, "%f", avgRPM2x);
	UART_tx_string(rpmChar2x);
	UART_tx_string("\n\n");
}

// ISR for capture
ISR(TIMER1_CAPT_vect){
	if (mode == RISING){
		revTick1x = ICR1; // store time of last revolution
		period1x = (revTick1x + (OF1x * 65535)); // period of the motor encoder waveform in cycles
		OF1x = 0; // reset overflows
		TCNT1 = 0; // reset timer for the next revolution
		// switch to falling edge capture mode
		TCCR1B |= (0 << ICES1);
		mode = FALLING;
	} else {
		revTick2x = ICR1; // store time of last revolution
		// period of rising and falling edge of the motor encoder waveform in cycles
		period2x = (revTick2x + (OF2x * 65535) + period1x);
		OF2x = 0; // reset overflows
		TCNT1 = 0; // reset timer for the next revolution
		// switch to rising edge capture mode
		TCCR1B |= (1 << ICES1);
		mode = RISING;
	}
}

// ISR for overflow
ISR(TIMER1_OVF_vect){
	if (mode == RISING) {
		OF1x++; // Increments overflows in 1x mode
	} else {
		OF2x++; // Increments overflows in 2x mode
	}
}

