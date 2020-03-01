.include <m328pbdef.inc>
;
; DA2B.asm
;
;
; Author : Mateo Markovic
;

.macro delay2_5ms ; macro to delay .25ms x provided number
	ldi R20, @0
L1: ldi  r18, 52
    ldi  r19, 242
L2: dec  r19
    brne L2
    dec  r18
    brne L2
    nop
	dec R20
	brne L1
.endmacro

.org 0x00
JMP start
.org 0x02 ; Location for external interupt 0
JMP PUSHED_ISR

start:
	LDI R20, HIGH(RAMEND) ; Initialize the Stack
	OUT SPH, R20
	LDI R20, LOW (RAMEND)
	OUT SPL, R20

	LDI R20, 0x2
	STS EICRA, R20 ; Set INT0 as falling edge triggered
	SBI PORTD, 2 ; Set PORTD.2 as a pull-up
	LDI R20, 1<<INT0
	OUT EIMSK, R20 ; enable INT0
	SEI ; Finally enable interupts

CYCLE: 
	SBI DDRB, 3 ; Set PORTB.3 as an output
	SBI PORTB, 3 ; Set PINB.3 High
	delay2_5ms 165 ; Delay for 0.4125 secs @16MHz
	CBI PORTB, 3 ; Set PINB.3 Low
	delay2_5ms 135 ; Delay for 0.3375 secs @16MHz
	jmp CYCLE ; else repeat waveform

PUSHED_ISR:
	CBI DDRB, 3 ; Set PORTB.3 High
	SBI DDRB, 2 ; Set PORTB.2 High
	delay2_5ms 200 ; delay for 1/2 sec @16MHz
	delay2_5ms 200 ; delay for 1/2 sec @16MHz
	delay2_5ms 200 ; delay for 1/2 sec @16MHz
	delay2_5ms 200 ; delay for 1/2 sec @16MHz
	CBI DDRB, 2 ; Set PORTB.2 Low
	RETI ; Return back to the CYCLE



