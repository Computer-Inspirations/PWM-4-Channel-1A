
//************************************************************************************
//
// This source is Copyright (c) 2011 by Computer Inspirations.  All rights reserved.
// You are permitted to modify and use this code for personal use only.
//
//************************************************************************************
/**
* \file   	EEPROM.c
* \details  This module implements the interface to the Microchip 24LC256 EEPROM.
*			Code in comments was the original (unsuccessful) attempt to use the I2C
*			hardware port.  I eventually resorted to using a bit-banged I2C software
*			implementation (see \em I2C.c) due to the problems I was having.  
*
*			If anyone can get the hardware I2C working, please send me the code.  To
*			simplify maintenance, I suggest putting the hardware I2C code in the
*			I2C.c module in place of the existing software I2C.
* \author   Michael Griebling
* \date   	10 Nov 2011
*/ 
//************************************************************************************

#include "Types.h"					// Required to interface with delay routines
#include "EEPROM.h"
#include "I2C.h"

//#define EEPROM_DEVICE	(0xA0)		// Base device address for EEPROM
#define PAGE_SIZE		(64)		// Write page size for Microchip's 24xx256 EEPROM
#define EEPROM_BYTES	(1024*32)	// 32KB EEPROM

// Register bit definitions
//#define SEN		SSP1CON2bits.SEN
//#define RSEN	SSP1CON2bits.RSEN
//#define RCEN	SSP1CON2bits.RCEN
//#define PEN		SSP1CON2bits.PEN
//#define ACKEN	SSP1CON2bits.ACKEN
//#define ACKDT	SSP1CON2bits.ACKDT
//#define ACKSTAT	SSP1CON2bits.ACKSTAT
//#define BF		SSP1STATbits.BF

void EEPROM_Init (void) {
 	// Set up the I2C registers
//	TRISBbits.TRISB4 = 1;			// Set I2C pins as inputs
//	TRISBbits.TRISB6 = 1;
//	SSP1CON = 0x38;					// Set I2C master mode
//	SSP1CON2 = 0x00;
//	SSP1ADD = 10;					// Baud rate initialization for 100KHz
//	SSP2STATbits.CKE = 1;
//	SSP2STATbits.SMP = 1;
//	
//	SSP1IF = 0;						// clear SSPIF interrupt flag
//	BCL1IF = 0;						// clear bus collision flag	
	
//  	SSP1STAT &= 0x3F;           // power on state 
//  	SSP1CON = 0x00;            	// power on state
//  	SSP1CON2 = 0x00;            // power on state
//  	SSP1CON |= 0b00001000;   	// select serial mode (Master)
//  	SSP1STAT |= 0b10000000;    	// slew rate off 
//	SSP1ADD = 10;				// Baud rate initialization for 100KHz
//
//  	TRISBbits.TRISB4 = 1;       // Set I2C pins as inputs
//  	TRISBbits.TRISB6 = 1;       // 
	I2C_BEGIN();

//  	SSP1CON1 |= SSPENB;        // enable synchronous serial port 
}

//#define Wait()  SSP1IF = 0;	while (SSP1IF == 0);	// Wait for start to complete

//static void Wait (void) {
//  	while ( ( SSP1CON2 & 0x1F ) | ( SSP1STATbits.R_nW ) )
//     	continue;
//}		

//static void SendStart (void) {
//	Wait();
//	SEN = 1;						// Generate a start condition
//}
//
//static void SendRestart (void) {
//	Wait();
//	RSEN = 1;						// Generate a restart condition
//}
//
//static void SendStop (void) {
//	Wait();
//	PEN = 1;						// Generate a stop condition
//}	

//static unsigned char SendByte (unsigned char b) {
////	Wait();
////	SSP1BUF = b;					// Send out a byte
////	return (!ACKSTAT);				// Returns '1' if transmission was acknowledged
//	return I2C_Send(b);
//}
//
//static unsigned char ReadByte (unsigned char ack) {
////	unsigned char i2cdata;
////	
////	Wait();
////	RCEN = 1;						// Clock in a data byte
////	Wait();
////	i2cdata = SSP1BUF;				// Get the data locally	
////	Wait();
////	if (ack) ACKDT = 0;
////	else ACKDT = 1;
////	ACKEN = 1;
////	return i2cdata;
//}

