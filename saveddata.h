#ifndef Saveddata_h
#define Saveddata_h
#include <inttypes.h>

#define numberOfCardDataSaveSlots 26
#define numberOfDigitsRequiredToRepresentSaveSlots 2

/*
* Save a string of card data to the chosen eeprom save slot
* @param [inputcarddata] String of characters to input. 39 characters max
* @param [inputlength] Number of characters
* @param [positiontosaveto] Slot to save into. Valid input: 0-numberofCardDataSaveSlots
* @return (none) length of the loaded card data
*/
void saveCardData(char *inputcarddata, int inputlength, uint8_t positiontosaveto);


/*
* Read a string of card data from a chosen eeprom save slot into the provided buffer array
* @param [buffer] Buffer to read the card data into. Must be at least 39 chars long. 
* @param [positiontoreadfrom] Save slot to read from. Valid input: 0-numberofCardDataSaveSlots
* @return (uint8_t) length of the loaded card data
*/
uint8_t retrieveCardData(char *buffer, uint8_t positiontoreadfrom);


#endif