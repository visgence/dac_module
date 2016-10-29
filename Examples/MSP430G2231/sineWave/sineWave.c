/////////////////////////////////////////////////////////////////////////////////
//Sine Table Lookup
//This program creates a sine wave using the am 8bit lookup table
//Tested on MSP430G2 Launchpad
//
//Pins
// CS   => P1.4
// CLK  => P1.5
// MOSI => P1.6
//
///2016 Visgence Inc
/////////////////////////////////////////////////////////////////////////////////
#include <msp430.h>				
#include <stdint.h>

#define SS BIT4 //Slave Select is P1.4
#define SSOUT P1OUT //Save the Port for Slave select

//8bit Sin Table
const uint8_t sinetable[256] = {
  128,131,134,137,140,143,146,149,152,156,159,162,165,168,171,174,
  176,179,182,185,188,191,193,196,199,201,204,206,209,211,213,216,
  218,220,222,224,226,228,230,232,234,236,237,239,240,242,243,245,
  246,247,248,249,250,251,252,252,253,254,254,255,255,255,255,255,
  255,255,255,255,255,255,254,254,253,252,252,251,250,249,248,247,
  246,245,243,242,240,239,237,236,234,232,230,228,226,224,222,220,
  218,216,213,211,209,206,204,201,199,196,193,191,188,185,182,179,
  176,174,171,168,165,162,159,156,152,149,146,143,140,137,134,131,
  128,124,121,118,115,112,109,106,103,99, 96, 93, 90, 87, 84, 81,
  79, 76, 73, 70, 67, 64, 62, 59, 56, 54, 51, 49, 46, 44, 42, 39,
  37, 35, 33, 31, 29, 27, 25, 23, 21, 19, 18, 16, 15, 13, 12, 10,
  9,  8,  7,  6,  5,  4,  3,  3,  2,  1,  1,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  1,  1,  2,  3,  3,  4,  5,  6,  7,  8,
  9,  10, 12, 13, 15, 16, 18, 19, 21, 23, 25, 27, 29, 31, 33, 35,
  37, 39, 42, 44, 46, 49, 51, 54, 56, 59, 62, 64, 67, 70, 73, 76,
  79, 81, 84, 87, 90, 93, 96, 99, 103,106,109,112,115,118,121,124
};

void writeMCP492x(uint16_t data,uint8_t ss);

int main(void) {
	WDTCTL = WDTPW | WDTHOLD;		// Stop watchdog timer
	P1DIR |= 0x01;					// Set P1.0 to output direction

	//Set Slave Select Pin as output
	P1OUT = SS;
	P1DIR = SS;
	P1OUT |= SS;

	//Configure the SPI Registers to be master
	USICTL0 |= USIPE5 + USIMST + USIOE + USISWRST; // Port, SPI master
	USICTL1 |= USICKPH + USIIE;                     // Counter interrupt, flag remains set
	USICKCTL = USIDIV_0 + USISSEL_2;      // /16 SMCLK
	USICTL0 &= ~USISWRST;                 // USI released for operation

	USICNT = 1; // clear 1st transmit due to errata USI5
	__delay_cycles(50); // finish clearing (minimum (n+1)*16 cycles)
	USICTL0 |= USIPE7 + USIPE6; // Enable SDI/SDO pins.
	USICTL1 &= ~USIIFG;

	uint16_t i;

	//This is the main loop
	for(;;) {
		for (i=0;i<256;i++) {
			writeMCP492x(sinetable[i] * 16, SS);
		}
	}

	return 0;
}

void writeMCP492x(uint16_t data,uint8_t ss) {
  // Take the top 4 bits of config and the top 4 valid bits (data is actually a 12 bit number)
  //and OR them together
  uint8_t top_msg = (0x30 & 0xF0) | (0x0F & (data >> 8));

  // Take the bottom octet of data
  uint8_t lower_msg = (data & 0x00FF);

  // Select our DAC, Active LOW
  SSOUT &= ~ss;

  // Send first 8 bits
  USISRL = top_msg;
  USICNT = 8; // init-load counter
  while (!(USIIFG & USICTL1));

  // Send second 8 bits
  USISRL= lower_msg;
  USICNT = 8; // init-load counter
  while (!(USIIFG & USICTL1));

  //Deselect DAC
  SSOUT |= ss;
}

