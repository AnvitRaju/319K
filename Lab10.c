// Lab10.c
// Runs on TM4C123
// Anvit Raju and Sarthak Gupta
// EE319K Lab 10

/* 
 Copyright 2021 by Jonathan W. Valvano, valvano@mail.utexas.edu
    You may use, edit, run or distribute this file
    as long as the above copyright notice remains
 THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
 OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 VALVANO SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL,
 OR CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 For more information about my classes, my research, and my books, see
 http://users.ece.utexas.edu/~valvano/
 */
// ******* Hardware I/O connections*******************
// Slide pot pin 1 connected to ground
// Slide pot pin 2 connected to PD2/AIN5
// Slide pot pin 3 connected to +3.3V 
// fire button connected to PE2
// play/pause button connected to PE1
// 8*R resistor DAC bit 0 on PB0 (least significant bit)
// 4*R resistor DAC bit 1 on PB1
// 2*R resistor DAC bit 2 on PB2
// 1*R resistor DAC bit 3 on PB3 (most significant bit)

// VCC   3.3V power to OLED
// GND   ground
// SCL   PD0 I2C clock (add 1.5k resistor from SCL to 3.3V)
// SDA   PD1 I2C data

//************WARNING***********
// The LaunchPad has PB7 connected to PD1, PB6 connected to PD0
// Option 1) do not use PB7 and PB6
// Option 2) remove 0-ohm resistors R9 R10 on LaunchPad
//******************************

#include <stdint.h>
#include "../inc/tm4c123gh6pm.h"
#include "../inc/CortexM.h"
#include "SSD1306.h"
#include "Print.h"
#include "Random.h"
#include "ADC.h"
#include "Images.h"
#include "Sound.h"
#include "Timer0.h"
#include "Timer1.h"
#include "TExaS.h"
#include "Switch.h"
//********************************************************************************
// debuging profile, pick up to 7 unused bits and send to Logic Analyzer
#define PB54                  (*((volatile uint32_t *)0x400050C0)) // bits 5-4
#define PF321                 (*((volatile uint32_t *)0x40025038)) // bits 3-1
// use for debugging profile
#define PF1       (*((volatile uint32_t *)0x40025008))
#define PF2       (*((volatile uint32_t *)0x40025010))
#define PF3       (*((volatile uint32_t *)0x40025020))
#define PB5       (*((volatile uint32_t *)0x40005080)) 
#define PB4       (*((volatile uint32_t *)0x40005040)) 
	
#define PE3210				(*((volatile uint32_t *)0x4002403C)) //bits 3,2,1,0
// TExaSdisplay logic analyzer shows 7 bits 0,PB5,PB4,PF3,PF2,PF1,0 
// edit this to output which pins you use for profiling
// you can output up to 7 pins
void LogicAnalyzerTask(void){
  UART0_DR_R = 0x80|PF321|PB54; // sends at 10kHz
}
void ScopeTask(void){  // called 10k/sec
  UART0_DR_R = (ADC1_SSFIFO3_R>>4); // send ADC to TExaSdisplay
}
// edit this to initialize which pins you use for profiling
void Profile_Init(void){
  SYSCTL_RCGCGPIO_R |= 0x22;      // activate port B,F
  while((SYSCTL_PRGPIO_R&0x20) != 0x20){};
  GPIO_PORTF_DIR_R |=  0x0E;   // output on PF3,2,1 
  GPIO_PORTF_DEN_R |=  0x0E;   // enable digital I/O on PF3,2,1
  GPIO_PORTB_DIR_R |=  0x30;   // output on PB4 PB5
  GPIO_PORTB_DEN_R |=  0x30;   // enable on PB4 PB5  
}

void SysTick_Init20Hz(){
	NVIC_ST_CTRL_R = 0; //disabling SysTick during setup
	NVIC_ST_RELOAD_R = 3999999; //20 Hz
	NVIC_ST_CURRENT_R = 0; //any write to current clears it
	NVIC_SYS_PRI3_R = (NVIC_SYS_PRI3_R & 0x00FFFFFF) | 0x20000000; //priority level 1
	NVIC_ST_CTRL_R = 0x0007; //enable SysTick with core clock and interrupts
}

