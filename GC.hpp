#include "Memory.hpp"
#include "Stack.hpp"
#include "Types.hpp"
#include <condition_variable>
#include <cstddef>
#include <memory>

namespace TTT {

	// TODO swap allocators when cell is empty -> notify collector -> block thread on exhaust

	struct Allocator {

		size_t * const end;
		size_t * const current;
		Heap_p *const available;

		[[nodiscard]] auto alloc(Type_t type) const -> Heap_p {
			if (current[type] == end[type]) return nullptr;
			return available[current[type]++];
		}

		constexpr Allocator(size_t size, size_t maxTypes)
		: end(std::allocator<size_t>{}.allocate(maxTypes)),
		  current(std::allocator<size_t>{}.allocate(maxTypes)),
		  available(std::allocator<Heap_p>{}.allocate(size)) {
			std::fill(end, end+maxTypes, 0);
			std::fill(current, end+maxTypes, 0);
		}
		~Allocator();
	};

	class GC {
		friend Stack;
    public:
		
		struct Settings {
			size_t heapChunks       = 1 << 12;
			size_t stackSize        = 1 << 15;
			size_t maxTypes	       = 1 << 10;
			size_t allocationBuffer = 1 << 20;
			size_t chunkreallocation= 32;
		};
		
		template<class ...Args>
		static auto init(Args&& ...args) -> bool {
			delete instance;
			instance = new GC(std::forward<Args>(args)...);
			return true;
		}
		
		static auto getInstance() -> GC & {
			assert(instance != nullptr &&
				   "Gargabe Collector has not been initilized!");
			return *instance;
		}

		void mark(Heap_p cell);
		
		auto getInfo(Type_t type) -> ChunkInfo {
			return m_typeInfo[type];
		}

		Stack stack;

	private:
		auto allocChunk(Type_t) -> Chunk *;
		auto swapAllocator() -> bool;
		auto getChunk(Heap_p cell) -> Chunk &;
		auto alloc(Type_t) -> Heap_p;
		
		void run();
		void mark();
		void collect();


		constexpr GC(Settings settings)
		: stack(settings.stackSize), m_heap(settings.heapChunks),
		  m_freeChunks(settings.chunkreallocation),
		  m_allocChunks(std::allocator<Chunk *>{}.allocate(
														   settings.maxTypes)),
		  m_allocIndex(
					   std::allocator<size_t>{}.allocate(settings.maxTypes)),
		  m_allocators(
					   {{settings.allocationBuffer, settings.maxTypes},
						{settings.allocationBuffer, settings.maxTypes}}),
		  m_typeStress(std::allocator<float>{}.allocate(settings.maxTypes)),
		  m_typeInfo(settings.maxTypes)
		{
			for (auto ci : TypeInfo::Result<ChunkInfo::BuiltIn>)
				m_typeInfo.emplace(ci);
			
			std::fill(m_typeStress, m_typeStress+settings.maxTypes, 1.0F);
		}


		static GC * instance;

		Buffer<Chunk> m_heap;
		Buffer<Chunk*> m_freeChunks;

		
		Chunk **m_allocChunks;
		size_t *m_allocIndex;
		
		uint8_t m_currentAllocator{};
		std::array<Allocator, 2> m_allocators;
		std::mutex m_allocatorMtx;
		std::condition_variable m_allocatorCond;

		Heap_p m_currentCollect;
		float *m_typeStress;


		Buffer<ChunkInfo> m_typeInfo;

		bool running = true;
		
		
    };

}
