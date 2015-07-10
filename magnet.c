/**************************************************************
Magnetic Swipe Card Track 2 Emulator Code
**************************************************************/
/*
ISO standard track 2 data format:
1. Leading clocking zeros: Approximately 22
2. The start sentinel (hexadecimal "B", ASCII ';')
3. Up to 37 hexadecimal numeric characters selected by the user
4. The end sentinel (hexadecimal "F", ACSCII '?')
5. The LRC (longitudinal redundancy check character)
6. Trailing clocking zeros: A minimum of 22 to the end of the card

195 bits of data not counting the LRC
*/
#include "globaldefinitions.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdbool.h>
#include <avr/eeprom.h>
#include <stdio.h>
#include <string.h>
#include <avr/pgmspace.h>
#include <util/delay.h>

uint8_t magnetPolarity = 0; //direction of the last strip of magnet that was emulated
uint8_t magnetbitbufferlength=0;
char magnetbitbuffer[40*5]; //Track 2 has up to 40 5-bit characters
char lrcbyte=0;

//Physical output code for writing bits to the card reader
//The reader detects the positions where a change in polarity occurs in a strip of magnets dragged across it
//1 bits have two polarity changes, with half the frequency of 0 bits.
//Example:
//   0  |   1  |  0  |   0
//N----SS--NN--SS----NN----S
void writeEMagBit(char lowOrHigh)
{
	if(lowOrHigh == 1)
	{
		if(magnetPolarity == 0)
		{
			LOW(pinEMAGN);
			HIGH(pinEMAGS);
			_delay_us(swipeCardSpeed); //magpolarity=1 now
			LOW(pinEMAGS);
			HIGH(pinEMAGN);
			_delay_us(swipeCardSpeed);//magpolarity=0 now
		}
		else
		{
			LOW(pinEMAGS);
			HIGH(pinEMAGN);
			_delay_us(swipeCardSpeed); //magpolarity=0 now
			LOW(pinEMAGN);
			HIGH(pinEMAGS);
			_delay_us(swipeCardSpeed); //magpolarity=1 now
		}
	}
	else
	{
		if(magnetPolarity == 0)
		{
			LOW(pinEMAGN);
			HIGH(pinEMAGS);
			_delay_us(swipeCardSpeed * 2);
			magnetPolarity = 1;
		}
		else
		{
			LOW(pinEMAGS);
			HIGH(pinEMAGN);
			_delay_us(swipeCardSpeed * 2);
			magnetPolarity = 0;
		}
	}
}

/*
Calculating odd parity and LRC for the message 123:
                  P 8 4 2 1     Hex Value Decimal Value
              Less Parity Less Parity
Start Sentinel: B 0 1 0 1 1   B 11 (8+2+1)
                1 0 0 0 0 1   1 1
                2 0 0 0 1 0   2 2
                3 1 0 0 1 1   3 3 (2+1)
End Sentinel:   F 1 1 1 1 1   F 15 (8+4+2+1)
                ___________   __
LRC             4 0 0 1 0 0   4 4
Calculating odd parity: scan each horizontal row and enter a zero or one In the P column so that
each row has an odd number of one's.
To find the LCR: disregard the parity bits, scan up each vertical column, and make sure that each
column has a even number of one's in it. Enter a zero or one in the LCR position, so that it does.
Next, calculate odd parity for the LCR character, in the horizontal direction.
*/
void calculateLRC(void){
  char bit8, bit4, bit2, bit1;
  char numberofones=0;
  
  for(int i=3;i<magnetbitbufferlength;i+=5){
    if(magnetbitbuffer[i]==1){
      numberofones++;
    }
  }
  if(numberofones%2==0 || numberofones==0){//was an even number of ones in the column
    bit8=0;
  }else{//was an odd number of ones in the column, make even
    bit8=1;
  }
  
  numberofones=0;
  for(int i=2;i<magnetbitbufferlength;i+=5){
    if(magnetbitbuffer[i]==1){
      numberofones++;
    }
  }
  if(numberofones%2==0 || numberofones==0){
    bit4=0;
  }else{
    bit4=1;
  }
  
  numberofones=0;
  for(int i=1;i<magnetbitbufferlength;i+=5){
    if(magnetbitbuffer[i]==1){
      numberofones++;
    }
  }
  if(numberofones%2==0 || numberofones==0){
    bit2=0;
  }else{
    bit2=1;
  }
  
  numberofones=0;
  for(int i=0;i<magnetbitbufferlength;i+=5){
    if(magnetbitbuffer[i]==1){
      numberofones++;
    }
  }
  if(numberofones%2==0 || numberofones==0){
    bit1=0;
  }else{
    bit1=1;
  }
  
  lrcbyte=bit8*8 + bit4*4 + bit2*2 + bit1;
  
  //if(debugmode){
  //  Serial.print("Calculated lrc: ");
  //  Serial.println(lrcbyte);
  //}
}

