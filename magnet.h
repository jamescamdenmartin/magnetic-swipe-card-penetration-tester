#ifndef Magnet_h
#define Magnet_h

#define maxNumberOfCardCharacters 40 
//Track 2 has up to 40 5-bit characters

/**
* Write a Track 2 data character to the electromagnet buffer. This method calculates the parity and writes the correct bits.
*
* @param [character] Track 2 data character to write to the buffer. Valid inputs:'0'-'9', ':', ';', '<', '=', '>', '?'
* @return (none)
*/
void writeCharToBuffer(char character);

/**
* Outputs the contents of the buffer over the electromagnet.
*
* @return (none)
*/
void writeOutEMagBitBuffer(void);

/**
* Clears the electromagnet bit buffer and resets the write position to the beginning.
*
* @return (none)
*/
void resetMagnetBitBuffer(void);

#endif