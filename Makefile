CC=gcc
CPP=g++


CPPFLAGS=-g -c
LDFLAGS=-g
LDLIBS=

SRCS=grid.cpp main.cpp region.cpp solver.cpp spingrid.cpp spinoff.cpp
OBJS=$(subst .cpp,.o,$(SRCS))

SOLVER_DEP=grid.h region.h solver.h

OUT=minesweep.exe

all: $(OBJS)

install: $(OUT)

$(OUT):$(OBJS)
	$(CPP) $(OBJS) -o $(OUT) $(LDFLAGS) $(LDLIBS)
region.o: region.cpp region.h
	$(CPP) $(CPPFLAGS) region.cpp -o region.o
solver.o: solver.cpp $(SOLVER_DEP)
	$(CPP) $(CPPFLAGS) solver.cpp -o solver.o
grid.o: grid.cpp grid.h
	$(CPP) $(CPPFLAGS) grid.cpp -o grid.o
main.o: main.cpp $(SOLVER_DEP)
	$(CPP) $(CPPFLAGS) main.cpp -o main.o
spingrid.o: spingrid.cpp
	$(CPP) $(CPPFLAGS) spingrid.cpp -o spingrid.o
spinoff.o: spinoff.cpp spinoff.h $(SOLVER_DEP)
	$(CPP) $(CPPFLAGS) spinoff.cpp -o spinoff.o


clean:
	rm -f $(OBJS)

distclean: clean
	rm -f $(OUT)