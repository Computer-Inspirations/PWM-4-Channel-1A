//************************************************************************************
//
// This source is Copyright (c) 2011 by Computer Inspirations.  All rights reserved.
// You are permitted to modify and use this code for personal use only.
//
//************************************************************************************
/**
* \file   	main.c
* \details  This module implements the top-level RGBW user interface and responds to
*			pushbutton presses and/or the RS-485 command interface while also cycling
*			through the currently-defined sequences.  Depending on the state of the
*			\em STARTSEQADD location in internal EEPROM, either a set of macros will
*			be executed from internal EEPROM that point to the actual sequences in
*			external EEPROM or a contiguous run of sequences in external EEPROM will
*			be executed directly.
*			Pushbuttons can be used to define the macros (see below) or put the
*			RGBW hardware into a sleep mode (holding PB2 for 5 secs).  By default,
*			all the sequences in external EEPROM will be sequentially executed. 
* \author   Michael Griebling
* \date   	10 Nov 2011
*/ 
//************************************************************************************
#include <xc.h>

// PIC16F1829 Configuration Bit Settings

// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.

// CONFIG1
#pragma config FOSC = INTOSC    // Oscillator Selection (INTOSC oscillator: I/O function on CLKIN pin)
#pragma config WDTE = OFF       // Watchdog Timer Enable (WDT disabled)
#pragma config PWRTE = OFF      // Power-up Timer Enable (PWRT disabled)
#pragma config MCLRE = ON       // MCLR Pin Function Select (MCLR/VPP pin function is MCLR)
#pragma config CP = OFF         // Flash Program Memory Code Protection (Program memory code protection is disabled)
#pragma config CPD = OFF        // Data Memory Code Protection (Data memory code protection is disabled)
#pragma config BOREN = ON       // Brown-out Reset Enable (Brown-out Reset enabled)
#pragma config CLKOUTEN = OFF   // Clock Out Enable (CLKOUT function is disabled. I/O or oscillator function on the CLKOUT pin)
#pragma config IESO = ON        // Internal/External Switchover (Internal/External Switchover mode is enabled)
#pragma config FCMEN = ON       // Fail-Safe Clock Monitor Enable (Fail-Safe Clock Monitor is enabled)

// CONFIG2
#pragma config WRT = OFF        // Flash Memory Self-Write Protection (Write protection off)
#pragma config PLLEN = OFF      // PLL Enable (4x PLL disabled)
#pragma config STVREN = ON      // Stack Overflow/Underflow Reset Enable (Stack Overflow or Underflow will cause a Reset)
#pragma config BORV = LO        // Brown-out Reset Voltage Selection (Brown-out Reset Voltage (Vbor), low trip point selected.)
#pragma config LVP = OFF        // Low-Voltage Programming Enable (High-voltage on MCLR/VPP must be used for programming)

#include "Types.h"
#include "SBUS.h"
#include "PWM.h"
#include "Pushbuttons.h"
#include "Sequences.h"
#include "NightSense.h"
#include "MemoryMap.h"
#include "Macros.h"
#include "EEPROM.h"
#include <stdlib.h>

// Temporarily define FLASHCOPY to initialize the external EEPROM with the contents
// of the Sequences.inc file.  Two blue flashes indicate the programming is done.
// Once the EEPROM is programmed, comment out the FLASHCOPY define to restore normal
// program functions.  This kludge is necessary because both the program and sequences
// can't fit into Flash at the same time.
//#define FLASHCOPY

#ifdef FLASHCOPY
#include "Sequences.inc"
#endif

#define FW_VERSION	(2)

// Initial internal EEPROM default contents
// Default data definitions for internal EEPROM:
//  state = Night mode on, 
// 	offDelayTime = 5 mins,
//	onDelayTime = 5 mins, 
//	duration = 360 mins = 6 hrs
//  start Sequence = 0
//  last Sequence = 61
//  device Address = FF
//  macro area blank (no macros to play)
__EEPROM_DATA(0x01, 0x05, 0x05, 0x01, 0x68, 0x00, 0x00, 0x00);	// Internal state variables - 16 bytes
__EEPROM_DATA(61, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF);
__EEPROM_DATA(0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF);	// EEPROM macro sequences - 200 bytes

