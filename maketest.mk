
TEST_CPPFLAGS=$(CPPFLAGS) -DTESTMODE
TEST_SRC_PREFIX=/test/
TEST_SRCS=grid_test.cpp test.cpp
TEST_MODULES=$(SRCS:.cpp=)
TEST_OBJS=$(SRCS:.cpp=.o)

