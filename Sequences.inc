//************************************************************************************
//
// This source is Copyright (c) 2011 by Computer Inspirations.  All rights reserved.
// You are permitted to modify and use this code for personal use only.
//
//************************************************************************************
/**
* \file   	Sequences.inc
* \details  This module defines the fault sequences that are stored in the external
*			EEPROM.
* \author   Michael Griebling
* \date   	10 Nov 2011
*/ 
//************************************************************************************

//	Sequence Data Format
// --------------------
//	fade_rate, hold_time, red, green, blue (all values in decimal)
//	values should be 0 <= value <= 255
//
//	End of current sequence
//	-----------------------
//	fade_rate =  255,tells code that this is then end of the current
//	sequence data.
//
//	End of all sequence data
//	------------------------
//	fade_rate and hold_time =  255,tells the code that this is the end
// 	of the current sequence data AND the end of all sequences.
//	
//	Fade Rate
//	---------
//	fade_rate = 0 --> no fade, new values update immediateley
//	fade_rate >0 and < 255,fades from current to new values. 
//	The colours fade from the current value to the new value
//	in steps of 1 (i.e. 0 to 100 requires 100 steps)
//	each step takes ~5mS x Fade Rate.
//
//	max fade time with increments of 5mS x fade_rate. 
//	  1 x 5mS x  255,= 1.27 secs
//	  2 x 5mS x  255,= 2.50 secs
//	  3 x 5mS x  255,= 3.85 secs
//	  4 x 5mS x  255,= 5.10 secs
//	  5 x 5mS x  255,= 6.35 secs
//   6 x 5mS x  255,= 7.60 secs
//   ......
// 254 x 5mS x  255,= 3m23s 
//
//	Hold Time
//	---------
//	How long to hold the current RGB colours before getting
// 	the next set of RGB values.
// 	hold_time = 0 --> no hold, jump to next step in sequence
//	hold_time >0 and < 255,hold for 50ms x hold_time
//	1   x 50mS = 50mS
//	254 x 50mS = 12.7 secs
//
//      |--------------------------- Fade Rate
//      |    |---------------------- Hold time     
//      |    |    |----------------- Red
//      |    |    |    |------------ Green
//      |    |    |    |    |------- Blue  
//      |    |    |    |    |    |-- White
//		|    |    |    |    |    |
const unsigned char Sequences[] = { 

	     6,   0, 64, 64, 64, 64,
	     6,   0,  5,  5,  5,  5,
	   255,		// end of sequence 01


	     4,   0, 128, 128, 128, 128,
	     6,   0,   5,   5,   5,   5,
	   255,		// end of sequence 02


	     8,   0, 0xFF, 0xFF, 0xFF, 0xFF,
	     9,   0, 0x1F, 0x1F, 0x1F, 0x1F,
	   255,		// end of sequence 03


	    10,   0, 128, 128, 128, 128,
	    10,   0,   5,   5,   5,   5,
	   255,		// end of sequence 04


	    14,   0, 128, 128, 128, 128,
	    15,   0,   5,   5,   5,   5,
	   255,		// end of sequence 05


	     3,   0, 128, 128, 128, 128,
	     3,   2,   0,   0,   0,   0,
	   255,		// end of sequence 06


	     5,   0, 128, 128, 128, 128,
	     5,   2,   0,   0,   0,   0,
	   255,		// end of sequence 07


	     2,   5, 255,   0,   0, 255,
	     1,   0,   0,   0,   0,	  0,
	     2,   5,   0, 255,   0,	255,
	     1,   0,   0,   0,   0,	  0,
	     2,   5,   0,   0, 255, 255,
	     1,   0,   0,   0,   0,   0,
	   255,		// end of sequence 08

	
	     2,   5, 255,   0,   0, 255,
	     1,   0,   0,   0,   0,	  0,
	     2,   5,   0, 255,   0,	255,
	     1,   0,   0,   0,   0,	  0,
	     2,   5,   0,   0, 255, 255,
	     1,   0,   0,   0,   0,   0,
	   255,		// end of sequence 09


//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Marker - Sequence 10	
	     0,   5, 254, 254, 254, 254, 
	     0,  20,   0,   0,   0,   0,
	   255,		                  // end of sequence	
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=


	     2,   1, 255,   0,   0, 255,
	     2,   1,   0, 255,   0, 127,
	     2,   1,   0,   0, 255,   0,
	   255,		// end of sequence 11
	
	     2,   1, 255,  64,   0, 255,
	     2,   1,  64,   0, 255, 255,
	     2,   1,   0, 255,  64, 255,
	   255,		// end of sequence 12


	     0, 254, 255,   0,   0,	 0, // Red
	    255,		// end of sequence 13


	     0, 254,   0, 255,   0,	 0, // Green
	   255,		// end of sequence 14


//	     0, 254,   0,   0, 255,	 0, // Blue
	     0, 127,   0,   0, 255,	 0, // Blue
	    255,		// end of sequence 15


	     0, 254, 240,   0, 240,	 0, // Purple
	    255,		// end of sequence 16


	     0, 254, 255, 155,   0,	 0, // Orange
	   255,		// end of sequence 17


	     0, 254, 240, 240, 240, 240, // White Bright
	    255,		// end of sequence 18


	     0, 254, 128, 128, 128, 128, // White Half
	    255,		// end of sequence 19


//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Marker - Sequence 20	
	     0,   5, 254, 254, 254, 254, 
	     0,   3,   0,   0,   0,   0,
	     0,   5, 254, 254, 254, 254,
	     0,  20,   0,   0,   0,   0,
	   255,		                  // end of sequence	
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=


	
	     0, 254,  48,  48,  58,	58, // White Low
	    255,		// end of sequence 21

	
	     6,  10, 255,   0,   0,	 0, // Spectum fade
	     6,  10, 255, 255,   0,  0,
	     6,  10,   0, 255,   0,  0,
	     6,  10,   0, 255, 255,  0,
	     6,  10,   0,   0, 255,  0,
	     6,  10, 255,   0, 255,  0,
	   255,	  // end of sequence 22
	
	
	    25, 254, 255,   0,   0,	 0, // Spectum fade slow
	    25, 254, 255, 255,   0,  0,
	    25, 254,   0, 255,   0,  0,
	    25, 254,   0, 255, 255,  0,
	    25, 254,   0,   0, 255,  0,
	    25, 254, 255,   0, 255,  0,
	   255,		// end of sequence 23
	
	
	     4,  10, 254,  32,   0,	 0, // warm 
	     4,  10, 254, 0xB2,   0,  0, 
	     4,  10, 254, 240,   0,  0,
	     4,  10, 0xB2, 240,   0,  0,
	   255,		// end of sequence 24
	
	
	     4,  10,   0,  16, 255,	0, // cool
	     4,  10,   0, 0xB2, 255, 0,
	     4,  10,   0, 240, 0xB2, 0,
	     4,  10,   16, 16, 240, 0,
	     4,  10,  240, 16, 240, 0,
	     4,  10,   64,  0, 250, 0,
	   255,		// end of sequence 25
		
	    10,   4, 240,   0, 240,	0, // purple
	    10,   4,  32,   0, 240, 0,
	    10,   4, 0xB2,   0, 0xB2, 0,
	    10,   4, 240,   0,  32, 0,
	   255,		 // end of sequence 26

	     0,   1, 255,   0,   0, 255, // Cycle Red, Green, Blue. Cycle rate every 50mS
	     0,   1,   0, 255,   0, 127,
	     0,   1,   0,   0, 255,   0,
	   255,		// end of sequence 27


	     0,   2, 255,   0,   0,  255, // Cycle Red, Green, Blue. Cycle rate every 100mS (1/10 second)
	     0,   2,   0, 255,   0,  127,
	     0,   2,   0,   0, 255,    0,
	   255,		// end of sequence 28


	     0,   4, 255,   0,   0,  255, // Cycle Red, Green, Blue. Cycle rate every 200mS (1/5 second)
	     0,   4,   0, 255,   0,  127,
	     0,   4,   0,   0, 255,    0,
	   255,		// end of sequence 29



//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Marker - Sequence 30	
	     0,   5, 254, 254, 254, 254, 
	     0,   3,   0,   0,   0,   0,
	     0,   5, 254, 254, 254, 254,
	     0,   3,   0,   0,   0,   0,
	     0,   5, 254, 254, 254, 254,
	     0,  20,   0,   0,   0,   0,
	   255,		                  // end of sequence	
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=



	     0,   6, 255,   0,   0,  255, // Cycle Red, Green, Blue. Cycle rate every 300mS  
	     0,   6,   0, 255,   0,  127,
	     0,   6,   0,   0, 255,    0,
	   255,		// end of sequence 31


	     0,   8, 255,   0,   0,  255, // Cycle Red, Green, Blue. Cycle rate every 400mS
	     0,   8,   0, 255,   0,  127,
	     0,   8,   0,   0, 255,    0,
	   255,		// end of sequence 32


	     0,  10, 255,   0,   0,  255, // Cycle Red, Green, Blue. Cycle rate every 500mS (1/2 second)
	     0,  10,   0, 255,   0,  127,
	     0,  10,   0,   0, 255,    0,
	   255,		// end of sequence 33


	     0,  20, 255,   0,   0,  255, // Cycle Red, Green, Blue. Cycle rate every 10mS (1 second) 
	     0,  20,   0, 255,   0,  127,
	     0,  20,   0,   0, 255,    0,
	   255,		// end of sequence 34
	

// Last three sequences indicate end-of-sequence marker for easy viewing
	     0,   5,  10,   0,   0,	 0,	// dim red 
	     0,   5,   0,   0,   0,	 0,	// off
	   255,		// end of sequence	35 
	
	
	     0,   5,   0,  10,   0,	 0,	// dim green 
	     0,   5,   0,   0,   0,	 0,	// off
	   255,		// end of sequence 36
	
			
	     0,   5,   0,   0,  10,	 0,	// dim blue 
	     0,   5,   0,   0,   0,	 0,	// off
	   255,		// end of sequence 37

// End of Original Firmware Code


// New Sequences 2009-03-20

	   	0, 2, 255, 0, 0, 0,	// Red to green wig-wag 100ms each color on
     	0, 2, 0, 255, 0, 0,
	    255,		// end of sequence 38

	   	0, 3, 255, 0, 0, 0,	// Red to green wig-wag 150ms each color on
     	0, 3, 0, 255, 0, 0,
	    255,		// end of sequence 39



//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Marker - Sequence 40	
	     0,   5, 254, 254, 254, 254,
	     0,   3,   0,   0,   0,   0,
	     0,   5, 254, 254, 254, 254,
	     0,   3,   0,   0,   0,   0,
	     0,   5, 254, 254, 254, 254,
	     0,   3,   0,   0,   0,   0,
	     0,   5, 254, 254, 254, 254,
	     0,  20,   0,   0,   0,   0,
	   255,		                  // end of sequence	
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
		0, 4, 255, 0, 0, 0,	// Red to green wig-wag 200ms each color on
     	0, 4, 0, 255, 0, 0,
	    255,		// end of sequence 41


	   	0, 2, 255, 0, 0, 255,	// Double flash red, pause, double flash green, pause and repeat (100ms on, 150ms off)
     	0, 3, 0, 0, 0, 0,
     	0, 2, 255, 0, 0, 255,
     	0, 3, 0, 0, 0, 0,
     	0, 2, 0, 255, 0, 255,
     	0, 3, 0, 0, 0, 0,
     	0, 2, 0, 255, 0, 255, 
     	0, 3, 0, 0, 0, 0,
	    255,		// end of sequence 42


	   	0, 3, 255, 0, 0, 255,	// Double flash red, pause, double flash green, pause and repeat (150ms on, 200ms off)
     	0, 4, 0, 0, 0, 0,
     	0, 3, 255, 0, 0, 255,
     	0, 4, 0, 0, 0, 0,
     	0, 3, 0, 255, 0, 255,
     	0, 4, 0, 0, 0, 0,
     	0, 3, 0, 255, 0, 255,
     	0, 4, 0, 0, 0, 0,
	    255,		// end of sequence 43



	   	0, 2, 255, 0, 0, 255,	// Triple flash red, pause, triple flash green, pause and repeat (100ms on, 150ms off)
     	0, 3, 0, 0, 0, 0,
     	0, 2, 255, 0, 0, 255,
     	0, 3, 0, 0, 0, 0,
     	0, 2, 255, 0, 0, 255,
     	0, 3, 0, 0, 0, 0,
     	0, 2, 0, 255, 0, 255,
     	0, 3, 0, 0, 0, 0,
     	0, 2, 0, 255, 0, 255, 
     	0, 3, 0, 0, 0, 0,
     	0, 2, 0, 255, 0, 255,
     	0, 3, 0, 0, 0, 0,
	    255,		// end of sequence 44

	   	0, 3, 255, 0, 0, 255,	// Triple flash red, pause, triple flash green, pause and repeat (150ms on, 200ms off)
     	0, 4, 0, 0, 0, 0,
     	0, 3, 255, 0, 0, 255,
     	0, 4, 0, 0, 0, 0,
     	0, 3, 255, 0, 0, 255,
     	0, 4, 0, 0, 0, 0,
     	0, 3, 0, 255, 0, 255,
     	0, 4, 0, 0, 0, 0,
     	0, 3, 0, 255, 0, 255, 
     	0, 4, 0, 0, 0, 0,
     	0, 3, 0, 255, 0, 255,
     	0, 4, 0, 0, 0, 0,
	    255,		// end of sequence 45


	   	1, 1, 0, 0, 127,	0, // Dark blue to purple throb 
	   	1, 1, 64, 0, 96,	0, 
	    255,		// end of sequence 46


	   	2, 1, 064, 127, 127, 0,	// Light blue to purple, slow throb
	   	2, 1, 064, 0, 127, 0,	//
	    255,		// end of sequence 47


	   	2, 1, 127, 127, 127,	127, // Pastel colors morphing 
	   	2, 1, 127, 127, 0,	 85, //
	   	2, 1, 127, 96, 0,	 74, //
	   	2, 1, 127, 96, 127,	117, //
	   	2, 1, 127, 64, 127,	106, //
	   	2, 1, 127, 32, 0,	 53, //
	   	2, 1, 127, 64, 0,	 64, //
	   	2, 1, 127, 64, 127,	106, //
	   	2, 1, 127, 32, 127,	 95, //
	   	2, 1, 127, 32, 0,	 53, //
	   	2, 1, 127, 0, 0,	     42, //
	   	2, 1, 127, 0, 127,	 85, //
	    255,		// end of sequence 48


	   	1, 1, 0, 0, 255,	  85, // Full spectrum fade over 1.27 seconds per color, hold color for 50mS
	   	1, 1, 0, 255, 255,   170, //
	   	1, 1, 0, 255, 0,	  85, //
	   	1, 1, 255, 255, 0, 	 170, //
	   	1, 1, 255, 255, 255, 255, //
	   	1, 1, 255, 0, 255,   170, //
	   	1, 1, 255, 0, 0,	  85, //
	    255,		                  // end of sequence 49



//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Marker - Sequence 50	
	     0,   5, 254, 254, 254, 254,
	     0,   3,   0,   0,   0,   0,
	     0,   5, 254, 254, 254, 254,
	     0,   3,   0,   0,   0,   0,
	     0,   5, 254, 254, 254, 254,
	     0,   3,   0,   0,   0,   0,
	     0,   5, 254, 254, 254, 254,
	     0,   3,   0,   0,   0,   0,
	     0,   5, 254, 254, 254, 254,
	     0,  20,   0,   0,   0,   0,
	   255,		                  // end of sequence	
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=



	   	2, 1, 0, 0, 255,	  85, // Full spectrum fade over 2.50 seconds per color, hold color for 50mS
	   	2, 1, 0, 255, 255,   170, //
	   	2, 1, 0, 255, 0,	  85, //
	   	2, 1, 255, 255, 0,   170, //
	   	2, 1, 255, 255, 255, 255, //
	   	2, 1, 255, 0, 255,	 170, //
	   	2, 1, 255, 0, 0,	  85, //
	    255,		                  // end of sequence 51

	   	4, 1, 0, 0, 255,	  85, // Full spectrum fade over 5.10 seconds per color, hold color for 50mS
	   	4, 1, 0, 255, 255,   170, //
	   	4, 1, 0, 255, 0,	  85, //
	   	4, 1, 255, 255, 0,   170, //
	   	4, 1, 255, 255, 255, 255, //
	   	4, 1, 255, 0, 255,	 170, //
	   	4, 1, 255, 0, 0,	  85, //
	    255,		                  // end of sequence 52

	   	8, 1, 0, 0, 255,	  85, // Full spectrum fade over 10.2 seconds per color, hold color for 50mS
	   	8, 1, 0, 255, 255,   170, //
	   	8, 1, 0, 255, 0,	  85, //
	   	8, 1, 255, 255, 0,   170, //
	   	8, 1, 255, 255, 255, 255, //
	   	8, 1, 255, 0, 255,	 170, //
	   	8, 1, 255, 0, 0,	  85, //
	    255,		                  // end of sequence 53

	   	1, 2, 0, 0, 255,	  85, // Full spectrum fade over 1.27 seconds per color, hold color for 100mS
	   	1, 2, 0, 255, 255,   170, //
	   	1, 2, 0, 255, 0,	  85, //
	   	1, 2, 255, 255, 0,   170, //
	   	1, 2, 255, 255, 255, 255, //
	   	1, 2, 255, 0, 255,	 170, //
	   	1, 2, 255, 0, 0,	  85, //
	    255,		                  // end of sequence 54


	   	2, 2, 0, 0, 255,	  85, // Full spectrum fade over 2.50 seconds per color, hold color for 100mS
	   	2, 2, 0, 255, 255,   170, //
	   	2, 2, 0, 255, 0,	  85, //
	   	2, 2, 255, 255, 0,   170, //
	   	2, 2, 255, 255, 255, 255, //
	   	2, 2, 255, 0, 255,	 170, //
	   	2, 2, 255, 0, 0,	  85, //
	    255,		                  // end of sequence 55

	   	4, 2, 0, 0, 255,	  85, // Full spectrum fade over 5.10 seconds per color, hold color for 100mS
	   	4, 2, 0, 255, 255,   170, //
	   	4, 2, 0, 255, 0,	  85, //
	   	4, 2, 255, 255, 0,   170, //
	   	4, 2, 255, 255, 255, 255, //
	   	4, 2, 255, 0, 255,	 170, //
	   	4, 2, 255, 0, 0,	  85, //
	    255,		                  // end of sequence 56

	   	8, 2, 0, 0, 255,	  85, // Full spectrum fade over 10.2 seconds per color, hold color for 100mS
	   	8, 2, 0, 255, 255,   170, //
	   	8, 2, 0, 255, 0,	  85, //
	   	8, 2, 255, 255, 0,   170, //
	   	8, 2, 255, 255, 255, 255, //
	   	8, 2, 255, 0, 255,	 170, //
	   	8, 2, 255, 0, 0,	  85, //
	    255,		                  // end of sequence 57
	
	
	   	2, 2, 204, 0, 255,  93,	// Purple morph in and back out
	   	2, 2, 32,  0,  64,  32,	//
	    255,		                  // end of sequence	58

	
	   	2, 2, 204, 255, 0,  93,	// Lime-green morph in and back out
	   	2, 2, 32,   64, 0,  32,	//
	    255,		                  // end of sequence 59


	   	2, 2, 255, 155, 0, 137, // Orange morph in and back out
	   	2, 2,  64, 32, 0,	32, //
	    255,		                  // end of sequence 60

// This sequence generates one flash of Blue LEDs to indicate version 1 of firmware	
	     0,   5, 0, 0, 255, 0,	// blue 
	     0,   20, 0, 0, 0,	0,	// off
	   255,		// end of sequence	61



	   255		// end of all data
	};


