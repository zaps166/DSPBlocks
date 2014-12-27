#include "Global.hpp"

#ifdef Q_OS_LINUX
	#include <sched.h>

	bool Global::realTime = false;
	Global::RT_MODE Global::rt_mode = Global::CLOCK_NANOSLEEP;
	int Global::cpu = 0, Global::sched = SCHED_FIFO, Global::priority = DEFAULT_PRIORITY;
#endif

bool Global::nativeFileDialogFlag = true;
int Global::sampleRate = DEFAULT_SAMPLERATE, Global::refTime = DEFAULT_REFTIME;
