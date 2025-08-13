#include <cstddef>
#include <cstdint>
#include <memory>
#include <ankerl/unordered_dense.h>
#include <vector>
#include "Atom.hpp"

namespace TTT {


	struct Allocation {
		std::byte *start;
    };
    struct GCHeader {
		bool marked;
    };

    struct GC;
    
	// Memory-Aligned Dynamic Storage
	// Should not contain Refernces to Stack
    class Heap {

		friend GC;
        std::unique_ptr<std::byte> data;
        std::byte *current;
        std::byte *end;
        std::vector<Allocation> allocations;

        auto get_chunk(void *ptr) -> Allocation *;
        void mark(void *);
	public:
        auto alloc(size_t bytes) -> std::byte*;
        template <class T>
        auto alloc() -> T* {return reinterpret_cast<T*>(alloc(sizeof(T)));}

        Heap(size_t size)
        : data(new std::byte[size]), current(data.get()), end(current+size)
        {}
    };

    // Reference to Argument on Stack for capture
    struct Argument {
		uint16_t ascend; 		// Stackframe from top of Stack
		uint16_t frame_index; 	// Offset within that Frame
    };

    // Atom-Aligned Argument Storage
    // Can contain references to Dynamic Memory
    class Stack {
        std::unique_ptr<Atom> data;
        Atom *end;
		std::vector<Atom *> frame_base;
        
    public:
		Stack(size_t size)
		: data(new Atom[size]), end(data.get()+size), frame_base({data.get()})
		{}
        void push_frame(size_t num_args);
        
        // capture Variables for Closure Environments returns location of arg
        auto capture(Argument arg) -> Atom *;

        // Pop Frame and construct Linked Environemnt if capture required
        void pop_stack(Heap &store);
	};
	
} // namespace TTT
