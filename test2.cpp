#include "RingBuffer.hpp"
#include <chrono>
#include <print>

using namespace TTT;

int main() {

	Ring<int> r1(100);

    std::thread t([&r1] {
		for (int i = 0; i < 10; i++) {
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
			r1.push(i);
		}
    });
	t.detach();

    while (true) {
		if(r1.size()!=0) std::print("{}\n", r1.read());
    }

    int a[3];
    int * b = a;
	std::span<int> s{a, 3};
}
