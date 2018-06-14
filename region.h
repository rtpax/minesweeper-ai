#ifndef MS_REGION_H
#define MS_REGION_H

#include <vector>
#include <assert.h>
#include <stddef.h>

namespace ms {

	struct rc_coord {
		rc_coord() { row = 0; col = 0; }
		rc_coord(unsigned int r, unsigned int c) { row = r; col = c; }
		rc_coord(const rc_coord& copy) { row = copy.row; col = copy.col; }
		unsigned int row, col;
		bool operator==(rc_coord comp) const { return row == comp.row && col == comp.col; }
	};

	struct region {
	private:
		std::vector<rc_coord> _cells;
		unsigned int _max, _min;
	public:
		unsigned int max() const { return _max; }
		unsigned int min() const { return _min; }
		int set_range(unsigned int min, unsigned int max) { if(min > max || max > _cells.size()) return 1; _min = min; _max = max; return 0; }
		int set_count(unsigned int minmax) { if(minmax > _cells.size()) return 1; _min = _max = minmax; return 0; }

		region() {}
		region(const region& copy) { _cells = copy._cells; _max = copy._max; _min = copy._min; }

		region intersect(const region& arg) const;
		region unite(const region& arg) const;
		region subtract(const region& arg) const;
		region merge(const region& arg) const;

		region operator|(const region& arg) const { return unite(arg); }
		region operator/(const region& arg) const { return subtract(arg); }
		region operator&(const region& arg) const { return intersect(arg); }
		region operator<<(const region& arg) const { return merge(arg); }

		const region& operator|=(const region& arg) { return *this = unite(arg); }
		const region& operator/=(const region& arg) { return *this = subtract(arg); }
		const region& operator&=(const region& arg) { return *this = intersect(arg); }
		const region& operator<<=(const region& arg) { 
#ifdef DEBUG
			assert(!samearea(arg));
#endif
			return *this = merge(arg);
		}

		bool operator==(const region& comp) const { return samearea(comp) && _min == comp._min && _max == comp._max; }
		bool operator!=(const region& comp) const { return !samearea(comp) || _min != comp._min || _max != comp._max; }
		bool samearea(const region& comp) const { return _cells == comp._cells; }
		bool has_intersect(const region& chuck) const;
		int addcell(rc_coord rc);
		int remove_bomb(rc_coord bomb);
		int remove_safe(rc_coord safe);
		int trim();
		size_t size() const { return _cells.size(); }
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