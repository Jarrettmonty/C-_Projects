/*
*	FIle: t_clock.c
*/
#include<avr/io.h>
#include<avr/interrupt.h>
#include "clock.h"
#include <Arduino.h>
#include <LiquidCrystal.h>
#include <string.h>


volatile t_signal button1Press;
volatile t_signal button2Press;
LiquidCrystal lcd(p_RS, p_EN, p_D4, p_D5, p_D6, p_D7);

char* strList[9] =
	{
		"MENU/SET",			//0
		"MENU/OFF",			//1
		"OK/BACK",			//2
		"DELAY/OFF",		//3
		"NO ALARM SET    ",	//4
		"SET TIME       ",	//5
		"SET ALARM      ",	//6
		"SET VOL        "	//7
	};

unsigned long int get_milli_today() {
	
	return (millis()) % msPerDay;
}

long int get_sec_today(long int addtlSec) {
	Serial.println(get_milli_today());
	if (addtlSec > 0) 
	{
		return ((get_milli_today()/ msPerSec) + addtlSec)%86400;
	}
	else
		return (get_milli_today()/msPerSec)%86400; 
}


void trans_clock_state(t_clock* me, const t_event* e, State s_tone, State s_none, State s_sw1, State s_sw2) 
{
	//Assigns by priority (will be useful with ISR)
	//assigns a new state depending on which
	// t_event corresponds to true for a respective State s_CASE
	// 0 - if null
	switch (e->sig) 
	{
		
		case TONE_ACTIVE:
		assign_clock_state(me, s_tone);
		break;
		
		case NOT_ACTIVE:
		assign_clock_state(me, s_none);
		break;

		case SW_ONE_ACTIVE:
		assign_clock_state(me, s_sw1);
		break;

		case SW_TWO_ACTIVE:
    	assign_clock_state(me, s_sw2);
		break;
	}
	if(e->sig != NOT_ACTIVE) {lcd.clear();}
		
}
void alarm_ctor(t_clock* _ME) 
{
	assign_clock_state(_ME, &s_clock_home);
}
void process_event(t_clock* _ME, const t_event*_EVENT)
{
	/*
	*Using _MEs current state (function) and _EVENT as a condition,
	* runs whichever _EVENT has been passed in
	*/
	(*(_ME)->state_)(_ME, _EVENT);
}
void assign_clock_state(t_clock* _ME, State newState) {
	/*
	*	Will assign _ME->state the value of an arbitrary state function newState
	*		as well as record the current state_ as _prev (a 'shortcut' for state changes)
	*/
	_ME->prev_	= _ME->state_;
	_ME->state_ = newState;
}
void update_current_time(t_clock* me)
{	
	/* WIP
	*	Will update the given t_clock object's time parameters in milliseconds
	*		among others, and check whether time has been manually set
	*/
	
	me->currentTimeInSec = get_sec_today(me->additionalTimeInSec);
	sec_to_array_time(me->currentTimeInSec, me->currentTime);	
}
void s_clock_home(t_clock* me, const t_event* e)
{
	write_lcd_by_state(HOME_OFF, me);	
	update_current_time(me);
	trans_clock_state(me, e, NULL, &s_clock_home, &s_clock_menu, &s_clock_home_set);
	if (e->sig == SW_TWO_ACTIVE)			
		me->isAlarmSet = 1;
}
void s_clock_home_set(t_clock* me, const t_event*e)
{
	write_lcd_by_state(HOME_SET, me);
	update_current_time(me);
	trans_clock_state(me, e, &s_clock_tone, &s_clock_home_set, &s_clock_menu, &s_clock_home);
	if (is_alarm_ready(me))
		assign_clock_state(me, &s_clock_tone);
	if(e->sig == SW_TWO_ACTIVE) 
	{
		assign_clock_state(me, &s_clock_home);
		me->isAlarmSet = 0;
	}
}
void s_clock_tone(t_clock* me, const t_event*e) 
{
		write_lcd_by_state(TONE_, me);
		tone(p_PIEZO, 6000, 30);
		trans_clock_state(me, e, NULL, &s_clock_tone, &s_clock_home_set, &s_clock_home);
		if (e->sig == SW_TWO_ACTIVE)
		{
			me->isAlarmSet = 0;
		}
}
void s_clock_menu(t_clock* me, const t_event*e) 
{
		me->adjustKnobEnum = get_adc_value(2);
		switch (me->adjustKnobEnum) 
		{
			case 0: 
				write_lcd_by_state(MENU_0, me);
				trans_clock_state(me, e, &s_clock_tone, &s_clock_menu, &s_adjust_current_hour, &s_clock_home);
			break;
			case 1:
				write_lcd_by_state(MENU_1, me);
				trans_clock_state(me, e, &s_clock_tone, &s_clock_menu, &s_adjust_alarm_hour, &s_clock_home);
			break;
			case 2:
				//trans_clock_state(me, e, s_clock_tone, s_clock_menu, &s_vol, s_clock_home);
			break;
		}
}
void s_adjust_current_hour(t_clock* me, const t_event* e)
{
	int resultADC = get_adc_value(0);
	write_cur_time_to_lcd (ADJUST_HOUR, me, resultADC);
	trans_clock_state(me, e, NULL, &s_adjust_current_hour, &s_adjust_current_min, &s_clock_menu);
	if (e->sig == SW_ONE_ACTIVE)
	{
		me->setTime[0] = resultADC/10;
		me->setTime[1] = resultADC%10;
	}
}
void s_adjust_current_min(t_clock* me, const t_event* e)
{	
	
	int resultADC = get_adc_value(1);
	write_cur_time_to_lcd (ADJUST_MIN_CURRENT, me, resultADC);
	trans_clock_state(me, e, NULL, &s_adjust_current_min, &s_clock_home, &s_clock_home);
	if(e->sig == SW_ONE_ACTIVE)
	{
		me->setTime[2] = resultADC/10;
		me->setTime[3] = resultADC%10;	
		convert_set_time_to_sec(me);
		update_current_time(me);
	}
}
void s_adjust_alarm_hour(t_clock* me, const t_event*e)
{	
	int resultADC = get_adc_value(0);
	write_cur_time_to_lcd(ADJUST_HOUR, me, resultADC);
	trans_clock_state(me, e, NULL, &s_adjust_alarm_hour, &s_adjust_alarm_min, &s_clock_menu);
	if (e->sig == SW_ONE_ACTIVE) 
	{
		me->alarmTime[0] = resultADC/10;
		me->alarmTime[1] = resultADC%10;
	}
}
void s_adjust_alarm_min(t_clock* me, const t_event* e) 
{
	int resultADC = get_adc_value(1);
	write_cur_time_to_lcd(ADJUST_MIN_ALARM, me, resultADC);
	trans_clock_state(me, e, NULL, &s_adjust_alarm_min,&s_clock_home, &s_adjust_alarm_hour);
	if (e->sig == SW_ONE_ACTIVE) 
	{
		me->alarmTime[2] = resultADC/10;
		me->alarmTime[3] = resultADC%10;
		convert_alarm_time_to_sec(me);
		update_current_time(me);
	}
}
int get_adc_value(int timeType) 
{
	float result_8bit, result_Fixed;
	result_8bit = analogRead(p_TRIMPOT);
	switch(timeType) 
	{
		case 0:
			result_Fixed = 0.023*result_8bit;
		break;		
		case 1:
			result_Fixed = 0.059*result_8bit;
		break;
		case 2: 
			if(result_8bit < 512)
				result_Fixed = 0;
			else	
				result_Fixed = 1;
		break;
	}
	return result_Fixed;	
}
void write_lcd_by_state(e_STATE_NAMES option, t_clock* me) 
	/*	Fomatted LCD print function; 
			TODO: WRONG VALUES, BUT WORKIGN WELL*/	
{
		//	*	HOME_OFF = 0, HOME_ON =1, XXXXXXXMENU_0 = 2, MENU_1 = 3, MENU2 = 4, TONE_ = 5 == OPTION

	
	update_current_time(me);
	switch (option) {
		case HOME_OFF:	
			lcd.home();
			lcd.print(me->currentTime[0]); 
			lcd.print(me->currentTime[1]);
			lcd.write(':');		
			lcd.print(me->currentTime[2]);
			lcd.print(me->currentTime[3]);
			lcd.print("   ");
			
			lcd.setCursor(8,0); 
			lcd.print(strList[0]);
			lcd.setCursor(0,1);
			lcd.print(strList[4]);
			break;
		case HOME_SET:
			
			lcd.home();
			lcd.print(me->currentTime[0]);
			lcd.print(me->currentTime[1]);
			lcd.write(':');
			lcd.print(me->currentTime[2]);
			lcd.print(me->currentTime[3]);
			lcd.setCursor(8,0);
			lcd.print(strList[1]);
			
			lcd.setCursor(0,1);
			lcd.print(me->alarmTime[0]);
			lcd.print(me->alarmTime[1]);
			lcd.write(':');
			lcd.print(me->alarmTime[2]);
			lcd.print(me->alarmTime[3]);			
			break;
		case MENU_0:
			lcd.home();
			lcd.write('>');
			lcd.print(strList[5]);
			lcd.setCursor(0,1);
			lcd.print(strList[6]);
			break;
		case MENU_1:
			lcd.home();
			lcd.write('>');
			lcd.print(strList[6]);
			lcd.setCursor(0,1);
			lcd.print(strList[7]);
		break;
		case MENU_2:
			lcd.home();
			lcd.write('>');
			lcd.print(strList[7]);
		break;
		case TONE_:		

			lcd.print(strList[3]);
			lcd.setCursor(0,1);
			lcd.print(me->currentTime[0]);
			lcd.print(me->currentTime[1]);
			lcd.write(':');
			lcd.print(me->currentTime[2]);
			lcd.print(me->currentTime[3]);
			break;
	}

}

