CXX:=g++
LD:=g++
DEBUG_LEVEL := 0

CXXFLAGS := -O2 -std=c++17 -g -Wall -DNDEBUG
DEBUG_CXXFLAGS := -O0 -fno-inline -g -std=c++17 -Wall -DDEBUG=$(DEBUG_LEVEL)
PROF_CXXFLAGS := -O2 -std=c++17 -g -Wall -DNDEBUG -pg
CPPFLAGS := -I/c/msys64/mingw64/include
LDFLAGS := -L/c/msys64/mingw64/libs
LDLIBS := -lncurses

OBJS := grid.o region.o region_set.o solver.o ui.o
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
	$(LD) $(MAIN_OBJS) $(CXXFLAGS) $(LDFLAGS) $(LDLIBS) -o sweep.exe

debug.exe: $(DEBUG_OBJS)
	$(LD) $(DEBUG_OBJS) $(DEBUG_CXXFLAGS) $(LDFLAGS) $(LDLIBS) -o debug.exe

test.exe: $(TEST_OBJS)
	$(LD) $(TEST_OBJS) $(CXXFLAGS) $(LDFLAGS) $(LDLIBS) -o test.exe

prof.exe: $(PROF_OBJS)
	$(LD) $(PROF_OBJS) $(PROF_CXXFLAGS) $(LDFLAGS) $(LDLIBS) -o prof.exe

%.o: %.cpp
	$(CXX) -c $(CPPFLAGS) $(CXXFLAGS) $*.cpp -o $*.o
	$(CXX) -MM $(CPPFLAGS) $(CXXFLAGS) $*.cpp -MT $*.o > $*.d

%.debug.o: %.cpp
	$(CXX) -c $(CPPFLAGS) $(DEBUG_CXXFLAGS) $*.cpp -o $*.debug.o
	$(CXX) -MM $(CPPFLAGS) $(DEBUG_CXXFLAGS) $*.cpp -MT $*.debug.o > $*.debug.d

%.prof.o: %.cpp
	$(CXX) -c $(CPPFLAGS) $(PROF_CXXFLAGS) $*.cpp -o $*.prof.o
	$(CXX) -MM $(CPPFLAGS) $(PROF_CXXFLAGS) $*.cpp -MT $*.prof.o > $*.prof.d

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









