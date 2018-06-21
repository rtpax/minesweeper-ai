TEST_DIR=test/
TEST_SRCS_SHORT=grid_test.cpp test.cpp
TEST_SRCS=$(foreach src,$(TEST_SRCS_SHORT),$(TEST_DIR)$(src))
TEST_MODULES=$(TEST_SRCS:.cpp=)
TEST_OBJS=$(TEST_SRCS:.cpp=.o)

solver_test_DEP=solver_test.h $(solver_DEP)
region_test_DEP=region_test.h $(region_DEP)
spingrid_test_DEP=spingrid_test.h $(spingrid_DEP)
spinoff_test_DEP=spinoff_test.h $(spinoff_DEP)
grid_test_DEP=grid_test.h $(grid_DEP)

TEST_OUT=$(TEST_DIR)sweep_test.exe

$(TEST_OUT):$(OBJS) $(TEST_OBJS)
	$(CXX) $(OBJS) $(TEST_OBJS) -o $(TEST_OUT) $(LDFLAGS) $(LDLIBS)

$(foreach module,$(TEST_MODULES),$(eval CUR_MODULE:=$(module)) $(eval include maketestmodule.mk))
	
test: $(TEST_OUT)


testclean: clean
	rm -f $(TEST_OBJS)

testdistclean: testclean
	rm -f $(TEST_OUT)