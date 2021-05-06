// dac.c
// This software configures DAC output
// Lab 6 requires a minimum of 4 bits for the DAC, but you could have 5 or 6 bits
// Runs on TM4C123
// Program written by: Anvit Raju and Sarthak Gupta
//5/5/2021
// Lab number: 6
// Hardware connections
// TO STUDENTS "REMOVE THIS LINE AND SPECIFY YOUR HARDWARE********

#include <stdint.h>
#include "../inc/tm4c123gh6pm.h"
// Code files contain the actual implemenation for public functions
// this file also contains an private functions and private data

// **************DAC_Init*********************
// Initialize 4-bit DAC, called once 
// Input: none
// Output: none
void DAC_Init(void){
	SYSCTL_RCGCGPIO_R |= 0x02; //Turn on Port B clock (------1-)
	
	__asm__{ //wait
		NOP
		NOP
	}
	
	GPIO_PORTB_DIR_R |= 0x0F; //PB0-PB3 outputs (Set to 1) ----1111
	GPIO_PORTB_DEN_R |= 0x0F; //PB0-PB3 enable (Set to 1) ----1111
}

// **************DAC_Out*********************
// output to DAC
// Input: 4-bit data, 0 to 15 
// Input=n is converted to n*3.3V/15
// Output: none
void DAC_Out(uint32_t data){
	GPIO_PORTB_DATA_R = data; //output data to DAC (Port B data register)
}
