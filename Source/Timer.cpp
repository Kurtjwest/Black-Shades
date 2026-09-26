/**> HEADER FILES <**/
#include <string.h>
#include "Timer.h"
#include "Support.h"


/********************> Timer <*****/
void TimerInit(timer* theTimer)
{
	theTimer->mm_timer_start   = PlatformTimeNanos();
	theTimer->mm_timer_elapsed = theTimer->mm_timer_start;
}

double TimerGetTime(timer* theTimer)
{
	return PlatformTimeNanos() - theTimer->mm_timer_start;
}
