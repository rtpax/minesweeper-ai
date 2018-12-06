CXX:=g++
DEBUG_LEVEL := 0

RELEASE_CXXFLAGS := -O2 -std=c++17 -g -Wall -DNDEBUG
DEBUG_CXXFLAGS := -O0 -fno-inline -g -std=c++17 -Wall -DDEBUG=$(DEBUG_LEVEL)
PROF_CXXFLAGS := -O2 -std=c++17 -g -Wall -DNDEBUG -pg
RELEASE_BUILD_DIR := release
DEBUG_BUILD_DIR := debug
PROF_BUILD_DIR := prof

config:=release

INSTALL_DIR := ~/bin
BUILD_DIR := $(RELEASE_BUILD_DIR)
CXXFLAGS := $(RELEASE_CXXFLAGS)

ifeq ($(config), release)
BUILD_DIR := $(RELEASE_BUILD_DIR)
CXXFLAGS := $(RELEASE_CXXFLAGS)
endif
ifeq ($(config), debug)
BUILD_DIR := $(DEBUG_BUILD_DIR)
CXXFLAGS := $(DEBUG_CXXFLAGS)
endif
ifeq ($(config), prof)
BUILD_DIR := $(PROF_BUILD_DIR)
CXXFLAGS := $(PROF_CXXFLAGS)
endif

CPPFLAGS :=
LDFLAGS :=
LDLIBS := -lncurses

SHARED_SRCS := grid.cpp region.cpp region_set.cpp solver.cpp ui.cpp
SRCS := main.cpp $(SHARED_SRCS)
OBJS := $(SRCS:%.cpp=$(BUILD_DIR)/%.o)
DEPS := $(SRCS:%.cpp=$(BUILD_DIR)/%.d)
TEST_SRCS := test.cpp $(SHARED_SRCS)
TEST_OBJS := $(TEST_SRCS:%.cpp=$(BUILD_DIR)/%.o)
TEST_DEPS := $(TEST_SRCS:%.cpp=$(BUILD_DIR)/%.d)

.PHONY: all test sweep install

sweep: $(BUILD_DIR) $(BUILD_DIR)/sweep

all: sweep test

test: $(BUILD_DIR) $(BUILD_DIR)/test

install: sweep
	cp $(BUILD_DIR)/sweep $(INSTALL_DIR)

$(BUILD_DIR):
	mkdir $(BUILD_DIR)

$(BUILD_DIR)/sweep: $(OBJS)
	$(CXX) $(OBJS) $(CXXFLAGS) $(LDFLAGS) $(LDLIBS) -o $(BUILD_DIR)/sweep

$(BUILD_DIR)/test: $(TEST_OBJS)
	$(CXX) $(TEST_OBJS) $(CXXFLAGS) $(LDFLAGS) $(LDLIBS) -o $(BUILD_DIR)/test

$(BUILD_DIR)/%.o: %.cpp
	$(CXX) -c $(CPPFLAGS) $(CXXFLAGS) $< -o $@
	$(CXX) -MM $(CPPFLAGS) $(CXXFLAGS) $< -MT $@ > $(BUILD_DIR)/$*.d

-include $(DEPS)
-include $(TEST_DEPS)

clean:
	rm -rf $(BUILD_DIR)

clean-objs:
	rm $(OBJS) $(TEST_OBJS)









