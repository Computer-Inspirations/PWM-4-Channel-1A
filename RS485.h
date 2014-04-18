#ifndef _RS485_H_
#define _RS485_H_

#include "Types.h"

extern unsigned char RS485_RxBuf[256];		// receive buffer
extern unsigned char RS485_RdPtr;			// read pointer
extern unsigned char RS485_WtPtr;			// write pointer (interrupt)

void RS485_Init (void);

#define RS485_ClearBuffer()		RS485_RdPtr = 0; RS485_WtPtr = 0
BOOL RS485_CharReady (void);

void RS485_WriteChar(unsigned char ch);
void RS485_Write(unsigned char buffer[], unsigned int size);

unsigned char RS485_ReadChar(void);
void RS485_Read(unsigned char buffer[], unsigned int size);

#endif
