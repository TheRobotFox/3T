#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace TTT {
	
	using Type_t = uint8_t;
	using Bool_t = uint8_t;

	namespace util {


	
		// Calculate floor Log2 of Integer at compiletime
		constexpr auto log2i(size_t n) -> size_t {
			size_t i = 0;
			while (n > 1){
				n = n / 2;
				i++;
			}
			return i;
		}
		// Caluculate upper Bound of Bits needed to represent N states
		constexpr auto repBits(size_t n) -> size_t {
			return log2i(n) + 1;
		}


        /*
		 * Type Info Infrastructure
		 */
        
		template <size_t Id, class T> struct Assoc {
			constexpr static size_t id = Id;
            using Type = T;
		};
	
		template <class... Tp> class _TypeInfo {

			template <class T, size_t i = 0> static constexpr auto index() -> size_t {
				static_assert(i<count, "Unknown Type or Type Id is not enumeration!");
				if constexpr (std::is_same_v<T, typename Tp...[i]::Type>)
					return i;
				else return index<T, i+1>();
			}
			template <size_t id, size_t i = 0> static constexpr auto index() -> size_t {
				static_assert(i<count, "Unknown Type or Type Id is not enumeration!");
				if constexpr (id==Tp...[i]::id)
					return i;
				else return index<id, i+1>();
            }
			template <class Fn, size_t id> static auto invoke(Fn &&fn) {
				return fn.template operator()<id>();
			}
        public:
			static constexpr size_t count = sizeof...(Tp);
			static constexpr size_t union_size = std::max({sizeof(Tp) ...});
			
			// Get Associated Type from id
            template <size_t id> using Type = Tp...[index<id>()];

            // Get Associated Id from Type
            template <class T>
			static constexpr size_t id = Tp...[index<T>()] ::id;

            // generate VTables for type
            // Fn ::operator() gets instanciated via <size_t id> template
            template <class Fn>
            static constexpr decltype(((Fn *)nullptr)
                                          ->template operator()<0>()) (
                *vtable[count])(Fn &&){
              [Tp::id] = &invoke<Fn, Tp::id>...
            };

            // generate VTable for static Fn and Bake results
            // Fn ::operator() gets instanciated via <size_t id> template and called
			template <class Fn>
            static constexpr decltype(((Fn*)nullptr)->template operator()<0>()) btable[count] {[Tp::id] = Fn::template operator()<Tp::id>() ...};


        };
	}
}