void writeBitToBuffer(char paramBit)
{
	magnetbitbuffer[magnetbitbufferlength]=paramBit;
	magnetbitbufferlength++;
}



/*
Track 2 contains this 5 bit data format

 ANSI/ISO BCD Data Format
---------

 * Remember that b1 (bit #1) is the LSB (least significant bit)!
  * The LSB is read FIRST!
  * Hexadecimal conversions of the Data Bits are given in parenthesis (xH).

        --Data Bits--   Parity
        b1  b2  b3  b4   b5    Character  Function

        0   0   0   0    1        0 (0H)    Data
        1   0   0   0    0        1 (1H)      "
        0   1   0   0    0        2 (2H)      "
        1   1   0   0    1        3 (3H)      "
        0   0   1   0    0        4 (4H)      "
        1   0   1   0    1        5 (5H)      "
        0   1   1   0    1        6 (6H)      "
        1   1   1   0    0        7 (7H)      "
        0   0   0   1    0        8 (8H)      "
        1   0   0   1    1        9 (9H)      "
        0   1   0   1    1        : (AH)    Control
        1   1   0   1    0        ; (BH)    Start Sentinel
        0   0   1   1    1        < (CH)    Control
        1   0   1   1    0        = (DH)    Field Separator
        0   1   1   1    0        > (EH)    Control
        1   1   1   1    1        ? (FH)    End Sentinel


           ***** 16 Character 5-bit Set *****
                10 Numeric Data Characters
                3 Framing/Field Characters
                
                
The character is written "backwards" on the card starting with the least significant bit and ending
with the parity bit. The card data format Is "1, 2, 4, 8, parity".
*/
void writeCharToBuffer(char charToWrite)
{
  switch(charToWrite){
    case '0': writeBitToBuffer(0); writeBitToBuffer(0); writeBitToBuffer(0); writeBitToBuffer(0); writeBitToBuffer(1); break;
    case '1': writeBitToBuffer(1); writeBitToBuffer(0); writeBitToBuffer(0); writeBitToBuffer(0); writeBitToBuffer(0); break;
    case '2': writeBitToBuffer(0); writeBitToBuffer(1); writeBitToBuffer(0); writeBitToBuffer(0); writeBitToBuffer(0); break;    
    case '3': writeBitToBuffer(1); writeBitToBuffer(1); writeBitToBuffer(0); writeBitToBuffer(0); writeBitToBuffer(1); break; 
    case '4': writeBitToBuffer(0); writeBitToBuffer(0); writeBitToBuffer(1); writeBitToBuffer(0); writeBitToBuffer(0); break;
    case '5': writeBitToBuffer(1); writeBitToBuffer(0); writeBitToBuffer(1); writeBitToBuffer(0); writeBitToBuffer(1); break;    
    case '6': writeBitToBuffer(0); writeBitToBuffer(1); writeBitToBuffer(1); writeBitToBuffer(0); writeBitToBuffer(1); break;
    case '7': writeBitToBuffer(1); writeBitToBuffer(1); writeBitToBuffer(1); writeBitToBuffer(0); writeBitToBuffer(0); break;
    case '8': writeBitToBuffer(0); writeBitToBuffer(0); writeBitToBuffer(0); writeBitToBuffer(1); writeBitToBuffer(0); break;
    case '9': writeBitToBuffer(1); writeBitToBuffer(0); writeBitToBuffer(0); writeBitToBuffer(1); writeBitToBuffer(1); break;
    case ':': writeBitToBuffer(0); writeBitToBuffer(1); writeBitToBuffer(0); writeBitToBuffer(1); writeBitToBuffer(1); break;
    case ';': writeBitToBuffer(1); writeBitToBuffer(1); writeBitToBuffer(0); writeBitToBuffer(1); writeBitToBuffer(0); break;
    case '<': writeBitToBuffer(0); writeBitToBuffer(0); writeBitToBuffer(1); writeBitToBuffer(1); writeBitToBuffer(1); break;
    case '=': writeBitToBuffer(1); writeBitToBuffer(0); writeBitToBuffer(1); writeBitToBuffer(1); writeBitToBuffer(0); break;   
    case '>': writeBitToBuffer(0); writeBitToBuffer(1); writeBitToBuffer(1); writeBitToBuffer(1); writeBitToBuffer(0); break;   
    case '?': writeBitToBuffer(1); writeBitToBuffer(1); writeBitToBuffer(1); writeBitToBuffer(1); writeBitToBuffer(1); break;   
    default: /*if(debugmode){Serial.print("Tried to write an invalid character to the bit buffer: "); Serial.println(charToWrite);}*/ break;
  }
}