unsigned int activeSequence;			// active sequence to play
unsigned int maxAddress, minAddress;	// sequence start and end address
BOOL override;							// override outputs via SBUS
BOOL playMacros;						// play EEPROM macros if TRUE
	

// Wait for Sequence to finish playing while also scanning the pushbuttons and 
// handling any external commands
BOOL Scan (void) {
	unsigned int i;
	
	do {
		PushButtons_Scan();			// update pushbutton status
		for (i=0; i<10; i++) {
			SBUS_Process_Command();	// handle protocol commands
			__delay_ms(1);
		}
		if (PushButtons_Active(BUTTON1|BUTTON2)) return TRUE;
//		if (PushButtons_Active(BUTTON2)) return TRUE;
	} while (PWM_Busy());
	return FALSE;	
}

void ShowNumber (unsigned int version) {
	while (version > 0) {
		PWM_Ramp (0, 0, 255, 0, 0, 5);		// Flash blue
		Scan();
		PWM_Ramp (0, 0, 0, 0, 0, 20);		// Off
		Scan();
		version--;
	}
	
	// Channel test sequence
	PWM_Ramp (255, 0, 0, 0, 0, 5);		// Flash red
	Scan();
	PWM_Ramp (0, 0, 0, 255, 0, 5);		// Flash white
	Scan();
	PWM_Ramp (0, 255, 0, 0, 0, 5);		// Flash green
	Scan();
	PWM_Ramp (0, 0, 255, 0, 0, 5);		// Flash blue
	Scan();	
	PWM_Ramp (0, 0, 0, 0, 0, 5);		// Off
	Scan();
}

void ConfirmCommand (void) {
	PWM_Set (0, 0, 255, 0);				// Blue flash
	__delay_ms(500);	
	PWM_Set (0, 0, 0, 0);
	__delay_ms(250);	
}

void Error (void) {
	PWM_Set (255, 0, 0, 0);				// Red flash
	__delay_ms(2000);	
	PWM_Set (0, 0, 0, 0);
	__delay_ms(250);	
}		

// Play the 'sequence' numbered FLASH or EEPROM sequence.
void PlaySequence (unsigned int sequence) {
	BOOL ok;
	
	if (Seq_Find(sequence) != FIND_OK) {
		Error(); Scan(); return;
	}	 	
	do {
		PWM_Ramp (Seq_GetPWM(0), Seq_GetPWM(1), Seq_GetPWM(2), Seq_GetPWM(3), Seq_GetFade(), Seq_GetHold());
		if (Scan()) {
			PWM_Set(0, 0, 0, 0);
			return;         		// handle push buttons
		}	
		ok = Seq_Next(NOREPEAT);
	} while ((Seq_GetActive() == sequence) && ok);
}	

#ifndef FLASHCOPY
static void InitMode (void) {
	unsigned int total;

	// Validate the start/total sequence values
	activeSequence = ReadWord(STARTSEQADD);
	playMacros = FALSE;
	if (activeSequence == PLAYMACROS) {
		// play back internal EEPROM macros
		playMacros = TRUE;
		activeSequence = 0;
		total = Macros_Count();
	} else {
		// determine EEPROM maximum
		total = Seq_Count();
		minAddress = ReadWord(TOTALSEQADD);
		if (minAddress < total) total = minAddress;
		if (Seq_Find(activeSequence) != FIND_OK || (minAddress == 0)) {
			activeSequence = 0;
		}
	}
	minAddress = activeSequence; 
	maxAddress = activeSequence+total-1;
}	


