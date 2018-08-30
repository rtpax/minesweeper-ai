DEBUG_LEVEL := 0

CXXFLAGS := -O2 -std=c++17 -g -Wall -DNDEBUG
DEBUG_CXXFLAGS := -O0 -fno-inline -g -std=c++17 -Wall -DDEBUG=$(DEBUG_LEVEL)
PROF_CXXFLAGS := -O2 -std=c++17 -g -Wall -DNDEBUG -pg
CPPFLAGS := -I/c/msys64/mingw64/include
LDFLAGS := -L/c/msys64/mingw64/libs
LDLIBS :=

OBJS := grid.o region.o region_set.o solver.o
MAIN_OBJS := main.o $(OBJS)
TEST_OBJS := ./test/test.o $(OBJS)
DEBUG_OBJS := $(MAIN_OBJS:.o=.debug.o)
PROF_OBJS := $(MAIN_OBJS:.o=.prof.o)

all: sweep.exe debug.exe test.exe prof.exe

release: sweep.exe

debug: debug.exe

test: test.exe

prof: prof.exe

sweep.exe: $(MAIN_OBJS)
	g++ $(MAIN_OBJS) $(CXXFLAGS) $(LDFLAGS) $(LDLIBS) -o sweep.exe

debug.exe: $(DEBUG_OBJS)
	g++ $(DEBUG_OBJS) $(DEBUG_CXXFLAGS) $(LDFLAGS) $(LDLIBS) -o debug.exe

test.exe: $(TEST_OBJS)
	g++ $(TEST_OBJS) $(CXXFLAGS) $(LDFLAGS) $(LDLIBS) -o test.exe

prof.exe: $(PROF_OBJS)
	g++ $(PROF_OBJS) $(PROF_CXXFLAGS) $(LDFLAGS) $(LDLIBS) -o prof.exe

%.o: %.cpp
	g++ -c $(CPPFLAGS) $(CXXFLAGS) $*.cpp -o $*.o
	g++ -MM $(CPPFLAGS) $(CXXFLAGS) $*.cpp -MT $*.o > $*.d

%.debug.o: %.cpp
	g++ -c $(CPPFLAGS) $(DEBUG_CXXFLAGS) $*.cpp -o $*.debug.o
	g++ -MM $(CPPFLAGS) $(DEBUG_CXXFLAGS) $*.cpp -MT $*.debug.o > $*.debug.d

%.prof.o: %.cpp
	g++ -c $(CPPFLAGS) $(PROF_CXXFLAGS) $*.cpp -o $*.prof.o
	g++ -MM $(CPPFLAGS) $(PROF_CXXFLAGS) $*.cpp -MT $*.prof.o > $*.prof.d

-include $(MAIN_OBJS:.o=.d)
-include $(TEST_OBJS:.o=.d)
-include $(DEBUG_OBJS:.o=.d)
-include $(PROF_OBJS:.o=.d)

prof-clean:
	rm -rf *.prof.o *.prof.d prof.exe

debug-clean:
	rm -rf *.debug.o *.debug.d debug.exe

clean:
	rm -rf *.o *.d

distclean: clean
	rm -rf *.exe









