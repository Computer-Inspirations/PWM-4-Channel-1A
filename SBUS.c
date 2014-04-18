//************************************************************************************
//
// This source is Copyright (c) 2011 by Computer Inspirations.  All rights reserved.
// You are permitted to modify and use this code for personal use only.
//
//************************************************************************************
/**
* \file   	SBUS.c
* \details  This module implements the Simple Bus Exchange Protocol (SBUS) which is 
*			loosely based on the ASCII Modbus protocol.  The overall message format
*			consists of a start character ":", followed by a two-character hexadecimal
*			(single byte) device address, and then a two-character hexadecimal message 
*			type, followed by four or more characters of hexadecimal command-dependent 
*			data, and, one byte of checksum which is currently not used and
*			always set to "00", which is finally terminated by a <CR><LF>.  Every 
*			message generates a reply which repeats the 
*			first part of the sent message and then follows with the length word or 
*			an error code.  Only the device whose address matches the message will
*			reply.  A broadcast address of "FF" is responded to by all devices so it
*			is the user's responsibility to only use the broadcast mode when only
*			a single device is on the RS-485 bus to avoid bus contention.
*
*			For example, to request a status report, the following ASCII string
*			(without quotes) would be sent: ":FF60FFFF00"<CR><LF> and this reply is 
*			received: ":FF60FFFF00050501680000003DFF003D00"<CR><LF>.  Refer to the 
*			user manual or code for more details on the protocol commands.
* \author   Michael Griebling
* \date   	10 Nov 2011
*/ 
//************************************************************************************

#include <string.h>
#include "SBUS.h"
#include "RS485.h"
#include "Sequences.h"
#include "EEPROM.h"
#include "MemoryMap.h"
#include "NightSense.h"
#include "Macros.h"
#include "PWM.h"

#define CR			(0x0D)
#define LF			(0x0A)
#define READSEGS	(0x10)
#define WRITESEGS	(0x20)
#define RUNSEGS		(0x30)
#define ERASESEGS	(0x40)
#define CONFIGURE	(0x50)
#define REPORT		(0x60)
#define READMACROS	(0x70)
#define WRITEMACROS	(0x80)
#define DISPLAY		(0x90)

#define TIMEOUT		(500)		// time-out between characters in mS
#define ERROR		(0xFFFF)
#define ERRSTATUS	(0xEF00)

extern unsigned int activeSequence;			// active sequence to play (defined in main.c)
extern unsigned int maxAddress, minAddress;	// sequence start and end address (defined in main.c)
extern BOOL override;						// override outputs via SBUS (defined in main.c)
extern BOOL playMacros;						// play EEPROM macros if TRUE (defined in main.c)

static unsigned char parameters[256];
static unsigned char deviceAdd;	

void SBUS_Init (void) {
	RS485_Init();
	deviceAdd = eeprom_read(DEVICEADD);		// protocol address 
}

static BOOL checkChar (unsigned char expectedChar) {
	unsigned int timer = TIMEOUT;
	unsigned char ch = 0x00;
	
	while (timer > 0 && ch != expectedChar) {
		if (RS485_CharReady()) {
			ch = RS485_ReadChar();
			timer = TIMEOUT;
		}	
		__delay_ms(1); timer--;
	}
	return (ch == expectedChar);	
}

static BOOL getChar (unsigned char * receivedChar) {
	unsigned int timer = TIMEOUT;
	
	while (timer > 0) {
		if (RS485_CharReady()) {
			*receivedChar = RS485_ReadChar();
			return TRUE;
		}	
		__delay_ms(1); timer--;
	}
	return FALSE;	
}

static unsigned char toHex (unsigned char nibble) {
	if (nibble < 10) return ('0' + nibble);
	else return (('A' - 10) + nibble);
}

static unsigned char fromHex (unsigned char hex) {
	if (hex >= 'A' && hex <= 'F') return (hex - ('A' - 10));
	else if (hex >= 'a' && hex <= 'f') return (hex - ('a' - 10));
	else if (hex >= '0' && hex <= '9') return (hex - '0');
	else return 0;
}	

