#include "GC.hpp"
#include "Memory.hpp"
#include "Types.hpp"
#include <cstddef>
#include <cstdint>
#include <memory>
#include <mutex>

namespace TTT {


	/*
	 * Chunk
	 */


	auto Chunk::getCount() const -> size_t {
		
	}
	auto Chunk::getTypeSize() const -> size_t {
		
	}
	auto Chunk::getType() const -> Type_t {
		
	}
	auto Chunk::getData() -> std::byte * {
		
	}
	auto Chunk::getMarks() -> std::byte * {
		
	}

	auto Chunk::isMarked(size_t idx) -> bool {
		
	}
	auto Chunk::isInitized(size_t idx) -> bool {
		
	}
	auto Chunk::empty() -> bool {
		
	}
		
	void resetMarks();
	void setMark(void *obj);
	void setInitilized(void *obj);


	/*
	 * GC
	 */
	
	auto GC::allocChunk(Type_t type) -> Chunk * {
		// TODO lock thread?
	  if (m_freeChunks.size() > 0)
	      return m_freeChunks.pop();
	  return &m_heap.emplace(type);
	}

	auto GC::swapAllocator() -> bool {
		if (!m_allocatorMtx.try_lock())
			return false;
		m_currentAllocator ^= 1;
		m_allocatorMtx.unlock();
		m_allocatorCond.notify_one();
		return true;
	}
	
	
	auto GC::getChunk(Heap_p cell) -> Chunk & {
		return *reinterpret_cast<Chunk*>(reinterpret_cast<std::intptr_t>(cell) & ( Chunk::size -1));
	}

	
	auto GC::alloc(Type_t type) -> Heap_p {
		const Heap_p res = m_allocators[m_currentAllocator].alloc(type);
		if (!res) {
			if(swapAllocator()) return alloc(type);

			// allocate new Chunks
			auto info = getInfo(type);
			if (m_allocChunks[type] == nullptr || m_allocIndex[type] == info.elementCount) {
				m_allocChunks[type] = allocChunk(type);
				m_allocIndex[type] = 0;
			}
			return m_allocChunks[type]->getData() + m_allocIndex[type] * info.elementCount;
		}
		return res;
	}


	void GC::run() {
		uint8_t allocator = 0;
		std::unique_lock<std::mutex> lock(m_allocatorMtx);
		while (running) {

			m_allocatorCond.wait(lock, [this, &allocator] {
				return m_currentAllocator != allocator;
			});
			if (m_collectChunk - m_heap.begin() <
				m_heap.size() / 5)
                mark();

            

			// how long should objcts be protected
			// at least 1 cycle
			// allocatorSwapMutex is locked -> we know
			// which memory can be reached what about
			// newly allocated chunks??

			// copy State at timestamp
			// ignore freeChunks
			// ignore newly created chunks
			// ignore protected
			// chunks
			// ignore cells from freelist
				
				
							
		}
	}

	// TODO continued Colletion -> Thread notify empty Allocator
	//		sceduled Full Mark/Sweep

	void GC::mark() {

		
		/*
		 * Mark Cells
		 */

		for (Chunk &chunk : m_heap)
			chunk.resetMarks();

		for (Heap_p a : stack)
			mark(a);


		// Also mark all known free Cells to not free them twice, also
		// avoids freeing chunks, which are in the allocation list.

		// if done after thorough marking, shallow marking is
		// sufficent

		const auto &prevAllocator =
			m_allocators[m_currentAllocator];

		for (Heap_p cell : std::span{prevAllocator.available, prevAllocator.total}) 
			mark(cell);

	}
	
	void collect() {

			for (Chunk &chunk : std::span{heap, chunkCount}){
				auto &info = chunkInfo[chunk.getType()];

				if (chunk.empty(chunkInfo))
					freeChunks.push(&chunk);
				else for (size_t i = 0; i < info.elementCount ; i++){
					if (!chunk.isMarked(chunkInfo, i))
						if(!freeCells[chunk.getType()].push(
															 chunk.getData() + (i * info.typeSize)))
							break;
				}
			}

			/*
			 * Create new Chunks if needed
			 */
			for (Type_t t = 0; t < typeCount; t++) {
				const auto &info = chunkInfo[t];

				while (freeCells[t].size() < threshold &&
					   info.typeSize != 0) {
					Chunk &chunk = heap[chunkCount++];
					std::construct_at(&chunk, t);
				
					for (size_t i = 0; i < info.elementCount; i++)
						if (!freeCells[t].push(chunk.getData() + i*info.typeSize))
							break;
				}
			}
		}
	} // namespace TTT


        // conflict at marking phase
        // not marking allocated object
        // Solution: mark all memory visible by allocator
        // current ALlocation list
        // when does marking occur -> collect index too low (maybe move away
// from protected chunks)
// lock freeListswapMutex while marking => only current freelist is volatile
// collection is safe if marking is done convervatively
