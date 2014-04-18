#ifndef _PWM_H_
#define _PWM_H_

#include "Types.h"

#define CH1	  	0
#define CH2		1
#define CH3		2
#define CH4		3

#define PWM_MAX	255

extern void PWM_Init (void);

extern BOOL PWM_Busy (void);

extern void PWM_Set (unsigned char pwm1, unsigned char pwm2, unsigned char pwm3, unsigned char pwm4);
// Set the pwm value for channel ch.  The pwm value is
// applied during the next PWM period.  Function returns
// immmediately.  pwm value ranges from 0 to PWM_MAX where
// PWM_MAX represents 100% modulation.

extern void PWM_Ramp (unsigned char pwm1, unsigned char pwm2, unsigned char pwm3, unsigned char pwm4, 
					  unsigned char fade, unsigned char hold);
// Ramps from the previous pwm value for channel ch to the passed pwm
// value over the fade time which has units of 10 milliseconds.  The hold
// time has units of 50 milliseconds.  This function takes fade*10 + hold*50 
// milliseconds before it returns.

#endif