void DefineEEMacros (void) {
	unsigned int sequence = 0;
	unsigned int prevStart;

	PWM_Set (0, 0, 0, 0);	
	Seq_Find(sequence);
	prevStart = ReadWord(STARTSEQADD);
	if (prevStart == PLAYMACROS) prevStart = 0;	
	do {
		PlaySequence(sequence);
		if (PushButtons_Pressed(BUTTON1)) {
			// advance to next sequence	
			PWM_Set (0, 0, 0, 0);
			PushButtons_Clear(BUTTON1);
			sequence++;
			if (sequence >= Seq_Count()) sequence = 0;
			Seq_Find(sequence);
		}
		if (PushButtons_Pressed(BUTTON2)) {
			// add sequence to EEPROM	
			PWM_Set (0, 0, 0, 0);
			PushButtons_Clear(BUTTON2);
			if (Macros_Add(sequence)) {
				ConfirmCommand();
			} else Error();
		}	
	} while (!PushButtons_Held(BUTTON2));
	PushButtons_Clear(BUTTON2);
	
	if (Macros_Count() != 0) {
		WriteWord(STARTSEQADD, PLAYMACROS);	 	// enable macro playback
	} else {
		// restore previous playback mode
		WriteWord(STARTSEQADD, prevStart);		// enable normal playback		
	}	
	ConfirmCommand();
	ConfirmCommand();
	InitMode();	
}

static void DoSleep (void) {
	while (PWM_Busy());				// wait for PWM to complete
	PWM_Set(0, 0, 0, 0);
	__delay_ms(50);
	
	TRISBbits.TRISB4  = 1;			// change SDA to input temporarily
	TRISBbits.TRISB6  = 1;			// change SCL to input temporarily
	
	TRISAbits.TRISA0  = 1;			// temporarily make JTAG pins inputs
	TRISAbits.TRISA1  = 1;
	WPUAbits.WPUA0 = 1;				// enable pull-ups
	WPUAbits.WPUA1 = 1;	
	IOCANbits.IOCAN0 = 1;			// enable interrupt on negative edge on pin A0	
	
   	TRISBbits.TRISB5 = 1;			// Set as pulled up digital inputs
   	WPUBbits.WPUB5 = 1;	
   		
	TMR4IE = 0;						// Disable Timer4 interrupts 
	TMR6IE = 0;						// Disable Timer6 interrupts 

    FVRCON = 0;						// Disable voltage reference
    IOCIE = 1;						// Enable I/O interrupts
    
//*************************************************************************************     	  				
	SLEEP();						// processor goes into sleep mode and clears watchdog
//*************************************************************************************				
	NOP();
	IOCIE = 0;
	SBUS_Init();
	PWM_Init();
	PushButtons_Init();
	NightSense_Init();
					
	ConfirmCommand();
}

#else

extern unsigned char MAGIC[];

void CopyFlashToEEPROM (void) {
	unsigned char compare;
	unsigned int i;
	
	// Copies the contents of FLASH in Sequences[] to EEPROM
	if (EEPROM_Present()) {
		// Write Sequences data to external EEPROM
		EEPROM_Write(0x0000, (unsigned char *)Sequences, sizeof(Sequences));
		
		// Verify the external EEPROM contents
		for (i=0; i<sizeof(Sequences); i++) {
			compare = EEPROM_ReadChar(i);
			if (compare != Sequences[i]) {
				 Error();
				 return;	// abort	
			}	
		}
		EEPROM_Write(EEPROM_GetSize()-2, MAGIC, 2);	// initialize EEPROM magic number
		
		// Set up internal EEPROM start address and sequence length
		WriteWord(STARTSEQADD, 0x0000);			// enable normal playback		
		WriteWord(TOTALSEQADD, Seq_Count());	// identify how many sequences are defined
		ConfirmCommand();	
	} else {
		Error();
	}			
}

#endif					