static unsigned int getByte (void) {
	unsigned char ch;
	unsigned char byte;
	
	if (getChar(&byte)) {
		if (getChar(&ch)) return ((fromHex(byte) << 4) | fromHex(ch));
	}
	return ERROR;	
}

static unsigned int getWord (void) {
	unsigned int word = getByte();
	unsigned int result;
	
	if (word == ERROR) return ERROR;
	result = getByte();
	if (result == ERROR) return ERROR;
	return ((word << 8) | result);	
}

static void sendByte (unsigned char byte) {
	unsigned char buf[2];
	
	buf[0] = toHex(byte >> 4);
	buf[1] = toHex(byte & 0x0F);
	RS485_Write(buf, 2); 
}

static void sendWord (unsigned int word) {
	sendByte(word >> 8);
	sendByte(word & 0xFF);
}

static void sendPrefix (unsigned char id, unsigned char cmd, unsigned int address) {
	RS485_WriteChar(':');
	sendByte(id);
	sendByte(cmd);
	sendWord(address);
	cmd = address >> 8;
}

static void sendString (const unsigned char str[]) {
	RS485_Write((unsigned char *)str, strlen(str)); 
}

static void endOfMessage (void) {
	sendString("00\r\n");   // end of message
}

static unsigned int readParameters (void) {
	unsigned int command, length;

	length = 0;
	for (;;) {
		command = getByte();
		if (command == ERROR || length>255) break;
		parameters[length++] = command;
	}
	return (length-2);
}

static sendReportItem (unsigned int item) {
	unsigned int length;
	unsigned char onTime, offTime;
	BOOL flag;
	
	NightSense_GetParam(&flag, &length, &onTime, &offTime);
	switch (item) {
		case STATEADD: sendByte(flag); break;
		case OFFTIMEADD: sendByte(offTime); break;
		case ONTIMEADD: sendByte(onTime); break;
		case DURATIONADD: sendWord(length); break;
		case STARTSEQADD:
		case TOTALSEQADD: sendWord(ReadWord(item)); break;
		case DEVICEADD: sendByte(deviceAdd); break;
		case (DEVICEADD+1): sendWord(Seq_Count()); break;
		default: break;
	}
}

static sendAllReportItems () {
	unsigned int item;
	
	for (item=STATEADD; item<(DEVICEADD+2); item++) {
		sendReportItem(item);
	}
}							

