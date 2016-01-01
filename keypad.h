#ifndef Keypad_h
#define Keypad_h

/**
* Keypad input.
*
* @param [blockuntil] Blocks until blockuntil ms have passed. Blocks forever if until=-1
* @return (char) Detected keypad input. Returns ';' if no key input before until
*/
char inputpoll(long blockuntil);

/**
* Returns whether a char from inputpoll() is a valid Track 2 character.
*
* @param [input] Value to check
* @return (int) returns 0 if not a number, or false if it is
*/
int inputIsTrack2Char(char input);

#endif