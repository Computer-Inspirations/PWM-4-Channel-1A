#ifndef _MEMORYMAP_H_
#define _MEMORYMAP_H_

#include "Types.h"

#define NIGHTADD		(0x0000)				// Storage for Night Sense parameters
#define STATEADD		(NIGHTADD) 				// 1 byte - NightState state
#define OFFTIMEADD		(NIGHTADD+1)			// 1 byte - Delay before turning off in minutes
#define ONTIMEADD		(NIGHTADD+2)			// 1 byte - Delay before turning on in minutes
#define DURATIONADD		(NIGHTADD+3)			// 2 bytes - Total on time in minutes
#define STARTSEQADD		(NIGHTADD+5)			// 2 bytes - First sequence to play on power-up
#define TOTALSEQADD		(NIGHTADD+7)			// 2 bytes - Number of sequences to play
#define DEVICEADD		(NIGHTADD+9)			// 1 byte - Protocol address (0xFF is default)
#define EESEQADD		(0x10)					// Start of EEPROM macro sequences

#define MAXMACROS		(100)					// Allow up to 100 macros

#endif
