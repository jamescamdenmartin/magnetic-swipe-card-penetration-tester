#include "saveddata.h"
#include <avr/eeprom.h>
/*
*
* Handles any reading and writing to the eeprom.
*
*/

/* atmega8 has 512bytes of eeprom.
* Each Track 2 can have up to 39 characters, 4 bits needed for each character. 
* Data structure:
* starting at first address:
* 25 sections of 160 bits each, storing up to 26 saved cards. Starts at index=0, =20, =40, etc
* If the section is all zeros, consider the section is empty. Else when reading the section, the length is either the full 160 bits or until the card end sentinel character is read.
*/

void saveCardData(char *inputcarddata, int inputlength, uint8_t positiontosaveto){
	  if(positiontosaveto>25)return;//Only 0-25 are valid save slots
	  
	  
	/*inputlength=5;
	inputcarddata[0]='1';
	inputcarddata[1]='2';
	inputcarddata[2]='1';
	inputcarddata[3]='2';
	inputcarddata[4]='?';*/
		  
	  uint8_t bytetowrite=0;
	  uint8_t dataoffset=0;
	  //pack two characters into each byte before writing. Low bits even characters then high bits odd characters.
	  for(uint8_t i=0;i<inputlength;i+=2){
		  switch(inputcarddata[i]){
			  case '0': bytetowrite=0x00; break;
			  case '1': bytetowrite=0x01; break;
			  case '2': bytetowrite=0x02; break;
			  case '3': bytetowrite=0x03; break; 
			  case '4': bytetowrite=0x04; break; 
			  case '5':	bytetowrite=0x05; break;
			  case '6':	bytetowrite=0x06; break;
			  case '7': bytetowrite=0x07; break;
			  case '8': bytetowrite=0x08; break;
			  case '9': bytetowrite=0x09; break;
			  case ':': bytetowrite=0x0A; break;
			  case ';': bytetowrite=0x0B; break;
			  case '<': bytetowrite=0x0C; break;
			  case '=': bytetowrite=0x0D; break;
			  case '>': bytetowrite=0x0E; break;
			  case '?': bytetowrite=0x0F; break;
			  default: break;
			}
			if(i+1<inputlength){
				switch(inputcarddata[i+1]){
				  case '0': bytetowrite|=0x00; break;
				  case '1': bytetowrite|=0x10; break;
				  case '2': bytetowrite|=0x20; break;
				  case '3': bytetowrite|=0x30; break;
				  case '4': bytetowrite|=0x40; break;
				  case '5':	bytetowrite|=0x50; break;
				  case '6':	bytetowrite|=0x60; break;
				  case '7': bytetowrite|=0x70; break;
				  case '8': bytetowrite|=0x80; break;
				  case '9': bytetowrite|=0x90; break;
				  case ':': bytetowrite|=0xA0; break;
				  case ';': bytetowrite|=0xB0; break;
				  case '<': bytetowrite|=0xC0; break;
				  case '=': bytetowrite|=0xD0; break;
				  case '>': bytetowrite|=0xE0; break;
				  case '?': bytetowrite|=0xF0; break;
				}
			}
			eeprom_update_byte((uint8_t*)(positiontosaveto*20+dataoffset),(uint8_t)bytetowrite);
			dataoffset+=1;
		 }
		 //Clear the bytes that weren't written to by the string
		 for(uint8_t i=19;i>dataoffset-1;i--){
			eeprom_update_byte((uint8_t*)(positiontosaveto*20+i),(uint8_t)0);			 
		 }
}

uint8_t retrieveCardData(char *buffer, uint8_t positiontoreadfrom){
	if(positiontoreadfrom>25)return 0; //Only 0-25 are valid save slots.
	
	  uint8_t dataoffset=0;
	  uint8_t tempbyte=0;
	  int8_t numberofcharacters=0;
	  //pack two characters into each byte before writing. Low bits even characters then high bits odd characters.
	  while(1){
		  tempbyte=eeprom_read_byte((uint8_t*)(positiontoreadfrom*20+dataoffset));
		  
		  buffer[numberofcharacters]=tempbyte & 0x0F;
		  switch(buffer[numberofcharacters]){
			  case 0x0: buffer[numberofcharacters]='0'; break;
			  case 0x1: buffer[numberofcharacters]='1'; break;
			  case 0x2: buffer[numberofcharacters]='2'; break;
			  case 0x3: buffer[numberofcharacters]='3'; break;
			  case 0x4: buffer[numberofcharacters]='4'; break;
			  case 0x5: buffer[numberofcharacters]='5'; break;
			  case 0x6: buffer[numberofcharacters]='6'; break;
			  case 0x7: buffer[numberofcharacters]='7'; break;
			  case 0x8: buffer[numberofcharacters]='8'; break;
			  case 0x9: buffer[numberofcharacters]='9'; break;
			  case 0xA: buffer[numberofcharacters]=':'; break;
			  case 0xB: buffer[numberofcharacters]=';'; break;
			  case 0xC: buffer[numberofcharacters]='<'; break;
			  case 0xD: buffer[numberofcharacters]='='; break;
			  case 0xE: buffer[numberofcharacters]='>'; break;
			  case 0xF: buffer[numberofcharacters]='?'; break;
		  }

		  //Check for the end marker
		  if(buffer[numberofcharacters]=='?'){
			//Return everything except for the end marker, don't numberofcharacters+=1;
			break;
		  }else{
			numberofcharacters+=1;
		  }
		  
		  buffer[numberofcharacters]=(tempbyte >> 4) & 0x0F;
		  switch(buffer[numberofcharacters]){
			  case 0x0: buffer[numberofcharacters]='0'; break;
			  case 0x1: buffer[numberofcharacters]='1'; break;
			  case 0x2: buffer[numberofcharacters]='2'; break;
			  case 0x3: buffer[numberofcharacters]='3'; break;
			  case 0x4: buffer[numberofcharacters]='4'; break;
			  case 0x5: buffer[numberofcharacters]='5'; break;
			  case 0x6: buffer[numberofcharacters]='6'; break;
			  case 0x7: buffer[numberofcharacters]='7'; break;
			  case 0x8: buffer[numberofcharacters]='8'; break;
			  case 0x9: buffer[numberofcharacters]='9'; break;
			  case 0xA: buffer[numberofcharacters]=':'; break;
			  case 0xB: buffer[numberofcharacters]=';'; break;
			  case 0xC: buffer[numberofcharacters]='<'; break;
			  case 0xD: buffer[numberofcharacters]='='; break;
			  case 0xE: buffer[numberofcharacters]='>'; break;
			  case 0xF: buffer[numberofcharacters]='?'; break;
		  }
		  
		  //Check for the end marker
		  if(buffer[numberofcharacters]=='?'){
			  //Return everything except for the end marker, don't numberofcharacters+=1;
			  break;
		  }else{
			  numberofcharacters+=1;
		  }
		  
			//Check if we've reached the max number of characters
			if(numberofcharacters==39){
				//No end sentinel, data is either corrupt or the entry is empty.
				//Return an empty save slot.
				numberofcharacters=0;
				break;
			}
		  
		  dataoffset+=1;
	  }
	  return numberofcharacters;
}