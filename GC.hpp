#include <cstddef>
#include <thread>
#include <vector>

namespace TTT {

	class Buffer;
    class Stack;
    class Atom;
    class Header;

    class ConventionalMarker {
		std::vector<Atom *> stack;
	public:
		void mark(Buffer &heap, Stack &stack);
		static void clearMarks(Buffer &heap);
    };
    struct ParallelMarker;

    using use_marker = ConventionalMarker;

    namespace GC {

#define MAX_CELL_SIZE 16

		// class CompactGC {

		// 	static constexpr size_t chunk_size = 1 << 20;
		
		//     struct Pass {
		//         size_t pass_nr, count, offset;
		//         std::array<Cell *, chunk_size> chunk;
		//         auto get_offset(Cell *atom) const -> size_t;
		// 		void flush(Heap<CompactGC> &heap);
		// 		void add(Cell *dead) {
		// 			chunk[count++] = dead;
		// 		}
		// 	};

		//     void do_pass(const Pass &p);

		// public:
		//     struct Cell {
		// 		Atom atom;
		// 		uint_fast16_t pass_nr;
		//     };

		//     Stack<CompactGC> stack;
   		// 	Heap<CompactGC> heap;

		// 	// std::set<Atom*> mark; TODO
		// 	auto collect() -> size_t;
		//     auto alloc() -> Atom *;
        
		// 	CompactGC(Stack<CompactGC> &&stack, Heap<CompactGC> &&h)
		// 	: stack(std::move(stack)), heap(std::move(h))
		// 	{}
		// };

  
		class Concurrent {
            std::thread t;
            std::mutex mtx;
            bool running = true;
            void watch();

            use_marker marker;

            Buffer &heap;
            Stack &stack;

            struct Allocation {
				Atom *start;
				size_t length;
            };

            std::vector<Allocation> allocations;
        public:
        
			long long threshold;
			auto collect() -> size_t;
            auto alloc(Header h) -> Atom *;
            Concurrent(Buffer &heap, Stack &stack);
            ~Concurrent();
        };

        // Allocate control and data Arenas -> Atoms same size
        class Dynamic {};
        // Allocate control Pages for each Atom Type (or same size) and data Buffers
        class DynamicSep {
			
        };

	}

    using use_gc = GC::Concurrent;
	
}
