/********************************************************************************
Magstripe Emulator 2.0 
by Compton
********************************************************************************/

/********************************************************************************
Includes
********************************************************************************/
#include "globaldefinitions.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <avr/pgmspace.h>

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <util/delay.h>

#include "display.h"
#include "millis.h"
#include "magnet.h"
/********************************************************************************
Macros and Defines
********************************************************************************/
//Keypad key debounce time in milliseconds
#define keypad_debounce_time 25
 
#define mainMenuMaxPos 4 
#define debugMenuMaxPos 4 

/********************************************************************************
Function Prototypes
********************************************************************************/


/********************************************************************************
Global Constants
********************************************************************************/
enum menuState { MAIN, BRUTEFORCEWILDCARD, SAVEDCODES, SAVEDCODEEDITOR, DEBUGMENU };

#define testcardlength 5
char testcarddata[testcardlength]={';','1','2','3','?'};

/********************************************************************************
Global Variables
********************************************************************************/
enum menuState menustate = MAIN;
char menuPosition=0;
int testkeyinput=0;

/********************************************************************************
One pin analog keypad input. Halts execution until input or 'until' ms (if no input detected before until, returns -1)
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
			return -1;
		_delay_us(250);
	}
}

/*******************************
Temporary card data entry and output.
*******************************/
void manualCodeEntry(void){
	display_clear();
	display_setCursor(0,1);
	display_println("*=del #=output");
	char userinput[37];
	int userinputlength=0;
	int keyinput=-1;
	while(1){
		keyinput=inputpoll(-1);
		if(keyinput==-1)
			continue;
		else if(keyinput=='*'){
			if(userinputlength==0)//back to main menu
				return;
			userinput[userinputlength-1]=0;
			userinputlength--;
		}
		else if(keyinput=='#'){
			display_clear();
			display_setCursor(0,0);
			display_prints("Outputting...");
			resetMagnetBitBuffer();
			writeCharToBuffer(';');
			for(int i=0;i<userinputlength;i++){
				writeCharToBuffer(userinput[i]);
			}
			writeCharToBuffer('?');
			writeOutEMagBitBuffer();
			_delay_ms(1000); //Delay so the user doesn't instantly trigger it again
		}
		else{
			if(userinputlength<37){
				userinput[userinputlength]=keyinput;
				userinputlength++;
			}else{
				display_setCursor(0,1);
				display_println("At input limit");	
				_delay_ms(500);
				display_println("*=del #=output");					
			}
		}

		display_setCursor(0,1);
		display_println("*=del #=output");
		
		//display what the user has currently entered, showing the right part if the screen would overflow
		if(userinputlength>16){
			char text[17];
			text[16]='\0';
			for(int i=15;i>=0;i--){
				text[15-i]=userinput[userinputlength-1-i];
			}
			display_setCursor(0,0);
			display_prints(text);
		}else{
			display_setCursor(0,0);
			display_printl(userinput, userinputlength, 1);
		}
		
			
		}//end while
		//Cleanup

}

/******************************
Wild card output code
******************************/
void wildcardBruteForceOutput(char *inputstring, int inputstringlength, int *wildcardindices, int wildcardlength){
//User input backoff time
char backoffinput[9]={0};
int backoffinputlength=0;
long backofftime=50;
display_clear();
display_println("Backoff(ms): *->");

	while(1){
		int keyinput=inputpoll(-1);
		if(keyinput==-1)
		continue;
		else if(keyinput=='#'){
			if(backoffinputlength!=0){
				backoffinput[backoffinputlength-1]=0;
				backoffinputlength--;
			}
		}
		else if(keyinput=='*'){
			if(backoffinputlength!=0){
				backofftime=atol(backoffinput);
				break;
			}
		}else{
			if(backoffinputlength<8){
					backoffinput[backoffinputlength]=keyinput;
					backoffinputlength++;
				}else{
					display_setCursor(0,0);
					display_println("At input limit");
					_delay_ms(500);
					display_setCursor(0,0);
					display_println("Backoff(ms): #->");
			}
		}
		display_setCursor(0,1);
		display_printl(backoffinput,backoffinputlength,1);
	}
		
//Output then iterate the brute force
	millis_resume();
	long iterationstart;
	char everyother=0;
	while(1){
		display_clear();
		display_printl(inputstring, inputstringlength, 0);
		resetMagnetBitBuffer();
		writeCharToBuffer(';');
		for(int i=0;i<inputstringlength;i++){
			writeCharToBuffer(inputstring[i]);
		}
		writeCharToBuffer('?');
		if(everyother==0){
			display_setCursor(0,1);
			display_println("Outputting...");
			everyother=1;
		}else{
			display_setCursor(0,1);
			display_println("Hold 3=exit");
			everyother=0;
		}
		writeOutEMagBitBuffer();
		iterationstart=millis();
		
		//Iterate to next entry
		int i=0;
		while(1){
			int stringindex=wildcardindices[wildcardlength-1-i];
			if(inputstring[stringindex]=='9'){
				if(wildcardlength-1-i==0){//Reached end of the brute force
					display_clear();
					display_println("Reached end");
					display_setCursor(0,1);
					display_println("any=back to main");
					_delay_ms(500);
					inputpoll(-1);
					return;
				}
				inputstring[stringindex]='0';
				i++;
				continue;
			}else{
				inputstring[stringindex]+=1;
				break;
			}
		}
		
		if(inputpoll(keypad_debounce_time+5)=='3')//If 3 is held down exit to main menu
			return;
		millis_resume();//inputpoll pauses the millis timer
		
		//Backoff time
		while(millis()-iterationstart<backofftime)
			_delay_us(20);
		
	}
}

