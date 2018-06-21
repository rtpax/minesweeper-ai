#ifndef MS_TEST_GRID_TEST_H
#define MS_TEST_GRID_TEST_H

#include "../grid.h"
#include "test.h"

#include <vector>

namespace ms {
    
    class grid_test { 
    public:
        static std::vector<test_output> iscontained();
        static std::vector<test_output> open_get();
        static std::vector<test_output> flag();
        static std::vector<test_output> reset();
        static std::vector<test_output> updategamestate();
        static std::vector<test_output> constructor();
    };

}

#endif