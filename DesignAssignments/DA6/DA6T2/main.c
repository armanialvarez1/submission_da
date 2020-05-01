/*
 * DA6T2.c
 *
 * 
 * Author : Mateo Markovic
 */ 

#define F_CPU 16000000UL // 16MHz CPU Clock
#define ACCELEROMETER_SENSITIVITY 16384.0 // accelerometer sensitivity
#define GYROSCOPE_SENSITIVITY 16.4 // gyroscope sensitivity
#define dt 0.01 // 10 ms sample rate

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <inttypes.h>
#include <stdlib.h>
#include <stdio.h>

#include "MPU6050_def.h" // Include the MPU6050 register define file
#include "i2c_master.h" // Include the I2C Master header file
#include "uart.h" // include the UART header file

// values from MPU6050
float Acc_x,Acc_y,Acc_z,Temperature,Gyro_x,Gyro_y,Gyro_z;
float pitch, roll;

// function prototypes
void MPU6050_Init();
void MPU_Start_Loc();
void Read_RawValue();
void ComplementaryFilter();
void timerSetup();

int main(){
	I2C_Init(); // initialize I2C
	MPU6050_Init(); // initialize the MPU6050
	USART_Init(9600); // Initialize USART with 9600 Baud Rate
	timerSetup(); // initialize timer0 for 10ms delay
	
	while(1);
}

// function to initialize MPU6050
void MPU6050_Init(){
	_delay_ms(150); // wait 150ms for power up
	I2C_Start_Wait(0xD0); // start the device with write address
	I2C_Write(SMPLRT_DIV); // write to the sample rate register
	I2C_Write(0x07); // use a 1KHz sample rate
	I2C_Stop(); // stop I2C

	I2C_Start_Wait(0xD0);
	I2C_Write(PWR_MGMT_1); // write to the power management register
	I2C_Write(0x01); // reference frequency for gyro x-axis
	I2C_Stop();

	I2C_Start_Wait(0xD0);
	I2C_Write(CONFIG); // write to config register
	I2C_Write(0x00); // Fs = 8KHz
	I2C_Stop();

	I2C_Start_Wait(0xD0);
	I2C_Write(GYRO_CONFIG); // write to gyro config register
	I2C_Write(0x18); // use the full-scale range
	I2C_Stop();

	I2C_Start_Wait(0xD0);
	I2C_Write(INT_ENABLE); // write to the interrupt enable register
	I2C_Write(0x01);
	I2C_Stop();
}

// start the MPU loc
void MPU_Start_Loc(){
	I2C_Start_Wait(0xD0); // start the device with write address
	I2C_Write(ACCEL_XOUT_H); // write start location address from where to read
	I2C_Repeated_Start(0xD1); // I2C start with device read address
}

// read the gyroscope values
void Read_RawValue(){
	MPU_Start_Loc();
	Acc_x = (((int)I2C_Read_Ack()<<8) | (int)I2C_Read_Ack());
	Acc_y = (((int)I2C_Read_Ack()<<8) | (int)I2C_Read_Ack());
	Acc_z = (((int)I2C_Read_Ack()<<8) | (int)I2C_Read_Ack());
	Temperature = (((int)I2C_Read_Ack()<<8) | (int)I2C_Read_Ack());
	Gyro_x = (((int)I2C_Read_Ack()<<8) | (int)I2C_Read_Ack());
	Gyro_y = (((int)I2C_Read_Ack()<<8) | (int)I2C_Read_Ack());
	Gyro_z = (((int)I2C_Read_Ack()<<8) | (int)I2C_Read_Nack());
	I2C_Stop();
}

void ComplementaryFilter(){
	float pitchAcc, rollAcc;
	// Integrate the gyroscope data -> int(angularSpeed) = angle
	pitch += ((float)Gyro_x / GYROSCOPE_SENSITIVITY) * dt;
	// Angle around the X-axis
	roll -= ((float)Gyro_y / GYROSCOPE_SENSITIVITY) * dt;
	// Angle around the Y-axis
	// Compensate for drift with accelerometer data if !bullshit
	// Sensitivity = -2 to 2 G at 16Bit -> 2G = 32768 && 0.5G = 8192
	int forceMagnitudeApprox = \
	abs(Acc_x) + abs(Acc_y) + abs(Acc_z);
	if (forceMagnitudeApprox > 8192 && forceMagnitudeApprox < 32768)
	{
		// Turning around the X axis results in a vector on the Y-axis
		pitchAcc = atan2f((float)Acc_y, (float)Acc_z) * 180 / M_PI;
		pitch = pitch * 0.98 + pitchAcc * 0.02;
		// Turning around the Y axis results in a vector on the X-axis
		rollAcc = atan2f((float)Acc_x, (float)Acc_z) * 180 / M_PI;
		roll = roll * 0.98 + rollAcc * 0.02;
	}
}

// function that sets up timer0 to overflow every 10ms
void timerSetup(){
	TCCR0A = 0; // Normal Operation
	TCCR0B = 5; // set the pre-scaler to 1024 and start timer
	TCNT0=156; // start the count at 217
	TIMSK0 = (1<<TOIE0); // Enable the timer interrupt
	sei(); // Enable global interrupts
}

// function that prints MPU6050 values formatted for viewing in SerialPlot
void printForPlot(){
	char tempChar[20];
	// write Roll value
	snprintf(tempChar,sizeof(tempChar),"%f", roll); // convert temp to a char * (tempChar)
	USART_SendString(tempChar);
	USART_SendString(",");
	// write Pitch value
	snprintf(tempChar,sizeof(tempChar),"%f\r\n", pitch); // convert temp to a char * (tempChar)
	USART_SendString(tempChar);
}

// Overflow interrupt function (every 10ms)
ISR (TIMER0_OVF_vect){
	Read_RawValue(); // obtain the raw values
	ComplementaryFilter(); // smooth/filter the values
	
	// print the filtered values
	char tempChar[20];
	// write Roll value
	snprintf(tempChar,sizeof(tempChar),"%f", roll);
	USART_SendString(tempChar);
	USART_SendString(",");
	// write Pitch value
	snprintf(tempChar,sizeof(tempChar),"%f\r\n", pitch);
	USART_SendString(tempChar);
}