//********************************************************************************

void Delay100ms(uint32_t count); // time delay in 0.1 seconds

int level = 1; //starts at level 1
int language;

void PickLanguage(void){
	SSD1306_OutString("Press PE2 for English");
	SSD1306_SetCursor(0,1);
	SSD1306_OutString("Press PE1 for Spanish");
	while (((Switch_In()&0x04) != 0x04) && ((Switch_In()&0x02) != 0x02)){};
	if ((Switch_In()&0x04) == 0x04){ //PE2
		language = 1; //English chosen
	}
	if ((Switch_In()&0x02) == 0x02){ //PE1
		language = 2; //Spanish chosen
	}
}

char *first = "Three levels";
char *second = "Slide pot: move L/R";
char *third = "Button: laser";
char *fourth = "Enemy killed: +5";

char *sfirst = "Tres niveles";
char *ssecond = "deslice: mover L/R";
char *sthird = "Boton: laser";
char *sfourth = "Enemigo asesinado: +5";

void Instructions(void){
	if (language == 1){
		SSD1306_DrawString(1, 2, first, SSD1306_WHITE);
		SSD1306_OutBuffer();
		SSD1306_DrawString(1, 18, second, SSD1306_WHITE);
		SSD1306_OutBuffer();
		SSD1306_DrawString(1, 34, third, SSD1306_WHITE);
		SSD1306_OutBuffer();
		SSD1306_DrawString(1, 50, fourth, SSD1306_WHITE);
		SSD1306_OutBuffer();
	}
	else {
		SSD1306_DrawString(1, 2, sfirst, SSD1306_WHITE);
		SSD1306_OutBuffer();
		SSD1306_DrawString(1, 18, ssecond, SSD1306_WHITE);
		SSD1306_OutBuffer();
		SSD1306_DrawString(1, 34, sthird, SSD1306_WHITE);
		SSD1306_OutBuffer();
		SSD1306_DrawString(1, 50, sfourth, SSD1306_WHITE);
		SSD1306_OutBuffer();
	}
}

typedef enum {dead, alive} status_t;
struct sprite {
	int32_t x; //x coordinate
	int32_t y; //y coordinate
	int32_t vx; //pixels/50ms (x speed)
	int32_t vy; //pixels/50ms (y speed)
	const uint8_t *image; //pointer to image
	status_t life; //dead/life
};

typedef struct sprite sprite_t;
sprite_t Enemies[18]; //6 alive enemies, 12 dead enemies
sprite_t Player;
sprite_t Missiles[100]; //100 missiles
sprite_t Missiles2; //EnemyBoss Missiles
sprite_t EnemyBoss;
int NeedToDraw ; //1 means need to draw (semaphore)

void Init(void) {
	int i;
	int randnum = ((Random()>>24)% 126);
	
	EnemyBoss.x = randnum;
	EnemyBoss.y = 10;
	if (Random32()%2 == 0){
		EnemyBoss.vx = 1;
	}
	else {
		EnemyBoss.vx = -1;
	}
	EnemyBoss.image = AlienBossA;
	EnemyBoss.life = alive;
	
	Player.x = 64;
	Player.y = 57;
	Player.image = PlayerShip2;
	Player.life = alive;
	
	for (i=0; i<6; i++){
		Enemies[i].x = 20*i;
		Enemies[i].y = 10;
		if (i%2==0){
			Enemies[i].vx = 1; //right
		}
		if (i%2==1){
			Enemies[i].vx = -1; //left
		}
			Enemies[i].vy = 1; //down
			Enemies[i].image = Alien10pointA;
			Enemies[i].life = alive;
		}
	for (i=6; i<18; i++){
		Enemies[i].life = dead;
	}
}

