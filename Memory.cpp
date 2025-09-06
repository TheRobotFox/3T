#include "GC.hpp"
#include "Types.hpp"

namespace TTT {

	struct Destruct {
		Chunk &chunk;
		template <size_t id> void operator()() {
			if constexpr (!std::is_trivially_destructible_v<TypeInfo::Type<id>>) {
				using T = TypeInfo::Type<id>;

				T* data = reinterpret_cast<T*>(chunk.getData());
			
				for (size_t i = 0; i < GC::getInstance().getInfo(id).elementCount; i++)
					if(chunk.isInitized(i))
						std::destroy_at(data+i);
			}
		}
	};

	Chunk::~Chunk() {
		if(getType()<TypeInfo::count)
			TypeInfo::vtable<Destruct>[getType()]({*this});
		else assert(false && "Destructing Custom Obj not implemented!");
	}
	
} // namespace TTT
