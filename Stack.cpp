#include "Stack.hpp"
#include "Types.hpp"
#include <cassert>

namespace TTT {

	auto Stack::push(Heap_p obj) -> Heap_p& {return *current++ = obj;}
	void Stack::beginFrame() 	 {frame_base.push_back(current);}
	void Stack::dropFrame()		 {current = frame_base.back(); frame_base.pop_back();}
	auto Stack::span() -> std::span<Heap_p> {
		return {start, current};
    }

	Stack::Stack(size_t size) : start(new Heap_p[size]), current(start), end(start+size)
	{}

	
}