int elimbad1 = 0;
int elimplayer1 = 0;
int elimbad2 = 0;
int elimplayer2 = 0;
int elimbad3 = 0;
int elimplayer3 = 0;
int playerscore = 0;

void Collisions1(void){
	int i;
	int j;
	uint32_t x1,y1,x2,y2;
	
	for (j=0; j<18; j++){
		x2 = Enemies[j].x + 8;
		y2 = Enemies[j].y - 4;
		for (i=0; i<100; i++){
			if ((Missiles[i].life == alive)&&(Enemies[j].life == alive)){
				x1 = Missiles[i].x;
				y1 = Missiles[i].y;
				if (((x1-x2)*(x1-x2) + (y1-y2)*(y1-y2)) < 9){
					Enemies[j].life = dead;
					Missiles[i].life = dead;
					elimbad1 += 1; //winning condition
					playerscore += 5;
					Sound_Killed();
				}
			}
		}
	}
	if ((Missiles2.life == alive)&&(Player.life == alive)){
		x2 = Player.x + 8;
		y2 = Player.y + 4;
		x1 = Missiles2.x;
		y1 = Missiles2.y;
		if (((x1-x2)*(x1-x2) + (y1-y2)*(y1-y2)) < 9){
			Player.life = dead;
			Missiles2.life = dead;
			elimplayer1 = 1; //losing condition
			Sound_Killed();
		}
	}
	return;
}

void Collisions2(void){
	int i;
	int j;
	uint32_t x1,y1,x2,y2;
	
	for (j=0; j<18; j++){
		x2 = Enemies[j].x + 8;
		y2 = Enemies[j].y - 4;
		for (i=0; i<100; i++){
			if ((Missiles[i].life == alive)&&(Enemies[j].life == alive)){
				x1 = Missiles[i].x;
				y1 = Missiles[i].y;
				if (((x1-x2)*(x1-x2) + (y1-y2)*(y1-y2)) < 9){
					Enemies[j].life = dead;
					Missiles[i].life = dead;
					elimbad2 += 1; //winning condition
					playerscore += 5;
					Sound_Killed();
				}
			}
		}
	}
	if ((Missiles2.life == alive)&&(Player.life == alive)){
		x2 = Player.x + 8;
		y2 = Player.y + 4;
		x1 = Missiles2.x;
		y1 = Missiles2.y;
		if (((x1-x2)*(x1-x2) + (y1-y2)*(y1-y2)) < 9){
			Player.life = dead;
			Missiles2.life = dead;
			elimplayer2 = 1; //losing condition
			Sound_Killed();
		}
	}
	return;
}

void Collisions3(void){
	int i;
	int j;
	uint32_t x1,y1,x2,y2;
	
	for (j=0; j<18; j++){
		x2 = Enemies[j].x + 8;
		y2 = Enemies[j].y - 4;
		for (i=0; i<100; i++){
			if ((Missiles[i].life == alive)&&(Enemies[j].life == alive)){
				x1 = Missiles[i].x;
				y1 = Missiles[i].y;
				if (((x1-x2)*(x1-x2) + (y1-y2)*(y1-y2)) < 9){
					Enemies[j].life = dead;
					Missiles[i].life = dead;
					elimbad3 += 1; //winning condition
					playerscore += 5;
					Sound_Killed();
				}
			}
		}
	}
	if ((Missiles2.life == alive)&&(Player.life == alive)){
		x2 = Player.x + 8;
		y2 = Player.y + 4;
		x1 = Missiles2.x;
		y1 = Missiles2.y;
		if (((x1-x2)*(x1-x2) + (y1-y2)*(y1-y2)) < 9){
			Player.life = dead;
			Missiles2.life = dead;
			elimplayer3 = 1; //losing condition
			Sound_Killed();
		}
	}
	return;
}

int reachedbottom1 = 0;
static int delayer;

