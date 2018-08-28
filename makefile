DEBUG_LEVEL := 0

CXXFLAGS := -O2 -std=c++17 -g
DEBUG_CXXFLAGS := -O0 -fno-inline -g -std=c++17 -DDEBUG=$(DEBUG_LEVEL)
CPPFLAGS := -I/c/msys64/mingw64/include
LDFLAGS := -L/c/msys64/mingw64/libs
LDLIBS :=

OBJS := grid.o region.o region_set.o solver.o spingrid.o spinoff.o
MAIN_OBJS := main.o $(OBJS)
TEST_OBJS := ./test.o $(OBJS)
DEBUG_OBJS := $(MAIN_OBJS:.o=.debug.o)

all: sweep.exe debug.exe test.exe

release: sweep.exe

debug: debug.exe

test: test.exe

sweep.exe: $(MAIN_OBJS)
	g++ $(MAIN_OBJS) $(CXXFLAGS) $(LDFLAGS) $(LDLIBS) -o sweep.exe

debug.exe: $(DEBUG_OBJS)
	g++ $(DEBUG_OBJS) $(DEBUG_CXXFLAGS) $(LDFLAGS) $(LDLIBS) -o debug.exe

test.exe: $(TEST_OBJS)
	g++ $(TEST_OBJS) $(CXXFLAGS) $(LDFLAGS) $(LDLIBS) -o test.exe

%.o: %.cpp
	g++ -c $(CPPFLAGS) $(CXXFLAGS) $*.cpp -o $*.o
	g++ -MM $(CPPFLAGS) $(CXXFLAGS) $*.cpp > $*.d

%.debug.o: %.cpp
	g++ -c $(CPPFLAGS) $(DEBUG_CXXFLAGS) $*.cpp -o $*.debug.o
	g++ -MM $(CPPFLAGS) $(DEBUG_CXXFLAGS) $*.cpp > $*.debug.d

-include $(MAIN_OBJS:.o=.d)
-include $(TEST_OBJS:.o=.d)
-include $(DEBUG_OBJS:.o=.d)

debug-clean:
	rm -rf *.debug.o *.debug.d debug.exe

clean:
	rm -rf *.o *.d

distclean: clean
	rm -rf *.exe









