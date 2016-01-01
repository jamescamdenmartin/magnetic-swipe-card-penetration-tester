#include "savedcodemenu.h"
#include "globaldefinitions.h"
#include "saveddata.h"
#include "magnet.h"
#include "display.h"
#include "keypad.h"

#include <util/delay.h>

void savedCodeShortcutMenu(){
	int menupos=0;
	
	//Initialize to display the first saved data slot
	char menupostringbuffer[numberOfDigitsRequiredToRepresentSaveSlots+2]={0};
	char carddata[maxNumberOfCardCharacters]={0};
	uint8_t datalength=retrieveCardData(carddata, 0);
	
	while(1){
			display_clear();
			sprintf(menupostringbuffer, "%d)", menupos);
			display_prints(menupostringbuffer);
			display_printl(carddata, datalength, 1);
			display_setCursor(0,1);
			display_prints("#=OUT 1=U 2=D 3=B");
			
			char key;
			key=inputpoll(-1);
			switch(key){
				case '1':
						if(menupos==0)
							menupos=maxNumberOfCardCharacters-1;
						else
							menupos-=1;
							
						datalength=retrieveCardData(carddata, menupos);
						break;
				case '2':
						if(menupos==maxNumberOfCardCharacters-1)
							menupos=0;
						else
							menupos+=1;
							
						datalength=retrieveCardData(carddata, menupos);				
						break;
				case '#':
						display_clear();
						display_setCursor(0,0);
						display_prints("Outputting...");
						resetMagnetBitBuffer();
						writeCharToBuffer(';');
						for(uint8_t i=0;i<datalength;i++){
							writeCharToBuffer(carddata[i]);
						}
						writeCharToBuffer('?');
						writeOutEMagBitBuffer();
						_delay_ms(1000); //Delay so the user doesn't instantly trigger it again
						break;
				case '3': 
						return; 
						break;
			}
	}
}


void editCode(char* carddata, uint8_t datalength, uint8_t positiontosaveto){
	display_clear();
	char* userinput=carddata;
	char keyinput=-1;
	while(1){
		//display what the user has currently entered, showing the right part if the screen would overflow
		if(datalength>16){
			char text[17];
			text[16]='\0';
			for(int8_t i=15;i>=0;i--){
				text[15-i]=userinput[datalength-1-i];
			}
			display_setCursor(0,0);
			display_prints(text);
		}else{
			display_setCursor(0,0);
			display_printl(userinput, datalength, 1);
		}
		display_setCursor(0,1);
		display_println("*=del #=save");
		
		keyinput=inputpoll(-1);
		if(keyinput==';')
			continue;
		else if(keyinput=='*'){
			if(datalength==0)//back to main menu
			continue;
			userinput[datalength-1]=0;
			datalength--;
		}
		else if(keyinput=='#'){
			display_clear();
			display_setCursor(0,0);
			display_prints("Saving...");
			userinput[datalength]='?';//mark end of string
			datalength++;
			saveCardData(userinput, datalength, positiontosaveto);
			return;
		}
		else{
			if(datalength<maxNumberOfCardCharacters-2){//Automatically put in the start ; and end ? sentinels in the output code, so limit user to max characters-2
					userinput[datalength]=keyinput;
					datalength++;
			}else{
				display_setCursor(0,1);
				display_println("At input limit");
				_delay_ms(500);
				display_println("*=del #=output");
			}
		}
	}
}

void savedCodeEditor(){
	int menupos=0;
	
	//Initialize to display the first saved data slot
	char menupostringbuffer[numberOfDigitsRequiredToRepresentSaveSlots+2]={0};
	char carddata[maxNumberOfCardCharacters]={0};
	uint8_t datalength=retrieveCardData(carddata, 0);
	
	while(1){
		display_clear();
		sprintf(menupostringbuffer, "%d)", menupos);
		display_prints(menupostringbuffer);
		if(datalength==0)
			display_prints("Empty");
		else
			display_printl(carddata, datalength, 1);
		
		display_setCursor(0,1);
		display_prints("#=EDT 1=U 2=D 3=B");
		
		char key;
		key=inputpoll(-1);
		switch(key){
			case '1':
				if(menupos==0)
					menupos=maxNumberOfCardCharacters-1;
				else
					menupos-=1;
				
				datalength=retrieveCardData(carddata, menupos);
				break;
			case '2':
				if(menupos==maxNumberOfCardCharacters-1)
					menupos=0;
				else
					menupos+=1;
				
				datalength=retrieveCardData(carddata, menupos);
			break;
			case '#':
				editCode(carddata,datalength,menupos);
				datalength=retrieveCardData(carddata, menupos);
				break;
			case '3':
				return;
			break;
		}
	}
}