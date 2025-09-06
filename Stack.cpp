#include "Stack.hpp"
#include "GC.hpp"

namespace TTT {
	
	auto Stack::push(Type_t type) -> Heap_p & {
		// GC::getInstance(). (alloate)
	}
	void Stack::endFrame() {
		Heap_p *currentFrame = m_data.m_current;
		m_data.emplace(m_lastFrame);
		m_lastFrame = currentFrame;
	}
	void Stack::dropFrame() {
		m_data.m_current = reinterpret_cast<Heap_p**>(m_data.m_current)[-1];
	}

}
