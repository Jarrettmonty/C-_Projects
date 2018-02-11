/*
*	File: clock.h
*
*	*description* 
*
*	Naming Convention as follows:
*	State *Functions* -> s_foo()
*	Pin Names		-> p_Pin1;
*	Typedefs (exc. State) -> _t (Following  convention)
*	Enumerations    -> e_STATE_TYPE
*	Variables are lCamelCase, Functions are lower_only();
*/
#include <LiquidCrystal.h>
#include <stdbool.h>


enum e_EVENT_NAMES
{
	TONE_ACTIVE = -1,
	NOT_ACTIVE =0,
	SW_ONE_ACTIVE = 1,
	SW_TWO_ACTIVE = 2
};
enum e_STATE_NAMES
{
	HOME_OFF,
	HOME_SET,
	MENU_0,
	MENU_1,
	MENU_2,
	TONE_,
	ADJUST_HOUR,
	ADJUST_MIN_CURRENT,
	ADJUST_MIN_ALARM
};



/*	Conversion values */
const long int msPerSec = 1000;
const long int secPerHour = 3600; //3,600,000
const long int secPerMin = 60;    //60,000
const unsigned long int	msPerDay = (3600000*24);	//86.4e6

/*    Pin Mapping    */
const int p_RS = 2, p_EN = 4, p_D4 = 5, p_D5 = 6, p_D6 = 7, p_D7 = 8; //LCD
const int p_SW1 = 12, p_SW2 = 11;
const int p_TRIMPOT = 0; //A0
const int p_LED = 13, p_PIEZO = 10;

typedef struct t_clock t_clock;	
typedef short t_signal;
typedef struct t_event t_event;
typedef void (*State)(t_clock *, const t_event*);

extern char* strList[9];
extern LiquidCrystal lcd;
extern volatile t_signal button1Press;
extern volatile t_signal button2Press;

struct t_event
{
	t_signal sig;
	t_signal status;
	t_signal prev;
};

struct t_clock
{
	State state_;
	State prev_;
	/*other attributes*/
	int currentDate[3];	// MM,DD,YY
	int setTime[4];	
	int currentTime[4];
	int alarmTime[4];
	uint32_t currentTimeInSec;
	uint32_t alarmTimeInSec;
	uint32_t additionalTimeInSec = 0;
	short hasTimeChanged;
	short isAlarmSet;
	
	short adjustKnobEnum;	
};


void alarm_ctor(t_clock*);
void process_event(t_clock*, const t_event*);
void assign_clock_state(t_clock*, State);
void trans_clock_state(State, State, State, State);

void s_adjust_alarm_min(t_clock* me, const t_event* e);
void s_adjust_alarm_hour(t_clock* me, const t_event* e);
void s_adjust_current_hour(t_clock* me, const t_event* e);
void s_adjust_current_min(t_clock* me, const t_event* e);
void s_clock_menu(t_clock* me, const t_event* e);
void s_clock_tone(t_clock* me, const t_event* e);
void s_clock_home(t_clock* me, const t_event* e);
void s_clock_home_set(t_clock* me, const t_event* e);


 
void delay_clock_tone(t_clock* me);
void write_lcd_by_state(e_STATE_NAMES option, t_clock* me);
void write_cur_time_to_lcd(e_STATE_NAMES option, t_clock* me, int);
void write_alarm_time_to_lcd(e_STATE_NAMES option, t_clock* me, int);
void update_current_time(t_clock* me);
void convert_set_time_to_sec(t_clock* me);
void convert_alarm_time_to_sec(t_clock* me);

int get_adc_value(int timeType);
int get_hrs_today();
int get_60_min_today();
unsigned long int get_milli_today();
unsigned long int getClock_DELAY(t_clock* me);
void sec_to_array_time(long int, int*);
long int array_time_to_sec(int *);


/*various boolean checks*/
_Bool is_alarm_ready(t_clock* me);
_Bool was_prev_state(t_clock* me, State prevState);


