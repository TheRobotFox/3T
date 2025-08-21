#include "Memory.hpp"

namespace TTT {

	void ConventionalMarker::clearMarks(Buffer &heap) {
        for (Cell &c : std::span{heap.start.get(), heap.current})
            c.unmark();
    }

    void ConventionalMarker::mark(Buffer &heap, Stack &stack) {

        for (Atom &a : std::span{stack.start.get(), stack.current})
			a.mark_children(this->stack);
		
    }
	
};
