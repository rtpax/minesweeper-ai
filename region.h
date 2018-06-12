#ifndef MS_REGION_H
#define MS_REGION_H

#include <vector>
#include <assert.h>

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
	public:
		unsigned int max, min;

		region() {}
		region(const region& copy) { _cells = copy._cells; max = copy.max; min = copy.min; }

		region intersect(const region& arg) const;
		region unite(const region& arg) const;
		region subtract(const region& arg) const;
		region merge(const region& arg) const;
		bool operator==(const region& comp) const;
		bool samearea(const region& comp) const { return _cells == comp._cells; }
		bool has_intersect(const region& chuck) const;
		int addcell(rc_coord rc);
		int remove_bomb(rc_coord bomb);
		int remove_safe(rc_coord safe);
		int trim();
		int size() const { return _cells.size(); }
		rc_coord& operator[](int index) { return _cells[index]; }
		const rc_coord& operator[](int index) const { return _cells[index]; }
#ifdef DEBUG
		int assert_trim() const { region test = *this; test.trim(); assert(test == *this); return 0; }
		int assert_reasonable() const { assert(min <= max && max <= size()); return 0; }
#else
		int assert_trim() const { return 0; }
		int assert_reasonable() const { return 0; }
#endif
	};
}


#endif