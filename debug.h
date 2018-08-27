
#ifndef MS_DEBUG_H
#define MS_DEBUG_H


#if defined(DEBUG) && (DEBUG == 1) && !(DEBUG == 0)
	#include <cstdio>
	#define debug_printf(...) do { std::printf(__VA_ARGS__); fflush(stdout); } while(0)
#else
	#define debug_printf(...)
#endif

#if defined(DEBUG) && (DEBUG == 2) && !(DEBUG == 0)
	#define debug2_printf(...) do { std::printf(__VA_ARGS__); fflush(stdout); } while(0)
#else
	#define debug2_printf(...)
#endif




#endif //MS_DEBUG_H