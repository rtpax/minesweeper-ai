#include "region.h"
#include <algorithm>
#include "debug.h"

namespace ms {


	/**
	 *
	 * Adds a cell to a region if it is not contained already
	 * The resulting region is guaranteed to be trim if it was before addcell was called.
	 * 
	 * Return 1 if it adds the cell, 0 otherwise
	 * 
	 * Complexity \f$O(log(N))\f$
	 *
	 **/
	int region::addcell(rc_coord arg) {
		iterator it = _cells.lower_bound(arg); //user lower_bound instead of find, use `it` to speed up insertion
		if(*it == arg) { 
			return 0;
		} else {
			_cells.insert(it, arg); 
			return 1;
		}
	}

	/**
	 *
	 * Removes all repeated cells in a region.
	 * 
	 * A cell is guaranteed to be trim after calling trim.
	 * A cell is trim if it has no repeated cells in a region.
	 * 
	 * Returns the number of removed cells.
	 *
	 * Complexity \f$\Theta (N^2)\f$
	 * 
	 * Note: this function no longer serves any purpose.
	 * All regions should constantly be trim. This function returns zero;
	 * 
	 **/
	int region::trim() {
		assert(is_trim());
		return 0;
	}

	/**
	 * 
	 * Tests whether the region is trim.
	 * A cell is trim if it has no repeated cells in a region.
	 * 
	 * Returns true if the region is trim, false otherwise.
	 * 
	 * Complexity \f$O(N^2)\f$
	 * 
	 **/
	bool region::is_trim() const {
		if(!_cells.empty())
		for (iterator i = begin(); std::next(i,1) != end(); ++i) {
			for (iterator j = std::next(i, 1); j != end(); ++j) {
				if (*i == *j) {
					return false;
				}
			}
		}
		return true;
	}

