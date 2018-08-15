#ifndef MS_RC_COORD_H
#define MS_RC_COORD_H

namespace ms {

/**stores the row and column for a cell an provides some basic comparison functionality**/
struct rc_coord {
    unsigned int row, col;

    /**default constructor. zero initializes elements.**/
    rc_coord() { row = 0; col = 0; }
    /**equivalent to `rc_coord{r,c}`**/
    rc_coord(unsigned int r, unsigned int c) { row = r; col = c; }
    /**copy constructor**/
    rc_coord(const rc_coord& copy) { row = copy.row; col = copy.col; }
    /**iff both members compare equal return true**/
    bool operator==(rc_coord comp) const { return row == comp.row && col == comp.col; }
    /**iff either member compares unequal return false**/
    bool operator!=(rc_coord comp) const { return row != comp.row || col != comp.col; }
    /**treat row as the high order bits for comparison. supplied only for use with `std::set`.**/
    bool operator<(rc_coord comp) const {
        if(row != comp.row)
            return row < comp.row;
        return col < comp.col;
    }
};


}


#endif