//Writes the LRC out to the electromagnet now
void writeLRC(void)
{
  switch(lrcbyte){
    case 0: writeEMagBit(0); writeEMagBit(0); writeEMagBit(0); writeEMagBit(0); writeEMagBit(1); break;
    case 1: writeEMagBit(1); writeEMagBit(0); writeEMagBit(0); writeEMagBit(0); writeEMagBit(0); break;
    case 2: writeEMagBit(0); writeEMagBit(1); writeEMagBit(0); writeEMagBit(0); writeEMagBit(0); break;    
    case 3: writeEMagBit(1); writeEMagBit(1); writeEMagBit(0); writeEMagBit(0); writeEMagBit(1); break; 
    case 4: writeEMagBit(0); writeEMagBit(0); writeEMagBit(1); writeEMagBit(0); writeEMagBit(0); break;
    case 5: writeEMagBit(1); writeEMagBit(0); writeEMagBit(1); writeEMagBit(0); writeEMagBit(1); break;    
    case 6: writeEMagBit(0); writeEMagBit(1); writeEMagBit(1); writeEMagBit(0); writeEMagBit(1); break;
    case 7: writeEMagBit(1); writeEMagBit(1); writeEMagBit(1); writeEMagBit(0); writeEMagBit(0); break;
    case 8: writeEMagBit(0); writeEMagBit(0); writeEMagBit(0); writeEMagBit(1); writeEMagBit(0); break;
    case 9: writeEMagBit(1); writeEMagBit(0); writeEMagBit(0); writeEMagBit(1); writeEMagBit(1); break;
    case 10: writeEMagBit(0); writeEMagBit(1); writeEMagBit(0); writeEMagBit(1); writeEMagBit(1); break;
    case 11: writeEMagBit(1); writeEMagBit(1); writeEMagBit(0); writeEMagBit(1); writeEMagBit(0); break;
    case 12: writeEMagBit(0); writeEMagBit(0); writeEMagBit(1); writeEMagBit(1); writeEMagBit(1); break;
    case 13: writeEMagBit(1); writeEMagBit(0); writeEMagBit(1); writeEMagBit(1); writeEMagBit(0); break;   
    case 14: writeEMagBit(0); writeEMagBit(1); writeEMagBit(1); writeEMagBit(1); writeEMagBit(0); break;   
    case 15: writeEMagBit(1); writeEMagBit(1); writeEMagBit(1); writeEMagBit(1); writeEMagBit(1); break; 
    default: /*if(debugmode){Serial.print("LRC calculation broken, got an invalid value: "); Serial.println(lrcbyte);}*/ break;  
  }
}

//Call to write out the bit buffer contents to a card reader over the electromagnet
void writeOutEMagBitBuffer(){
	//if(debugmode){
	//	Serial.println("Attempting to write data over electromagnet");
	//}
	
	//Calculate the LRC from the bit buffer data
	calculateLRC();
	//The card reader clock syncs up with 0 bit pulses until the sentinel bit is sent. Number can vvaryery, 22 is the minimum set in the standard.
	for(int i = 0; i < 22; i ++)
	{
		writeEMagBit(0);
	}
	//Write out the actual card data
	for(int i=0;i<magnetbitbufferlength;i++){
		writeEMagBit(magnetbitbuffer[i]);
	}
	//Write out the calculated Longitudinal Redundancy Check
	writeLRC();
	//Cards contain clocking bits on the end too, in case of reverse swiping
	for(int i = 0; i < 22; i ++)
	{
		writeEMagBit(0);
	}
	
	
	 //Turn off the electromagnet
	LOW(pinEMAGN);
	LOW(pinEMAGS);
	
	//if(debugmode){
	//	Serial.println("Write attempt finished");
	//}
}

void resetMagnetBitBuffer(){
	for(int i=0;i<magnetbitbufferlength;i++){
		magnetbitbuffer[i]=0;
	}
	magnetbitbufferlength=0;
	lrcbyte=0;
}
