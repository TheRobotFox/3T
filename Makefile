CXX = clang++
CPPFLAGS = -MMD -MP -std=c++26 -ggdb -Wall #-Wextra -Wpedantic
BUILD_DIR := build


SOURCE := Atom.cpp Interpreter.cpp Special.cpp Module.cpp Memory.cpp repl.cpp

OBJECTS:= $(SOURCE:%=$(BUILD_DIR)/%.o)
DEPS := $(OBJECTS:.o=.d)


$(BUILD_DIR)/%.cpp.o: %.cpp
	mkdir -p $(dir $@)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

repl: $(OBJECTS)
	mkdir $(BUILD_DIR) -p
	$(CXX) $(OBJECTS) -o $@ $(LDFLAGS)

-include $(DEPS)
