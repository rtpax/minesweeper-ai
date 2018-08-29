#ifndef MS_REGION_H
#define MS_REGION_H

#include <vector>
#include <cassert>
#include <cstddef>
#include <set>
#include <iostream>
#include "rc_coord.h"
#include "debug.h"


namespace ms {

	class region_set;
	class region_cmp_no_min_max;

	struct region {
	private:
		friend region_set;
		friend region_cmp_no_min_max;

		std::set<rc_coord> _cells;
		mutable unsigned int _max, _min;
	public:
		/**The maximum number of bombs that could possibly be in the region.\n Complexity \f$O(1)\f$**/
		unsigned int max() const { return _max; }
		/**The minimum number of bombs that could possibly be in the region.\n Complexity \f$O(1)\f$**/
		unsigned int min() const { return _min; }
		/**Set the max and the min. Will fail if attempting to set an impossible value (min > max or max > size) and return 1. Otherwise returns 0.\n Complexity \f$O(1)\f$*/
		int set_range(unsigned int min, unsigned int max) { if(min > max || max > _cells.size()) return 1; _min = min; _max = max; return 0; }
		/**Set the max and min to the same value. Will fail if attempting to set an impossible value and return 1. Otherwise returns 0.\n Complexity \f$O(1)\f$**/
		int set_count(unsigned int minmax) { if(minmax > _cells.size()) return 1; _min = _max = minmax; return 0; }

		/**Default constructor. Contains 0 cells. `max = min = 0`.\n Complexity \f$O(1)\f$. **/
		region() { set_count(0); }
		/**Copy constructor.\n Complexity \f$O(N)\f$. **/
		region(const region& copy) { _cells = copy._cells; _max = copy._max; _min = copy._min; }

		region intersect(const region& arg) const;
		region unite(const region& arg) const;
		region subtract(const region& arg) const;
		region merge(const region& arg) const;
		region& merge_to(const region& arg);

		/**
		 * Returns true if the cells are the same area and have the same `min` and `max`, false otherwise. 
		 * Note that it does not check if both are trim. Follows the same rules as region::samearea for determining if all cells are the same.
		 * 
		 * Complexity \f$O(N^2)\f$.
		 **/
		bool operator==(const region& comp) const { return samearea(comp) && _min == comp._min && _max == comp._max; }
		/**Returns the opposite of `region::operator==`.**/
		bool operator!=(const region& comp) const { return !(*this == comp); }
		int compare_cells(const region& comp) const {
			return !(*this == comp);
		}

		bool samearea(const region& comp) const;
		bool has_intersect(const region& arg) const;
		int add_cell(rc_coord rc);
		int remove_bomb(rc_coord bomb);
		int remove_safe(rc_coord safe);
		/** returns true if the combination of max and min is possible with the current size**/
		bool is_reasonable() const { return min() <= max() && max() <= size(); }
		/**Returns the number of cells in the region.\n Complexity \f$O(1)\f$.**/
		bool is_helpful() const;
		size_t size() const { return _cells.size(); }
		/**Returns true if the region has no cells, false otherwise.\n Complexity \f$O(1)\f$.**/
		bool empty() const { return _cells.empty(); }
		bool contains(rc_coord cell) const { return _cells.find(cell) != _cells.end(); }

		typedef std::set<rc_coord>::iterator iterator;
		typedef std::set<rc_coord>::const_iterator const_iterator;
		iterator begin() { return _cells.begin(); }
		iterator end() { return _cells.end(); }
		const_iterator begin() const { return _cells.cbegin(); }
		const_iterator end() const { return _cells.cend(); }
		iterator find(const rc_coord& rc) { return _cells.find(rc); }
		const_iterator find(const rc_coord& rc) const { return _cells.find(rc); }
	};

	inline std::ostream& operator<<(std::ostream& os, const region& r) {
		os << "{ [" << r.size() << "|" << r.min() << "-" << r.max()  << "] ";
		if(!r.empty()) {
			auto it = r.begin();
			for(; std::next(it) != r.end(); ++it) {
				os << *it << ",";
			}
			os << *it;
		}
		return os << " }";
	}

	/**Print information about rc_coord iff `debug_print`ing is enabled**/
	inline void debug_print_rc_coord(const rc_coord& arg) {
		debug_printf("(%u,%u)",arg.row,arg.col);
	}

	/**Print information about region iff `debug_print`ing is enabled**/
	inline void debug_print_region(const region& arg) {
		debug_printf("{[%zu:%u,%u]",arg.size(),arg.min(),arg.max());
		for(rc_coord rc : arg) {
			debug_print_rc_coord(rc);
		}
		debug_printf("}");
	}
}


#endif