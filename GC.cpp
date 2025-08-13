#include "Atom.hpp"
#include "Memory.hpp"
#include <algorithm>
#include <cstddef>
#include <iostream>

namespace TTT {


	auto GC::collect() -> bool {

		// clear all Marks
		for (auto &cell : heap.data)
            cell.pass_nr = UNMARKED;


		// mark Atoms visible from Stack transitively
        struct MarkChildren {
			void operator()(Atom *child) {
				auto *cell = reinterpret_cast<Heap::Cell *>(child);
				if (cell->pass_nr == UNMARKED)
					return;
				cell->pass_nr = 1;
				cell->atom.visit(CallChildren(*this));
			}
		};

		Atom *stack_top = stack.frame_base.back();
		for (Atom *a = stack.data.get(); a < stack_top; a++)
			a->visit(CallChildren(MarkChildren{}));



                // compact in Chunks
                // all passes use same std::array containing dead cell addresses
                // to calculate pointer offsets after compaction
        
		Pass p{.pass_nr = 1, .count = 0, .offset = 0};

		Heap::Cell *read = heap.data.begin().base(),
		*write = heap.data.begin().base(),
		*end = heap.data.end().base();

		while (read != end) {
			if (read->pass_nr == UNMARKED) {
				if (p.count == p.chunk.size()) p.flush(heap);
				p.add(read);
            } else {
                *reinterpret_cast<Atom*>(write++)=*reinterpret_cast<Atom*>(read++);
			}
        }
        return p.count!=0;
    }

	auto GC::Pass::get_offset(Heap::Cell *atom) const -> size_t {
		size_t offset = this->offset;
		for (Heap::Cell *dead : chunk) {
			if (dead > atom)
				break;
			++offset;
		}
		return offset;
    }

    void GC::Pass::flush(Heap &heap) {

		
		struct UpdatePointers {
			Pass &p;
			void operator()(Atom *&child) {
				auto *cell = reinterpret_cast<Heap::Cell *>(child);
				if (cell->pass_nr == p.pass_nr)
					return;
					  
				child = reinterpret_cast<Atom *>(cell - p.get_offset(cell));
				cell->pass_nr = p.pass_nr;
				child->visit(CallChildren(*this));
			}
		};

		for (Heap::Cell &c : heap.data) 
			c.atom.visit(CallChildren(UpdatePointers{*this}));

		offset += count;
		count = 0;
		pass_nr++;
	}

	auto GC::alloc() -> Atom * {
		if(heap.available()>0)
            return heap.alloc();

        if (collect())
			return heap.alloc();
        
        std::cerr << "Reached Heap exhaustion!";
        exit(1);
	}
	
}