void Move1(void) {
	int i;
	uint32_t ADCdata;
	if (Player.life == alive){
		NeedToDraw = 1;
		ADCdata = ADC_In(); //0 to 4095
		Player.x = ((127-16)*ADCdata)/4096;
	}
	
	for (i=0; i<18; i++){
		if (Enemies[i].life == alive){
			NeedToDraw = 1;
			if (Enemies[i].x < 0){
				Enemies[i].x  = 2;
				Enemies[i].vx = -Enemies[i].vx;
			}
			if (Enemies[i].x > 126){
				Enemies[i].x  = 124;
				Enemies[i].vx = -Enemies[i].vx;
			}
			if ((Enemies[i].y < 62) && (Enemies[i].y > 0)){
					if (delayer%9 == 0){
						Enemies[i].x += Enemies[i].vx; //move right/left 20 pixels/sec
						Enemies[i].y += Enemies[i].vy; //move down/up 20 pixels/sec
					}
			}
				else {
					Enemies[i].life = dead; //at bottom
					reachedbottom1 = 1;
			}
		}
	}
	
	for (i=0; i<100; i++){ 
		if (Missiles[i].life == alive){
			NeedToDraw = 1;
			if (Missiles[i].y > 2){
					Missiles[i].y += Missiles[i].vy; //move down/up 20 pixels/sec
			}
			else {
				Missiles[i].life = dead; //at bottom
			}
		}
	}
	
	if (EnemyBoss.life == alive){
			NeedToDraw = 1;
		if (EnemyBoss.x < 0){
			EnemyBoss.x  = 2;
			EnemyBoss.vx = -EnemyBoss.vx;
		}
		if (EnemyBoss.x > 126){
			EnemyBoss.x  = 124;
			EnemyBoss.vx = -EnemyBoss.vx;
		}
		else {
			EnemyBoss.x += EnemyBoss.vx;
		}
	}
	
	if (Missiles2.life == alive){
		NeedToDraw = 1;
		if (Missiles2.y < 62){
			Missiles2.y += Missiles2.vy; //move down/up 20 pixels/sec
		}
		else {
			Missiles2.life = dead; //at bottom
		}
	}
}

int reachedbottom2 = 0;

void Move2(void) {
	int i;
	uint32_t ADCdata;
	if (Player.life == alive){
		NeedToDraw = 1;
		ADCdata = ADC_In(); //0 to 4095
		Player.x = ((127-16)*ADCdata)/4096;
	}
	
	for (i=0; i<18; i++){
		if (Enemies[i].life == alive){
			NeedToDraw = 1;
			if (Enemies[i].x < 0){
				Enemies[i].x  = 2;
				Enemies[i].vx = -Enemies[i].vx;
			}
			if (Enemies[i].x > 126){
				Enemies[i].x  = 124;
				Enemies[i].vx = -Enemies[i].vx;
			}
			if ((Enemies[i].y < 62) && (Enemies[i].y > 0)){
					if (delayer%6 == 0){
						Enemies[i].x += Enemies[i].vx; //move right/left 20 pixels/sec
						Enemies[i].y += Enemies[i].vy; //move down/up 20 pixels/sec
					}
			}
				else {
					Enemies[i].life = dead; //at bottom
					reachedbottom2 = 1;
			}
		}
	}
	
	for (i=0; i<100; i++){ 
		if (Missiles[i].life == alive){
			NeedToDraw = 1;
			if (Missiles[i].y > 2){
					Missiles[i].y += Missiles[i].vy; //move down/up 20 pixels/sec
			}
			else {
				Missiles[i].life = dead; //at bottom
			}
		}
	}
	
	if (EnemyBoss.life == alive){
			NeedToDraw = 1;
		if (EnemyBoss.x < 0){
			EnemyBoss.x  = 2;
			EnemyBoss.vx = -EnemyBoss.vx;
		}
		if (EnemyBoss.x > 126){
			EnemyBoss.x  = 124;
			EnemyBoss.vx = -EnemyBoss.vx;
		}
		else {
			EnemyBoss.x += EnemyBoss.vx;
		}
	}
	
	if (Missiles2.life == alive){
		NeedToDraw = 1;
		if (Missiles2.y < 62){
			Missiles2.y += 2*Missiles2.vy; //move down/up 20 pixels/sec
		}
		else {
			Missiles2.life = dead; //at bottom
		}
	}
}

