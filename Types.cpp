#include "Types.hpp"
#include "GC.hpp"
#include <cstddef>

namespace TTT {

	void Cons::markChildren() const {
		GC::getInstance().mark(car);
		GC::getInstance().mark(cdr);
    }

    void Array::markChildren() const {
		// TODO mark 64 bit Chunks a time using custom Bitset
        for (size_t i = 0; i < length; i++)
			GC::getInstance().mark(start + i);
    }

    void Table::markChildren() const {

		for (const auto &[_, cell] : value) 
			GC::getInstance().mark(cell);
    }

	void Closure::markChildren() const {
		environment.markChildren();
    }

    void Forward::markChildren() const {
			GC::getInstance().mark(ref);		
    }
}
