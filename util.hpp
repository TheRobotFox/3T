#include <algorithm>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <map>
#include <type_traits>
#include <variant>

namespace TTT {
	
	namespace util {

        template<std::integral T>
		constexpr auto checkTopBit(T num) -> bool {return num < (1ULL << (sizeof(T)*8 - 1));}        
        template<std::integral T>
		constexpr auto setTopBit(T num) -> T {return num | (1ULL << (sizeof(T)*8 - 1));}		   

	
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

		constexpr auto gcd(size_t a, size_t b) -> size_t {
			if (b % a == 0)
				return a;
			return gcd(b % a, a);
        }

		// Compute lowest common Mutiple using gcd 
		template <size_t a, size_t... Args> constexpr auto lcm() -> size_t {
			if constexpr (sizeof...(Args) == 0){
				return a;
			} else {
				constexpr size_t b = lcm<Args...>();
				return a*b/gcd(a,b);
			}
        }

		template <size_t n, size_t p = 0> constexpr auto align2pow() {
			if constexpr (n <= 1 << p)
				return p;
			else return align2pow<n, p+1>();
		}


		/*
		 * Type Info Infrastructure
         */

        // G++ does not currently support TypePack Index mangeling
        template <size_t idx, class T, class ...Tp> struct pack_index {
                  using Result = pack_index<idx -1,Tp...>::Result;
        };
        template <class T, class... Tp> struct pack_index<0, T, Tp...> {
			using Result = T;
        };

        
		template <size_t Id, class T> struct Assoc {
			constexpr static size_t id = Id;
            using Type = T;
		};

        template <class... Tp> class _TypeInfo {

			template <class T, size_t i = 0> static constexpr auto index() -> size_t {
				static_assert(i<sizeof...(Tp), "Unknown Type or Type Id is not enumeration!");
				if constexpr (std::is_same_v<T, typename Tp...[i]::Type>)
					return i;
				else return index<T, i+1>();
			}
			template <size_t id, size_t i = 0> static constexpr auto index() -> size_t {
				static_assert(i<sizeof...(Tp), "Unknown Type or Type Id is not enumeration!");
				if constexpr (id==Tp...[i]::id)
					return i;
				else return index<id, i+1>();
            }

			
			static constexpr auto checkTypeEnum() -> bool {
				size_t ids[]{Tp::id...};
				for (size_t i = 0; i < sizeof...(Tp); i++) {
					if(!std::ranges::contains(ids, i)) return false;
				}
                return true;
            }
			template <size_t id> static constexpr auto countEmpty() -> size_t {
				if constexpr (id == sizeof...(Tp))
					return id;
				else if constexpr (std::is_same_v<typename Tp...[index<id>()]::Type, std::monostate>)
					return countEmpty<id + 1>();
				else return id;
			}

			template <size_t id = 0>
			static constexpr auto countWhileProper() -> size_t {
				if constexpr (id == sizeof...(Tp) || std::is_same_v<typename Tp...[index<id>()]::Type, std::monostate>)
					return id;
				else
					return countWhileProper<id + 1>();
			}

			static_assert(checkTypeEnum(), "TypeInfo Ids are not continous sequence!");

			template <class Fn, size_t id> static auto invoke(Fn &&fn) {
				return fn.template operator()<id>();
			}
		public:
			static constexpr size_t count = sizeof...(Tp);
			// static constexpr size_t properCount = countWhileProper();
			// static_assert(countEmpty<properCount>() == count, "Empty Types should be listed last!");

			
				template <template<size_t...> class R>
				constexpr static auto Result = R<index<Tp::id>()...>::Result;


				// Get Associated Type from id
				template <size_t id> using Type = pack_index<index<id>(), Tp...>::Result::Type;

				// Get Associated Id from Type
				template <class T>
				static constexpr size_t id = pack_index<index<T>(), Tp...>::Result::id;

				// generate VTables for type
				// Fn ::operator() gets instanciated via <size_t id> template
				template <class Fn>
				static constexpr decltype(((Fn *)nullptr)
                                          ->template operator()<0>()) (
                *vtable[count])(Fn &&){
					&invoke<Fn, index<Tp::id>()>...
				};

			};
    } // namespace util

}


