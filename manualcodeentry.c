#include "manualcodeentry.h"

#include "globaldefinitions.h"
#include "display.h"
#include "magnet.h"
#include "millis.h"
#include "keypad.h"

#include <util/delay.h>

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
