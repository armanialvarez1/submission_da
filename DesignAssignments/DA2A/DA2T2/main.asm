.include <m328pbdef.inc>
;
; DA2T2.asm
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
start:

	CBI DDRC, 3 ; Set PORTC.3 as an input
	SBI PORTC, 3 ; Set PINC.3 active high (pull-up)

CYCLE: 
	SBI DDRB, 3 ; Set PORTB.3 as an output
	SBI PORTB, 3 ; Set PINB.3 High
	delay2_5ms 165 ; Delay for 0.4125 secs @16MHz
	CBI PORTB, 3 ; Set PINB.3 Low
	delay2_5ms 135 ; Delay for 0.3375 secs @16MHz

	SBIS PINC, 3 ;			When PINC.3 goes low
	call BUTTON_PRESSED ;	Call BUTTON_PRESSED function

	jmp CYCLE ; else repeat waveform

BUTTON_PRESSED:
	CBI DDRB, 3 ; Set PORTB.3 High
	SBI DDRB, 2 ; Set PORTB.2 High
	delay2_5ms 200 ; delay for 1/2 sec @16MHz
	delay2_5ms 200 ; delay for 1/2 sec @16MHz
	delay2_5ms 200 ; delay for 1/2 sec @16MHz
	delay2_5ms 200 ; delay for 1/2 sec @16MHz
	CBI DDRB, 2 ; Set PORTB.2 Low

	ret ; return to the cycle