# Usage:
#
# $(foreach module,$(input-modules),$(eval CUR_MODULE:=$(module)) $(eval include maketestmodule.mk))
#

$(shell echo $(CUR_MODULE):)
$(CUR_MODULE).o: $(CUR_MODULE).cpp $($(CUR_MODULE)_DEP)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -DTEST_MODE $< -o $@