//static void SendAddress (unsigned int add) {
//	SendStart();					// Start the write with a start condition
//	SendByte(EEPROM_DEVICE);		// Send out the device address
//	SendByte(add >> 8);				// Send out the high memory address byte
//	SendByte(add);					// Send out the low memory address byte
//}			

void EEPROM_WriteChar(unsigned int add, unsigned char ch) {
	/////////////////////////////////////////////////////////////////////////	
	// Send a data byte
	I2C_Send(add, ch);
   __delay_ms(6);					/* write time delay */	
//	
//	SendAddress(add);				// Send the device & memory address
//	SendByte(ch);					// Send the data byte to be written
//	SendStop();						// Indicate no more data to write
//	__delay_ms(11);					// delay unil write is done
	
}

//static void WritePage (unsigned int add, unsigned char buffer[], unsigned int size) {
//	/////////////////////////////////////////////////////////////////////////	
//	// Send a page (64 bytes) of data to EEPROM.  The caller must guarantee
//	// that only page writes will occur.
//	unsigned int i;
//	
//	SendAddress(add);				// Send the device & memory address
//	for (i=0; i<size; i++) {
//		SendByte(buffer[i]);		// Send the data bytes to be written
//	}
//	SendStop();						// Indicate no more data to write
//	__delay_ms(11);					// delay unil write is done
//}	
	
void EEPROM_Write(unsigned int add, unsigned char buffer[], unsigned int size) {
	/////////////////////////////////////////////////////////////////////////	
	// Write a block of data to EEPROM -- we check to ensure that writes across
	// page boundaries are handled properly.
	unsigned int i;
	unsigned int lsize;
	
	lsize = add & (PAGE_SIZE-1);
	if (lsize != 0) {
		// starting in the middle of an EEPROM page -- write partial page first
		lsize = PAGE_SIZE - lsize;
		if (lsize > size) lsize = size;
		I2C_SendBuf(add, buffer, lsize);
   		__delay_ms(6);					/* write time delay */	
//		WritePage(add, buffer, lsize);
		size -= lsize;
		add += lsize; 
	}
	
	// Write all the PAGE_SIZEd segments to EEPROM
	while (size >= PAGE_SIZE) {
//		WritePage(add, &buffer[lsize], PAGE_SIZE);
		I2C_SendBuf(add, &buffer[lsize], PAGE_SIZE);
   		__delay_ms(6);					/* write time delay */	
		size -= PAGE_SIZE;
		add += PAGE_SIZE;
		lsize += PAGE_SIZE; 		
	}
	
	// Write any remnant bytes
	if (size > 0) {
		I2C_SendBuf(add, &buffer[lsize], size);
   		__delay_ms(6);					/* write time delay */	
	}	
}

BOOL EEPROM_Present (void) {
//	unsigned char ack;
//	
//	SendStart();					// Start command sequence
//	ack = SendByte(EEPROM_DEVICE);	// Send out the device address
//	SendStop();
//	if (ack == 0) return FALSE;		// No acknowledge from slave
	return I2C_Device_Present();	
}

unsigned int EEPROM_GetSize (void) {
	return EEPROM_BYTES;	
}	

unsigned char EEPROM_ReadChar(unsigned int add) {
//	unsigned char data;
//	
//	///////////////////////////////////////////////////////////////////////////////
//	// Read a byte
//	SendAddress(add);				// Send the device & memory address		
//	SendRestart();					// Prepare to read
//	SendByte(EEPROM_DEVICE+1);		// Send out the device read address byte
//	data = ReadByte(0);				// Read back the data
//	SendStop();						// Close down the read session
//	return data;
	return I2C_Get(add);
}
	
void EEPROM_Read(unsigned int add, unsigned char buffer[], unsigned int size) {
//	unsigned int i;
//	
//	///////////////////////////////////////////////////////////////////////////////
//	// Read a block of bytes from EEPROM
//	SendAddress(add);				// Send the device & memory address		
//	SendRestart();					// Prepare to read
//	SendByte(EEPROM_DEVICE+1);		// Send out the device read address byte
//	for (i=0; i<size; i++) {
//		buffer[i] = ReadByte((i != size-1));	// Read back the data
//	}
//	SendStop();						// Close down the read session
	I2C_GetBuf(add, buffer, size);
}	