void write_cur_time_to_lcd(e_STATE_NAMES option, t_clock* me, int adcResult) 
	/* Hideous code intended to print the current time adjustment based on adcResult 
		TODO: CLEAN, BUT WORKING WELL */
{
		lcd.home();
		switch (option) 
		{
			case ADJUST_HOUR:
				if (adcResult<10) 
				{
					lcd.print('0');
					lcd.print(adcResult);	
					lcd.print(":00");
				}
				else
				{
					lcd.print(adcResult);
					lcd.print(":00");
					lcd.setCursor(8,0);
					lcd.print(strList[2]);
				}
			break;		
				
			case ADJUST_MIN_CURRENT:
				if(adcResult< 10) 
				{ 
					lcd.print(me->setTime[0]) + lcd.print(me->setTime[1]);
					lcd.print(":0") + lcd.print(adcResult);
					lcd.print("    ") + lcd.print(strList[2]);
				}
				else 
				{
					
					lcd.print(me->setTime[0]) + lcd.print(me->setTime[1]);
					lcd.print(':') + lcd.print(adcResult);
					lcd.print("    ") + lcd.print(strList[2]);
				}
			break;
				
			case ADJUST_MIN_ALARM:
				if(adcResult< 10) 
				{ 
					lcd.print(me->alarmTime[0]) + lcd.print(me->alarmTime[1]);
					lcd.print(":0") + lcd.print(adcResult);
					lcd.print("    ") + lcd.print(strList[2]);
				}
				else 
				{
					lcd.print(me->alarmTime[0]) + lcd.print(me->alarmTime[1]);
					lcd.print(':') + lcd.print(adcResult);
					lcd.print("    ") + lcd.print(strList[2]);
				}
			break;
			}
			
				lcd.setCursor(0,1);
				lcd.print(me->currentTime[0]) + lcd.print(me->currentTime[1]);
				lcd.write(':');
				lcd.print(me->currentTime[2]) +	lcd.print(me->currentTime[3]);			

		
}

