#include "keypad.h"
#include "globaldefinitions.h"
#include "millis.h"

#include <util/delay.h>
#include <avr/io.h>
/********************************************************************************
One pin analog keypad input. Halts execution until input or 'until' ms (if no input detected before until, returns ';')
********************************************************************************/
int old_key_input=-1;
int present_key_input=0;
long time_key_first_detected=0;
#define keypad_marginoferror 4
char inputpoll(long until) {
	millis_resume();
	long start=millis();
	//Turn on ADC and set prescaler to 64. F_CPU/Prescaler needs to be >50khz and <200khz for atmega8
	ADCSRA = (1<<ADEN) | (1<<ADPS2) | (1<<ADPS1);
	
	//Analog input pin select
	ADMUX=0x05;
	
	//#=10 *=11, then 0-9 keys = 0-9
	while(1){//Poll keypad
		ADCSRA  |= (1<<ADSC); //start analog to digital conversion
		while (ADCSRA &  (1<<ADSC));  // wait until analog to digital conversion completes; ADSC=0 means Complete
		
		//For avr simulator debugging:
		//present_key_input=testkeyinput;
		present_key_input = ADCW;
		if(present_key_input-keypad_marginoferror<old_key_input && present_key_input+keypad_marginoferror>old_key_input){//Input state changed, record when for debounce
			time_key_first_detected = millis();
			old_key_input=present_key_input;
		}

		if(millis()-time_key_first_detected > keypad_debounce_time){ //Input state has held constant for long enough, return the pressed key
			millis_pause();
			time_key_first_detected+=500;//Stop a held down key from instantly repeating.

			if(present_key_input<keypad_marginoferror)//No button held down
			millis_resume();

			if(1017-keypad_marginoferror < present_key_input && present_key_input < 1017+keypad_marginoferror){
				return '1';
			}
			else if(671-keypad_marginoferror < present_key_input && present_key_input < 671+keypad_marginoferror){
				return '2';
			}
			else if(500-keypad_marginoferror < present_key_input && present_key_input < 500+keypad_marginoferror){
				return '3';
			}
			else if(850-keypad_marginoferror < present_key_input && present_key_input < 850+keypad_marginoferror){
				return '4';
			}
			else if(594-keypad_marginoferror < present_key_input && present_key_input < 594+keypad_marginoferror){
				return '5';
			}
			else if(456-keypad_marginoferror < present_key_input && present_key_input < 456+keypad_marginoferror){
				return '6';
			}
			else if(732-keypad_marginoferror < present_key_input && present_key_input < 732+keypad_marginoferror){
				return '7';
			}
			else if(533-keypad_marginoferror < present_key_input && present_key_input < 533+keypad_marginoferror){
				return '8';
			}
			else if(419-keypad_marginoferror < present_key_input && present_key_input < 419+keypad_marginoferror){
				return '9';
			}
			else if(640-keypad_marginoferror < present_key_input && present_key_input < 640+keypad_marginoferror){
				return '*';
			}
			else if(482-keypad_marginoferror < present_key_input && present_key_input < 482+keypad_marginoferror){
				return '0';
			}
			else if(388-keypad_marginoferror < present_key_input && present_key_input < 388+keypad_marginoferror){
				return '#';
			}
			else if(400-keypad_marginoferror < present_key_input && present_key_input < 400+keypad_marginoferror){
				return ':';
			}
			else if(370-keypad_marginoferror < present_key_input && present_key_input < 370+keypad_marginoferror){
				return '<';
			}
			else if(346-keypad_marginoferror < present_key_input && present_key_input < 346+keypad_marginoferror){
				return '=';
			}
			else if(324-keypad_marginoferror < present_key_input && present_key_input < 324+keypad_marginoferror){
				return '>';
			}
			else{//Unknown key value. Check if there's a problem with the keypad resistor network. Or maybe the user is holding down multiple keys.
				millis_resume();
			}
		}
		
		if(until!=-1 && millis()-start>until) //waited long enough, give up
			return ';';
		_delay_us(250);
	}
}

int inputIsTrack2Char(char input) {
	if( input >= '0' && input <= '?' )
		return 1;
	else
		return 0;
}

