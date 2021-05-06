// ADC.c
// Runs on TM4C123
// Provide functions that initialize ADC0
// Last Modified: 1/16/2021
// Student names: Anvit Raju and Sarthak Gupta
// Last modification date: 5/5/2021

#include <stdint.h>
#include "../inc/tm4c123gh6pm.h"

// ADC initialization function 
// Input: none
// Output: none
// measures from PD2, analog channel 5
void ADC_Init(uint32_t sac){ 

	SYSCTL_RCGCGPIO_R |= 0x08; //Turn Port D Clock on
	while ((SYSCTL_PRGPIO_R&0x08) == 0){}; //Wait for clock to stabilize
	
	//Set up PD2
	GPIO_PORTD_DIR_R &= ~0x04;  //make PD2 input
	GPIO_PORTD_AFSEL_R |= 0x04; //enable alt fun on PD2
	GPIO_PORTD_DEN_R &= ~0x04; //disable digital I/O on PD2
	GPIO_PORTD_AMSEL_R |= 0x04; //enable analog fun on PD2
	
	//Initialize ADC
	SYSCTL_RCGCADC_R |= 0x01; //Activate ADC0
	volatile unsigned long delay; 
	//Wait for clock to stabilize
	delay = SYSCTL_RCGCADC_R; 
	delay = SYSCTL_RCGCADC_R;
	delay = SYSCTL_RCGCADC_R;
	delay = SYSCTL_RCGCADC_R;
	ADC0_PC_R = 0x01; //configure for 125K
	ADC0_SSPRI_R = 0x0123; //SEQ 3 is highest priority
	ADC0_ACTSS_R &= ~0x0008; //disable sample sequencer 3
	ADC0_EMUX_R &= ~0xF000; //seq3 is software trigger
	ADC0_SSMUX3_R = (ADC0_SSMUX3_R&0xFFFFFFF0)+5; //Ain5 (PD2)
	ADC0_SSCTL3_R = 0x006; //no TS0 D0, yes IE0 KND0
	ADC0_IM_R &= ~0x0008; //disable SS3 interrupts
	ADC0_ACTSS_R |= 0x0008; //enable sample sequencer 3
	ADC0_SAC_R = sac;
}

//------------ADC_In------------
// Busy-wait Analog to digital conversion
// Input: none
// Output: 12-bit result of ADC conversion
// measures from PD2, analog channel 5
uint32_t ADC_In(void){  
	
	uint32_t data;
	ADC0_PSSI_R = 0x0008;
	while ((ADC0_RIS_R & 0x08) == 0) {};
	data = ADC0_SSFIFO3_R & 0xFFF;
	ADC0_ISC_R = 0x0008;
	return data;
		
}


