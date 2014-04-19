//************************************************************************************
//
// This source is Copyright (c) 2011 by Computer Inspirations.  All rights reserved.
// You are permitted to modify and use this code for personal use only.
//
//************************************************************************************
/**
* \file   	PWM.c
* \details  This module implements the Pulse-Width Modulation (PWM) timers that drive
*			the external FETs.  Four independent hardware timers are used that can
*			generate PWM pulses from 0 to 100% with a resolution of 10 bits.  Note:
*			currently only the upper 8 bits of the PWM register are used.  
* \author   Michael Griebling
* \date   	10 Nov 2011
*/ 
//************************************************************************************
//#include "system.h"        /* System funct/params, like osc/peripheral config */
#include "PWM.h"
#include "NightSense.h"
#include "RS485.h"

#define	PERIOD		200							/*!< Desired clock in Hz - 5mS */
#define	SCALE		64							/*!</ Timer 4 prescaler */
#define	PRCOUNT		(IPERIOD/SCALE/PERIOD)

// PWM state definitions
typedef enum _PWMState {	
	OFF, FADING, HOLDING
} PWMState;

static unsigned int prevPWM[4];		/*!< previous pwm values */
static unsigned int newPWM[4];		/*!< new pwm values */
static unsigned int fadeCount;		/*!< count down fade value in 5mS increments */
static unsigned int counter;		/*!< counter used for fade/hold count down */
static unsigned int holdCount;		/*!< count down hold value in 5mS increments */
static PWMState pwmState;			/*!< current PWM state */

static unsigned char minuteTimer;	/*!< count up to one minute */

//const unsigned char ULogTable[] = 	// Upper 8-bit logarithmic light intensity
//{
//	0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 
//	0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 
//	0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 
//	0x03, 0x03, 0x03, 0x04, 0x04, 0x04, 0x04, 0x04, 
//	0x04, 0x04, 0x04, 0x05, 0x05, 0x05, 0x05, 0x05, 
//	0x05, 0x05, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 
//	0x07, 0x07, 0x07, 0x07, 0x07, 0x08, 0x08, 0x08, 
//	0x08, 0x09, 0x09, 0x09, 0x09, 0x0A, 0x0A, 0x0A, 
//	0x0A, 0x0B, 0x0B, 0x0B, 0x0C, 0x0C, 0x0C, 0x0D, 
//	0x0D, 0x0D, 0x0E, 0x0E, 0x0F, 0x0F, 0x0F, 0x10, 
//	0x10, 0x11, 0x11, 0x12, 0x12, 0x13, 0x13, 0x14, 
//	0x14, 0x15, 0x16, 0x16, 0x17, 0x17, 0x18, 0x19, 
//	0x1A, 0x1A, 0x1B, 0x1C, 0x1D, 0x1D, 0x1E, 0x1F, 
//	0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 
//	0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2E, 0x2F, 0x30, 
//	0x32, 0x33, 0x34, 0x36, 0x37, 0x39, 0x3A, 0x3C, 
//	0x3E, 0x3F, 0x41, 0x43, 0x45, 0x47, 0x49, 0x4B, 
//	0x4D, 0x4F, 0x51, 0x53, 0x56, 0x58, 0x5B, 0x5D, 
//	0x60, 0x62, 0x65, 0x68, 0x6B, 0x6E, 0x71, 0x74, 
//	0x77, 0x7B, 0x7E, 0x81, 0x85, 0x89, 0x8D, 0x90, 
//	0x94, 0x99, 0x9D, 0xA1, 0xA6, 0xAA, 0xAF, 0xB4, 
//	0xB9, 0xBE, 0xC3, 0xC9, 0xCE, 0xD4, 0xDA, 0xE0, 
//	0xE6, 0xEC, 0xF3, 0xFA, 0xFF, 0xFF
//};
//
//const unsigned char LLogTable[] = 	// Lower 2-bit logarithmic light intensity (compressed)
//{
//	0x01, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11,
//	0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22,
//	0x22, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x30,
//	0x00, 0x00, 0x00, 0x00, 0x11, 0x11, 0x11, 0x12,
//	0x22, 0x22, 0x23, 0x33, 0x33, 0x00, 0x00, 0x01,
//	0x11, 0x12, 0x22, 0x23, 0x33, 0x00, 0x01, 0x11,
//	0x22, 0x23, 0x30, 0x01, 0x12, 0x23, 0x30, 0x01,
//	0x12, 0x23, 0x00, 0x12, 0x23, 0x01, 0x12, 0x30,
//	0x12, 0x30, 0x12, 0x30, 0x12, 0x30, 0x23, 0x02,
//	0x30, 0x23, 0x12, 0x02, 0x31, 0x31, 0x31, 0x31,
//	0x31, 0x32, 0x02, 0x13, 0x21, 0x02, 0x10, 0x03,
//	0x21, 0x10, 0x00, 0x00, 0x00, 0x01, 0x12, 0x30,
//	0x12, 0x01, 0x31, 0x31, 0x32, 0x03, 0x22, 0x11,
//	0x11, 0x12, 0x23, 0x12, 0x02, 0x03, 0x21, 0x11,
//	0x12, 0x20, 0x13, 0x20, 0x03, 0x30, 0x12, 0x02,
//	0x11, 0x11, 0x20, 0x21, 0x11, 0x23, 0x10, 0x33
//};
//
//static unsigned char UpperLog (unsigned char index) {
//	if (index < 47) return 0;	// compress the table
//	if (index < 74) return 1;
//	return ULogTable[index-74];
//}
//
//static unsigned char LowerLog (unsigned char index) {
//	unsigned char value = LLogTable[index >> 1];
//	if (index & 1) return (value >> 4);
//	return (value & 0xF);
//}	
	
