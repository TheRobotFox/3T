#include "GC.hpp"
#include <algorithm>
#include <cassert>
#include <cstddef>

namespace TTT {

    // Memory Cell used on Stack
	class Atom {
    public:
		template <size_t id> auto get() -> TypeInfo::Type<id>& {
			if (type == FORWARD) {
				assert(GC::getInstance().getType(data) && "Requested wrong Atom Type!");
				return *reinterpret_cast<TypeInfo::Type<id> *>(data);
			}

			assert(type == id && "Requested wrong Atom Type!");
			return *reinterpret_cast<TypeInfo::Type<id>*>(&data);
        }
        
		auto getType() const -> Type_t {
			if (type == FORWARD)
				return GC::getInstance().getType(*reinterpret_cast<const Heap_p*>(&data));
			return type;
		}
		void moveToHeap() {
			*reinterpret_cast<Heap_p*>(&data) =
				TypeInfo::vtable<Move>[type]({*this});
			type = FORWARD;
		}

    private:
        template<class ...Tp>
		struct UnionSize {
			static constexpr size_t Result = std::max({sizeof(Tp)...});
		};

		std::byte data[TypeInfo::Result<UnionSize>];
		Type_t type;

		// Move Atom from Stack to Heap
		struct Move {
			Atom &self;
			template <size_t id> auto operator()() -> Heap_p {
				auto &value = self.get<id>();
				return nullptr;//GC::getInstance().make<id>(std::move(value));
			}
        };
	};
};
