
#ifndef MS_DEBUG_H
#define MS_DEBUG_H

#include <iostream>

namespace dbg {

struct dummy_ostream : std::ostream {
	template <typename T> dummy_ostream& operator<<(T arg) { return *this; }
};

#if defined(DEBUG) && (DEBUG == 1) && !(DEBUG == 0)
	inline std::ostream& cout = std::cout;	
#else
	inline dummy_ostream cout;
#endif

#if defined(DEBUG) && (DEBUG == 2) && !(DEBUG == 0)
	inline std::ostream& cout2 = std::cout;
#else
	inline dummy_ostream cout2;
#endif


}


#endif //MS_DEBUG_H