//#define SETPWM1(pwm) 	{ CCP2CONbits.DC2B = LowerLog(pwm); CCPR2L = UpperLog(pwm); } 	
//#define SETPWM2(pwm) 	{ CCP3CONbits.DC3B = LowerLog(pwm); CCPR3L = UpperLog(pwm); }	
//#define SETPWM3(pwm) 	{ CCP1CONbits.DC1B = LowerLog(pwm); CCPR1L = UpperLog(pwm); }	
//#define SETPWM4(pwm) 	{ CCP4CONbits.DC4B = LowerLog(pwm); CCPR4L = UpperLog(pwm); }	

#define SETPWM1(pwm) 	{ CCP2CONbits.DC2B = 0; CCPR2L = pwm; } 	
#define SETPWM2(pwm) 	{ CCP3CONbits.DC3B = 0; CCPR3L = pwm; }	
#define SETPWM3(pwm) 	{ CCP1CONbits.DC1B = 0; CCPR1L = pwm; }	
#define SETPWM4(pwm) 	{ CCP4CONbits.DC4B = 0; CCPR4L = pwm; }	

//********************************************************************************
/**
* \details  Shared interrupt service routine for the PWM timers, night sense
*			state machine timing, and the receive UART. 
* \author   Michael Griebling
* \date   	10 Nov 2011
*/ 
//********************************************************************************
static void interrupt generic_isr(void)
{
	unsigned char i;
	unsigned char done;

	// PWM timer code
	if ((TMR4IE) && (TMR4IF)) {
		switch (pwmState) {
			case FADING:
				if (counter == 0) {
					// Fade the PWM values
					done = 0;
					for (i=CH1; i<=CH4; i++) {
						if (prevPWM[i] < newPWM[i]) prevPWM[i]++;
						else if (prevPWM[i] > newPWM[i]) prevPWM[i]--;
						else done++;
					}

					// Update the PWM outputs with new values
					SETPWM1(prevPWM[CH1]);
					SETPWM2(prevPWM[CH2]);
					SETPWM3(prevPWM[CH3]);
					SETPWM4(prevPWM[CH4]);

					if (done == 4) {
						// change to holding state
						counter = holdCount;
						pwmState = HOLDING;
					} else {
						counter = fadeCount;
					}
				}
				break;
			case HOLDING:
				if (counter == 0) {
					pwmState = OFF;		// finished holding
				}
				break;
			default:
				// OFF state
				break;
		}
		if (counter > 0) counter--;
		TMR4IF = 0;				// Clear Timer4 interrupt flag bit
		
	} else if ((TMR6IE) && (TMR6IF)) {
		// This timer is actually for the NightSense.c logic but unfortunately only one interrupt
		// is used for everything so this handler ends up being a bit kludgey.
	
		if (minuteTimer < 240)
			minuteTimer++;
		else {
			minuteTimer = 0;
			NightSense_UpdateState();
		}
		TMR6IF = 0;				// Clear Timer6 interrupt flag bit

	} else if (RCIF) {
		// Handle the UART receive interrupt	
		// Add character to receive buffer
		RS485_RxBuf[RS485_WtPtr++] = RCREG;
		
	} else if (IOCAF != 0) {
		// Handle the I/O interrupt	
		// Clear interrupt flag
		IOCAF = 0;
	}
}

