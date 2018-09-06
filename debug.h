
#ifndef MS_DEBUG_H
#define MS_DEBUG_H

#include <iostream>

namespace dbg {

struct dummy_ostream : std::ostream {
	dummy_ostream() : std::ostream(0) {}
	template <typename T> dummy_ostream& operator<<(T arg) { return *this; }
};

#if defined(DEBUG) && (DEBUG == 1) && !(DEBUG == 0)
	inline std::ostream& cout = std::cout;	
#else
	inline std::ostream cout(0);
#endif

#if defined(DEBUG) && (DEBUG == 2) && !(DEBUG == 0)
	inline std::ostream& cout2 = std::cout;
#else
	inline std::ostream cout2(0);
#endif


}


#endif //MS_DEBUG_H