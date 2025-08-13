#include <cstddef>
#include <cstdint>
#include <memory>
#include <ankerl/unordered_dense.h>
#include <span>
#include <vector>
#include "Atom.hpp"

namespace TTT {

	class GC;
#define UNMARKED 0

	
	// Atom-Aligned Dynamic Storage
	// Should not contain Refernces to Stack
	class Heap {
		struct Cell {
			Atom atom;
			uint_fast16_t pass_nr;
		};

		friend GC;
        std::unique_ptr<Cell> start;
        Cell *current, *end;
        std::span<Cell> data;
        
			void mark(Atom *);
			auto alloc() -> Atom *;
			auto available() const -> size_t;
		
		public:

		Heap(size_t size) : start(new Cell[size]), current(start.get()), end(start.get()+size), data(start.get(), end)
		{}
	};


	// Reference to Argument on Stack for capture
	struct Argument {
		uint16_t ascend;		// Stackframe from top of Stack
		uint16_t frame_index;	// Offset within that Frame
	};

	// Atom-Aligned Argument Storage
	// Can contain references to Dynamic Memory
	class Stack {
		friend GC;
		
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

    class GC {
		static constexpr size_t chunk_size = 1 << 20;
		
        struct Pass {
            size_t pass_nr, count, offset;
            std::array<Heap::Cell *, chunk_size> chunk;
            auto get_offset(Heap::Cell *atom) const -> size_t;
			void flush(Heap &heap);
            void add(Heap::Cell *dead) {
				chunk[count++] = dead;
            }
		};
		Heap heap;

		auto collect() -> bool;
        void do_pass(const Pass &p);
	public:
		Stack stack;
		// std::set<Atom*> mark; TODO
		auto alloc() -> Atom *;
	};
	
} // namespace TTT