//********************************************************************************
/**
* \details  PWM timer and I/O port initialization.
* \author   Michael Griebling
* \date   	10 Nov 2011
*/ 
//********************************************************************************
void PWM_Init (void) {
	prevPWM[0] = 0; prevPWM[1] = 0; prevPWM[2] = 0; prevPWM[3] = 0;
	
	APFCON1 = 0x01;					// PWM2 output on pin RA5
	ANSELA = 0;						// All analog inputs are digital
	ANSELB = 0;
	ANSELC = 0;
	
	// Set up PWM registers
	TRISAbits.TRISA5 = 1;			// disable PWM output until set up
	TRISAbits.TRISA2 = 1;			// disable PWM output until set up
	TRISCbits.TRISC5 = 1;			// disable PWM output until set up
	TRISCbits.TRISC6 = 1;			// disable PWM output until set up
	
	// PWM1 initialization -- initially off
	CCP1CONbits.CCP1M = 0b1100;		// PWM mode
	CCP1CONbits.DC1B = 0b00;		// Lowest 2 bits of PWM duty cycle
	CCPR1L = 0x00;					// Upper 8 bits of PWM duty cycle (50% duty cycle)
	CCPTMRSbits.C1TSEL = 0b00;		// Use Timer2 for this PWM
	
	// PWM2 initialization -- initially off
	CCP2CONbits.CCP2M = 0b1100;		// PWM mode
	CCP2CONbits.DC2B = 0b00;		// Lowest 2 bits of PWM duty cycle
	CCPR2L = 0x00;					// Upper 8 bits of PWM duty cycle (50% duty cycle)
	CCPTMRSbits.C2TSEL = 0b00;		// Use Timer2 for this PWM
	
	// PWM3 initialization -- initially off
	CCP3CONbits.CCP3M = 0b1100;		// PWM mode
	CCP3CONbits.DC3B = 0b00;		// Lowest 2 bits of PWM duty cycle
	CCPR3L = 0x00;					// Upper 8 bits of PWM duty cycle (50% duty cycle)
	CCPTMRSbits.C3TSEL = 0b00;		// Use Timer2 for this PWM
	
	// PWM4 initialization -- initially off
	CCP4CONbits.CCP4M = 0b1100;		// PWM mode
	CCP4CONbits.DC4B = 0b00;		// Lowest 2 bits of PWM duty cycle
	CCPR4L = 0x00;					// Upper 8 bits of PWM duty cycle (50% duty cycle)
	CCPTMRSbits.C4TSEL = 0b00;		// Use Timer2 for this PWM
	
	// Set up pwm Timer 2 registers
	PR2 = 0xFF;						// PWM period value
	PIR1bits.TMR2IF = 0;			// Clear Timer2 interrupt flag bit
	T2CONbits.T2CKPS = 0b10;		// Set up Timer2 prescale to /16
	T2CONbits.TMR2ON = 1;			// Enable Timer2
	
	// Turn on the PWM outputs
	TRISAbits.TRISA5 = 0;			// enable PWM output
	TRISAbits.TRISA2 = 0;			// enable PWM output
	TRISCbits.TRISC5 = 0;			// enable PWM output
	TRISCbits.TRISC6 = 0;			// enable PWM output
	
	// Initialize Timer 4 for pwm updates
	PR4 = PRCOUNT-1;			// PWM update period
	PIR3bits.TMR4IF = 0;			// Clear Timer4 interrupt flag bit
	T4CONbits.T4CKPS = 0b11;		// Set up Timer4 prescale to /64
	TMR4IE = 1;				// Enable Timer4 interrupts
	PEIE = 1;				// Also enable peripheral interrupts for Timer4 use
	T4CONbits.TMR4ON = 1;			// Enable Timer4
	ei();					// Global interrupts enabled
	
	counter = 0;
	minuteTimer = 0;
	pwmState = OFF;				// prevent PWM action
}

