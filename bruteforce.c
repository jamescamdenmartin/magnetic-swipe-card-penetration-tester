#include "bruteforce.h"
#include "globaldefinitions.h"
#include "display.h"
#include "magnet.h"
#include "millis.h"
#include <util/delay.h>
#include <stdlib.h>
#include "keypad.h"

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
		char keyinput=inputpoll(-1);
		if(keyinput==':')
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
	char keyinput=';';
	while(1){
		keyinput=inputpoll(-1);
		if(keyinput==';')
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
