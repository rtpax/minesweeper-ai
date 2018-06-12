#ifndef MS_SPINGRID_H
#define MS_SPINGRID_H

#include "grid.h"

namespace ms {
	class spingrid : public grid {
	public:
		spingrid();
		spingrid(grid * parent);
		int set(unsigned int row, unsigned int column);
	private:
		
	};
}

#endif