//********************************************************************************
/**
* \details  Returns \em TRUE iff the PWM state machine is currently performing a
*			PWM fade or hold function.  Although the PWM pulses are hardware-based,
*			the fading and hold features require software timers.
* \author   Michael Griebling
* \date   	10 Nov 2011
*/ 
//********************************************************************************
BOOL PWM_Busy (void) {
	return (pwmState != OFF);
}		

//********************************************************************************
/**
* \details  Override the active PWM fade/hold functions by setting fixed PWM 
*			outputs for the four channels.  The pwm value is applied during the 
*			next PWM period.  Function returns immmediately.  PWM values range 
*			from 0 to PWM_MAX where PWM_MAX represents 100% duty cycle.
* \author   Michael Griebling
* \date   	10 Nov 2011
*/ 
//********************************************************************************
void PWM_Set (unsigned char pwm1, unsigned char pwm2, unsigned char pwm3, unsigned char pwm4) {
	pwmState = OFF;	// Stop ramping now
	__delay_ms(10);	// Wait for next interrupt
	
	prevPWM[CH1] = pwm1; SETPWM1(pwm1);
	prevPWM[CH2] = pwm2; SETPWM2(pwm2);
	prevPWM[CH3] = pwm3; SETPWM3(pwm3);
	prevPWM[CH4] = pwm4; SETPWM4(pwm4);
}	

//********************************************************************************
/**
* \details 	Ramps from the previous pwm values for all channels to the passed pwm
*			value over the fade time which has units of 10 milliseconds.  The hold
*			time has units of 50 milliseconds.  The total fade/hold function takes 
*			fade*5 + hold*50 milliseconds.  This function returns immediately and
*			will prevent another PWM fade/hold event until the current event has
*			completed.  See also the \em PWM_Busy function.
* \author   Michael Griebling
* \date   	10 Nov 2011
*/ 
//********************************************************************************
void PWM_Ramp (unsigned char pwm1, unsigned char pwm2, unsigned char pwm3, unsigned char pwm4, 
			   unsigned char fade, unsigned char hold) {
	if (pwmState != OFF) return;			// don't add a new ramp until the current one is finished

	// Initialize the next ramping stage
	newPWM[CH1] = pwm1;
	newPWM[CH2] = pwm2;
	newPWM[CH3] = pwm3;
	newPWM[CH4] = pwm4;
	holdCount = 10*(unsigned int)hold;
	fadeCount = fade;
	
	// Start next ramping now
	counter = fadeCount;
	pwmState = FADING;
}	
