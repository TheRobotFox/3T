#include <cstddef>
#include <cstdint>
#include <memory>
#include <ankerl/unordered_dense.h>
#include <span>
#include <vector>
#include "GC.hpp"

namespace TTT {

	class Buffer {

        friend use_gc;
        friend use_marker;
        
        std::unique_ptr<Cell[]> start;
        Cell *current, *end;
        
		void mark(Atom *);
		auto alloc() -> Atom *;

		[[nodiscard]] auto span() const -> std::span<Cell> {return std::span{start.get(), current};}

    public:
        [[nodiscard]] auto available() const -> size_t;
		Buffer(size_t size) : start(new Cell[size]), current(start.get()), end(start.get()+size)
		{}
	};


	// Reference to Argument on Stack for capture
	struct Argument {
		uint16_t ascend;		// Stackframe from top of Stack
		uint16_t frame_index;	// Offset within that Frame
	};

    class Stack {

		friend use_gc;
        friend use_marker;
        
		std::unique_ptr<Atom[]> start;
		Atom *current;
		Atom *end;
		std::vector<Atom *> frame_base;

    public:
        
		Stack(size_t size)
		: start(new Atom[size]), current(start.get()), end(start.get()+size), frame_base({start.get()})
		{}
        void end_frame();
        auto push() -> Atom&;
		
		void drop_frame();
	};
	
} // namespace TTT
