//************************************************************************************
//
// This source is Copyright (c) 2011 by Computer Inspirations.  All rights reserved.
// You are permitted to modify and use this code for personal use only.
//
//************************************************************************************
/**
* \file   	Pushbuttons.c
* \details  This module implements the push button interface.  The two pushbuttons
*		 	are debounced and a toggle or held state is determined based on how long
*			the pushbutton is held down.  If held down for 5 seconds, a \em HOLD state
*			is entered; otherwise, a \em PRESSED state is set.  Each pushbutton has
*			an independent state machine.  Both state machines are sequenced by the
*			\em Scan function which is invoked periodically every 10mS by an shared
*			interrupt routine in the PWM module.  
* \author   Michael Griebling
* \date   	10 Nov 2011
*/ 
//************************************************************************************

#include "Pushbuttons.h"

// Pushbutton state definitions
typedef enum _PBState {	
	INACTIVE, DOWNTIMING, PRESSED, HOLD, RELEASECHECK, RELEASECHECK2, UPTIMING, UPTIMING2, WASPRESSED
} PBState;

#define DEBOUNCETIME	(10)				/*!<  100mS with 10mS scan */
#define HOLDTIME		(500)				/*!<  5 Sec with 10mS scan */
#define PB1				(PORTAbits.RA0)		/*!<  PB1 bit input */
#define PB2				(PORTCbits.RC2)		/*!<  PB1 bit input */
#define UP				(1)
#define DOWN			(0)

static PBState b1State, b2State;			/*!<  PB1 & PB2 states */
static BOOL b1Pressed, b2Pressed;
static unsigned int b1Timer, b2Timer;

//********************************************************************************
/**
* \details  Initialize the Pushbuttons state variables and I/O registers.
* \author   Michael Griebling
* \date   	10 Nov 2011
*/ 
//********************************************************************************
void PushButtons_Init (void) {
	TRISCbits.TRISC2 = 1;		// set to input for PB2
	WPUCbits.WPUC2 = 1;			// set pull-up for PB2
	TRISAbits.TRISA0 = 1;		// set to input for PB1
	WPUAbits.WPUA0 = 1;			// set pull-up for PB1
	OPTION_REGbits.nWPUEN = 0;	// enable pull-ups
	
	b1State = INACTIVE;			// initialize the button state
	b2State = INACTIVE;
	b1Timer = DEBOUNCETIME;		// set up timers
	b2Timer = DEBOUNCETIME;
}

//********************************************************************************
/**
* \details  Internal pushbutton scan function.  This code is reused for each
*			pushbutton.
* \author   Michael Griebling
* \date   	10 Nov 2011
*/ 
//********************************************************************************
static void Scan (PBState* state, unsigned int* tim, unsigned char pb) {
	unsigned int timer = *tim;
	
	switch (*state) {
		case INACTIVE:
			if (pb == DOWN) {
				// start debouncing the down transition
				timer = DEBOUNCETIME;
				*state = DOWNTIMING;
			}
			break;
		case DOWNTIMING:
			if (timer > 0) timer--;
			else {
				*state = PRESSED;
				timer = HOLDTIME;
			}	
			break;
			
		// Normal WasPressed/Release recovery
		case UPTIMING:
			if (timer > 0) timer--;
			else *state = WASPRESSED;
			break;
		case RELEASECHECK:
			if (pb == UP) {
				// start debouncing the up transition
				timer = DEBOUNCETIME;
				*state = UPTIMING;
			}
			break;
			
		// These two states are for the Hold recovery to avoid a
		// WasPressed output after a Hold output
		case UPTIMING2:
			if (timer > 0) timer--;
			else *state = INACTIVE;
			break;
		case RELEASECHECK2:
			if (pb == UP) {
				// start debouncing the up transition
				timer = DEBOUNCETIME;
				*state = UPTIMING2;
			}
			break;
			
		case PRESSED:
			// check for held status
			if (pb == DOWN && timer > 0) {
				// check how long button is pressed
				timer--;
				if (timer == 0) *state = HOLD;
			} else *state = RELEASECHECK; 		// button was released before 5 seconds
			break;	
		default:
			// Hold or WasPressed state		
			break;
	}
	*tim = timer;	
}		

//********************************************************************************
/**
* \details  Interrupt-driven pushbutton scan function.  See also the PWM module
*			for details on the interrupt handler that calls this function.
* \author   Michael Griebling
* \date   	10 Nov 2011
*/ 
//********************************************************************************
void PushButtons_Scan (void) {
	// Routine should be called every 10 milliseconds
	Scan(&b1State, &b1Timer, PB1);		// push button 1 state machine/scanning
	Scan(&b2State, &b2Timer, PB2);		// push button 2 state machine/scanning		
}

//********************************************************************************
/**
* \details  Clears the pushbutton state machine for the \em button.  It is 
*			necessary to call this function to acknowledge that the pushbutton 
*			state has been seen and to reset the state machine to capture the 
*			next pushbutton event.
* \author   Michael Griebling
* \date   	10 Nov 2011
*/ 
//********************************************************************************
void PushButtons_Clear (unsigned char button) {
	// Clear any latched conditions
	if (button & BUTTON1) {
		if (b1State == HOLD) b1State = RELEASECHECK2;	// release hold state
		else b1State = INACTIVE;						// release was pressed state
	}
	if (button & BUTTON2) {
		if (b2State == HOLD) b2State = RELEASECHECK2;	// release hold state
		else b2State = INACTIVE;						// release was pressed state
	}
}	

//********************************************************************************
/**
* \details  Returns \em TRUE iff the \em button is in a \em PRESSED state.
* \author   Michael Griebling
* \date   	10 Nov 2011
*/ 
//********************************************************************************
BOOL PushButtons_Pressed (unsigned char button) {
	if ((button & BUTTON1) && (b1State == WASPRESSED)) return TRUE;
	if ((button & BUTTON2) && (b2State == WASPRESSED)) return TRUE;
	return FALSE;	
}

//********************************************************************************
/**
* \details  Returns \em TRUE iff the \em button is in a \em HOLD state.
* \author   Michael Griebling
* \date   	10 Nov 2011
*/ 
//********************************************************************************
BOOL PushButtons_Held (unsigned char button) {
	if ((button & BUTTON1) && (b1State == HOLD)) return TRUE;
	if ((button & BUTTON2) && (b2State == HOLD)) return TRUE;
	return FALSE;	
}

//********************************************************************************
/**
* \details  Returns \em TRUE iff the \em button is either in a \em PRESSED or
*			\em HOLD state.
* \author   Michael Griebling
* \date   	10 Nov 2011
*/ 
//********************************************************************************
BOOL PushButtons_Active (unsigned char button) {
	return (PushButtons_Pressed(button) || PushButtons_Held(button));	
}
	

