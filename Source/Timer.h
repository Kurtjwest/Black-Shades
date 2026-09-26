#ifndef _TIMER_H_
#define _TIMER_H_

/*
 * Frame timer.  The original stored a 32-bit microsecond count, which wrapped
 * after ~71 minutes of play (and broke outright on 64-bit, where the wrap no
 * longer happened in the subtraction); this keeps a 64-bit monotonic base and
 * returns nanoseconds as a double, the same unit the game loop expects.
 */
class timer
{
	public:
		double mm_timer_start;
		double mm_timer_elapsed;
};

void TimerInit(timer* theTimer);
double TimerGetTime(timer* theTimer);

#endif