int reachedbottom3 = 0;

void Move3(void) {
	int i;
	uint32_t ADCdata;
	if (Player.life == alive){
		NeedToDraw = 1;
		ADCdata = ADC_In(); //0 to 4095
		Player.x = ((127-16)*ADCdata)/4096;
	}
	
	for (i=0; i<18; i++){
		if (Enemies[i].life == alive){
			NeedToDraw = 1;
			if (Enemies[i].x < 0){
				Enemies[i].x  = 2;
				Enemies[i].vx = -Enemies[i].vx;
			}
			if (Enemies[i].x > 126){
				Enemies[i].x  = 124;
				Enemies[i].vx = -Enemies[i].vx;
			}
			if ((Enemies[i].y < 62) && (Enemies[i].y > 0)){
					if (delayer%3 == 0){
						Enemies[i].x += Enemies[i].vx; //move right/left 20 pixels/sec
						Enemies[i].y += Enemies[i].vy; //move down/up 20 pixels/sec
					}
			}
				else {
					Enemies[i].life = dead; //at bottom
					reachedbottom3 = 1;
			}
		}
	}
	
	for (i=0; i<100; i++){ 
		if (Missiles[i].life == alive){
			NeedToDraw = 1;
			if (Missiles[i].y > 2){
					Missiles[i].y += Missiles[i].vy; //move down/up 20 pixels/sec
			}
			else {
				Missiles[i].life = dead; //at bottom
			}
		}
	}
	
	if (EnemyBoss.life == alive){
			NeedToDraw = 1;
		if (EnemyBoss.x < 0){
			EnemyBoss.x  = 2;
			EnemyBoss.vx = -EnemyBoss.vx;
		}
		if (EnemyBoss.x > 126){
			EnemyBoss.x  = 124;
			EnemyBoss.vx = -EnemyBoss.vx;
		}
		else {
			EnemyBoss.x += EnemyBoss.vx;
		}
	}
	
	if (Missiles2.life == alive){
		NeedToDraw = 1;
		if (Missiles2.y < 62){
			Missiles2.y += 3*Missiles2.vy; //move down/up 20 pixels/sec
		}
		else {
			Missiles2.life = dead; //at bottom
		}
	}
}

int missilesavevx1;
int missilesavevy1;

void Fire(int32_t vx1, int32_t vy1){
	int i;
	i=0;
	//allows player to spam lasers
	while (Missiles[i].life == alive){
		i++;
		if (i==100) return;
	}
	Missiles[i].x = Player.x + 7;
	Missiles[i].y = Player.y - 4;
	Missiles[i].vx = vx1;
	missilesavevx1 = vx1;
	Missiles[i].vy = -vy1;
	missilesavevy1 = -vy1;
	Missiles[i].image = Laser0;
	Missiles[i].life = alive;
	Sound_Shoot();
}

void BossFire(int32_t vx1, int32_t vy1){
	Missiles2.x = EnemyBoss.x + 7;
	Missiles2.y = EnemyBoss.y + 4;
	Missiles2.vx = vx1;
	Missiles2.vy = vy1;
	Missiles2.image = Laser0;
	Missiles2.life = alive;
}