	/**
	 * 
	 * Returns a region with all shared points (mathematical intersection: `this` &cap; `arg`).
	 * Guaranteed to return a trim region.
	 * 
	 * `arg` and `this` MUST be trim.
	 * 
	 * Complexity \f$O(N \cdot log(M))\f$
	 *
	 **/
	region region::intersect(const region& arg) const {
		assert(is_trim() && "intersect");
		assert(arg.is_trim() && "intersect");

		region ret;
		for (iterator it = begin(); it != end(); ++it) {
			if(arg.find(*it) != arg.end()) {
				ret.addcell(*it);
			}
		}

		int subsize = size() - ret.size();
		int argsubsize = arg.size() - ret.size();

		ret._max = std::min<int>(std::min<int>(_max, arg._max), ret.size());
		ret._min = std::max<int>(
			_min - std::min<int>(subsize, _min), 
			arg._min - std::min<int>(argsubsize, arg._min)
		);

		if(!ret.is_reasonable()) {
			debug_printf("{");
			for(unsigned int i = 0; i < size(); ++i)
				debug_printf("(%u,%u)",(*this)[i].row,(*this)[i].col);
			debug_printf("}\\{");
			for(unsigned int i = 0; i < arg.size(); ++i)
				debug_printf("(%u,%u)",(arg)[i].row,(arg)[i].col);
			debug_printf("} -> {");
			for(unsigned int i = 0; i < ret.size(); ++i)
				debug_printf("(%u,%u)",(ret)[i].row,(ret)[i].col);
			debug_printf("}\n");
			debug_printf("{%zu : %u - %u}\\{%zu : %u - %u} -> {%zu : %u - %u}\n", size(), min(), max(), arg.size(), arg.min(), arg.max(), ret.size(), ret.min(), ret.max());
		}
		assert(ret.is_reasonable() && "intersect");
		assert(ret.is_trim() && "intersect");

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
	/**
	 *
	 * Returns a region with all points in either region (mathematical union: `this` &cup; `arg`).
	 * Calculates what the union's `max` and `min` number of bombs is. 
	 * Guaranteed to return a trim region.
	 * 
	 * `this` and `arg` MUST be trim
	 * 
	 * Complexity \f$O(M \cdot log(M + N))\f$ where M is the size of `arg` and N is the size of `this`.
	 * 
	 **/
	region region::unite(const region& arg) const {
		assert(is_trim() && "unite");
		assert(arg.is_trim() && "unite");

		region ret;
		ret._cells = _cells;
		int common = 0;
		for (iterator it = arg.begin(); it != arg.end(); ++it) {
			if (!ret.addcell(*it))
				++common;
		}

		ret._max = std::min<int>(arg._max + _max - std::max<int>(_max - (size() - common), 0),
			arg._max + _max - std::max<int>(arg._max - (arg.size() - common), 0));
		ret._min = arg._min + _min - std::min<int>(std::min<int>(arg._min, _min), common);

		if(!ret.is_reasonable()) {
			debug_printf("{");
			for(unsigned int i = 0; i < size(); ++i)
				debug_printf("(%u,%u)",(*this)[i].row,(*this)[i].col);
			debug_printf("}\\{");
			for(unsigned int i = 0; i < arg.size(); ++i)
				debug_printf("(%u,%u)",(arg)[i].row,(arg)[i].col);
			debug_printf("} -> {");
			for(unsigned int i = 0; i < ret.size(); ++i)
				debug_printf("(%u,%u)",(ret)[i].row,(ret)[i].col);
			debug_printf("}\n");
			debug_printf("{%zu : %u - %u}\\{%zu : %u - %u} -> {%zu : %u - %u}\n", size(), min(), max(), arg.size(), arg.min(), arg.max(), ret.size(), ret.min(), ret.max());
		}
		assert(ret.is_reasonable() && "unite");
		assert(ret.is_trim() && "unite");

		return ret;
	}

	/**
	 *
	 * Returns a region with all points in the calling region that
	 * are not in the argument region (mathematical compliment : `this`\\`arg`).
	 * Guaranteed to return a trim region
	 *
	 * `this` and `arg` MUST be trim
	 * 
	 * Complexity \f$O(M \cdot log(M + N))\f$ where M is the size of `arg` and N is the size of `this`.
	 * 
	 **/
	region region::subtract(const region& arg) const {
		assert(is_trim() && "subtract");
		assert(arg.is_trim() && "subtract");

		region ret;
		ret._cells = _cells;

		for (iterator it = arg.begin(); it != arg.end(); ++it) {
			ret.remove_safe(*it);
		}
		int common = size() - ret.size();
		int othersubsize = arg.size() - common;

		int intersect_min_given_max_bombs =  std::max(
			_max - std::min<int>(ret.size(), _max), 
			arg._min - std::min<int>(othersubsize, arg._min) //we only assume `this` has max bombs
		);

		int intersect_max_given_min_bombs = std::min<int>(std::min<int>(_min, arg._max), common); //we only assume `this` has min bombs

		ret._max = std::min<int>(_max - intersect_min_given_max_bombs, ret.size()); 
		ret._min = _min - intersect_max_given_min_bombs;

		if(!ret.is_reasonable()) {
			debug_printf("{");
			for(unsigned int i = 0; i < size(); ++i)
				debug_printf("(%u,%u)",(*this)[i].row,(*this)[i].col);
			debug_printf("}\\{");
			for(unsigned int i = 0; i < arg.size(); ++i)
				debug_printf("(%u,%u)",(arg)[i].row,(arg)[i].col);
			debug_printf("} -> {");
			for(unsigned int i = 0; i < ret.size(); ++i)
				debug_printf("(%u,%u)",(ret)[i].row,(ret)[i].col);
			debug_printf("}\n");
			debug_printf("{%zu : %u - %u}\\{%zu : %u - %u} -> {%zu : %u - %u}\n", size(), min(), max(), arg.size(), arg.min(), arg.max(), ret.size(), ret.min(), ret.max());
		}
		assert(ret.is_reasonable() && "subtract");
		assert(ret.is_trim() && "subtract");

		return ret;
	}

    /**
	 *
	 * If two regions cover the same area, creates one
	 * region that gives all the information of the two
	 * (`max` = the smaller max, `min` = the larger min).
	 * Otherwise returns an empty region
	 * 
	 * Complexity \f$O(log(N))\f$ if they cannot merge, \f$O(N)\f$ if they can merge
	 *
	 **/
	region region::merge(const region& arg) const {
		region ret;
		if (samearea(arg)) {
			ret._cells = _cells;
			ret._max = std::min(_max, arg._max);
			ret._min = std::max(_min, arg._min);
		}
		assert(ret.is_reasonable());
		return ret;
	}

	/**
	 *
	 * removes a cell at the specified location treating it as a bomb:
	 * decreases min and max by 1, removes from the region, and returns 0
	 * if bomb is not contained in the region, do nothing and return 1
	 * if bomb is located in the region, but the region has no bombs (max of 0) return 2, but still remove
	 * 
	 * Complexity \f$O(log(N))\f$
	 * 
	 **/
	int region::remove_bomb(rc_coord bomb) {
		iterator i = _cells.find(bomb);
		if(i == _cells.end()) {
			return 1;
		} else {
			assert(*i == bomb);
			_cells.erase(i);

				if(_min != 0)
					--_min;
				if(_max != 0)
					--_max;
				else
					return 2;

				return 0;
		}
	}

	/**
	 *
	 * removes a cell at the specified location treating it as a non bomb
	 * removes from _cells, and returns 0
	 * if safe is not contained in the region, do nothing and return 1
	 * if safe is located in the region, but there are no safe spaces (min == size), remove the cell but return 2;
	 * 
	 * Complexity \f$O(log(N))\f$
	 *
	 **/
	int region::remove_safe(rc_coord safe) {
		iterator i = _cells.find(safe);
		if(i == _cells.end()) {
			return 1;
		} else {
			assert(*i == safe);
			_cells.erase(i);

			if(_max > size())
				_max = size();
			if(_min > _max) { //implies _min > size()
				_min = _max;
				return 2;
			}
			return 0;
		}
	}

	/**
	 * 
	 * Returns true if the regions contain all of the same cells. 
	 * 
	 * Complexity \f$O(N)\f$.
	 * 
	 **/
	bool region::samearea(const region& comp) const { 
		return _cells == comp._cells;
	}

	/**
	 * 
	 * Returns true if the intersection of two regions is nonzero in size. Not much 
	 * faster than region::intersect, so if you actually intend to use the intersection
	 * it is probably better to take it directly.
	 * 
	 * Complexity \f$O(M \cdot log(N))\f$, where M is the size of `arg` and N is the size of `this`.
	 * 
	 **/
	bool region::has_intersect(const region& arg) const {
		for (iterator it = arg.begin(); it != arg.end(); ++it) {
			if(find(*it) != end())
				return 1;
		}

		return 0;
	}


}