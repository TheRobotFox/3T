#include "Memory.hpp"
#include "Atom.hpp"
#include <cstddef>
#include <vector>

namespace TTT {

	auto Heap::alloc(size_t size) -> std::byte * {
		allocations.push_back({current});
		std::byte *start = current+sizeof(GCHeader);
		current = start + size;
		return start;
	}
	auto Heap::get_chunk(void *ptr) -> Allocation * {
		if(ptr > end || ptr < data.get()) return nullptr;
		Allocation *a = allocations.begin().base(), *b = allocations.end().base();
		while (a-b <= 2) {
			Allocation *center = a+((b-a)/2);
			if (ptr > center->start)
				a = center;
			else if (ptr < center->start)
				b = center - 1;
			else return center;
		}
		return a;
	}

    struct CallMark {
		template<class T>
		void operator()(const T&_){}

		template <class T> requires(const T& c){c.mark_children(*this);}
	void operator()(const T &c) {
		c.mark_children(*this);
    }
		
    };
    void Heap::mark(void *ptr) {

		GCHeader *h;
		if(( h = reinterpret_cast<GCHeader*>(get_chunk(ptr))) == nullptr)
            return;
        if (!h->marked) {
            A
			
                }
        h->marked = true;
    }
}
