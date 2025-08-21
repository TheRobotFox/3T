#include "Memory.hpp"
#include <cstddef>
#include <mutex>
#include <thread>

namespace TTT::GC {
	Concurrent::Concurrent(Buffer &heap, Stack &stack)
	: t([this]() { watch(); }), heap(heap), stack(stack) {
		t.detach();
	}
	Concurrent::~Concurrent() {running = false;}


    void Concurrent::watch() {
		while (running) {
            if (heap.end - heap.current >= threshold) {
				std::this_thread::sleep_for(std::chrono_literals::microseconds(1000));
				continue;
            }
			collect();
		}
    }

    auto Concurrent::collect() -> size_t {

        marker.clearMarks(heap);
        marker.mark(heap, stack);

        
    }
}
