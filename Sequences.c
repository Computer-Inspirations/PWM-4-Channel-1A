//************************************************************************************
//
// This source is Copyright (c) 2011 by Computer Inspirations.  All rights reserved.
// You are permitted to modify and use this code for personal use only.
//
//************************************************************************************
/**
* \file   	Sequences.c
* \details  The PWM sequence management is controlled by this module.  Using the
*			contained routines it is possible to define, delete, and read sequences
*			which are stored in external EEPROM.  Each sequence consists of of one
*			or more entries of fade, hold, and PWM settings for four channels. 
* \author   Michael Griebling
* \date   	10 Nov 2011
*/ 
//************************************************************************************

#include "Sequences.h"
#include "EEPROM.h" 

static unsigned int activeSeq;		// active sequence address
static unsigned int activeIndex;	// address of sequence in FLASH/EEPROM
static unsigned int lastSeq;		// last sequence address in FLASH/EEPROM
static unsigned int lastIndex;		// address of last sequence
static BOOL EEPROMPresent;			// set to TRUE if EEPROM is present

unsigned char MAGIC[] = {0x55, 0xAA};	// special value to check for EEPROM initialization

void Seq_Init (void) {
	// Initializes the sequence buffers, points to the first sequence (0), and verifies that EEPROM is
	// present and how many sequences are stored there.
	unsigned char buffer[2];
	unsigned int lastAdd;
	
	EEPROM_Init();
	activeSeq = 0; activeIndex = 0;
	EEPROMPresent = FALSE;
	if (EEPROM_Present()) {
		// Check if EEPROM needs initialization
		lastAdd = EEPROM_GetSize() - 2;
		EEPROMPresent = TRUE;
		EEPROM_Read(lastAdd, buffer, 2);
		if ((buffer[0] != MAGIC[0]) || (buffer[1] != MAGIC[1])) {
			Seq_DeleteAll();					// erase all sequences
			EEPROM_Write(lastAdd, MAGIC, 2);	// initialize EEPROM
		}
	}	
}

static unsigned int SkipToEnd (unsigned int add) {
	// Moves to the end of the active sequence
	unsigned char eechar;
	
	eechar = EEPROM_ReadChar(add);
	while (eechar != ENDMARK) {
		add += BYTESPERSEQ;
		eechar = EEPROM_ReadChar(add);
	}	
	return add;
}

FindResult Seq_Find (unsigned int seqNumber) {
	unsigned int add = 0;
	unsigned int seq;
	
	// Check if any sequences are defined
	if (EEPROM_ReadChar(0) == ENDMARK) {
		lastIndex = 0; lastSeq = 0; 
		return NO_SEQUENCES;
	}	
	
	// Look for sequence
	for (seq=0; seq<seqNumber; seq++) {
		add = SkipToEnd(add);
		
		if (EEPROM_ReadChar(add+1) == ENDMARK) {
			// update the last sequence variables
			lastSeq = seq; 
			lastIndex = add;
			if (seq == 0) return AT_LAST_SEQUENCE;
			
			// find the start of the sequence
			add--;
			while (EEPROM_ReadChar(add) != ENDMARK) add -= BYTESPERSEQ;
			lastIndex = add+1;
			return AT_LAST_SEQUENCE;
		}
		add++;	// skip end of sequence marker	
	}
	activeSeq = seqNumber;
	activeIndex = add;
	return FIND_OK;
}

unsigned int Seq_CopyToBuffer (unsigned int seqNumber, unsigned char buffer[]) {
	unsigned int seqStart, seqEnd, i;
	
	if (Seq_Find(seqNumber) == FIND_OK) {
		seqStart = activeIndex; seqEnd = SkipToEnd(seqStart);
		i = 0;
		while (seqStart < seqEnd) buffer[i++] = EEPROM_ReadChar(seqStart++);
		return i;
	}
	return 0;	
}	

unsigned int Seq_GetActive (void) {
	return activeSeq;
}

BOOL Seq_Next (BOOL repeat) {
	if (EEPROM_ReadChar(activeIndex+BYTESPERSEQ) != ENDMARK) {
		activeIndex += BYTESPERSEQ;
	} else {
		if (repeat) {
			// repeat the active sequence
			Seq_Find(activeSeq);
		} else {
			// advance to the next sequence	
			if (EEPROM_ReadChar(activeIndex+(BYTESPERSEQ+1)) == ENDMARK) return FALSE;
			activeIndex += BYTESPERSEQ+1; activeSeq++;
		}		
	}
	return TRUE;	
}

unsigned char Seq_GetPWM (unsigned char ch) {
	// Gets the PWM level for the active sequence associated with channel 'ch' where ch ranges from 0 to 3.
	// If no sequence is active, sequence 0 is accessed.
	return EEPROM_ReadChar(activeIndex+ch+2);
}

unsigned char Seq_GetHold (void) {
	// Gets the hold time for the active sequence. If no sequence is active, sequence 0 is accessed.
	return EEPROM_ReadChar(activeIndex+1);
}	

