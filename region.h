#ifndef MS_REGION_H
#define MS_REGION_H

#include <vector>
#include <assert.h>
#include <stddef.h>

namespace ms {

	struct rc_coord {
		unsigned int row, col;

		rc_coord() { row = 0; col = 0; }
		rc_coord(unsigned int r, unsigned int c) { row = r; col = c; }
		rc_coord(const rc_coord& copy) { row = copy.row; col = copy.col; }
		bool operator==(rc_coord comp) const { return row == comp.row && col == comp.col; }
		bool operator!=(rc_coord comp) const { return row != comp.row || col != comp.col; }
	};

	struct region {
	private:
		std::vector<rc_coord> _cells;
		unsigned int _max, _min;
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

		/**Equivalent to region::union**/
		region operator|(const region& arg) const { return unite(arg); }
		/**Equivalent to region::subtract**/
		region operator/(const region& arg) const { return subtract(arg); }
		/**Equivalent to region::intersect**/
		region operator&(const region& arg) const { return intersect(arg); }
		/**Equivalent to region::merge**/
		region operator<<(const region& arg) const { return merge(arg); }

		/**`A |= B` is equivalent to `A = A | B`. See region::operator| **/
		const region& operator|=(const region& arg) { return *this = unite(arg); }
		/**`A /= B` is equivalent to `A = A / B`. See region::operator/ **/
		const region& operator/=(const region& arg) { return *this = subtract(arg); }
		/**`A &= B` is equivalent to `A = A & B`. See region::operator & **/
		const region& operator&=(const region& arg) { return *this = intersect(arg); }
		/**`A <<= B` is equivalent to `A = A << B`. See region::operator<< **/
		const region& operator<<=(const region& arg) { 
#ifdef DEBUG
			assert(!samearea(arg));
#endif
			return *this = merge(arg);
		}

		/**
		 * Returns true if the cells are the same area and have the same `min` and `max`, false otherwise. 
		 * Note that it does not check if both are trim. Follows the same rules as region::samearea for determining if all cells are the same.
		 * 
		 * Complexity \f$O(N^2)\f$.
		 **/
		bool operator==(const region& comp) const { return samearea(comp) && _min == comp._min && _max == comp._max; }
		/**Returns the opposite of `region::operator==`.**/
		bool operator!=(const region& comp) const { return !(*this == comp); }

		bool samearea(const region& comp) const;
		bool has_intersect(const region& arg) const;
		int addcell(rc_coord rc);
		/**Adds a cell to a region. Does not check for duplicates. The resulting region is not guaranteed to be trim.\n Complexity \f$O(1)\f$**/
		int forcecell(rc_coord rc) { _cells.push_back(rc); return 1; }
		int remove_bomb(rc_coord bomb);
		int remove_safe(rc_coord safe);
		int trim();
		bool is_trim() const;
		bool is_reasonable() const { return min() <= max() && max() <= size(); }
		/**Returns the number of cells in the region.\n Complexity \f$O(1)\f$.**/
		size_t size() const { return _cells.size(); }
		/**Returns true if the region has no cells, false otherwise.\n Complexity \f$O(1)\f$.**/
		bool empty() const { return _cells.empty(); }
		/**Accesses the element at the specified index.\n Complexity \f$O(1)\f$.**/
		rc_coord& operator[](int index) { return _cells[index]; }
		/**Accesses the element at the specified index. The reference returned by this is const, for the nonconst version see region::operator[].\n Complexity \f$O(1)\f$.**/
		const rc_coord& operator[](int index) const { return _cells[index]; }
		std::vector<rc_coord>::iterator begin() { return _cells.begin(); }
		std::vector<rc_coord>::iterator end() { return _cells.end(); }
		std::vector<rc_coord>::const_iterator begin() const { return _cells.begin(); }
		std::vector<rc_coord>::const_iterator end() const { return _cells.end(); }

#ifdef DEBUG
		#define assert_trim(arg) do{ region test = (arg); test.trim(); assert(test == (arg)); }while(0)
		#define assert_reasonable(arg) do{ assert((arg).min() <= (arg).max() && (arg).max() <= (arg).size()); }while(0)
		#define assert_nonempty(arg) do{ assert((arg).size() != 0); }while(0)
#else
		#define assert_trim(arg)
		#define assert_reasonable(arg)
		#define assert_nonempty(arg)
#endif
	};
}


#endif