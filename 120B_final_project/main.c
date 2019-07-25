/*	Author: xhe058
 *  Partner(s) Name: zhangcheng liang
 *	Lab Section:
 *	Assignment: Lab #  Exercise #
 *	Exercise Description: [optional - include for your own benefit]
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 */
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <util/delay.h>
#include "scheduler.h"
#include "io.h"
#include "io.c"
#include "timer.h"
#include "bit.h"
#include "seven_seg.h"

#define HC595_PORT   PORTB
#define HC595_DDR    DDRB

#define HC595_DS_POS PB0      //Data pin (DS) pin location

#define HC595_SH_CP_POS PB1      //Shift Clock (SH_CP) pin location
#define HC595_ST_CP_POS PB2      //Store Clock (ST_CP) pin location

enum shucai{food,fxxd,players};
enum Game{K_start, Begin, up, down, Reset, Win, Lose};
enum Left{L_start, L_Wait, moving};

static task task1, task2;
task *tasks[] = {&task1, &task2 };
	
unsigned char foodway[3] = {2, 9, 28};
unsigned char fxxdway[3] = {7, 20, 26};
unsigned char playway = 17;
	
unsigned int i = 0;
unsigned char point = 0;
unsigned char lol = 0;
unsigned char speed = 0;

void HC595Init()
{
	//Make the Data(DS), Shift clock (SH_CP), Store Clock (ST_CP) lines output
	HC595_DDR|=((1<<HC595_SH_CP_POS)|(1<<HC595_ST_CP_POS)|(1<<HC595_DS_POS));
}

#define HC595DataHigh() (HC595_PORT|=(1<<HC595_DS_POS))

#define HC595DataLow() (HC595_PORT&=(~(1<<HC595_DS_POS)))

void HC595Pulse()
{
	//Pulse the Shift Clock

	HC595_PORT|=(1<<HC595_SH_CP_POS);//HIGH

	HC595_PORT&=(~(1<<HC595_SH_CP_POS));//LOW

}

void HC595Latch()
{
	//Pulse the Store Clock

	HC595_PORT|=(1<<HC595_ST_CP_POS);//HIGH
	_delay_loop_1(1);

	HC595_PORT&=(~(1<<HC595_ST_CP_POS));//LOW
	_delay_loop_1(1);
}

void HC595Write(uint8_t data)
{
	//Send each 8 bits serially

	//Order is MSB first
	for(uint8_t i=0;i<8;i++)
	{
		//Output the data on DS line according to the
		//Value of MSB
		if(data & 0b10000000)
		{
			//MSB is 1 so output high

			HC595DataHigh();
		}
		else
		{
			//MSB is 0 so output high
			HC595DataLow();
		}

		HC595Pulse();  //Pulse the Clock line
		data=data<<1;  //Now bring next bit at MSB position

	}

	//Now all 8 bits have been transferred to shift register
	//Move them to output latch at one
	HC595Latch();
}

void Wait()
{
	for(uint8_t i=0;i<30;i++)
	{
		_delay_loop_2(0);
	}
}
	
void staff(){
	unsigned char caneat[8] = {0x00, 0x0E, 0x17, 0xFF, 0x0A, 0x0A, 0x0E, 0x00};
	unsigned char connoteat[8] = {0x0E, 0xFF, 0x15, 0xFF, 0x0A, 0x00, 0x0A, 0x0E};
	unsigned char player[8] = {0x0E, 0x0E, 0x04, 0xFF, 0x04, 0x0A, 0x0A, 0x0A};
	LCD_Custom_Char(food,caneat);
	LCD_Custom_Char(fxxd,connoteat);
	LCD_Custom_Char(players,player);
	
}

void displaychar(){
	
	LCD_Cursor(playway);
	LCD_Char(players);
	
	for(i=0;i<3;i++){
		LCD_Cursor(foodway[i]);
		LCD_Char(food);
		LCD_Cursor(fxxdway[i]);
		LCD_Char(fxxd);	
	}
}

void initialdisplay(){
	LCD_ClearScreen();
}

void movething(){
	for(i = 0;i<3;i++){
		if (foodway[i] == 1){
			foodway[i] = 16;
		}else if(foodway[i] == 0){
			foodway[i] = 16;
		}else if(foodway[i] == 17){
			foodway[i] = 31;
		}else{
			foodway[i]--;
		}
		if(fxxdway[i] == 0){
			fxxdway[i] = 15;
		}else if(fxxdway[i] == 17){
			fxxdway[i] = 32;
		}else{
			fxxdway[i]--;
		}
	}
}

