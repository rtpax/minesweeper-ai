
#ifndef MS_DEBUG_H
#define MS_DEBUG_H


#if DEBUG>1
	#include <cstdio>
	#define debug_printf(...) std::printf(__VA_ARGS__)
#else
	#define debug_printf(...)
#endif

#if DEBUG>2
	#define debug2_printf(...) std::print(__VA_ARGS__)
#else
	#define debug2_printf(...)
#endif




#endif //MS_DEBUG_H