void Draw(void) {
	int i;
	SSD1306_ClearBuffer();
	if (Player.life == alive){
		SSD1306_DrawBMP(Player.x, Player.y, Player.image, 0, SSD1306_INVERSE); //Player on bottom center of screen
	}
	if (EnemyBoss.life == alive){
		SSD1306_DrawBMP(EnemyBoss.x, EnemyBoss.y, EnemyBoss.image, 0, SSD1306_INVERSE); //EnemyBoss on top spawned randomly on screen
	}
	for (i=0; i<18; i++){
		if (Enemies[i].life){
			SSD1306_DrawBMP(Enemies[i].x, Enemies[i].y, Enemies[i].image, 0, SSD1306_INVERSE); //Enemies spawned on the top of screen spread out evenly
		}
	}
	for (i=0; i<100; i++){
		if(Missiles[i].life == alive){
			SSD1306_DrawBMP(Missiles[i].x, Missiles[i].y, Missiles[i].image, 0, SSD1306_INVERSE); //Player Missiles 
    }
  }
	if(Missiles2.life == alive){
		SSD1306_DrawBMP(Missiles2.x, Missiles2.y, Missiles2.image, 0, SSD1306_INVERSE); //EnemyBoss Missiles
   }
  
	SSD1306_OutBuffer();
	NeedToDraw = 0;
}

void pausegame(void){
	DisableInterrupts();
//pauses gameplay
	int i;
	//stop enemy velocity
	for (i=0; i<6; i++){
		Enemies[i].vx = 0; //left
		Enemies[i].vy = 0; //down
	}
	//stop enemyboss velocity
	EnemyBoss.vx = 0;
	//stop missiles velocity
	i=0;
	Missiles[i].vx = 0;
	Missiles[i].vy = 0;
	//stop enemyboss missiles velocity
	Missiles2.vx = 0;
	Missiles2.vy = 0;
	//clear screen
	SSD1306_OutClear();
	//show pause screen
	if (language == 1){
		SSD1306_OutString("Game Paused!");
	}
	else {
		SSD1306_OutString("Juego pausado!");
	}
	//SSD1306_DrawString(2, 2, paused, SSD1306_WHITE);
	//SSD1306_OutBuffer();
	//wait for button input
	while ((Switch_In()&0x08) != 0x08){}; //PE3
	//reset enemies velocity
	for (i=0; i<6; i++){
		if (i%2==0){
			Enemies[i].vx = 1; //right
		}
		if (i%2==1){
			Enemies[i].vx = -1; //left
		}
		Enemies[i].vy = 1; //down
	}
	//reset enemyboss velocity
	if (Random32()%2 == 0){
		EnemyBoss.vx = 1;
	}
	else {
		EnemyBoss.vx = -1;
	}
	//reset missiles velocity
	i=0;
	Missiles[i].vx = 0;
	Missiles[i].vy = 0;
	//reset enemyboss missiles velocity
	Missiles2.vx = missilesavevx1;
	Missiles2.vy = missilesavevy1;
	EnableInterrupts();
}


void SysTick_Handler(void){
	static uint32_t lastdown = 0;
	uint32_t down = Switch_In()&0x04; //PE2
	if ((down==0x04) && (lastdown == 0)){
		Fire(0,1);
	}
	if ((Switch_In()&0x08)==0x08){ //PE3
		pausegame();
	}
	if ((EnemyBoss.life == alive)&&(Missiles2.life == dead)){
		BossFire(0,1);
	}
	if (level == 1){
		Move1();
		Collisions1();
	}
	if (level == 2){
		Move2();
		Collisions2();
	}
	if (level == 3){
		Move3();
		Collisions3();
	}
	lastdown = down;
	delayer++;
}

void SysTick_Init20Hz(void);
	
