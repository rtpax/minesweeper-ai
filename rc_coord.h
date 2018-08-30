#ifndef MS_RC_COORD_H
#define MS_RC_COORD_H

#include <limits>
#include <iostream>
#include <functional>

namespace ms {

/**stores the row and column for a cell an provides some basic comparison functionality**/
struct rc_coord {
    unsigned int row, col;

    /**default constructor. zero initializes elements.**/
    constexpr rc_coord() : row(0), col(0) { }
    /**equivalent to `rc_coord{r,c}`**/
    constexpr rc_coord(unsigned int r, unsigned int c) : row(r), col(c) { }
    /**copy constructor**/
    constexpr rc_coord(const rc_coord& copy) : row(copy.row), col(copy.col) {  }
    /**iff both members compare equal return true**/
    constexpr bool operator==(rc_coord comp) const { return row == comp.row && col == comp.col; }
    /**iff either member compares unequal return false**/
    constexpr bool operator!=(rc_coord comp) const { return row != comp.row || col != comp.col; }
    /**treat row as the high order bits for comparison. supplied only for use with `std::set`.**/
    constexpr bool operator<(rc_coord comp) const {
        if(row != comp.row)
            return row < comp.row;
        return col < comp.col;
    }
};

/**indicates a bad value for an rc_coord, such as when searching for an rc_coord but none is found**/
inline constexpr rc_coord BAD_RC_COORD{ std::numeric_limits<unsigned>::max(), std::numeric_limits<unsigned>::max() };
	
inline std::ostream& operator<<(std::ostream& os, const rc_coord& rc) {
    return os << "(" << rc.row << "," << rc.col << ")";
}

struct rc_coord_hash {
    size_t operator()(rc_coord rc) const {
        std::hash<unsigned> my_hash;
        if(sizeof(unsigned) >= 4)
            return my_hash((unsigned int) (rc.row << 16) | rc.col);
        else
            return my_hash((unsigned int) (rc.row << 8) | rc.col);
    }
};


}


#endif