$(shell echo $(CUR_MODULE):)
$(CUR_MODULE).o: $(CUR_MODULE).cpp $($(CUR_MODULE)_DEP)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $< -o $@