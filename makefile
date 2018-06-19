CC=gcc
CXX=g++

CPPFLAGS=-DDEBUG=2
CFLAGS=-g -c -Wall
CXXFLAGS=-g -c -Wall -O0 -fno-inline
LDFLAGS=-g
LDLIBS=

SRCS=grid.cpp main.cpp region.cpp solver.cpp spingrid.cpp spinoff.cpp
MODULES=$(SRCS:.cpp=)
OBJS=$(SRCS:.cpp=.o)

solver_DEP=solver.h $(grid_DEP) $(region_DEP)
region_DEP=region.h
spingrid_DEP=spingrid.h $(grid_DEP)
spinoff_DEP=spinoff.h $(solver_DEP) $(spingrid_DEP)
main_DEP=solver.h spinoff.h
grid_DEP=grid.h

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