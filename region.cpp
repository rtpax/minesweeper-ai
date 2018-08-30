#include "region.h"
#include <algorithm>
#include "debug.h"

namespace ms {


	/**
	 * Adds a cell to a region if it is not contained already
	 * 
	 * Return 1 if it adds the cell, 0 otherwise
	 * 
	 * Complexity \f$O(log(N))\f$
	 **/
	int region::add_cell(rc_coord arg) {
		return _cells.insert(arg).second;
	}

	/**
	 * Returns a region with all shared points (mathematical intersection: `this` &cap; `arg`).
	 * 
	 * Complexity \f$O(N \cdot log(M))\f$
	 **/
	region region::intersect(const region& arg) const {
		region ret;
		for (iterator it = begin(); it != end(); ++it) {
			if(arg.find(*it) != arg.end()) {
				ret.add_cell(*it);
			}
		}

		int subsize = size() - ret.size();
		int argsubsize = arg.size() - ret.size();

		ret._max = std::min<int>(std::min<int>(_max, arg._max), ret.size());
		ret._min = std::max<int>(
			_min - std::min<int>(subsize, _min), 
			arg._min - std::min<int>(argsubsize, arg._min)
		);

		return ret;
	}

	/**
	 * Returns a region with all points in either region (mathematical union: `this` &cup; `arg`).
	 * Calculates what the union's `max` and `min` number of bombs is. 
	 * 
	 * Complexity \f$O(M \cdot log(M + N))\f$ where M is the size of `arg` and N is the size of `this`.
	 **/
	region region::unite(const region& arg) const {
		region ret;
		ret._cells = _cells;
		int common = 0;
		for (iterator it = arg.begin(); it != arg.end(); ++it) {
			if (!ret.add_cell(*it))
				++common;
		}

		//ret.max = sum of bombs - min bombs in intersection (assuming max everywhere)
		ret._max = std::min<int>(arg._max + _max - std::max<int>(_max - (size() - common), 0),
			arg._max + _max - std::max<int>(arg._max - (arg.size() - common), 0));
		//ret.min = sum of bombs - max bombs in intersection (assuming min everywhere)
		ret._min = arg._min + _min - std::min<int>(std::min<int>(arg._min, _min), common);

		return ret;
	}

	/**
	 * Returns a region with all points in the calling region that
	 * are not in the argument region (mathematical compliment : `this`\\`arg`).
	 * 
	 * Complexity \f$O(M \cdot log(M + N))\f$ where M is the size of `arg` and N is the size of `this`.
	 **/
	region region::subtract(const region& arg) const {
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

		return ret;
	}

	/**
	 * removes all points in the calling region that
	 * are not in the argument region (mathematical compliment : `this`\\`arg`).
	 *
	 * Complexity \f$O(M \cdot log(M + N))\f$ where M is the size of `arg` and N is the size of `this`.
	 **/
	region& region::subtract_to(const region& arg) {
		size_t start_size = size();
		unsigned start_min = min();
		unsigned start_max = max();

		for (iterator it = arg.begin(); it != arg.end(); ++it) {
			remove_safe(*it);
		}
		int common = start_size - size();
		int othersubsize = arg.size() - common;

		int intersect_min_given_max_bombs =  std::max(
			start_max - std::min<int>(size(), start_max), 
			arg._min - std::min<int>(othersubsize, arg._min) //we only assume `this` has max bombs
		);

		int intersect_max_given_min_bombs = std::min<int>(std::min<int>(start_min, arg._max), common); //we only assume `this` has min bombs

		_max = std::min<int>(start_max - intersect_min_given_max_bombs, size()); 
		_min = start_min - intersect_max_given_min_bombs;

		assert(is_reasonable());

		return *this;
	}

    /**
	 * If two regions cover the same area, creates one
	 * region that gives all the information of the two
	 * (`max` = the smaller max, `min` = the larger min).
	 * Otherwise returns an empty region
	 * 
	 * Complexity \f$O(log(N))\f$ if they cannot merge, \f$O(N)\f$ if they can merge
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
	 * Merges a region with another region that covers the same area
	 * 
	 * Complexity \f$O(1)\f$
	 * 
	 * \warning The result of this function is undefined if `this->samearea(arg)` is false 
	 **/
	region& region::merge_to(const region& arg) {
		assert(samearea(arg));
		if(_max > arg._max)
			_max = arg.max();
		if(_min < arg._min)
			_min = arg.min();
		assert(is_reasonable());
		return *this;
	}

	/**
	 * removes a cell at the specified location treating it as a bomb:
	 * decreases min and max by 1, removes from the region, and returns 0
	 * if bomb is not contained in the region, do nothing and return 1
	 * if bomb is located in the region, but the region has no bombs (max of 0) return 2, but still remove
	 * 
	 * Complexity \f$O(log(N))\f$
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
	 * removes a cell at the specified location treating it as a non bomb
	 * removes from _cells, and returns 0
	 * if safe is not contained in the region, do nothing and return 1
	 * if safe is located in the region, but there are no safe spaces (min == size), remove the cell but return 2;
	 * 
	 * Complexity \f$O(log(N))\f$
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
	 * Returns true if the regions contain all of the same cells. 
	 * 
	 * Complexity \f$O(N)\f$.
	 **/
	bool region::samearea(const region& comp) const { 
		return _cells == comp._cells;
	}

	/**
	 * Returns true if the intersection of two regions is nonzero in size. Not much 
	 * faster than region::intersect, so if you actually intend to use the intersection
	 * it is probably better to take it directly.
	 * 
	 * Complexity \f$O(M \cdot log(N))\f$, where M is the size of `arg` and N is the size of `this`.
	 **/
	bool region::has_intersect(const region& arg) const {
		for (iterator it = arg.begin(); it != arg.end(); ++it) {
			if(find(*it) != end())
				return 1;
		}

		return 0;
	}

	/**
	 * Checks if the cell gives any useful information about the number of bombs.
	 * 
	 * Returns false if min/max can be inferred from size, true if they cannot
	 **/
	bool region::is_helpful() const {
		return !(size() == max() && min() == 0);//size == 0 evaluates to false for valid regions
	}


}