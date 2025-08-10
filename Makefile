CXX = clang++
CPPFLAGS = -std=c++26 -ggdb -Wall #-Wextra -Wpedantic
BUILD_DIR := build


SOURCE := Atom.cpp Interpreter.cpp Special.cpp Module.cpp Memory.cpp

OBJECTS:= $(SOURCE:%=$(BUILD_DIR)/%.o)

$(BUILD_DIR)/%.cpp.o: %.cpp %.hpp
	mkdir -p $(dir $@)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

repl: $(OBJECTS) repl.o
	mkdir $(BUILD_DIR) -p
	$(CXX) $(OBJECTS) repl.o -o $@ $(LDFLAGS)
