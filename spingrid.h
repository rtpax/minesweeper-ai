#ifndef MS_SPINGRID_H
#define MS_SPINGRID_H

#include "grid.h"

namespace ms {
	class spingrid {
	private:
		grid _base;
	public:
		spingrid(const grid& parent) : _base(parent, grid::SURFACE_COPY) {}

		int set(unsigned int row, unsigned int col, grid::cell value) { _base._visgrid[row][col] = value; return 0; }
		const grid& base() const { return _base; }
	};
}

#endif