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
		/****/
		unsigned int max() const { return _max; }
		/****/
		unsigned int min() const { return _min; }
		/****/
		int set_range(unsigned int min, unsigned int max) { if(min > max || max > _cells.size()) return 1; _min = min; _max = max; return 0; }
		/****/
		int set_count(unsigned int minmax) { if(minmax > _cells.size()) return 1; _min = _max = minmax; return 0; }

		/**Default constructor. Contains 0 cells. `max = min = 0`. **/
		region() {}
		/**Copy constructor.**/
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

		/**Returns true if the cells are the same area and have the same `min` and `max`, false otherwise. Note that it does not check if both are trim. Follows the same rules as region::samearea for determining if all cells are the same.**/
		bool operator==(const region& comp) const { return samearea(comp) && _min == comp._min && _max == comp._max; }
		/**Returns the opposite of `region::operator==`.**/
		bool operator!=(const region& comp) const { return !(*this == comp); }

		bool samearea(const region& comp) const;
		bool has_intersect(const region& chuck) const;
		int addcell(rc_coord rc);
		int forcecell(rc_coord rc);
		int remove_bomb(rc_coord bomb);
		int remove_safe(rc_coord safe);
		int trim();
		bool is_trim() const;
		size_t size() const { return _cells.size(); }
		bool empty() const { return _cells.empty(); }
		rc_coord& operator[](int index) { return _cells[index]; }
		const rc_coord& operator[](int index) const { return _cells[index]; }
#ifdef DEBUG
		int assert_trim() const { region test = *this; test.trim(); assert(test == *this); return 0; }
		int assert_reasonable() const { assert(_min <= _max && _max <= size()); return 0; }
		int assert_nonempty() const { assert(size() != 0); return 0; }
#else
		int assert_trim() const { return 0; }
		int assert_reasonable() const { return 0; }
		int assert_nonempty() const { return 0; }
#endif
	};
}


#endif