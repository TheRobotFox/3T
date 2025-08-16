#include <cstddef>
#include <cstdint>
#include <memory>
#include <ankerl/unordered_dense.h>
#include <span>
#include <thread>
#include <vector>
#include "Atom.hpp"

namespace TTT {

	class GC;
	#define UNMARKED 0


        // Atom-Aligned Dynamic Storage
    // Should not contain Refernces to Stack
    template<class GC>
	class Heap {

		friend GC;
        std::unique_ptr<typename GC::Cell[]> start;
        GC::Cell *current, *end;
        
			void mark(Atom *);
			auto alloc() -> Atom *;

		[[nodiscard]] auto span() const -> std::span<typename GC::Cell> {return std::span{start.get(), current};}

    public:
        [[nodiscard]] auto available() const -> size_t;
		Heap(size_t size) : start(new typename GC::C[size]), current(start.get()), end(start.get()+size)
		{}
	};


	// Reference to Argument on Stack for capture
	struct Argument {
		uint16_t ascend;		// Stackframe from top of Stack
		uint16_t frame_index;	// Offset within that Frame
	};

        // Atom-Aligned Argument Storage
    // Can contain references to Dynamic Memory
    template<class GC>    
	class Stack {
		friend GC;
		
		std::unique_ptr<Atom[]> data;
		Atom *current;
		Atom *end;
		std::vector<Atom *> frame_base;
		
	public:
		Stack(size_t size)
		: data(new Atom[size]), current(data.get()), end(data.get()+size), frame_base({data.get()})
		{}
        void end_frame();
        auto push() -> Atom&;
		
		// capture Variables for Closure Environments returns location of arg
		auto capture(Argument arg, Heap<GC> &store) -> Atom *;
		void drop_frame();
	};

    class CompactGC {

		static constexpr size_t chunk_size = 1 << 20;
		
        struct Pass {
            size_t pass_nr, count, offset;
            std::array<Cell *, chunk_size> chunk;
            auto get_offset(Cell *atom) const -> size_t;
			void flush(Heap<CompactGC> &heap);
			void add(Cell *dead) {
				chunk[count++] = dead;
			}
		};

        void do_pass(const Pass &p);

    public:
        struct Cell {
			Atom atom;
			uint_fast16_t pass_nr;
        };

        Stack<CompactGC> stack;
   		Heap<CompactGC> heap;

		// std::set<Atom*> mark; TODO
		auto collect() -> size_t;
        auto alloc() -> Atom *;
        
		CompactGC(Stack<CompactGC> &&stack, Heap<CompactGC> &&h)
		: stack(std::move(stack)), heap(std::move(h))
		{}
    };

    class ConcurrentGC {
		std::thread t;
	public:
      struct Cell {
          Atom atom;
          bool marked;
      };
		size_t threshold;
		auto collect() -> size_t;
		auto alloc() -> Atom *;
    };
	
} // namespace TTT
