#include "Memory.hpp"
#include <cassert>

namespace TTT {

	auto Stack::push() -> Atom & 	{ assert(!frame_base.empty()); return *current++; }
	void Stack::end_frame() 		{frame_base.push_back(current);}
	void Stack::drop_frame() 		{current = frame_base.back(); frame_base.pop_back();}
    
	
}
