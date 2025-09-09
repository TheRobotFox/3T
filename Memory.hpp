#pragma once
#include "Types.hpp"
#include "RingBuffer.hpp"
#include "Buffer.hpp"
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <new>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

namespace TTT {

#define CHUNK_SIZE_EXP 13

	// TODO Make FreeCells more predictable -> reduce initilize Flags to fixed Overhead counter

	/*
	  [Garbage Collector]

The Heap is managed by keeping track of Chunks each of which able to
store multiple objects of a single type in a continuous Sequence.

	 */

	/*
	  ChunkInfo holds information about the Structure of a Chunk.
	  Using this Inforamtion Chunkdata can be retrieved from a
	  heap Pointer to the start of a data section.

		  Chunk Data is aligned to a power of two, which allows to
		  obtain the beginning of a Chunks Datasection by masking the
		  lowest bits.
  
		  A chunks Datasection has folloing Structure:
  
		  | Type_t type		   | <-+
			+--------------------+	 |
			| Element[N] data	 |	 +--[2^(sizeExp) Bytes]
			  +--------------------+   |
			  | Bitset<N> marked   | <-+
	 */

	class alignas(1 << CHUNK_SIZE_EXP) Chunk {
	public:

		constexpr static size_t size = 1 << CHUNK_SIZE_EXP;

		auto getCount() const -> size_t;
		auto getTypeSize() const -> size_t;
		auto getType() const -> Type_t;
		auto getData() -> std::byte *;
		auto getMarks() -> std::byte *;
		
		auto isMarked(size_t idx) -> bool;
		auto isInitized(size_t idx) -> bool;
		auto empty() -> bool;
		
		void resetMarks();
		void setMark(void *obj);
		void setInitilized(void *obj);

		constexpr Chunk(Type_t type) {
		  *reinterpret_cast<Type_t *>(data) = type;
		}

		~Chunk();

	private:
		std::byte data[size];
	};

	
	struct ChunkInfo {

		const size_t typeSize;
		const size_t elementCount;

		static constexpr auto calculateElementCount(size_t typeSize,
													size_t bitsPerElem)
			-> size_t {
			
			if (typeSize == 0)
				return 0;
			
			// how many Elements can fit inside the Chunk without Metadata

			size_t naiveCount	= Chunk::size / sizeof(T),
			fixedCost	= sizeof(Type_t),
			alignMetaData = 7*(bitsPerElem-1), // ensure bye Boundary between Marks and other
			unusedSpace	= Chunk::size - naiveCount * sizeof(T);

			size_t n = naiveCount * bitsPerElem + alignMetaData +
				  8 * (fixedCost - unusedSpace),
			d = (8 * sizeof(T) + bitsPerElem);

			// how many fewer Elements are needed to fit Metadata
			return naiveCount - (n + d -1)/d; // round up
		}

		constexpr ChunkInfo(size_t typeSize, size_t overheadPerElement)
		: typeSize(typeSize),
		  elementCount(calculateElementCount(sizeof(T),
											 overheadPerElement)) {};


		/*
			BuiltIn Types Info Table

			- BuiltIn-Types need two bits of Metadata to decide
			  whether
			  1. The Object is marked
			  2. If the Cell is active and has to be destructed

			- Types which are trivially destructable only need the
			  marking Bit

			- Empty Types (std::monostate) get special treatment to avoid allocating any Chunks
		 */
		
		template <class T> struct build {
			constexpr static auto operator()() -> ChunkInfo {
				if constexpr (std::is_same_v<T, std::monostate>)
					return {0,0};
				else if constexpr (std::is_trivially_destructible_v<T>)
					return {sizeof(T), 1};
				else
					return {sizeof(T), 2};
			}
		};
		
		template <size_t... ids> struct BuiltIn {
		  constexpr static std::array<ChunkInfo, sizeof...(ids)> Result {
		      ChunkInfo::build<TypeInfo::Type<ids>>{}()...};
		};
    };

}


// Gargabe collect <- Heap + Stack => FreeList
// Heap
// Stack
// FreeList 
// Stack <-- Allocation <- Freelist + protect Chunks + mutex
