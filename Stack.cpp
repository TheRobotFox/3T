#include "Stack.hpp"
#include "GC.hpp"

namespace TTT {
	
	auto Stack::push(Type_t type) -> Heap_p & {
		// GC::getInstance(). (alloate)
	}
	void Stack::endFrame() {
		Heap_p *currentFrame = m_data.m_current;
		m_data.emplace(m_frameBase);
		m_frameBase = currentFrame;
	}
	void Stack::dropFrame() {
		m_frameBase = m_data.m_current = reinterpret_cast<Heap_p**>(m_data.m_current)[-1];
	}

}

VVAVAVVVA
  0 3   5
0 3 5   8
        
