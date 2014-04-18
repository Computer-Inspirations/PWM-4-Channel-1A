
#include "NightSense.h"
#include "MemoryMap.h"
#include "Macros.h"

#define	PERIOD		4							// Desired clock in Hz - 1/4 Sec
#define	SCALE		(64*16)						// Timer 6 prescale & postscale
#define	PRCOUNT		(IPERIOD/SCALE/PERIOD)

// Night sense state definitions
typedef enum _NightState {	
	DISABLED, ACTIVE=0x01, NIGHTTIMING, DAYTIMING, ISDARK
} NightState;

static unsigned char onDelayTime; 	// delay before turning on in minutes
static unsigned char offDelayTime;	// delay before turning off in minutes
static unsigned int duration;		// total on time in minutes
static unsigned int timer;
static NightState state;			// night sense state machine


void NightSense_UpdateState (void) {
	// Function called by interrupt every minute
	switch (state) {
		case ACTIVE:
			if (PORTAbits.RA4 != 0) {
				// dusk detected
				state = NIGHTTIMING;
				timer = onDelayTime;
				if (timer > 0) timer--;  // adjust by 1 min to account for this state's time
			}	
			break;
		case NIGHTTIMING:
			if (timer > 1) timer--;		 // "> 1" adjusts for extra minute due to count start at n	
			else {
				timer = duration;
				state = ISDARK;
			}	
			break;
		case ISDARK:
			if (timer > 1) timer--;		// "> 1" adjusts for extra minute due to count start at n
			if (PORTAbits.RA4 == 0) {
				// day detected
				state = DAYTIMING;
				timer = offDelayTime;
			}	
			break;
		case DAYTIMING:
			if (timer > 0) timer--;		
			else state = ACTIVE;
			break;
		default:
			// inactive state
			break;
	}
}		 	 

void NightSense_Init (void) {
	TRISAbits.TRISA4 = 1;			// set to input for optics sensor
	
	// Initialize Timer 6 for clock timing
	PR6 = PRCOUNT-1;				// PWM update period
	PIR3bits.TMR6IF = 0;			// Clear Timer6 interrupt flag bit
	T6CONbits.T6CKPS = 0b11;		// Set up Timer6 prescale to /64
	T6CONbits.T6OUTPS = 0b1111;		// Set up Timer6 postscale to /16
	TMR6IE = 1;						// Enable Timer6 interrupts
	T6CONbits.TMR6ON = 1;			// Enable Timer6
	
	// Read the EEPROM configuration data
	state = eeprom_read(STATEADD);
	offDelayTime = eeprom_read(OFFTIMEADD);
	onDelayTime = eeprom_read(ONTIMEADD);
	duration = ReadWord(DURATIONADD);
}	

void NightSense_Enable (BOOL on) {
	if (on) state = ACTIVE;
	else state = DISABLED;
	eeprom_write(STATEADD, state);
}	

BOOL NightSense_IsNight (void) {
	if (state == DISABLED) return TRUE;			// LEDs are always on
	return (((state == ISDARK) || (state == DAYTIMING)) && (timer > 0));	// LEDs on during nighttime
}	

void NightSense_SetOnDelay (unsigned char time) {
	onDelayTime = time;
	eeprom_write(ONTIMEADD, time);
}

void NightSense_SetOffDelay (unsigned char time) {
	offDelayTime = time;
	eeprom_write(OFFTIMEADD, time);
}

void NightSense_SetDuration (unsigned int time) {
	duration = time;
	WriteWord(DURATIONADD, time);	
}	

void NightSense_GetParam (BOOL *enabled, unsigned int *onTime, unsigned char *onDelay, unsigned char *offDelay) {
	*enabled = (state != DISABLED);
	*onDelay = onDelayTime;
	*onTime = duration;	
	*offDelay = offDelayTime;
}	

