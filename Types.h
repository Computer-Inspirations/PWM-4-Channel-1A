#ifndef _TYPES_H_
#define _TYPES_H_

#include <xc.h>	// Required to interface with delay routines

#ifndef _XTAL_FREQ
 // Unless already defined assume 4MHz system frequency
 // This definition is required to calibrate __delay_us() and __delay_ms()
 	#define _XTAL_FREQ 	4000000
	#define IPERIOD		(_XTAL_FREQ/4)		// Instruction clock in Hz
#endif

#ifndef BOOL
#define BOOL unsigned char
#endif

#ifndef FALSE
#define FALSE  0
#endif

#ifndef TRUE
#define TRUE  1
#endif

typedef unsigned char BOOLEAN;
typedef unsigned char TCHAR;
typedef signed char SHORTINT;
typedef int INTEGER;
typedef long LONGINT;
typedef unsigned int CARDINAL;
typedef unsigned long LONGCARD;
typedef float REAL;
typedef unsigned char SHORTSET;
typedef unsigned short SET;
typedef unsigned long LONGSET;

#endif
