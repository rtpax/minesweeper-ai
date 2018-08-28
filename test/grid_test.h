#ifndef MS_TEST_GRID_TEST_H
#define MS_TEST_GRID_TEST_H

#include <catch.hpp>

#include "../grid.h"
#include <functional>
#include <sstream>

TEST_CASE("grid: bounds checking", "grid::is_contained") {
    using namespace ms;

    grid testgrid(10,10,10);

    CHECK(testgrid.iscontained(5,2));
    CHECK_FALSE(testgrid.iscontained(100,4));
    CHECK_FALSE(testgrid.iscontained(7,100));
    CHECK(testgrid.iscontained(9,9));
    CHECK_FALSE(testgrid.iscontained(10,1));
    CHECK_FALSE(testgrid.iscontained(1,10));
}

TEST_CASE("grid: cell access", "grid::open, grid::get") {
    using namespace ms;

    const grid::cell _F = grid::cell::ms_bomb;
    const grid::cell _0 = grid::cell::ms_0;
    const grid::cell _1 = grid::cell::ms_1;
    const grid::cell _2 = grid::cell::ms_2;
    const grid::cell _3 = grid::cell::ms_3;
    const grid::cell _4 = grid::cell::ms_4;
    const grid::cell _5 = grid::cell::ms_5;
    const grid::cell _6 = grid::cell::ms_6;
    const grid::cell _7 = grid::cell::ms_7;
    const grid::cell _8 = grid::cell::ms_8;
    const grid::cell _H = grid::cell::ms_hidden;
    const grid::cell _R = grid::cell::ms_error;
    
    grid::cell *init[10];
    
    init[0] = new grid::cell[10]{ _0,_F,_0,_0,_0,_0,_0,_0,_0,_0 };
    init[1] = new grid::cell[10]{ _0,_0,_0,_F,_0,_0,_0,_0,_F,_0 };
    init[2] = new grid::cell[10]{ _0,_0,_0,_0,_0,_0,_0,_0,_0,_0 };
    init[3] = new grid::cell[10]{ _0,_0,_0,_0,_0,_0,_F,_0,_0,_0 };
    init[4] = new grid::cell[10]{ _0,_0,_0,_0,_0,_0,_0,_0,_0,_0 };
    init[5] = new grid::cell[10]{ _0,_0,_F,_0,_0,_0,_0,_0,_0,_0 };
    init[6] = new grid::cell[10]{ _0,_0,_0,_0,_0,_0,_0,_0,_0,_0 };
    init[7] = new grid::cell[10]{ _0,_F,_0,_0,_0,_0,_F,_0,_0,_0 };
    init[8] = new grid::cell[10]{ _0,_0,_0,_F,_0,_0,_0,_0,_0,_0 };
    init[9] = new grid::cell[10]{ _0,_0,_0,_0,_0,_0,_0,_F,_0,_0 };

    grid::cell *check[11];
    
    check[0] = new grid::cell[11]{ _H,_H,_H,_H,_H,_H,_H,_H,_H,_H,_R };
    check[1] = new grid::cell[11]{ _H,_H,_H,_H,_H,_H,_H,_H,_H,_H,_R };
    check[2] = new grid::cell[11]{ _H,_H,_H,_H,_H,_H,_H,_H,_H,_H,_R };
    check[3] = new grid::cell[11]{ _H,_H,_H,_H,_H,_H,_H,_H,_H,_H,_R };
    check[4] = new grid::cell[11]{ _H,_H,_H,_H,_H,_H,_H,_H,_H,_H,_R };
    check[5] = new grid::cell[11]{ _H,_H,_H,_H,_H,_H,_H,_H,_H,_H,_R };
    check[6] = new grid::cell[11]{ _H,_H,_H,_H,_H,_H,_H,_H,_H,_H,_R };
    check[7] = new grid::cell[11]{ _H,_H,_H,_H,_H,_H,_H,_H,_H,_H,_R };
    check[8] = new grid::cell[11]{ _1,_1,_2,_H,_H,_H,_2,_H,_H,_H,_R };
    check[9] = new grid::cell[11]{ _0,_0,_1,_H,_H,_H,_H,_H,_1,_H,_R };
    check[10]= new grid::cell[11]{ _R,_R,_R,_R,_R,_R,_R,_R,_R,_R,_R };
    
    grid testgrid(10,10,init);

    CHECK(testgrid.get(0,1) == grid::cell::ms_hidden);
    CHECK(testgrid.get(5,5) == grid::cell::ms_hidden);
    CHECK(testgrid.get(40,30) == grid::cell::ms_error);
    CHECK(testgrid.open(9,8) == 1);
    CHECK(testgrid.get(9,8) == grid::cell::ms_1);
    CHECK(testgrid.open(8,6) == 1);
    CHECK(testgrid.get(8,6) == grid::cell::ms_2);
    CHECK(testgrid.open(9,0) == 6);

    for(int r = 0; r < 11; ++r) {
        for(int c = 0; c < 11; ++c) {
            INFO("coordinates: [" << r << "][" << c << "]");
            CHECK(testgrid.get(r,c) == check[r][c]);
        }
    }

    CHECK(testgrid.open(7,1) == 1);
    CHECK(testgrid.get(7,1) == grid::cell::ms_bomb);
}

