

CC=gcc
CXX=g++

OPTIMIZATION=-O0 -fno-inline
DEBUG_LEVEL=-DDEBUG

CPPFLAGS=$(DEBUG_LEVEL)
CXXFLAGS=-g -c -Wall -std=c++17 $(OPTIMIZATION)
LDFLAGS=-g
LDLIBS=

SRCS=grid.cpp main.cpp region.cpp solver.cpp spingrid.cpp spinoff.cpp
MODULES=$(SRCS:.cpp=)
OBJS=$(SRCS:.cpp=.o)

debug_DEP=
test_DEP=

solver_DEP=solver.h $(grid_DEP) $(region_DEP) $(test_DEP)
region_DEP=region.h $(test_DEP)
spingrid_DEP=spingrid.h $(grid_DEP) $(test_DEP)
spinoff_DEP=spinoff.h $(solver_DEP) $(spingrid_DEP) $(test_DEP)
main_DEP=solver.h spinoff.h $(test_DEP)
grid_DEP=grid.h $(test_DEP)

MODULE_CPPFLAGS=CPPFLAGS
MODULE_CXXFLAGS=CXXFLAGS

OUT=sweep.exe

all: $(OUT)

set-fast-vars:
	$(eval OPTIMIZATION=-O2) 
	$(eval DEBUG_LEVEL=-DNDEBUG) 

fast: set-fast-vars all

set-debug-vars:
	$(eval DEBUG_LEVEL=-DDEBUG=2)

debug: set-debug-vars all

nolink: $(OBJS)

$(OUT):$(OBJS)
	$(CXX) $(OBJS) -o $(OUT) $(LDFLAGS) $(LDLIBS)

$(foreach module,$(MODULES),$(eval CUR_MODULE:=$(module)) $(eval include makemodule.mk))



clean:
	rm -f $(OBJS)

distclean: clean
	rm -f $(OUT)

include maketest.mk




