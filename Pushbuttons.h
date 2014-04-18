#ifndef _PUSHBUTTONS_H_
#define _PUSHBUTTONS_H_

#include "Types.h"

#define BUTTON1		(1)
#define BUTTON2		(2)

void PushButtons_Init (void);

void PushButtons_Scan (void);

void PushButtons_Clear (unsigned char button);

BOOL PushButtons_Active (unsigned char button);

BOOL PushButtons_Pressed (unsigned char button);

BOOL PushButtons_Held (unsigned char button);

#endif
