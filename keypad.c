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

			if(1019-keypad_marginoferror < present_key_input && present_key_input < 1019+keypad_marginoferror){
				return '1';
			}
			else if(671-keypad_marginoferror < present_key_input && present_key_input < 671+keypad_marginoferror){
				return '2';
			}
			else if(499-keypad_marginoferror < present_key_input && present_key_input < 499+keypad_marginoferror){
				return '3';
			}
			else if(900-keypad_marginoferror < present_key_input && present_key_input < 900+keypad_marginoferror){
				return '4';
			}
			else if(617-keypad_marginoferror < present_key_input && present_key_input < 617+keypad_marginoferror){
				return '5';
			}
			else if(469-keypad_marginoferror < present_key_input && present_key_input < 469+keypad_marginoferror){
				return '6';
			}
			else if(805-keypad_marginoferror < present_key_input && present_key_input < 805+keypad_marginoferror){
				return '7';
			}
			else if(571-keypad_marginoferror < present_key_input && present_key_input < 571+keypad_marginoferror){
				return '8';
			}
			else if(442-keypad_marginoferror < present_key_input && present_key_input < 442+keypad_marginoferror){
				return '9';
			}
			else if(729-keypad_marginoferror < present_key_input && present_key_input < 729+keypad_marginoferror){
				return '*';
			}
			else if(531-keypad_marginoferror < present_key_input && present_key_input < 531+keypad_marginoferror){
				return '0';
			}
			else if(418-keypad_marginoferror < present_key_input && present_key_input < 418+keypad_marginoferror){
				return '#';
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
