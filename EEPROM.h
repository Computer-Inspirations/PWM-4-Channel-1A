#ifndef _EEPROM_H_
#define _EEPROM_H_

#include "Types.h"

extern void EEPROM_Init (void);
extern BOOL EEPROM_Present (void);
extern unsigned int EEPROM_GetSize (void);

extern void EEPROM_WriteChar(unsigned int add, unsigned char ch);
extern void EEPROM_Write(unsigned int add, unsigned char buffer[], unsigned int size);

extern unsigned char EEPROM_ReadChar(unsigned int add);
extern void EEPROM_Read(unsigned int add, unsigned char buffer[], unsigned int size);

#endif
