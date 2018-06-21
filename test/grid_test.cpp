#include "grid_test.h"
#include <functional>
#include <sstream>

namespace ms {

    std::vector<test_output> grid_test::iscontained() {
        std::vector<test_output> ret;
        grid testgrid(10,10,10);
        int expected, result;

        expected = 1;
        result = testgrid.iscontained(5,2);
        ret.push_back(test_output{ expected,result,"5,2 contained",expected==result });

        expected = 0;
        result = testgrid.iscontained(100,4);
        ret.push_back(test_output{ expected,result,"100,4 contained",expected==result });

        expected = 0;
        result = testgrid.iscontained(7,100);
        ret.push_back(test_output{ expected,result,"7,100 contained",expected==result });

        expected = 1;
        result = testgrid.iscontained(9,9);
        ret.push_back(test_output{ expected,result,"9,9 contained",expected==result });

        expected = 0;
        result = testgrid.iscontained(10,1);
        ret.push_back(test_output{ expected,result,"10,1 contained",expected==result });

        expected = 0;
        result = testgrid.iscontained(1,10);
        ret.push_back(test_output{ expected,result,"1,10 contained",expected==result });

        return ret;
    }

    std::vector<test_output> grid_test::open_get() {
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
        const grid::cell _R = grid::cell::error;
        

        std::vector<test_output> ret;
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
        
        check[0] = new grid::cell[11]{ _H,_F,_H,_H,_H,_H,_H,_H,_H,_H,_R };
        check[1] = new grid::cell[11]{ _H,_H,_H,_F,_H,_H,_H,_H,_F,_H,_R };
        check[2] = new grid::cell[11]{ _H,_H,_H,_H,_H,_H,_H,_H,_H,_H,_R };
        check[3] = new grid::cell[11]{ _H,_H,_H,_H,_H,_H,_F,_H,_H,_H,_R };
        check[4] = new grid::cell[11]{ _H,_H,_H,_H,_H,_H,_H,_H,_H,_H,_R };
        check[5] = new grid::cell[11]{ _H,_H,_F,_H,_H,_H,_H,_H,_H,_H,_R };
        check[6] = new grid::cell[11]{ _H,_H,_H,_H,_H,_H,_H,_H,_H,_H,_R };
        check[7] = new grid::cell[11]{ _H,_F,_H,_H,_H,_H,_F,_H,_H,_H,_R };
        check[8] = new grid::cell[11]{ _1,_1,_2,_F,_H,_H,_2,_H,_H,_H,_R };
        check[9] = new grid::cell[11]{ _0,_0,_1,_H,_H,_H,_H,_F,_1,_H,_R };
        check[10]= new grid::cell[11]{ _R,_R,_R,_R,_R,_R,_R,_R,_R,_R,_R };
        
        grid testgrid(10,10,init);
        int expected, result;

        expected = grid::cell::ms_hidden;
        result = testgrid.get(0,1);
        ret.push_back(test_output{ expected,result,"0,1 opened hidden",expected==result });

        expected = grid::cell::ms_hidden;
        result = testgrid.get(5,5);
        ret.push_back(test_output{ expected,result,"5,5 opened hidden",expected==result });

        expected = grid::cell::ms_error;
        result = testgrid.get(40,30);
        ret.push_back(test_output{ expected,result,"40,30 opened error",,expected==result });

        testgrid.open(9,8);
        expected = grid::cell::ms_1;
        result = testgrid.get(9,8);
        ret.push_back(test_output{ expected,result,"9,8 opened value '1'",expected==result });

        testgrid.open(8,6);
        expected = grid::cell::ms_2;
        result = testgrid.get(8,6);
        ret.push_back(test_output{ expected,result,"8,6 opened value '2'",expected==result });

        testgrid.open(9,0);
        expected = 1;
        result = 1;
        std::string desc="";
        std::stringstream ss(desc);
        ss << "9,0 opened value '0'\n";
        for(int r = 0; r < 11; ++r) {
            for(int c = 0; c < 11; ++r) {
                ss << desc << testgrid.get(r,c) << "/" << check[r][c];
                if(testgrid.get(r,c) != check[r][c])
                    result = 0;
            }
            ss << "\n";
        }
        ret.push_back(test_output{ expected,result,desc,expected==result});

        testgrid.open(7,1);
        expected = grid::cell::ms_bomb;
        result = testgrid.get(7,1);
        ret.push_back(test_output{ expected,result,"7,1 opened value 'bomb'",expected==result });

        return ret;
    }

    std::vector<test_output> grid_test::flag() {
        std::vector<test_output> ret;
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
        int expected, result;

        testgrid.flag(9,8);
        expected = grid::cell::ms_flag;
        result = testgrid.get(9,8);
        ret.push_back(test_output{ expected,result,"9,8 hid->flag",expected==result });

        testgrid.flag(8,6);
        expected = grid::cell::ms_flag;
        result = testgrid.get(8,6);
        ret.push_back(test_output{ expected,result,"8,6 hid->flag",expected==result });

        expected = 1;
        result = testgrid.flag(10,21);
        ret.push_back(test_output{ expected,result,"10,21 hid->flag (out of range) return value",expected==result });

        testgrid.flag(8,6);
        expected = grid::cell::ms_question;
        result = testgrid.get(8,6);
        ret.push_back(test_output{ expected,result,"8,6 flag->?",expected==result });

        testgrid.flag(8,6);
        expected = grid::cell::ms_hidden;
        result = testgrid.get(8,6);
        ret.push_back(test_output{ expected,result,"8,6 ?->hid",expected==result });

        testgrid.set_flag(9,8,grid::cell::ms_hidden);
        expected = grid::cell::ms_hidden;
        result = testgrid.get(8,6);
        ret.push_back(test_output{ expected,result,"9,8 set hid (was flag)",expected==result });

        testgrid.set_flag(5,5,grid::cell::ms_hidden);
        expected = grid::cell::ms_hidden;
        result = testgrid.get(8,6);
        ret.push_back(test_output{ expected,result,"5,5 set hid (was hid)",expected==result });

        testgrid.set_flag(6,6,grid::cell::ms_question);
        expected = grid::cell::ms_hidden;
        result = testgrid.get(8,6);
        ret.push_back(test_output{ expected,result,"6,6 set ? (was hid)",expected==result });


        return ret;
    }

    // std::vector<test_output> grid_test::open();
    // std::vector<test_output> grid_test::reset();


}