void delay_clock_tone(t_clock* me)
/*Functions as the snooze feature, which increments the alarm by 10 minutes
	TO-DO: VARIALBE */
{
	me->alarmTimeInSec += (60)*10;
	sec_to_array_time(me->alarmTimeInSec, me->alarmTime);
}

bool was_prev_state(t_clock* me, State prevState) 
	/* A simple check, primarily for certain event-triggered responses */
{	
	if (me->prev_ == prevState) 
	{	
		return true;
	}
}

void convert_set_time_to_sec(t_clock* me)
{
	me->additionalTimeInSec = array_time_to_sec(me->setTime);
}

bool is_alarm_ready(t_clock* me)
{
	if(me->alarmTimeInSec == me->currentTimeInSec) 
		return 1;
	else
		return 0;
	
}

long int array_time_to_sec(int* arr) 
{
	long int result = 0;
	result = *arr *(36000); arr++;	
	result += (*arr) * (3600); arr++;
	result += (*arr) *10* (60); arr++;
	result += (*arr) * (60); arr++;
	
	return result;
}

void sec_to_array_time(long int sec, int* arr) 
{ Serial.print("sec = ") + Serial.println(sec);
	int array[4];
    array[0] = (sec)/3600;
    array[2] = (sec/6 - (array[0]*60*10))/10;	
	array[1] = array[0] % 10; 
    array[3] = array[2] % 10;
    array[0] /= 10;
    array[2] /= 10; 
	
	int i;
	for (i = 0; i<4; i++)
	{	
		*arr = array[i]; arr++;	
	}
}

void convert_alarm_time_to_sec(t_clock* me) 
{
	me->alarmTimeInSec = array_time_to_sec(me->alarmTime);
}

/*ISR(TIMER2_OVF_vect)
{
	if (digitalRead(SW1) ==0) 
	{
		button1Press = 1;
	}
	else if (digitalRead(SW2)==0) 
	{
		button2Press = 2;
		//Serial.print("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
	}
	
	TCCR2B &= 0xF8;	//clear prescaler bits to stop timer
}

ISR(PCINT0_vect)
{
		// set up timer with prescaler = 1024
		TCCR2B |= 0x7;
		
		// initialize counter
		TCNT2 = 0;
		
		// enable overflow interrupt
		TIMSK2 |= 1<<TOIE2;
} */