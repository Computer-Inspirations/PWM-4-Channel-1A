#ifndef _NightSense_H_
#define _NightSense_H_

#include "Types.h"

extern void NightSense_Init (void);

extern void NightSense_UpdateState (void);

extern void NightSense_Enable (BOOL on);

extern BOOL NightSense_IsNight (void);

extern void NightSense_SetOnDelay (unsigned char time);
extern void NightSense_SetOffDelay (unsigned char time); 

extern void NightSense_SetDuration (unsigned int time);

extern void NightSense_GetParam (BOOL *enabled, unsigned int *onTime, unsigned char *onDelay, unsigned char *offDelay);

#endif