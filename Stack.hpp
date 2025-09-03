#pragma once
#include "Types.hpp"
#include <cstddef>
#include <vector>
#include <span>

namespace TTT {

	class Stack {

		Heap_p *start;
		Heap_p *current;
		Heap_p *end;
		std::vector<Heap_p *> frame_base;

    public:
        
        Stack(size_t size);
        ~Stack() {free(start);}
        void beginFrame();
        
        auto push(Heap_p) -> Heap_p&;

        void dropFrame();
		auto span() -> std::span<Heap_p>;
	};
} // namespace TTT
