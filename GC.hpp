#include "Types.hpp"
#include <memory>

namespace TTT {


	class GC {

        // struct TypedChunk {
        //     std::bitset

		// };

    public :
		static auto getInstance() -> GC & {
            static GC* instance{new GC()};
            return *instance;
        }
        auto getType(Heap_p e) const -> Type_t;
        auto alloc(Type_t type) -> Heap_p{return nullptr;}
        template <Type_t id, class... Args> auto make(Args&&... args) -> TypeInfo::Type<id> {
			return std::construct_at(alloc(id), std::forward<Args>(args)...);
        }
    };
}
