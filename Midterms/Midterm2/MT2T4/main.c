/*
 * MT2T4.c
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
void calcRPM4x();
void printRPM();

volatile float potValue = 0; // global variable to hold potentiometer value
char modeA = FALLING; // global variable to track fall/rise edge
char modeB = FALLING; // global variable to track fall/rise edge

volatile uint32_t revTickARise = 0; // global variable to count A rise revTicks
volatile uint32_t OFARise = 0;  // global variable to track A rise overflows for small rotations
volatile uint32_t periodARise = 0; // global variable to store A rise period

volatile uint32_t revTickAFall = 0; // global variable to count A fall revTicks
volatile uint32_t OFAFall = 0;  // global variable to track A fall overflows for small rotations
volatile uint32_t periodAFall = 0; // global variable to store A fall period

volatile uint32_t revTickBRise = 0; // global variable to count B rise revTicks
volatile uint32_t OFBRise = 0;  // global variable to track B rise overflows for small rotations
volatile uint32_t periodBRise = 0; // global variable to store B rise period

volatile uint32_t revTickBFall = 0; // global variable to count B fall revTicks
volatile uint32_t OFBFall = 0;  // global variable to track B fall overflows for small rotations
volatile uint32_t periodBFall = 0; // global variable to store B fall period

volatile double avgRPM = 0; // global variable to store average RPM in 4x mode

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
		calcRPM4x(); // calculate the rpm
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
	// setup PB0 as an active high input
	DDRE &= ~(1 << DDE2);
	PORTE |= (1 << DDE2);
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
	
	TCNT3 = 0; // initial timer value
	// capture on rising edge
	TCCR3A = 0;
	TCCR3B = (0 << ICNC3) | (1 << ICES3);
	TCCR3C = 0;
	// setup interrupt
	TIFR3 = (1 << ICF3) | (1 << TOV3);
	TIMSK3 = (1 << ICIE3) | (1 << TOIE3);
	// start timer with no pre-scaler
	TCCR3B |= (1 << CS30);
	
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

// function to calculate rpm in 2x mode
void calcRPM4x(){
	volatile double rpm = 0; // temp rpm variable
	
	for (int i = 0; i < 20; i++){
		// take the average period of both rising and falling edges
		rpm = ((double)periodAFall + (double)periodARise + (double)periodBFall + (double)periodBRise)/4;
		rpm = rpm/16000000; // convert period to seconds
		rpm = rpm * 8; // Multiply by 8 because there is 8 rising edges in 1 revolution
		rpm = rpm * 120; // multiply 120 to get period of shaft
		rpm = 1/rpm; // get the frequency of the shaft (rps)
		rpm = rpm * 60; // convert to rpm
		avgRPM += rpm; // add to rpm total
	}
	avgRPM = avgRPM/20; // take the average of the rpm
	// for super low RPMs, set the RPM to zero
	if (avgRPM < 2.5)
		avgRPM = 0;
}

// function to send the RPM to the terminal
void printRPM(){
	UART_tx_string("4x Mode RPM: ");
	char rpmChar4x[50];
	sprintf(rpmChar4x, "%f", avgRPM);
	UART_tx_string(rpmChar4x);
	UART_tx_string("\n\n");
}

// ISR for capture A
ISR(TIMER1_CAPT_vect){
	if (modeA == RISING){
		revTickARise = ICR1; // store time of last revolution
		periodARise = (revTickARise + (OFARise * 65535)); // period of rising edge of the motor encoder A waveform in cycles
		OFARise = 0; // reset overflows
		TCNT1 = 0; // reset timer for the next revolution
		// switch to falling edge capture mode
		TCCR1B |= (0 << ICES1);
		modeA = FALLING;
	} else {
		revTickAFall = ICR1; // store time of last revolution
		periodAFall = (revTickAFall + (OFAFall * 65535)); // period of falling edge of the motor encoder A waveform in cycles
		OFAFall = 0; // reset overflows
		TCNT1 = 0; // reset timer for the next revolution
		// switch to rising edge capture mode
		TCCR1B |= (1 << ICES1);
		modeA = RISING;
	}
}

// ISR for overflow A
ISR(TIMER1_OVF_vect){
	if (modeA == RISING) {
		OFARise++; // Increments overflows in 1x mode
	} else {
		OFAFall++; // Increments overflows in 2x mode
	}
}

// ISR for capture B
ISR(TIMER3_CAPT_vect){
	if (modeB == RISING){
		revTickBRise = ICR3; // store time of last revolution
		periodBRise = (revTickBRise + (OFBRise * 65535)); // period of rising edge of the motor encoder B waveform in cycles
		OFBRise = 0; // reset overflows
		TCNT3 = 0; // reset timer for the next revolution
		// switch to falling edge capture mode
		TCCR3B |= (0 << ICES3);
		modeB = FALLING;
		} else {
		revTickBFall = ICR3; // store time of last revolution
		periodBFall = (revTickBFall + (OFBFall * 65535)); // period of falling edge of the motor encoder B waveform in cycles
		OFBFall = 0; // reset overflows
		TCNT3 = 0; // reset timer for the next revolution
		// switch to rising edge capture mode
		TCCR3B |= (1 << ICES3);
		modeB = RISING;
	}
}

// ISR for overflow B
ISR(TIMER3_OVF_vect){
	if (modeB == RISING) {
		OFBRise++; // Increments overflows in 1x mode
		} else {
		OFBFall++; // Increments overflows in 2x mode
	}
}

