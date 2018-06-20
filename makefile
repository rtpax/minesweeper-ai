CC=gcc
CXX=g++

CPPFLAGS=-DDEBUG=2
CFLAGS=-g -c -Wall
CXXFLAGS=-g -c -Wall -O0 -fno-inline
# CXXFLAFS=-g -c -Wall -02
LDFLAGS=-g
LDLIBS=

SRCS=grid.cpp main.cpp region.cpp solver.cpp spingrid.cpp spinoff.cpp
MODULES=$(SRCS:.cpp=)
OBJS=$(SRCS:.cpp=.o)

debug_DEP=
test_DEP=test/test.h

solver_DEP=solver.h $(grid_DEP) $(region_DEP) $(test_DEP)
region_DEP=region.h $(test_DEP)
spingrid_DEP=spingrid.h $(grid_DEP) $(test_DEP)
spinoff_DEP=spinoff.h $(solver_DEP) $(spingrid_DEP) $(test_DEP)
main_DEP=solver.h spinoff.h $(test_DEP)
grid_DEP=grid.h $(test_DEP)


TEST_CPPFLAGS=$(CPPFLAGS) -DTESTMODE
TEST_SRC_PREFIX=/test/
TEST_SRCS=grid_test.cpp test.cpp
TEST_MODULES=$(SRCS:.cpp=)
TEST_OBJS=$(SRCS:.cpp=.o)

solver_test_DEP=solver_test.h $(solver_DEP)
region_test_DEP=region_test.h $(region_DEP)
spingrid_test_DEP=spingrid_test.h $(spingrid_DEP)
spinoff_test_DEP=spinoff_test.h $(spinoff_DEP)
grid_test_DEP=grid_test.h $(grid_DEP)

OUT=sweep.exe

all: $(OUT)

nolink: $(OBJS)

$(OUT):$(OBJS)
	$(CXX) $(OBJS) -o $(OUT) $(LDFLAGS) $(LDLIBS)


$(foreach module,$(MODULES),$(eval CUR_MODULE:=$(module)) $(eval include makemodule.mk))


clean:
	rm -f $(OBJS)

distclean: clean
	rm -f $(OUT)