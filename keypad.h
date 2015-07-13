#ifndef Keypad_h
#define Keypad_h

/**
* Keypad input.
*
* @param [blockuntil] Blocks until blockuntil ms have passed. Blocks forever if until=-1
* @return (char) Detected keypad input. Returns ';' if no key input before until
*/
char inputpoll(long blockuntil) ;


#endif