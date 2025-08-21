#include "Atom.hpp"
#include <thread>
#include <vector>

namespace TTT {

	class Buffer;
    class Stack;

    class ConventionalMarker {
		std::vector<Atom *> stack;
	public:
		void mark(Buffer &heap, Stack &stack);
		static void clearMarks(Buffer &heap);
    };
    struct ParallelMarker;

    using use_marker = ConventionalMarker;

	namespace GC {

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


		template <class A> struct GetCell {
			class Cell : public A {
				Bool_t marked = 1;

			public:
				auto isMarked() const -> Bool_t { return marked; }
				void mark() {marked = 1;}
				void unmark() {marked = 0;}
			};
		};
    
		template <Header3Col A>struct GetCell<A> {
			using Cell = A;
		};
    
		class Concurrent {
            std::thread t;
            std::mutex mtx;
            bool running = true;
            void watch();

            use_marker marker;

            Buffer &heap;
			Stack &stack;

        public:
        
			using Cell = GetCell<Atom>::Cell;
        
			long long threshold;
			auto collect() -> size_t;
            auto alloc() -> Atom *;
            Concurrent(Buffer &heap, Stack &stack);
            ~Concurrent();
        };

        // Allocate control and data Arenas -> Atoms same size
        class Dynamic {
			
        }
	}

    using use_gc = GC::Concurrent;

    
    using Cell = typename use_gc::Cell;
	
}
