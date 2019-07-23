/*
 * 120B_final_project.c
 *
 * Created: 7/22/2019 9:27:29 PM
 * Author : hexia
 */ 

#include <avr/io.h>
#include "scheduler.h"
#include "io.h"
#include "io.c"
#include "timer.h"
#include "bit.h"
#include "seven_seg.h"

enum woqu{food,fxxd,players};

static task task1, task2,task3;
task *tasks[] = {&task1, &task2, &task3};
	
unsigned char foodway[3] = {2, 9, 28};
unsigned char fxxdway[3] = {7, 20, 26};
unsigned char playway = 17;
	
unsigned int i = 0;
	
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
	
	
	const unsigned short numTasks = sizeof(tasks)/sizeof(task*);
	
	task1.state =  Start;//Task initial state
	task1.period = 5;//Task Period
	task1.elapsedTime = task1.period;//Task current elapsed time.
	task1.TickFct = &Game;//Function pointer for the tick
	
	
	 unsigned long int GCD = tasks[0]->period;
	 unsigned short i;//Scheduler for-loop iterator
	 for( i = 1; i < numTasks;i ++){
		 GCD = findGCD(tasks[i]-> period, GCD);
	 }
	 
	 staff();
	 displaychar();
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