/*******************************
Wild card brute force input and output
*******************************/
void wildcardBruteForce(void){
	display_clear();
	display_setCursor(0,1);
	display_println("*=wild #=submenu");
	char userinput[37];
	int userinputlength=0;
	int keyinput=-1;
	while(1){
		keyinput=inputpoll(-1);
		if(keyinput==-1)
			continue;
		else if(keyinput=='#'){
			display_setCursor(0,1);
			display_println("*=next #=delete");
			keyinput=inputpoll(-1);
			if(keyinput=='#'){
				if(userinputlength==0)//back to main menu
					return;
				userinput[userinputlength-1]=0;
				userinputlength--;
			}else if(keyinput=='*'){
				//Find the number of wild cards, then fill an array with the indices of wild cards in the user input string
				int numberofwildcards=0;
				for(int i=0;i<userinputlength;i++){
					if(userinput[i]=='*')
						numberofwildcards++;
				}
				int wildcardindices[numberofwildcards];
				int j=0;
				for(int i=0;i<userinputlength;i++){
					if(userinput[i]=='*')
						wildcardindices[j++]=i;
				}
				//Now that we have an array of which positions are wild cards, replace the wild cards in the user string with 0s
				for(int i=0;i<userinputlength;i++){
					if(userinput[i]=='*')
						userinput[i]='0';
				}
				//go to output
				wildcardBruteForceOutput(userinput, userinputlength, wildcardindices, numberofwildcards);
				//after the above function returns, it's over
				return;
			}else{
				continue; //back to upper level entry
			}
			_delay_ms(100); //Delay so the user doesn't instantly trigger it again
		}
		else{
			if(userinputlength<37){
				userinput[userinputlength]=keyinput;
				userinputlength++;
				}else{
				display_setCursor(0,1);
				display_println("At input limit");
				_delay_ms(500);
				display_println("*=del #=output");
			}
		}

		display_setCursor(0,1);
		display_println("*=wild #=submenu");
		
		//display what the user has currently entered, showing the right part if the screen would overflow
		if(userinputlength>16){
			char text[17];
			text[16]='\0';
			for(int i=15;i>=0;i--){
				text[15-i]=userinput[userinputlength-1-i];
			}
			display_setCursor(0,0);
			display_prints(text);
			}else{
			display_setCursor(0,0);
			display_printl(userinput, userinputlength, 1);
		}
		
		
	}//end while
}

/********************************************************************************
Writes to display based on current menu state
********************************************************************************/
void updateMenuDisplay(void){
	display_setCursor(0,0);
	switch(menustate){
		case MAIN:
		switch(menuPosition){
			case 0:
			display_println("#=Manual Entry");
			display_setCursor(0,1);
			display_prints("1=Up 2=Dwn 3=Sav");
			break;
			case 1:
			display_println("#=EditSavedCodes");
			display_setCursor(0,1);
			display_prints("1=Up 2=Dwn 3=Sav");
			break;
			case 2:
			display_println("#=BruteForceWild");
			display_setCursor(0,1);
			display_prints("1=Up 2=Dwn 3=Sav");
			break;
			case 3:
			display_println("#=BruteForceMax");
			display_setCursor(0,1);
			display_prints("1=Up 2=Dwn 3=Sav");
			break;
			case 4:
			display_println("#=Debug Menu");
			display_setCursor(0,1);
			display_prints("1=Up 2=Dwn 3=Sav");
			break;
			default:
			display_clear();
			display_println("Eror: StateUnknwn");
			char textout[10];
			display_setCursor(0,1);
			itoa(menuPosition, textout, 10);
			display_println(textout);
			break;
		}
		break;
		case DEBUGMENU:
		switch(menuPosition){
			case 0:
			display_println("#=Output Test");
			display_setCursor(0,1);
			display_prints("1=Up 2=Dn 3=Back");
			break;
			case 1:
			display_println("Blank");
			display_setCursor(0,1);
			display_prints("1=Up 2=Dn 3=Back");
			break;
			case 2:
			display_println("Blank2");
			display_setCursor(0,1);
			display_prints("1=Up 2=Dn 3=Back");
			break;
			case 3:
			display_println("Blank3");
			display_setCursor(0,1);
			display_prints("1=Up 2=Dn 3=Back");
			break;
			case 4:
			display_println("Blank4");
			display_setCursor(0,1);
			display_prints("1=Up 2=Dn 3=Back");
			break;
			default:
			display_clear();
			display_println("Eror: StateUnknwn");
			char textout[10];
			display_setCursor(0,1);
			itoa(menuPosition, textout, 10);
			display_println(textout);
			break;
		}
		default:
			display_println("Eror: UnkwnMenu");
		break;
	}

}

