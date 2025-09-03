#include <atomic>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <sys/types.h>
#include <vector>
#include <thread>
#include <cassert>
#include <algorithm>
#include <ranges>

namespace TTT {
	
	template<class T>
	class Ring {
	public:
        struct iterator {

            T *start, *end, *it;
            bool wrapped = false;
			auto operator++() -> iterator & {
                it++;
                if (it == end) {
					it = start;
					wrapped = true;
				}
				return *this;
			}
            auto operator++(int) -> iterator {
                iterator prev = *this;
				it++;
                if (it == end) {
					it = start;
					wrapped = true;
				}
				return prev;
			}
			auto operator==(const auto &other) const -> bool {
				return it == other.it && (wrapped || !other.wrapped);
			}
			auto operator*() const -> T & {
				return *it;
			}
		};
		
		Ring(size_t initial = 100) : start(reinterpret_cast<T*>(malloc(sizeof(T)*initial))), capacity(initial), length(initial) {
		}

		auto size() const -> size_t {
			return available;
		}

		auto push(T val) -> bool { // TODO fixed Size and warn about exhaustion
			if (available.load(std::memory_order_relaxed) == length) {
				if(capacity == length){
					mtx.lock();

					capacity *= 2;
					start = reinterpret_cast<T*>(realloc(start, sizeof(T)*capacity));
					
					mtx.unlock();
				}
				start[length] = val;
				length++;
				write_idx = 0;
				read_idx = 0;

			} else {
				start[write_idx] = std::move(val);
                write_idx++;
				if (write_idx == length)
					write_idx = 0;
			}
			available.fetch_add(1, std::memory_order_relaxed);
			return true;
		}

		auto read() -> T {
			mtx.lock();
			assert(available!=0 && "Out of Heap memory!");

            T result = start[read_idx++];
			if (read_idx == length)
				read_idx = 0;
            
			available.fetch_sub(1, std::memory_order_relaxed);

			mtx.unlock();
			return result;
		}
		~Ring() { free(start); }

		void clear() {
			read_idx = write_idx = 0;
			available = 0;
		}

		auto begin() -> iterator {
			return {.start = start,
			   .end = start + length,
			   .it = start + read_idx};
		}
        auto end() -> iterator {
          return {.start = nullptr,
                  .end = nullptr,
                  .it = start + write_idx,
                  .wrapped = available.load(std::memory_order_relaxed) == length};
			
		}

		auto duplicates() -> std::vector<T> {
            std::vector<T> dups, found;
            for (auto e : *this) {
              if (std::ranges::contains(found, e))
                  dups.push_back(e);
			  found.push_back(e);
            }
			return dups;
		}
		
	private:
		std::mutex mtx;
		T* start;
		size_t capacity;
		std::atomic<size_t> available{ 0 };

		size_t read_idx{0}, write_idx{0}, length{0};
	};
}

