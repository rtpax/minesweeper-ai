#ifndef MS_TEST_REGION_TEST_H
#define MS_TEST_REGION_TEST_H

#include <catch.hpp>
#include "../region.h"


TEST_CASE("region: comparision", "region::samearea, region::operator==, region::operator!=") {
    using namespace ms;

    region r1, r2;
    
    r1.add_cell(rc_coord{1,2});
    r1.add_cell(rc_coord{3,4});
    r1.add_cell(rc_coord{5,6});
    
    r2.add_cell(rc_coord{5,6});
    r2.add_cell(rc_coord{3,4});
    r2.add_cell(rc_coord{1,2});

    CHECK(r1.samearea(r2));

    r1.set_range(2,3);
    r2.set_range(2,3);

    CHECK(r1 == r2);
    CHECK_FALSE(r1 != r2);

    r1.set_range(0,1);

    CHECK(r1 != r2);
    CHECK_FALSE(r1 == r2);

    r1.set_range(2,3);
    r1.add_cell(rc_coord{7,8});

    CHECK(r1 != r2);
    CHECK_FALSE(r1 == r2);
}

TEST_CASE("region: combining operations", "region::unite, region::intersect, region::subtract, region::merge") {
    using namespace ms;
    
    region r1, r2, r1ur2, r1xr2, r1sr2;

    r1.add_cell(rc_coord{1,2});
    r1.add_cell(rc_coord{2,3});
    r1.set_count(1);
    r2.add_cell(rc_coord{1,2});
    r2.add_cell(rc_coord{2,1});
    r2.add_cell(rc_coord{3,2});
    r1.set_count(1);
    
    r1ur2.add_cell(rc_coord{1,2});
    r1ur2.add_cell(rc_coord{2,3});
    r1ur2.add_cell(rc_coord{2,1});
    r1ur2.add_cell(rc_coord{3,2});
    r1ur2.set_range(1, 2);
    
    CHECK(r1.unite(r2) == r1ur2);
   
    r1xr2.add_cell(rc_coord{1,2});
    r1xr2.set_range(0, 1);

    CHECK(r1.intersect(r2) == r1xr2);
    
    r1sr2.add_cell(rc_coord{2,3});
    r1sr2.set_range(0, 1);

    CHECK(r1.subtract(r2) == r1sr2);

    CHECK(r1.merge(r2).size() == 0);

    region m1;

    m1.add_cell(rc_coord{1,2});
    m1.add_cell(rc_coord{2,3});
    m1.add_cell(rc_coord{3,4});
    region m2 = m1;
    region m1m2 = m1;
    m1.set_range(0,2);
    m2.set_range(1,3);
    m1m2.set_range(1,2);
    
    CHECK(m1.merge(m2) == m1m2);
}

#endif