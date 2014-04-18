#ifndef _SEQUENCES_H_
#define _SEQUENCES_H_

#include "Types.h"

#define REPEAT		TRUE			// arguments for Seq_Next repeat parameter
#define NOREPEAT	FALSE

#define EEMAX		1000			// maximum sequence count in EEPROM
#define ENDMARK		255
#define BYTESPERSEQ	  6

// Search result codes
typedef enum _FindResult {	
	FIND_OK, AT_LAST_SEQUENCE, NO_SEQUENCES
} FindResult;

extern const unsigned char Sequences[];

extern void Seq_Init (void);
// Initializes the sequence buffers, points to the first sequence (0), and verifies that EEPROM is
// present and how many sequences are stored there.

extern FindResult Seq_Find (unsigned int seqNumber);
// Find the sequence 'seqNumber'.  As a convention, sequences in Flash are numbered 0 to 255.  Sequences
// stored in EEPROM are numbered from 256 to 65535.  AT_LEAST_SEQUENCE is returned if the sequence 
// doesn't exist, NO_SEQUENCES is returned if no sequences are defined, and FIND_OK is returned if the
// sequence was found.

extern unsigned int Seq_CopyToBuffer (unsigned int seqNumber, unsigned char buffer[]);
// Find the sequence 'seqNumber' and copy it into the 'buffer'.  FALSE is returned if the sequence 
// doesn't exist; TRUE is returned otherwise.

extern unsigned int Seq_GetActive (void);
// Returns the active sequence number from 0 to 65535.

extern BOOL Seq_Next (BOOL repeat);
// Advance to the next part of the active sequence.  If repeat is TRUE, the active sequence is
// repeated; otherwise, the next sequence is activated once at the end of the current sequence. 
// FALSE is returned if the end of all the sequences is reached.

extern unsigned char Seq_GetPWM (unsigned char ch);
// Gets the PWM level for the active sequence associated with channel 'ch' where ch ranges from 0 to 3.
// If no sequence is active, sequence 0 is accessed.

extern unsigned char Seq_GetHold (void);
// Gets the hold time for the active sequence. If no sequence is active, sequence 0 is accessed.

extern unsigned char Seq_GetFade (void);
// Gets the fade rate for the active sequence. If no sequence is active, sequence 0 is accessed.

extern BOOL Seq_New (unsigned char rgbw[], unsigned char hold, unsigned char fade);
// Creates a new sequence in EEPROM with 1 segment.  Sequences stored in EEPROM are 
// numbered from EESTART to EEMAX. A TRUE is returned once the new sequence has been created and 
// initialized. The created or overwritten sequence becomes active.  Flash sequences
// cannot be altered with this function.

//extern BOOL Seq_New_Multi (unsigned char rgbw[], unsigned char hold[], unsigned char fade[], unsigned char blocks);
// Creates a new sequence in EEPROM with "blocks" segments.  Sequences stored in EEPROM are 
// numbered from EESTART to EEMAX. A TRUE is returned once the new sequence has been created and 
// initialized. The created or overwritten sequence becomes active.  Flash sequences
// cannot be altered with this function.

extern BOOL Seq_AddTo (unsigned int seqNumber, unsigned char rgbw[], unsigned char hold, unsigned char fade);
// Adds to the sequence 'seqNumber'.  If the sequence doesn't exist or isn't writeable, a FALSE is returned.

//extern BOOL Seq_AddToMulti (unsigned int seqNumber, unsigned char rgbw[], unsigned char hold[], 
//							unsigned char fade[], unsigned char blocks);
// Adds 'blocks' segments to the sequence 'seqNumber'.  If the sequence doesn't exist or isn't writeable, 
// a FALSE is returned.  Up to 13 segments can be added at one time.

extern BOOL Seq_Delete_Range (unsigned int seqStart, unsigned int seqEnd);
// Deletes the range of sequences from 'seqStart' to 'seqEnd'.  If the sequence doesn't exist or isn't 
// writeable, a FALSE is returned.

extern BOOL Seq_DeleteAll (void);
// Deletes all sequences in EEPROM.  Returns FALSE if sequences couldn't be deleted.

extern unsigned int Seq_Count (void);
// Returns a count of all sequences in EEPROM if 'EEPROM' is TRUE and all sequences defined in FLASH, otherwise

#endif