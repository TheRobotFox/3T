#include <cstddef>
#include <memory>
#include <utility>

namespace TTT {

	/*
	 * Static Buffer
	 */
	template <class T> struct Buffer {

		T* const m_start, * const m_end;
		T *m_current;

		auto size() const -> size_t { return m_current-m_start; }
		auto capacity() const -> size_t { return m_end-m_start; }
		template<class ...Args>
		auto emplace(Args&& ...args) -> T& {
			std::construct_at(m_current,
							  std::forward<Args>(args)... );
			return *m_current++;
		}
		auto pop() -> T {
			return *m_current--;
		}
		constexpr Buffer(size_t size)
		: m_start(std::allocator<T>{}.allocate(size)), m_end(m_start + size),
		  m_current(m_start) {}
		~Buffer() {
			std::destroy(m_start, m_current);
			std::allocator<T>{}.deallocate(m_start, m_end-m_start);
		}

		auto operator[](size_t i) -> T & {
			return m_start[i];
		}

		auto begin() const -> T* {return m_start;}
		auto end() const -> T* {return m_current;}
	};
	
}