int main(void){
  DisableInterrupts();
  // pick one of the following three lines, all three set to 80 MHz
  //PLL_Init();                   // 1) call to have no TExaS debugging
  TExaS_Init(&LogicAnalyzerTask); // 2) call to activate logic analyzer
  //TExaS_Init(&ScopeTask);       // or 3) call to activate analog scope PD2
  SSD1306_Init(SSD1306_SWITCHCAPVCC);
  SSD1306_OutClear();   
  Random_Init(1);
  Profile_Init(); // PB5,PB4,PF3,PF2,PF1 
  SSD1306_ClearBuffer();
  SSD1306_DrawBMP(2, 62, SpaceInvadersMarquee, 0, SSD1306_WHITE);
  SSD1306_OutBuffer();
  Delay100ms(20);
	SysTick_Init20Hz();
	ADC_Init(4);
	Sound_Init();
	Switch_Init();
	SSD1306_OutClear();
	PickLanguage();
	SSD1306_OutClear();
	Instructions();
	Delay100ms(100);
	SSD1306_OutClear();
	Init();
	Random_Init(1);
	EnableInterrupts();
	
  while(1){
		if (NeedToDraw){
			Draw();
			//Player beats all levels if he gets all 6 enemies while enemies didnt reach the bottom yet
			//Each level gets harder in difficulty with respect to the enemies speed
			if ((elimbad1 == 6) && (reachedbottom1 == 0)){
				DisableInterrupts();
				SSD1306_OutClear();
				if (language == 1){
					SSD1306_OutString("Level 1 done!");
				}
				else {
					SSD1306_OutString("Nivel uno hecho!");
				}
				SSD1306_SetCursor(0,1);
				if (language == 1){
					SSD1306_OutString("Score: ");
					LCD_OutDec(playerscore);
				}
				else {
					SSD1306_OutString("Puntaje: ");
					LCD_OutDec(playerscore);
				}
				elimbad1 = 0;
				Delay100ms(20);
				level = 2;
				Init();
				EnableInterrupts();
			}
			if ((elimbad2 == 6) && (reachedbottom2 == 0)){
				DisableInterrupts();
				SSD1306_OutClear();
				if (language == 1){
					SSD1306_OutString("Level 2 done!");
				}
				else {
					SSD1306_OutString("Nivel dos hecho!");
				}
				SSD1306_SetCursor(0,1);
				if (language == 1){
					SSD1306_OutString("Score: ");
					LCD_OutDec(playerscore);
				}
				else {
					SSD1306_OutString("Puntaje: ");
					LCD_OutDec(playerscore);
				}
				elimbad2 = 0;
				Delay100ms(20);
				level = 3;
				Init();
				EnableInterrupts();
			}
			if ((elimbad3 == 6) && (reachedbottom3 == 0)){
				DisableInterrupts();
				SSD1306_OutClear();
				if (language == 1){
					SSD1306_OutString("You are a champion!");
				}
				else {
					SSD1306_OutString("Eres un campeon!");
				}
				SSD1306_SetCursor(0,1);
				if (language == 1){
					SSD1306_OutString("Score: ");
					LCD_OutDec(playerscore);
				}
				else {
					SSD1306_OutString("Puntaje: ");
					LCD_OutDec(playerscore);
				}
			}
			if ((reachedbottom1 == 1) || (reachedbottom2 == 1) || (reachedbottom3 == 1) || (elimplayer1 == 1) || (elimplayer2 == 1) || (elimplayer3 == 1)){
				DisableInterrupts();
				SSD1306_OutClear();
				if (language == 1){
					SSD1306_OutString("Sorry, you lose!");
					SSD1306_SetCursor(0,2);
					SSD1306_OutString("You got to level ");
					LCD_OutDec(level);
				}
				else {
					SSD1306_OutString("Lo siento, pierdes!");
					SSD1306_SetCursor(0,2);
					SSD1306_OutString("Tienes que nivelar ");
					LCD_OutDec(level);
				}
				SSD1306_SetCursor(0,1);
				if (language == 1){
					SSD1306_OutString("Score: ");
					LCD_OutDec(playerscore);
				}
				else {
					SSD1306_OutString("Puntaje: ");
					LCD_OutDec(playerscore);
				}
			}
		}
  }
}

// You can't use this timer, it is here for starter code only 
// you must use interrupts to perform delays
void Delay100ms(uint32_t count){uint32_t volatile time;
  while(count>0){
    time = 727240;  // 0.1sec at 80 MHz
    while(time){
	  	time--;
    }
    count--;
  }
}

