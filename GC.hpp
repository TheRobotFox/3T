#include "Memory.hpp"
#include "Stack.hpp"
#include "Types.hpp"
#include <condition_variable>
#include <cstddef>
#include <memory>

namespace TTT {

	// TODO swap allocators when cell is empty -> notify collector -> block thread on exhaust

	struct FreeList {

        struct FreeListIterator {
			FreeList &a;
			Type_t t = 0;
            Heap_p *pivot = 0;
            Heap_p *it = 0;

            auto operator*() const -> Heap_p { return *it; }
			auto operator++() -> FreeListIterator & {
				if(it < pivot)
					it = *it == nullptr ? pivot : it--;
				else {
					if (*it == nullptr) {
						pivot = a.typeStart[++t];
						it = pivot-1;
					} else {
						it++;
					} 
				}
				return *this;
            }
            auto operator==(const FreeListIterator &other) const -> bool {
				return it == other.it;
            }
        };

        auto begin(/* TODO  */) -> FreeListIterator {return {*this};}
		auto end(/* TODO  */) -> FreeListIterator {
			size_t last = 0, curr;
			for (curr = 1; typeStart[curr] > typeStart[last]; curr++)
				;
			Heap_p *it = typeStart[curr];
			while(it!=nullptr) it++;
			return {.a=*this, .t=curr, .pivot=typeStart[curr], .it=it};
        }


        // nullptr seperated list of available Cells
		Heap_p **const typeStart;
        Heap_p *const freeList;
        

		[[nodiscard]] auto alloc(Type_t type) const -> Heap_p {
			Heap_p res = typeStart[type];
			if (res != nullptr) typeStart[type]++;
			return res;
		}

        auto allocChunk(Type_t) -> Chunk *;
        auto alloc(Type_t) -> Heap_p;

        auto offer(Type_t, Heap_p) -> bool;
		void setPriority(float *prio);

		constexpr FreeList(size_t freeListSize, size_t maxTypes)
		: typeStart(std::allocator<Heap_p *>{}.allocate(maxTypes)),
		  freeList(std::allocator<Heap_p>{}.allocate(freeListSize)){
			std::fill(typeStart, typeStart+maxTypes, nullptr);
			std::fill(freeList, freeList+maxTypes, nullptr);
		}
		~FreeList();
	};

	class Memory {
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
		auto swapAllocator() -> bool;
        auto getChunk(Heap_p cell) -> Chunk &;


        // Spinlocked Thread
        void run();



		constexpr GC(Settings settings)
		: stack(settings.stackSize), m_heap(settings.heapChunks),
		  m_freeChunks(settings.chunkreallocation),
		  m_allocChunks(std::allocator<Chunk *>{}.allocate(
														   settings.maxTypes)),
		  m_allocIndex(
					   std::allocator<size_t>{}.allocate(settings.maxTypes)),
		  m_freeLists(
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
		std::array<FreeList, 2> m_freeLists;
		std::mutex m_freeListMtx;
		std::condition_variable m_freeListCond;

		Chunk *m_collectChunk;
		size_t m_collectindex;
		float *m_typeStress;


		Buffer<ChunkInfo> m_typeInfo;

		bool running = true;
    };

    class GC {
    public:
		GC();


		struct ProtectInfo {
            Chunk **freeChunks;
            Chunk **activeChunks;
            
		size_t protectedChunksCount;
		size_t chunkCount;
        FreeList *list;
		size_t typeCount;
    };



        // perform marking cylce -> requires Protection State
        // (TODO) reset allocation read ptr
        void mark();

        // regenerate alternate Readlist
        // 1. Calculate current Stress on each Type
        // 2. Expand and regenerate AllocatorList by continueing traversal through heap
        

        void collect();

    };
}