/********************************************************************************
Main
********************************************************************************/

int main(void) {
	
	//Set all unused pins to low outputs, to prevent floating and save on power consumption
	OUTPUT(pinUnused1);
	LOW(pinUnused1);
	OUTPUT(pinUnused2);
	LOW(pinUnused2);
	OUTPUT(pinUnused3);
	LOW(pinUnused3);
	OUTPUT(pinUnused4);
	LOW(pinUnused4);
	OUTPUT(pinUnused5);
	LOW(pinUnused5);
	OUTPUT(pinUnused6);
	LOW(pinUnused6);
	OUTPUT(pinUnused7);
	LOW(pinUnused7);
	OUTPUT(pinUnused8);
	LOW(pinUnused8);
	OUTPUT(pinUnused9);
	LOW(pinUnused9);
	OUTPUT(pinUnused10);
	LOW(pinUnused10);
	OUTPUT(pinUnused11);
	LOW(pinUnused11);
			
	// initialize code
	OUTPUT(pinEMAGN);
	OUTPUT(pinEMAGS);
	LOW(pinEMAGN);
	LOW(pinEMAGS);
	OUTPUT(testLED);
		
	display_init(1);
	
	display_setCursor(0,0);
	display_prints("#=Manual Entry");
	display_setCursor(0,1);
	display_prints("1=Up 2=Dwn 3=Sav");

//display_clear();
//display_setCursor(0,0);

	/*//Turn on ADC and set prescaler to 64. F_CPU/Prescaler needs to be >50khz and <200khz for atmega8
	ADCSRA = (1<<ADEN) | (1<<ADPS2) | (1<<ADPS1);
	//Analog input pin select
	ADMUX=0x05;

	while(1){
		ADCSRA  |= (1<<ADSC); //start analog to digital conversion
		while (ADCSRA &  (1<<ADSC));  // wait until analog to digital conversion completes; ADSC=0 means Complete
		int present_key_input = ADCW;
		
		char textout[10];
		display_clear();
		itoa(present_key_input, textout, 10);
		display_print(textout);
		_delay_ms(500);
	}
*/

/*while(1){
	display_setCursor(0,0);
	char text[3];
	text[0]=' ';
	text[1]=inputpoll(500);
	text[2]='\0';
	if(text[1]==-1){
		text[0]='-';
		text[1]='1';	
	}
		
	display_print(text);
}*/

	
	millis_init(); //Initialize millisecond timer
	millis_pause();
	sei(); // Enable interrupts
	
	// main loop
	while (true) {
		int key;
		switch(menustate){
			case MAIN:;
					updateMenuDisplay();
					key=inputpoll(-1);
					switch(key){
						case '1':
							if(menuPosition==0)
								menuPosition=mainMenuMaxPos;
							else
								menuPosition-=1;

							break;
						case '2':
							menuPosition+=1;
							if(menuPosition>mainMenuMaxPos)
							menuPosition=0;
							break;
						case '3': //Go to the saved code shortcut menu
							break;
						case '#':
							switch(menuPosition){
								case 0:
									manualCodeEntry();
									break;
								case 1:
									display_clear();
									display_prints("Not Implemented");
									_delay_ms(500);
									break;
								case 2:
									wildcardBruteForce();
									break;
								case 4:
									menustate=DEBUGMENU;
									menuPosition=0;
									break;									
								default:
									break;
							}
							break;
						default:
							break;
					}
					break;
			case DEBUGMENU:;
					updateMenuDisplay();
					key=inputpoll(-1);
					switch(key){
						case '1':
							if(menuPosition==0)
								menuPosition=debugMenuMaxPos;
							else
								menuPosition-=1;
							break;
						case '2':
							menuPosition+=1;
							if(menuPosition>debugMenuMaxPos)
							menuPosition=0;
							break;
						case '3': //Back to main menu
							menustate=MAIN;
							menuPosition=0;
							break;
						case '#': //#, activate the selected menu item
							switch(menuPosition){
								case 0:
								while(1){
									display_clear();
									display_setCursor(0,0);
									display_prints("1=Output 2=Back");
									display_setCursor(0,1);
									display_prints("Data:");
									display_setCursor(4,1);
									display_printl(testcarddata, testcardlength, 0);

									key=inputpoll(-1);
									if(key=='1'){
										display_clear();
										display_setCursor(0,0);
										display_prints("Outputting...");
										resetMagnetBitBuffer();
										for(int i=0;i<testcardlength;i++){
											writeCharToBuffer(testcarddata[i]);
										}
										writeOutEMagBitBuffer();
										_delay_ms(1000); //Delay so the user doesn't instantly trigger it again
									}else if(key=='2'){
										updateMenuDisplay();
										break;
									}
								}
								break;
						}
						break;
						default:
						break;
					}
					break;
			default:
				break;
		}

	}

	return 1;
}
