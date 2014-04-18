#ifndef _MACROS_H_
#define _MACROS_H_

#include "Types.h"

#define ENDMACRO	(0xFFFF)
#define PLAYMACROS	(0xFFFF)

// read/write a word from internal EEPROM
extern unsigned int ReadWord (unsigned char address);
extern void WriteWord (unsigned char address, unsigned int data);

extern unsigned int Macros_Count (void);

extern BOOL Macros_Add(unsigned int macro);
extern unsigned int Macros_Read(unsigned int macroID);

extern void Macros_Init(void);

#endif