void SBUS_Process_Command (void) {
	unsigned char ch, onTime, offTime, deviceID;
	BOOL flag;
	unsigned int command, address, length, index, i, size;
	
	if (RS485_CharReady()) {
		ch = RS485_ReadChar();
		if (ch == ':') {
			// valid start of command
			deviceID = getByte();
			if (deviceID == 0xFF || deviceID == deviceAdd) {
				// received valid starting byte 'FF' or should be our internal address
				command = getByte();	// retrieve the next command byte
				address = getWord(); 	// retrieve the address
				switch (command) {
					case READSEGS:
						length = getWord();
						checkChar(LF);		// skip the LRC, CR, and LF	
									
						// read EEPROM or FLASH contents
						sendPrefix(deviceID, READSEGS, address); sendWord(length);
						if (command != ERROR) {
							while (length > 0) {
								size = Seq_CopyToBuffer(address, parameters);
								if (size == 0) break;
								sendByte(size);
								for (i=0; i<size; i++) sendByte(parameters[i]);  // send sequence data
								address++; length--;
							}
						} else sendWord(ERRSTATUS | READSEGS);
						break;
						
					case WRITESEGS:
						length = readParameters();
						
						// write the seqences to memory
						sendPrefix(deviceID, WRITESEGS, address);
						if (length >= BYTESPERSEQ) {
							index = 0;							
							while (length >= BYTESPERSEQ) {
								// extract RGBW, hold, fade and add to or create a sequence
								if (address != 0xFFFF) {
									if (!Seq_AddTo(address, &parameters[index+2], parameters[index+1], parameters[index])) 
										break;
								} else {
									if (!Seq_New(&parameters[index+2], parameters[index+1], parameters[index]))
										break;
									address = Seq_GetActive();  // any more blocks will be added to this sequence
								}
								length -= BYTESPERSEQ; index += BYTESPERSEQ;	
							}	
							if (length == 0) sendWord(index);
							else sendWord(ERRSTATUS | WRITESEGS);
						} else sendWord(ERRSTATUS | WRITESEGS);						
						break;
						
					case RUNSEGS:
						length = getWord();
						checkChar(LF);		// skip the LRC, CR, and LF
						
						// set up the run parameters
						sendPrefix(deviceID, RUNSEGS, address);
						if (Seq_Find(address) == FIND_OK && Seq_Find(address+length-1) == FIND_OK) {
							WriteWord (STARTSEQADD, address);
							WriteWord (TOTALSEQADD, length);
							minAddress = address;
							maxAddress = address+length-1;
							sendWord(length);
							activeSequence = minAddress;
							override = FALSE;
						} else sendWord(ERRSTATUS | RUNSEGS);
						break;
						
					case DISPLAY:
						length = getWord();
						checkChar(LF);		// skip the LRC, CR, and LF
						
						// set up the run parameters
						sendPrefix(deviceID, DISPLAY, address);
						override = TRUE;
						PWM_Set (address >> 8, address & 0xFF, length >> 8, length & 0xFF);
						sendWord(length);
						break;
						
					case ERASESEGS:
						length = getWord();
						checkChar(LF);		// skip the LRC, CR, and LF
						
						// erase the segments in this range
						sendPrefix(deviceID, ERASESEGS, address);
						if (Seq_Delete_Range (address, length)) sendWord(length);						
						else sendWord(ERRSTATUS | ERASESEGS);	
						break;
						
					case CONFIGURE:
						length = getWord();
						checkChar(LF);		// skip the LRC, CR, and LF
						
						// update the configuration parameter
						sendPrefix(deviceID, CONFIGURE, address);
						switch (address) {
							case STATEADD: NightSense_Enable(length != 0); break;
							case OFFTIMEADD: NightSense_SetOffDelay(length); break;
							case ONTIMEADD: NightSense_SetOnDelay(length); break;
							case DURATIONADD: NightSense_SetDuration(length); break;
							case STARTSEQADD:
							case TOTALSEQADD: WriteWord(address, length); break;
							case DEVICEADD: eeprom_write(address, length); deviceAdd = length; break;
							default: address = 0xFFFF;	
						}	
						if (address == 0xFFFF) sendWord(ERRSTATUS | CONFIGURE); 
						else sendWord(length);
						break;
						
					case REPORT:
						checkChar(LF);		// skip the LRC, CR, and LF
						
						// reply with this configuration parameter
						sendPrefix(deviceID, REPORT, address);
						if (address == 0xFFFF) sendAllReportItems();
						else sendReportItem(address);
						break;
						
					case READMACROS:
						length = getWord();
						checkChar(LF);		// skip the LRC, CR, and LF
						
						// set up the run parameters
						sendPrefix(deviceID, READMACROS, address);
						if (address == 0xFFFF) length = Macros_Count();
						if (length <= Macros_Count()) {
							sendWord(length);
							for (i=0; i<length; i++) {
								sendWord(Macros_Read(i));
							}	
						} else sendWord(ERRSTATUS | READMACROS);
						break;
						
					case WRITEMACROS:
						length = readParameters();
						
						// Write macros to EEPROM
						sendPrefix(deviceID, WRITEMACROS, address);
						if ((address == 0) && ((length>>1) < MAXMACROS)) {
							sendWord(length); address = length;
							while (length >= 2) {
								Macros_Add(((unsigned int)parameters[length] << 8) | parameters[length+1]);
								length -= 2;
							}
							if (address > 0) {
								activeSequence = 0;
								minAddress = activeSequence;
								maxAddress = activeSequence+Macros_Count()-1;
								WriteWord(STARTSEQADD, PLAYMACROS);	 	// enable macro playback
								playMacros = TRUE;
							}							
						} else sendWord(ERRSTATUS | WRITEMACROS);	
						break;
						
					default:
						checkChar(LF);	// ignore command
						break;
				}
				endOfMessage();						
			}	
		} else {
			// ignore everything up to next LF or time-out
			checkChar(LF);
		}
	}
	RS485_ClearBuffer();
}	
	