int Game(int state){
	unsigned char tempA = PINA;
	switch(state){
		case K_start:{
			if((~PINA & 0x04) == 0x04){
				state = Begin;
			}
			break;
		}
		
		case Begin:{
			if((tempA & 0x01) == 0x00){
				state = up;
				break;
			}else if((tempA & 0x02) == 0x00){
				state = down;
				break;
			}else if((tempA & 0x08) == 0x00){
				state = Reset;
				break;
			}else{
				state = Begin;
				break;
			}
		}
		
		
		case up:{
			playway = 1;


				state = Begin;
			break;
		}
		
		case down:{
			playway = 17;


				state = Begin;
			break;
		}
		
		case Reset:{
			lol = 0;
			point = 0;
			playway = 17;
			foodway[0] = 2;
			foodway[1] = 9;
			foodway[2] = 28;
			fxxdway[0] = 7;
			fxxdway[1] = 20;
			fxxdway[2] = 26;
			state = K_start;
			speed = 0;
			task2.period = 30;
			break;	
		}
		
		case Win:{
			if ((~PINA & 0x08) == 0x08){
				state = Reset;
	
			}else{
				state = Win;

			}
			break;
		}
		
		case Lose:{
			if ((~PINA & 0x08) == 0x08){
				state = Reset;

			}else{
				state = Lose;

			}
			break;
		}
		
		
		default:
		break;
	}
	
	switch(state){
		case K_start:{
				if(lol == 0){
				const unsigned char string[32] = "Press 3 to start    ";
				LCD_DisplayString(1,string);
				point = 0;
				lol++;}
				break;
			//	}
			}
		
		case Begin:{
			lol = 0;
			if (speed == 0){
				HC595Write(0b00000000);
			}else if(speed == 1){
				HC595Write(0b00000011);
			}else if(speed == 2){
				HC595Write(0b00001111);
			}else if(speed == 3){
				HC595Write(0b00111111);
			}else if(speed == 4){
				HC595Write(0b11111111);
			}
			for(i=0;i<3;i++){
				if (playway == foodway[i]){
					foodway[i] = 0;
					if(point == 3){
							point++;
							speed++;
							task2.period -= 5;
						}else if(point == 6){
							point++;
							speed++;
							task2.period -= 5;
						}else if (point == 9){
							point++;
							speed++;
							task2.period -= 5;
						}else if (point == 12){
							point++;
							speed++;
							task2.period -= 5;
						}else if (point == 15){
							state = Win;
							break;
						}else{
						point++;
						break;
					}
					}else if(playway == fxxdway[i]){
					state = Lose;
					break;
				}
			}
			break;
		}
		
		case up:{
			break;
		}
		
		case down:{
			break;
		}
		
		case Reset:{
			break;
		}
		
		case Win:{
			if(lol == 0){
				eeprom_write_byte((uint8_t*)10,1);
				eeprom_write_byte((uint8_t*)1,5);
			const unsigned char WinOut[32] = "You Win! Press 4 play again!";
			LCD_DisplayString(1,WinOut);
			lol++;}
			break;
		}
		
		case Lose:{
			if(lol == 0){
			unsigned short heigh=0;
			heigh = eeprom_read_byte((uint8_t*)1) + eeprom_read_byte((uint8_t*)10)*10;
			if(point>heigh){
				eeprom_write_byte((uint8_t*)10,point/10);
				eeprom_write_byte((uint8_t*)1,point%10);
				heigh = point;
			}
			const unsigned char LoseOut[32] = "Lose!You got:   Highest:    ";
			LCD_DisplayString(1,LoseOut);
			LCD_Cursor(14);
			LCD_WriteData(point/10+'0');
			LCD_Cursor(15);
			LCD_WriteData(point%10+'0');
			LCD_Cursor(26);
			LCD_WriteData(heigh/10+'0');
			LCD_Cursor(27);
			LCD_WriteData(heigh%10+'0');
			lol++;}
			break;
		}
		
		default:
		break;
	}
	return state;
}

int Left(int state){
	switch(state){
		case L_start:{
			state = L_Wait;
			break;
		}
		
		case L_Wait:{
			if(task1.state == up || task1.state == down || task1.state == Begin ){
				state = moving;
				break;
			}else{
				state = L_Wait;
				break;
			}
		}
		
		case moving:{
			if(task1.state == Reset || task1.state == Win || task1.state == Lose){
				state = L_Wait;
				break;
			}
		}
		
		default:
			state = L_start;
			break;
	}
	
	switch(state){
		case L_start:{
			break;
		}
		
		case L_Wait:{
			break;
		}
		
		case moving:{
			initialdisplay();
			displaychar();
			movething();
			break;
		}
		
		default:
		break;
	}
	return state;
}
	
	



int main(void)
{
    /* Replace with your application code */
	DDRA = 0x00;
	DDRB = 0xFF;
	DDRC = 0xFF;
	DDRD = 0xFF;
	PORTA = 0xFF;
	PORTB = 0x00;
	PORTC = 0x00;
	PORTD = 0x00;
	
	LCD_init();
	HC595Init();
	staff();
	//eeprom_write_byte((uint8_t*)10,0);
	//eeprom_write_byte((uint8_t*)1,0);
	
	
	const unsigned short numTasks = sizeof(tasks)/sizeof(task*);
	
	task1.state =  K_start;//Task initial state
	task1.period = 5;//Task Period
	task1.elapsedTime = task1.period;//Task current elapsed time.
	task1.TickFct = &Game;//Function pointer for the tick
	
	task2.state =  L_start;//Task initial state
	task2.period = 30;//Task Period
	task2.elapsedTime = task2.period;//Task current elapsed time.
	task2.TickFct = &Left;//Function pointer for the tick
	
	
	 unsigned long int GCD = tasks[0]->period;
	 unsigned short i;//Scheduler for-loop iterator
	 for( i = 1; i < numTasks;i ++){
		 GCD = findGCD(tasks[i]-> period, GCD);
	 }
	 
	 //staff();
	 //displaychar();
	// HC595Write(0b10000001);
	 TimerSet(GCD);
	 TimerOn();
	 
	 
	
    while (1) 
    {
		
		  for(i = 0; i < numTasks; i++){//Scheduler code
			  if(tasks[i]->elapsedTime == tasks[i]->period){//Task is ready to tick
				  tasks[i]->state= tasks[i]->TickFct(tasks[i]->state);//set next state
				  tasks[i]->elapsedTime = 0;//Reset the elapsed time for next tick;
			  }
			  tasks[i]->elapsedTime += GCD;
		  }
		  while(!TimerFlag);
		  TimerFlag = 0;
		  
		
		
    }
}