TEST_CASE("grid: flag", "grid::flag, grid::set_flag, grid::get") {
    using namespace ms;

    grid::cell *init[10];
    const grid::cell _F = grid::cell::ms_bomb;
    const grid::cell _0 = grid::cell::ms_0;
    
    init[0] = new grid::cell[10]{ _0,_F,_0,_0,_0,_0,_0,_0,_0,_0 };
    init[1] = new grid::cell[10]{ _0,_0,_0,_F,_0,_0,_0,_0,_F,_0 };
    init[2] = new grid::cell[10]{ _0,_0,_0,_0,_0,_0,_0,_0,_0,_0 };
    init[3] = new grid::cell[10]{ _0,_0,_0,_0,_0,_0,_F,_0,_0,_0 };
    init[4] = new grid::cell[10]{ _0,_0,_0,_0,_0,_0,_0,_0,_0,_0 };
    init[5] = new grid::cell[10]{ _0,_0,_F,_0,_0,_0,_0,_0,_0,_0 };
    init[6] = new grid::cell[10]{ _0,_0,_0,_0,_0,_0,_0,_0,_0,_0 };
    init[7] = new grid::cell[10]{ _0,_F,_0,_0,_0,_0,_F,_0,_0,_0 };
    init[8] = new grid::cell[10]{ _0,_0,_0,_F,_0,_0,_0,_0,_0,_0 };
    init[9] = new grid::cell[10]{ _0,_0,_0,_0,_0,_0,_0,_F,_0,_0 };
    
    grid testgrid(10,10,init);

    CHECK(testgrid.flag(9,8) == 0);
    CHECK(testgrid.get(9,8) == grid::cell::ms_flag);    
    CHECK(testgrid.flag(8,6) == 0);
    CHECK(testgrid.get(8,6) == grid::cell::ms_flag);
    CHECK_FALSE(testgrid.flag(10,21) == 0);
    CHECK(testgrid.get(10,21) == grid::cell::ms_error);
    CHECK(testgrid.flag(8,6) == 0);
    CHECK(testgrid.get(8,6) == grid::cell::ms_question);
    CHECK(testgrid.flag(8,6) == 0);
    CHECK(testgrid.get(8,6) == grid::cell::ms_hidden);
    CHECK(testgrid.set_flag(9,8,grid::cell::ms_hidden) == 0);
    CHECK(testgrid.get(9,8) == grid::cell::ms_hidden);
    CHECK(testgrid.set_flag(5,5,grid::cell::ms_hidden) == 0);
    CHECK(testgrid.get(5,5) == grid::cell::ms_hidden);
    CHECK(testgrid.set_flag(6,6,grid::cell::ms_question) == 0);
    CHECK(testgrid.get(6,6) == grid::cell::ms_question);
}

    // TODO grid::reset();


#endif