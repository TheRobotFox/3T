#include "Memory.hpp"
#include "Types.hpp"


namespace TTT {

	class Stack {
	public:
		Stack(size_t size) : m_data(size), m_frameBase(m_data.begin()) {}

		auto push(Type_t) -> Heap_p&;
		void endFrame();
		void dropFrame();

		auto begin() {return m_data.begin();}
		auto end() {return m_data.end();}

    private:
		Buffer<void *> m_data;
		Heap_p *m_frameBase;
		
	};
	
}
