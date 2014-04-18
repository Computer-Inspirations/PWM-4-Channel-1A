//************************************************************************************
//
// This source is Copyright (c) 2011 by Computer Inspirations.  All rights reserved.
// You are permitted to modify and use this code for personal use only.
//
//************************************************************************************
/**
* \file   	RS485.c
* \details  This module implements the hardware-level serial port code that manages
*			the external RS-485 driver chip and internal UART.  The receive buffer
*			is defined here but is written to by the shared interrupt routine in the
*			PWM module.  This code also automatically handles the half-duplex RS-485
*			mode switches between receive and transmit operation. 
* \author   Michael Griebling
* \date   	10 Nov 2011
*/ 
//************************************************************************************

#include "RS485.h"

#define BAUD		9600
#define HIGH_SPEED 	1

#if HIGH_SPEED == 1
#define SPEED 0x4
#else
#define SPEED 0
#endif

#define RX_PIN TRISB5
#define TX_PIN TRISB7

#define Enable_Transmit()	LATCbits.LATC0 = 1; LATCbits.LATC4 = 1; __delay_ms(5); TxActive=TRUE;
#define Enable_Receive()	__delay_ms(5); LATCbits.LATC0 = 0; LATCbits.LATC4 = 0; TxActive=FALSE;

static BOOL TxActive;
unsigned char RS485_RxBuf[256];		// receive buffer
unsigned char RS485_RdPtr;			// read pointer
unsigned char RS485_WtPtr;			// write pointer (interrupt)

/* Serial initialization */
void RS485_Init (void) {
	// Set up RS485 control pins
	TRISCbits.TRISC4 = 0;			// Set up RS-485 enable pins
	TRISCbits.TRISC0 = 0;
	Enable_Receive();
	
	// Set up hardware UART
	RX_PIN = 1;
	TX_PIN = 0;
	SPBRG = (_XTAL_FREQ/(16UL * BAUD) - 1);   
	RCSTA = 0x90;
	TXSTA = (SPEED|0x20);
	
	// Clear receive buffer
	RS485_ClearBuffer();
	
	// Enable receive interrupt
	RCIE = 1;
}

void putch(unsigned char byte) 
{
	/* output one byte */
	while (!TXIF)	/* set when register is empty */
		continue;
	TXREG = byte;
}

unsigned char getch() {
	/* retrieve one byte */
	while (RS485_RdPtr == RS485_WtPtr)	/* check for received characters */
		continue;
	return RS485_RxBuf[RS485_RdPtr++];	
}

unsigned char getche(void)
{
	unsigned char c;
	putch(c = getch());
	return c;
}

void RS485_WriteChar(unsigned char ch) {
	if (!TxActive) Enable_Transmit();
	putch(ch);
}
	
void RS485_Write(unsigned char buffer[], unsigned int size) {
	unsigned int index = 0;
	if (!TxActive) Enable_Transmit();
	while (size > 0) { putch(buffer[index++]); size--; }
}

BOOL RS485_CharReady (void) {
	if (TxActive) Enable_Receive();
	return (RS485_RdPtr != RS485_WtPtr);		/* check for received characters */
}		

unsigned char RS485_ReadChar(void) {
	return getch();
}
	
void RS485_Read(unsigned char buffer[], unsigned int size) {
	unsigned int index = 0;
	while (size > 0) { buffer[index++] = getch(); size--; }
}	


