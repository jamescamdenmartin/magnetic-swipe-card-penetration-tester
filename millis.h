#ifndef Millis_h
#define Millis_h

#define millis() millis_get()

/**
* Initialise, must be called before anything else
*
* @return (none)
*/
void millis_init(void);
/**
* Get milliseconds.
*
* @return Milliseconds.
*/
long millis_get(void);
/**
* Turn on timer and resume time keeping.
*
* @return (none)
*/
void millis_resume(void);
/**
* Pause time keeping and turn off timer to save power.
*
* @return (none)
*/
void millis_pause(void);
/**
* Reset milliseconds count to 0.
*
* @return (none)
*/
void millis_reset(void);
/**
* Add time.
*
* @param [ms] Milliseconds to add.
* @return (none)
*/
void millis_add(long ms);
/**
* Subtract time.
*
* @param [ms] Milliseconds to subtract.
* @return (none)
*/
void millis_subtract(long ms);

#endif