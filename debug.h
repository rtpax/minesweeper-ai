
#ifndef MS_DEBUG_H
#define MS_DEBUG_H

#include <iostream>

namespace dbg {

#if __cpp_inline_variables >= 201606
#define STORAGE_SPEC inline
#else
#define STORAGE_SPEC static
#endif

#if defined(DEBUG) && (DEBUG == 1) && !(DEBUG == 0)
	STORAGE_SPEC std::ostream& cout = std::cout;	
#else
	STORAGE_SPEC std::ostream cout(0);
#endif

#if defined(DEBUG) && (DEBUG == 2) && !(DEBUG == 0)
	STORAGE_SPEC std::ostream& cout2 = std::cout;
#else
	STORAGE_SPEC std::ostream cout2(0);
#endif

#undef STORAGE_SPEC

}


#endif //MS_DEBUG_H