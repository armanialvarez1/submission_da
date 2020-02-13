.include <m328pdef.inc>
;
; iterativeMultiply.asm
;
; Author : Mateo Markovic
; Multiplies a 32 bit by 32 bit number iteratively
; Tests 0xAB75_F1BB * 0x012F_FE23
; Result should be 0xCB9AC_F934C_9691
;

start:
	CLR R0			; Make sure R0 is zero

    LDI R19, 0xAB	; Load the two MSB of the multiplicand into R19
	LDI R18, 0x75	; Load the next two bytes of the multiplicand into R18
	LDI R17, 0xF1	; Load the next two bytes of the multiplicand into R17
	LDI R16, 0xBB	; Load the two LSB of the multiplicand into R16

	LDI R23, 0x01	; Load the two MSB of the multiplier into R23
	LDI R22, 0x2F	; Load the next two bytes of the multiplier into R22
	LDI R21, 0xFE	; Load the next two bytes multiplier into R21
	LDI R20, 0x23	; Load the two LSB of the multiplier into R20

	CLR R31			; Initialize the result registers to zero
	CLR R30			; Initialize the result registers to zero
	CLR R29			; Initialize the result registers to zero
	CLR R28			; Initialize the result registers to zero
	CLR R27			; Initialize the result registers to zero
	CLR R26			; Initialize the result registers to zero
	CLR R25			; Initialize the result registers to zero
	CLR R24			; Initialize the result registers to zero

multiply:
	ADD R24, R16	; Add the first two LSB of the multiplicand with the first two LSB of the result
	ADC R25, R17	; Add the next two bytes of the multiplicand with the first next two bytes of the result with above carry
	ADC R26, R18	; Add the next two bytes of the multiplicand with the first next two bytes of the result with above carry
	ADC R27, R19	; Add the next two bytes of the multiplicand with the first next two bytes of the result with above carry
	ADC R28, R0		; Store any left over carry
	ADC R29, R0		; Store any left over carry
	ADC R30, R0		; Store any left over carry
	ADC R31, R0		; Store any left over carry

	DEC R20			; Decrease the multipler by 1
	CPI R20, 0xFF	; if the multiplier is now FF (underflow), we need to decrement the next bit
	BRNE skip		; if underflow didn't occur, skip this
	
	DEC R21			; Decrease the next bit of the multiplier
	CPI R21, 0xFF	; Check if this byte also underflew
	BRNE skip		; skip if it didn't

	DEC R22			; Decrease the next bit of the multiplier
	CPI R22, 0xFF	; Check if this byte also underflew
	BRNE skip		; skip if it didn't

	DEC R23			; Decrease the next bit of the multiplier
skip:
	CPI R20, 0		; Check if the first byte is zero
	BRNE multiply	; if not we need to keep multiplying

	CPI R21, 0		; check the next byte if it is zero
	BRNE multiply	; if not we need to keep multiplying

	CPI R22, 0		; check the next byte if it is zero
	BRNE multiply	; if not we need to keep multiplying

	CPI R23, 0		; check the next byte if it is zero
	BRNE multiply	; if not we need to keep multiplying

end: rjmp end		; When the enter multiplier is zero, we are done