unsigned char Seq_GetFade (void) {
	// Gets the fade rate for the active sequence. If no sequence is active, sequence 0 is accessed.
	return EEPROM_ReadChar(activeIndex);
}

static void MoveBlock (unsigned int srcAdd, unsigned int destAdd, unsigned int total) {
	// Non-overlapping block move
	unsigned char buffer[64];
	
	while (total > sizeof(buffer)) {
		EEPROM_Read(srcAdd, buffer, sizeof(buffer)); srcAdd += sizeof(buffer);
		EEPROM_Write(destAdd, buffer, sizeof(buffer)); destAdd += sizeof(buffer);
		total -= sizeof(buffer);		
	}
	if (total > 0) {
		EEPROM_Read(srcAdd, buffer, total); 
		EEPROM_Write(destAdd, buffer, total);
	}	
}	

static void MoveBytes (unsigned int srcAdd, unsigned int destAdd, unsigned int total) {
	// Shift a 'total' number of bytes from the srcAdd to the destAdd in EEPROM.  Overlapping
	// memory areas are handled properly.  Any bytes moved beyond the end of memory are lost.
	unsigned int overlap = destAdd - srcAdd;
	if (destAdd > srcAdd && total > overlap) {
		MoveBlock(destAdd, destAdd+overlap, total - overlap);
		total = overlap;
	}	
	if (total > 0) MoveBlock(srcAdd, destAdd, total);
}		

BOOL Seq_AddTo (unsigned int seqNumber, unsigned char rgbw[], unsigned char hold, unsigned char fade) {
	// Adds to the sequence 'seqNumber'.  If the sequence doesn't exist or isn't writeable, a FALSE is returned.
	unsigned int sadd, eadd, size;
	unsigned char buffer[8];
	
	if (EEPROMPresent) {
		// set up the sequence contents
		buffer[0] = fade; buffer[1] = hold;
		buffer[2] = rgbw[0]; buffer[3] = rgbw[1];
		buffer[4] = rgbw[2]; buffer[5] = rgbw[3];
		buffer[6] = ENDMARK; size = BYTESPERSEQ+1;
		
		// make room for sequence
		if (Seq_Find(seqNumber) == FIND_OK) {
			// make room for new sequence data
			sadd = SkipToEnd(activeIndex);
			Seq_Find(EEMAX);									// move to very last sequence
			if (lastIndex > (EEPROM_GetSize() - 256)) return FALSE;
			eadd = SkipToEnd(lastIndex);
			MoveBytes(sadd, sadd+BYTESPERSEQ, eadd-sadd+2);		// make room for new addition
		} else {
			// add data to the end of all the sequences
			sadd = SkipToEnd(lastIndex);
			if (sadd != 0) sadd++;
			buffer[7] = ENDMARK; size++;
		}
		
		// write the new sequence addition
		EEPROM_Write(sadd, buffer, size);
		return TRUE;
	}
	return FALSE;	
}

BOOL Seq_Delete_Range (unsigned int seqStart, unsigned int seqEnd) {
	// Deletes the range of sequences from 'seqStart' to 'seqEnd'.  If the sequence doesn't exist or isn't 
	// writeable, a FALSE is returned.
	unsigned int startAdd, endAdd, lastAdd;
	
	if ((seqEnd >= seqStart) && EEPROMPresent) {
		if (Seq_Find(seqStart) == FIND_OK) {
			startAdd = activeIndex;
			if (Seq_Find(seqEnd) == FIND_OK) {
				// need to move all following sequences to startAdd
				endAdd = SkipToEnd(activeIndex);
				Seq_Find(EEMAX); lastAdd = SkipToEnd(lastIndex);	// go to last address in sequence
				if (lastAdd > endAdd) {
					MoveBytes(endAdd+1, startAdd, lastAdd-endAdd+1);
					return TRUE;
				}	
			} 
			// deleting everything from StartAdd to end
			// mark end of all sequences at startAdd
			EEPROM_WriteChar(startAdd, ENDMARK);
			return TRUE;
		}		
	}
	return FALSE;	
}

BOOL Seq_DeleteAll (void) {
	// Just write two markers at the beginning of EEPROM
	EEPROM_WriteChar(0, ENDMARK);
	EEPROM_WriteChar(1, ENDMARK);
	return TRUE;	
}		

BOOL Seq_New (unsigned char rgbw[], unsigned char hold, unsigned char fade) {
	// Creates a new sequence in EEPROM.  Sequences stored in EEPROM are numbered from 
	// EESTART to EEMAX. A TRUE is returned once the new sequence has been created and 
	// initialized. The created or overwritten sequence becomes active.  Flash sequences
	// cannot be altered with this function.
	return Seq_AddTo(EEMAX, rgbw, hold, fade);
}

unsigned int Seq_Count (void) {
	// Returns a count of all sequences in EEPROM
	unsigned int total = 0;
	
	if (Seq_Find(EEMAX) != NO_SEQUENCES) total = lastSeq + 1;
	return total;
}	

