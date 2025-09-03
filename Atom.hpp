#pragma once
#include "GC.hpp"
#include "Types.hpp"
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
				return **reinterpret_cast<TypeInfo::Type<id> **>(data);
			}

			assert(type == id && "Requested wrong Atom Type!");
			return *reinterpret_cast<TypeInfo::Type<id>*>(data);
        }
        
		auto getType() const -> Type_t {
			if (type == FORWARD)
				return GC::getInstance().getType(reinterpret_cast<const Forward*>(&data)->ref);
			return type;
		}
		void moveToHeap() {
			reinterpret_cast<Forward*>(&data)->ref =
				TypeInfo::vtable<Move>[type]({*this});
			type = FORWARD;
        }

        void mark() { TypeInfo::vtable<Mark>[type]({.cell = data}); }

		template <size_t id, class... Args> void construct(Args &&...args) {
			type = id;
			assert(type == id && "Huuch!");
			std::construct_at(&get<id>(),
							  std::forward<Args>(args)...);
			assert(type == id && "Overflow!");
		}

	private:
		
		template<class ...Tp>
		struct UnionSize {
			static constexpr size_t Result = std::max({sizeof(typename Tp::Type)...});
		};

		std::byte data[TypeInfo::Result<UnionSize>];
		Type_t type;

		// Move Atom from Stack to Heap
		struct Move {
			Atom &self;
			template <size_t id> auto operator()() -> Heap_p {
				auto &value = self.get<id>();
				return &GC::getInstance().make<id>(std::move(value));
			}
        };
	};
};
