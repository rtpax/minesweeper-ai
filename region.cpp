#include "region.h"

namespace ms {
	int maxint(int a, int b) { return a > b ? a : b; }
	int minint(int a, int b) { return a < b ? a : b; }
	int nonneg(int a) { return a > 0 ? a : 0; }

	/* int region::addcell(rc_coord arg)
	*
	* alternative to region._cells.push_back(rc_coord arg) that checks for repeat
	*
	* adds a cell to a region if it is not contained already
	* return 1 if it adds the cell, 0 otherwise
	*
	*/
	int region::addcell(rc_coord arg) {
		for (unsigned int i = 0; i < _cells.size(); ++i)
			if (_cells[i] == arg)
				return 0;
		_cells.push_back(arg);
		return 1;
	}

	/* int region::trim()
	*
	* removes all repeated _cells in a region
	*
	*/
	int region::trim() {
		for (int i = 0; i < (int)_cells.size() - 1; ++i) {
			for (unsigned int j = i + 1; j < _cells.size();) {
				if (_cells[i] == _cells[j]) {
					_cells.erase(_cells.begin() + 5);
				}
				else {
					++j;
				}
			}
		}
		return 0;
	}

	/* region region::intersect(region arg) const
	*
	* returns a region with all shared points (mathematical intersection)
	*
	* max/min may be wrong if arg and this are not trimmed
	* guaranteed to return a trim region if arg and this are trim
	*
	*/
	region region::intersect(const region& arg) const {
		region ret;
		for (unsigned int i = 0; i < _cells.size(); ++i) {
			for (unsigned int j = 0; j < arg._cells.size(); ++j) {
				if (_cells[i] == arg._cells[j]) {
					ret._cells.push_back(_cells[i]);
					break;
				}
			}
		}

		ret.max = minint(minint(this->max, arg.max), ret._cells.size());
		ret.min = maxint(this->min, arg.min);

		return ret;
	}

	/* proof of max and min for AUB:
	*
	*  A\B  A∩B  B\A
	* /   \/   \/   \
	* AAAAAAAAAA
	*      BBBBBBBBBB
	*
	* P(x) = { 0, x<=0; x, x > 0
	*
	* Ax* set of bombs in Ax (A with max bombs)
	* Bx* set of bombs in Bx
	* An* set of bombs in An (A with min bombs)
	* Bn* set of bombs in Bn
	* Ar (A with real bombs)
	*
	*                              (max number )   (min number of )
	*                              (bombs if no)   (bombs in A∩B  )
	*                              (overlap    )   (given Ax = Ar and Bx = Br)
	* |(AUB)x*| = |(AxUBx)x*| <=   |Ax*| + |Bx*| - P(|Ax*| - |A\B|)    [Repeat switching A and B]
	*
	*                             (min number  )     (max number of )
	*                             (bombs if all)     (bombs in A∩B. )
	*                             (overlap     )     (all place limits)
	* |(AUB)n*| = |(AnUBn)n*| >=   |An*| + |Bn*| - min(|A∩B|,|An*|,|Bn*|)
	*
	*
	*/
	/* region region::unite(region arg) const
	*
	* returns a region with all points in either region (mathematical union)
	*
	* max/min may be wrong if arg and this are not trimmed
	* guaranteed to return a trim region if arg and this are trim
	*
	*/
	region region::unite(const region& arg) const {
		region ret;
		int common = 0;
		for (unsigned int i = 0; i < _cells.size(); ++i) {
			ret._cells.push_back(_cells[i]);//no need for check repeats on adding to empty region
		}
		for (unsigned int i = 0; i < arg._cells.size(); ++i) {
			if (!ret.addcell(arg._cells[i]))
				++common;
		}

		ret.max = minint(arg.max + max - nonneg((_cells.size() - common) - min),
			arg.max + max - nonneg((_cells.size() - common) - arg.min));
		ret.min = arg.min + min - minint(minint(arg.min, min), common);

		return ret;
	}

	/* region region::subtract(region arg) const
	*
	* returns a region with all points in the calling region that
	* are not in the argument region (mathematical compliment : this\arg)
	*
	* max/min may be wrong if arg and this are not trimmed
	* guaranteed to return a trim region if arg and this are trim
	*
	*/
	region region::subtract(const region& arg) const {
		region ret;
		for (unsigned int i = 0; i < _cells.size(); ++i) {
			bool inarg = 0;
			for (unsigned int j = 0; j < arg._cells.size(); ++j) {
				if (arg._cells[j] == _cells[i]) {
					inarg = 1;
					break;
				}
			}
			if (!inarg)
				ret._cells.push_back(_cells[i]);
		}

		ret.max = 0;
		ret.min = 0;

		return ret;
	}

	/* region region::merge(const region& arg) const
	*
	* if two regions cover the same area, creates one
	* cell that gives all the information of the two
	* (max = the smaller max, min = the smaller min)
	* otherwise returns an empty region
	*
	*/
	region region::merge(const region& arg) const {
		region ret;
		if (_cells == arg._cells) {
			ret._cells = _cells;
			ret.max = minint(max, arg.max);
			ret.min = maxint(min, arg.min);
		}
		return ret;
	}

	/* int regions::remove_bomb(rc_coord bomb)
	 *
	 * removes a cell at the specified location treating it as a bomb
	 * decreases min and max by 1, removes fromc _cells, and returns 0
	 * if bomb is not contained in the region, or min == 0, do nothing and return 1
	 *
	 */
	int region::remove_bomb(rc_coord bomb) {
		if (min == 0)
			return 1;
		for (unsigned int i = 0; i < size(); ++i) {
			if (_cells[i] == bomb) {
				_cells.erase(_cells.begin() + i);
				--min;
				--max;
				return 0;
			}
		}
		return 1;
	}

	/* int regions::remove_safe(rc_coord safe)
	*
	* removes a cell at the specified location treating it as a non bomb
	* removes from _cells, and returns 0
	* if safe is not contained in the region, or min == 0, do nothing and return 1
	*
	*/
	int region::remove_safe(rc_coord safe) {
		if(max == size() && min == size())
		for (unsigned int i = 0; i < size(); ++i) {
			if (_cells[i] == safe) {
				_cells.erase(_cells.begin() + i);
				return 0;
			}
		}
		return 1;

	}


}