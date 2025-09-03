#pragma once
#include "Types.hpp"
#include "RingBuffer.hpp"
#include "Stack.hpp"
#include <cassert>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <new>
#include <variant>
#include <vector>

namespace TTT {

	// TODO use std::array<std::byte, size> as chunk! Avoid confusing cells with cunks

	/*
	  [Garbage Collector]

The Heap is managed by keeping track of Chunks each of which able to
store multiple objects of a single type in a continuous Sequence.

	 */
	struct ChunkInfo;
	
	class Chunk {
	public:

		constexpr static size_t sizeExp = 13;
		constexpr static size_t size = 1 << sizeExp;

		auto getType() const -> Type_t;
        auto getData() -> std::byte *;
        auto getMarks(std::vector<ChunkInfo> &typeInfo) -> std::byte *;
        auto isMarked(std::vector<ChunkInfo> &typeInfo, size_t idx) -> bool;
        void resetMarks(std::vector<ChunkInfo> &typeInfo);
        auto empty(std::vector<ChunkInfo> &typeInfo) -> bool;
        void mark(std::vector<ChunkInfo> &typeInfo, void *obj);

		Chunk(Type_t type) {*reinterpret_cast<Type_t *>(data.data()) = type;}

    private:
		std::array<std::byte, size> data;
    };
    
	struct ChunkInfo {
		/*
		  struct Chunk hold sinformation about the Structure of a
		  Chunk.  Using this Inforamtion Chunkdata can be retrieved
		  from a heap Pointer to the start of a data section.

		  Chunk Data is aligned to a power of two, which allows to
		  obtain the beginning of a Chunks Datasection by masking the
		  lowest bits.
  
		  A chunks Datasection has folloing Structure:
  
		  | Type_t type        | <-+
			+--------------------+   |
			| Element[N] data    |   +--[2^(sizeExp) Bytes]
			  +--------------------+   |
			  | Bitset<N> marked   | <-+
		 */

        
        const size_t typeSize;
        const size_t elementCount;


        static constexpr auto calcCount(size_t typeSize) -> size_t {
			if(typeSize == 0) return 0;
			// how many Elements can fit inside the Chunk without Metadata
			size_t maxCount = Chunk::size / typeSize;

			size_t n = maxCount + 8 * (sizeof(Type_t) -
								  Chunk::size +
								  maxCount * typeSize),
			d = (8 * typeSize + 1);
				
			return maxCount - (n+d -1)/d;
		}
		constexpr ChunkInfo(size_t typeSize)
		: typeSize(typeSize), elementCount(calcCount(typeSize)) {}


    };

    class GC {
	public :
		static auto getInstance() -> GC & {
			assert(instance != nullptr && "Garbage Collector has not been initilized!");
			return *instance;
		}
		auto getType(Heap_p cell) const -> Type_t {
			if (cell >= &chunkInfo.front() && cell < &chunkInfo.back())
				return reinterpret_cast<ChunkInfo*>(cell)-chunkInfo.data();
			return getChunk(cell).getType();
		}
		auto getChunk(Heap_p cell) const -> Chunk& {
			assert(cell >= heap && cell < heap + chunkCount && "Cannot get Chunk of non Heap Object!");
			return *reinterpret_cast<Chunk*>(reinterpret_cast<intptr_t>(cell) & ~(Chunk::size - 1));
		}

        auto alloc(Type_t type) -> Heap_p {
            if(chunkInfo[type].typeSize == 0) return &chunkInfo[type];
			assert(freeCells[type].size() != 0 &&
				   "Heap is out of Memory! (Maybe pause exceution "
				   "instead?)");

			void *cell = freeCells[type].read();
			markShallow(cell);
			return cell;
		}
		template <Type_t id, class... Args> auto make(Args&&... args) -> TypeInfo::Type<id>& {
			return *std::construct_at(reinterpret_cast<TypeInfo::Type<id>*>(alloc(id)), std::forward<Args>(args)...);
		}

        void mark(Heap_p cell) {
			markShallow(cell);
			TypeInfo::vtable<Mark>[getType(cell)]({cell});
		}

		static auto init(size_t stack_size, size_t maxChunks) -> bool {
			delete instance;
			instance = new GC{stack_size, maxChunks};
			instance->collect();
			return true;
		}

		size_t threshold = 100;
        Stack stack;

        auto collectStats() -> long long {
			size_t before = 0;
			for (Ring<Heap_p> &r : freeCells) 
				before += r.size();
			collect();
			size_t after = 0;
			for (Ring<Heap_p> &r : freeCells) 
				after += r.size();
			return after - before;
		}

		

//	private:
		GC(size_t stack_size, size_t maxChunks = 1024)
		: stack(stack_size),
		  freeCells(TypeInfo::count),
		  heap(reinterpret_cast<Chunk*>(std::aligned_alloc(Chunk::size, Chunk::size * maxChunks))),
		  maxChunks(maxChunks) {}

		void markShallow(Heap_p cell) {
			if(cell > heap && cell < heap + chunkCount)
				getChunk(cell).mark(chunkInfo, cell);
		}

		template <size_t... ids> struct BuildInInfoTable {
            template<class T> static constexpr ChunkInfo info {sizeof(T)};
            template<> constexpr ChunkInfo info<std::monostate> {0};
			constexpr static std::array<ChunkInfo, sizeof...(ids)> Result {info<typename TypeInfo::Type<ids>> ... };
		};


		std::vector<ChunkInfo> chunkInfo {
			TypeInfo::Result<BuildInInfoTable>.begin(),
			TypeInfo::Result<BuildInInfoTable>.end()};
		
		std::vector<std::vector<Type_t>> customLayouts;
		Type_t typeCount = TypeInfo::count;
		
		inline static GC* instance {};
		bool running = true;


		std::vector<Ring<void *>> freeCells;
		Ring<Chunk *> freeChunks{32};

		Chunk* heap;
		size_t chunkCount {};
		const size_t maxChunks {};

		void run();
		void collect();


		struct Destruct {
			void *element;
			
			template <size_t id> auto operator()() {
				std::destroy_at(
								reinterpret_cast<TypeInfo::Type<id> *>(element));
				
			}
		};

		void destructBuiltInChunk(Chunk &chunk) {
			auto destruct =	TypeInfo::vtable<Destruct>[chunk.getType()];

			std::byte *start = chunk.getData();
			std::byte *marks = chunk.getMarks(chunkInfo);
			for (size_t i = 0; i < chunkInfo[chunk.getType()].elementCount; i++) {
				if ((marks[i / 8] & std::byte(1 << (i % 8))) !=
					std::byte{0})
					destruct({start + i*chunkInfo[chunk.getType()].typeSize});
			}
		}

		void destructCustom(Chunk &chunk) {
			assert(false && "Custom Types not implemented!");
		}

		~GC() {
            for (Chunk &chunk : std::span{heap, chunkCount}) {
				if (chunk.getType() < TypeInfo::count)
					destructBuiltInChunk(chunk);
				else destructCustom(chunk);
			}
			free(heap);
		}


	};
}
