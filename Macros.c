//************************************************************************************
//
// This source is Copyright (c) 2011 by Computer Inspirations.  All rights reserved.
// You are permitted to modify and use this code for personal use only.
//
//************************************************************************************
/**
* \file   	Macros.c
* \details  This module implements the macro features.  Macros provide a method of
*			allowing users without an RS-485 connection to reorder the sequences in
*			the external EEPROM without requiring any changes to EEPROM.  Of course,
*			if you have a compiler it will be simpler just to rearrange the sequences
*			in the Sequences.inc file and recompile the code.  The RS-485 interface
*			lets you erase, read, and write either the macros or the sequences in the
*			external EEPROM more conveniently without recompilation or manual
*			pushbutton entry of the macros.
* \author   Michael Griebling
* \date   	10 Nov 2011
*/ 
//************************************************************************************

#include "Macros.h"
#include "MemoryMap.h"

static unsigned int MaxMacros;

unsigned int ReadWord (unsigned char address) {
	unsigned int word = eeprom_read(address);
	word = (word << 8) | eeprom_read(address+1);
	return word;
}

void WriteWord (unsigned char address, unsigned int data) {
	eeprom_write(address, data >> 8);
	eeprom_write(address+1, data);
}

unsigned int Macros_Count (void) {
	// count defined macros in EEPROM
	return MaxMacros;
}	

BOOL Macros_Add (unsigned int macro) {
	unsigned int add = (MaxMacros<<1) + EESEQADD;
	if (MaxMacros < MAXMACROS) {
		WriteWord(add, macro);
		WriteWord(add+2, ENDMACRO);
		MaxMacros++;
		return TRUE;
	}
	return FALSE;
}
	
unsigned int Macros_Read(unsigned int macroID) {
	unsigned int add = (macroID<<1) + EESEQADD;
	if (macroID < MaxMacros) {
		return ReadWord(add);
	}
	return ENDMACRO;	
}

void Macros_Init(void) {
	// determine total defined macros
	unsigned char start = EESEQADD;
	
	while (ReadWord(start) != ENDMACRO) start += 2;
	MaxMacros = ((start - EESEQADD) >> 1);
}	


