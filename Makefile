CXX = clang++
CPPFLAGS = -MMD -MP -std=c++26 -ggdb -Wall -Wextra -Wpedantic
BUILD_DIR := build

MEMORY := GC.cpp Stack.cpp Types.cpp
INTERP := Interpreter.cpp Special.cpp Module.cpp $(MEMORY)

ALL := $(INTERP)

DEPS := $(ALL:%=$(BUILD_DIR)/%.d)

test: $(patsubst %, $(BUILD_DIR)/%.o, $(MEMORY) test.cpp)
	mkdir $(BUILD_DIR) -p
	$(CXX) $^ -o $@ $(LDFLAGS)

$(BUILD_DIR)/%.cpp.o: %.cpp 
	mkdir -p $(dir $@)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

repl: $(patsubst %.cpp, $(BUILD_DIR)/%.o, $(INTERP) repl.cpp)
	mkdir $(BUILD_DIR) -p
	$(CXX) $^ -o $@ $(LDFLAGS)
clean:
	rm -rf $(BUILD_DIR)


-include $(DEPS)
