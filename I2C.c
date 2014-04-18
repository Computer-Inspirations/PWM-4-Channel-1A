//************************************************************************************
//
// This source is Copyright (c) 2011 by Computer Inspirations.  All rights reserved.
// You are permitted to modify and use this code for personal use only.
//
//************************************************************************************
/**
* \file   	I2C.c
* \details  This module implements a software-based I2C bus protocol.  I used a
*			software I2C implementation even though this device has a hardware I2C
*			port because there were some issues in getting the hardware port to work
*			with the Microchip EEPROM.  If anyone else can get the hardware port
*			to work, please send me the code.
* \author   Michael Griebling
* \date   	10 Nov 2011
*/ 
//************************************************************************************

#include "I2C.h"
#include "Types.h"

#define SCLDIR TRISBbits.TRISB6		/* Clock on B6 */
#define SDADIR TRISBbits.TRISB4 	/* Data on B4 */
#define SDAIN  PORTBbits.RB4
#define SDA	   LATBbits.LATB4
#define SCL	   LATBbits.LATB6
#define IN	   1
#define OUT    0

#define I2C_device	0xA0		// Base device address for EEPROM

static BOOLEAN Ack(void)
{ 
   BOOLEAN ack;
	
   SCLDIR = IN;   			/* set SCL as an input so SCL goes high */
   
   /* assume SDA is input */
   ack = (SDAIN == 0);		/* sample SDA acknowledge */
   if (ack) ;				/* delay clock */				
   SCLDIR = OUT; 			/* set SCL to an output so SCL goes low */
   return ack;
} /* end Ack() */


static void SendByte(TCHAR b)
{
   INTEGER cnt = 8;
	
   while (cnt--) {  
      if (b&0x80) SDADIR = IN;  /* set SDA as input -> goes high */
      else SDADIR = OUT;		/* set SDA as output -> goes low */
      SCLDIR = IN;		 		/* set SCL as input -> goes high */
      b <<= 1; 		 	 		/* shift left 1 bit (part of delay) */			 
      SCLDIR = OUT;	 	 		/* set SCL as output -> goes low */    
   } /* end while */
   cnt = 0;						/* delay for data hold */
   SDADIR = IN;					/* leave SDA high for acknowledge */
} /* end SendByte() */


static BOOLEAN SendByteAck(TCHAR b)
{
   SendByte(b);
   return Ack();
} /* end SendByteAck() */


static TCHAR ReceiveByte(void)
{
   INTEGER cnt = 8;
   TCHAR lb = 0;
	
   while (cnt--) {
      lb<<=1;			 	/* shift left 1 bit position */   	
      SCLDIR = IN;		 	/* set SCL as input -> goes high */   	  
      if (SDAIN) lb|=1;  	/* set LSB of byte */
      SCLDIR = OUT;			/* set SCL as output -> goes low */    
   } /* end while */  
   return lb;	
} /* end ReceiveByte() */


static TCHAR ReceiveByteAck(void)
{
   TCHAR lb = ReceiveByte();
   SDADIR = OUT;		 	/* leave SDA low for acknowledge */    
   Ack();
   SDADIR = IN;		 		/* set SDA high again */   
   return lb;
} /* end ReceiveByteAck() */


static void DoStart(void)
{
   /* do start bit */
   SDADIR = IN;				/* set SCL, SDA as inputs -> go high */
   SCLDIR = IN;
   __delay_us(5);			/* set-up time delay */
   SDADIR = OUT;            /* set SDA as output -> goes low */ 
   __delay_us(5);			/* hold time delay */
   SCLDIR = OUT;          	/* set SCL as output -> goes low */ 
} /* end DoStart() */


void I2C_Power(BOOLEAN TurnOn, BOOLEAN Count)
{	
}

static void Start(TCHAR b, LONGINT adr)
{
   I2C_Power(TRUE, FALSE);			/* Power on memories */
   DoStart(); 

   /* start sending the data */
   SendByteAck(b);
   SendByteAck((TCHAR)(adr>>8));	/* output remainder of address */
   SendByteAck((TCHAR)(adr));  
} /* end Start() */


static void Stop(void)
{
   /* do stop bit */
   SDADIR = OUT;		/* set SDA as output -> goes low */
   __delay_us(1);		/* set-up time delay */	 
   SCLDIR = IN;         /* set SCL as input -> goes high */ 
   __delay_us(5);		/* set-up time delay */
   SDADIR = IN;	        /* set SDA as input -> goes high */  
} /* end Stop() */	

static void I2C_Init(void)
{
   /* set up registers for I2C communication */
   SDADIR = IN;				/* set SCL, SDA as inputs -> go high */
   SCLDIR = IN;
   SDA = 0;					/* set SDA/SCL low */
   SCL = 0;
   I2C_Power(FALSE, FALSE);	/* initially turn off I2C power */	
}
	

void I2C_GetBuf(LONGINT adr, TCHAR buf[], CARDINAL size)
{
   CARDINAL ind;
   
   Start(I2C_device, adr);			/* output start bit and device address */
   
   /* do start bit again */
   DoStart();     
   SendByteAck(I2C_device+1);
   for (ind=0; ind<size-1; ind++) {
      buf[ind] = ReceiveByteAck(); 	/* receive data buffer */
   } /* end for */
   buf[size-1] = ReceiveByte();
   Stop();
} /* end GetBuf() */


TCHAR I2C_Get(LONGINT adr)
{
   TCHAR ch;
	
   Start(I2C_device, adr);		/* output start bit and device address */
   
   /* do start bit */
   DoStart();     
   SendByteAck(I2C_device+1);	/* send device address -- read mode */
   ch = ReceiveByte();			/* receive byte */
   Stop();						/* output stop bit */
   return ch;
} /* end Get() */


BOOLEAN I2C_GetAck(void)
{
   TCHAR ch;
   BOOLEAN ack;

   /* do start bit */
   DoStart();   
   ack = SendByteAck(I2C_device+1); 	/* send device address -- read mode */
   ch = ReceiveByte();					/* receive current address */
   Stop();								/* output stop bit */
   return ack;
} /* end GetAck() */


void I2C_SendBuf(LONGINT adr, TCHAR buf[], CARDINAL size)
{
   CARDINAL ind;

   Start(I2C_device, adr);			/* output start bit and device address */
   for (ind=0; ind<size-1; ind++) {
      SendByteAck(buf[ind]); 		/* output data buffer */
   } /* end for */
   SendByteAck(buf[size-1]);
   Stop();
} /* end SendBuf() */


BOOLEAN I2C_Device_Present(void)
{
    // Check for device twice before giving up
    if (I2C_GetAck()) return TRUE;
    return (I2C_GetAck());         
}


BOOLEAN I2C_Send(LONGINT adr, TCHAR byte)
{
	BOOLEAN ack;
   	Start(I2C_device, adr);		/* output start bit and device address */
   	ack = SendByteAck(byte); 	/* output data */
   	Stop();						/* output stop bit */
   	return ack;
} /* end Send() */

void I2C_BEGIN(void)
{  
   I2C_Init();
}