main() {
	
	// Select the internal 4MHz oscillator
	OSCCONbits.IRCF = 0b1101;		// 4MHz clock select
	OSCCONbits.SCS = 0b11;			// Internal oscillator
	OSCCONbits.SPLLEN = 0;			// x4 PLL disabled

	SBUS_Init();
	EEPROM_Init();
	Macros_Init();
	PWM_Init();
	PushButtons_Init();
	NightSense_Init();
	Seq_Init();						// Start out at the first sequence
	
//	opMode = LIGHTING_MODE;	
 	override = FALSE;
	
	// Play FLASH/EEPROM sequences or macros from internal EEPROM, changes operating modes, or define EEPROM macros
#ifndef FLASHCOPY
	InitMode();
#else
	CopyFlashToEEPROM();
#endif

	// Display the firmware version number
	ShowNumber(FW_VERSION);

	for (;;) {
//		if (NightSense_IsNight()) {
//			if (opMode == HALLOWEEN_MODE) {
//				amp = rand() % 120 + 135;
//				delay = rand() % 100;
//				mode = rand() % 1000;
//				if (mode == 64) {
//					delay += 2000;
//					PWM_Set (0, 0, 0, 0);										// Black
//					for (i=0; i<delay; i++) __delay_ms(1);
//				} else if (mode == 78) {
//					delay += 500;
//					PWM_Set (255, 255, 255, 0);									// White
//					for (i=0; i<delay; i++) __delay_ms(1);
//					PWM_Set (0, 0, 0, 0);										// Black
//					for (i=0; i<delay/2; i++) __delay_ms(1);
//					PWM_Set (255, 255, 255, 0);									// White
//					for (i=0; i<delay/4; i++) __delay_ms(1);
//					PWM_Set (0, 0, 0, 0);										// Black
//					for (i=0; i<delay/2; i++) __delay_ms(1);
//					PWM_Set (255, 255, 255, 0);									// White
//					for (i=0; i<delay/8; i++) __delay_ms(1);
//					PWM_Set (0, 0, 0, 0);										// Black
//					for (i=0; i<delay; i++) __delay_ms(1);
//				} else {	
//					PWM_Set (0, (255*amp)/256, 0, 0);		// Green flame
//					for (i=0; i<delay; i++) __delay_ms(1);
//				}
//			} else if (opMode == CHRISTMAS_MODE) {
//				PWM_Ramp(255, 0, 0, 0, 0, 254);			// Red - 10 secs
//				Scan ();
//				PWM_Ramp(0, 255, 0, 0, 254, 254);  		// Green - Fade/Hold
//				Scan ();
//				PWM_Ramp(0, 0, 255, 0, 254, 254);  		// Blue - Fade/Hold
//				Scan ();
//				PWM_Ramp(255, 0, 0, 0, 254, 254);  		// Red - Fade/Hold
//				Scan ();
//			} else {
//				// normal lighting mode
//				PWM_Set (255, 255, 255, 255);									// White
//			}
//			
//			// check for mode changes
//			PushButtons_Scan();			// update pushbutton status
//			__delay_ms(10);
//			if (PushButtons_Pressed(BUTTON2)) {
//				// Change operating modes
//				PushButtons_Clear(BUTTON2);
//				ConfirmCommand();
//				opMode++;
//				if (opMode > LIGHTING_MODE) opMode = 0;
//			}		
//		} else {
//			PWM_Ramp (0, 0, 0, 0, 1, 0);
//		}
		if (NightSense_IsNight()) {
			if (!override) {
				if (playMacros) PlaySequence(Macros_Read(activeSequence));
				else PlaySequence(activeSequence);
				if (activeSequence < maxAddress) activeSequence++;
				else activeSequence = minAddress;
			} else {
				Scan();
			}
		} else {
			if (!override) PWM_Ramp (0, 0, 0, 0, 1, 0);
			Scan();
		}

#ifndef FLASHCOPY		
		// handle pushbuttons
		if (PushButtons_Pressed(BUTTON2)) {
			// Change operating modes
			PushButtons_Clear(BUTTON2);
			ConfirmCommand();
			DefineEEMacros();
		}
		if (PushButtons_Held(BUTTON2)) {
			PushButtons_Clear(BUTTON2);
			DoSleep();
		}
#endif	
	}
}


