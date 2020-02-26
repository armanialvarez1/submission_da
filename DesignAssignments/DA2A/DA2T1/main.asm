.include <m328pbdef.inc>
;
; DA2T1.asm
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
	SBI DDRB, 3 ; Set PORTB.3 as an output

CYCLE: 
	SBI PORTB, 3 ; Set PINB.3 High
	delay2_5ms 165 ; Delay for 0.4125 secs @16MHz
	CBI PORTB, 3 ; Set PINB.3 Low
	delay2_5ms 135 ; Delay for 0.3375 secs @16MHz
	jmp CYCLE ; repeat waveform

