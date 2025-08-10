#include <cstddef>
#include <memory>
#include <vector>
#include "Atom.hpp"

namespace TTT {


	class Memory {
		std::unique_ptr<Atom> heap;
		size_t current {};
	public:
		auto alloc() -> Atom*;
		void collect();
		void mark(Atom *);
		Memory(size_t initial_size) : heap(new Atom[initial_size]){}
	};
	
} // namespace TTT
