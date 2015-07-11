#include "savedcodemenu.c"
#include "saveddata.h"
#include "magnet.h"
#include "display.h"
#include "keypad.h"

void savedCodeShortcutMenu(){
	int menupos=0;
	
	//Initialize to display the first saved data slot
	char menupostringbuffer[numberOfDigitsRequiredToRepresentSaveSlots+2]={0};
	char carddata[maxNumberOfCardCharacters]={0};
	uint8_t datalength=retrieveCardData(carddata, 0);
	
	while(1){
			display_clear();
			sprintf(menupostringbuffer, "%d)", menupos)
			display_prints(menupostringbuffer);
			display_printl(carddata, datalength, 1);
			display_setCursor(0,1);
			display_prints("#=OUT 1=U 2=D 3=B");
			
			char key;
			key=inputpoll(-1);
			switch(key){
				case '1': 
						if(menupos==0)
							menupos==maxNumberOfCardCharacters-1;
						else
							menupos-=1;
							
						datalength=retrieveCardData(carddata, menupos);
						break;
				case '2':
						if(menupos==maxNumberOfCardCharacters-1)
							menupos==0;
						else
							menupos+=1;
							
						datalength=retrieveCardData(carddata, menupos);				
						break;
				case '#':
						display_clear();
						display_setCursor(0,0);
						display_prints("Outputting...");
						resetMagnetBitBuffer();
						for(int i=0;i<datalength;i++){
							writeCharToBuffer(carddata[i]);
						}
						writeOutEMagBitBuffer();
						_delay_ms(1000); //Delay so the user doesn't instantly trigger it again
						break;
				case '3': 
						return; 
						break;
			}
	}
}

void savedCodeEditor(){
	int menupos=0;
	while(1){
	
	}
}