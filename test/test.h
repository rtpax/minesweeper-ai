#ifndef MS_TEST_UNITTEST_H
#define MS_TEST_UNITTEST_H

/* test/test.h
 *
 * defines macros to change program behavior for when running test
 * vs when running normally.
 * 
 * use DO_IF_TEST sparingly as excessive use defeats the purpose
 * of running tests
 * 
 * test classes are forward declared here when TESTMODE is true
 * 
 * 
 */

#include <string>

struct test_output {
    int expected_output;
    int actual_output;
    std::string desc;
    bool pass;
};


#ifdef TESTMODE

#define DECL_TEST(class_name) class class_name ## _test
#define BEFRIEND_TEST(class_name) friend class_name ## _test
#define IF_TEST(code_to_do) code_to_do



#else

#define DECL_TEST(class_name)
#define BEFRIEND_TEST(class_name)
#define IF_TEST(code_to_do)

#endif

DECL_TEST(grid)
DECL_TEST(region)
DECL_TEST(solver)
DECL_TEST(spinoff)
DECL_TEST(spingrid)

#endif