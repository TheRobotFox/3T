#include "GC.hpp"
#include "Types.hpp"
#include <cstddef>
#include <memory>

namespace TTT {


	/*
	 * Chunk
	 */
	
	auto Chunk::getType() const -> Type_t {
		return *reinterpret_cast<const Type_t *>(data.data());
	}
	auto Chunk::getData() -> std::byte * {
		return reinterpret_cast<std::byte *>(data.data()) + sizeof(Type_t);
	}
	auto Chunk::getMarks(std::vector<ChunkInfo> &typeInfo) -> std::byte * {
		auto &info = typeInfo[getType()];
		return reinterpret_cast<std::byte *>(data.data()) +
		  info.typeSize * info.elementCount;
	}
	void Chunk::resetMarks(std::vector<ChunkInfo> &typeInfo) {
		memset(getMarks(typeInfo), 0,
			   (typeInfo[getType()].elementCount + 7) / 8);
	}
	auto Chunk::empty(std::vector<ChunkInfo> &typeInfo) -> bool {
		const auto *start = getMarks(typeInfo);
		for (size_t i = 0; i < (typeInfo[getType()].elementCount + 7) / 8;
			 i++) {
			if (start[i] != std::byte{0})
				return false;
		}
		return true;
	}
	auto Chunk::isMarked(std::vector<ChunkInfo> &typeInfo, size_t idx)
			-> bool {
		return (getMarks(typeInfo)[idx / 8] & std::byte(1 << (idx % 8))) !=
		  std::byte{0};
	}

	void Chunk::mark(std::vector<ChunkInfo> &typeInfo, void *obj) {
		size_t idx = (reinterpret_cast<std::byte *>(obj) - getData()) /
				typeInfo[getType()].typeSize;
		getMarks(typeInfo)[idx / 8] |= std::byte(1 << (idx % 8));
	}



	
	/*
	 * GC
	 */
	
	void GC::run() {
		while (running) {
			if (!std::ranges::any_of(freeCells, [this](const auto &e) {
				return e.size() < threshold;
			})) {
				std::this_thread::sleep_for(std::chrono::microseconds(100));
				continue;
			}
			collect();
		}
	}

	void GC::collect() {

		
		/*
		 * Mark Cells
		 */

		// Reset Marks
		for (Chunk &chunk : std::span{heap, chunkCount})
			chunk.resetMarks(chunkInfo);

		// Mark stack reachable
		for (Heap_p a : stack.span())
			mark(a);


		// Also mark all known free Cells to not free them twice, also
		// avoids freeing chunks, which are in the allocation list.

		// if done after thorough marking, shallow marking is
		// sufficent
		for (Type_t t = 0; t < typeCount; t++) {
			for (Heap_p cell : freeCells[t]) 
				markShallow(cell);
		}

		/*
		 * Collect Unmarked Cells
		 */

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
