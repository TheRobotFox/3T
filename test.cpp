#include "GC.hpp"
#include "Types.hpp"
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <iostream>
#include <optional>
#include <print>
#include <thread>
#include <vector>
#include <cassert>

#define DEBUG

#ifndef DEBUG
#define FAIL(REASON)                                                           \
  return std::optional {                                                       \
    Failure { REASON, __LINE__, __FILE__ }                                     \
}
#else
#define FAIL(REASON)                                                           \
  do {                                                                         \
    std::cerr << REASON << '\n';                                                       \
    assert(false);                                                             \
	} while (false)
#endif
#define ASSERT(COND, REASON)												   \
  if (!(COND)) {															   \
FAIL(std::string("Assertion Failed: ") + std::string(#COND) +			   \
std::string(" -- ") + std::string(REASON));							  \
}

#define EXPECT(GOT, SHOULD, REASON)											   \
  do {																		   \
auto a = SHOULD;														   \
auto b = GOT;															   \
if (a != b) {															   \
FAIL(std::format("Expected {} got {}!\n{}", a, b, REASON));				 \
}																		   \
} while (false)


#define SUCCESS return std::optional<Failure>{std::nullopt};

struct Failure {
	std::string error;
	size_t line;
	const char *file;
};
class Test {
	friend void runTests(std::vector<Test> &&tests);
	std::string name;
	std::function<std::optional<Failure>()> run;

public:
	Test(std::string &&name, std::function<std::optional<Failure>()> &&fn)
	: name(name), run(fn)
	{}
};

void runTests(std::vector<Test> &&tests) {
	int failing = 0;
	for (size_t t = 0; t<tests.size(); t++) {
		std::print("[{}/{}] {}", t+1, tests.size(), tests[t].name);
		std::cout.flush();
		if (auto f = tests[t].run()) {
			std::print(" at {}:{} has Failed!\nReason:\n{}\n", f->file, f->line, f->error);
			failing++;
		} else std::print(" Succeeded!\n");
	}
	if(failing>0) std::print("{}/{} Tests have failed!", failing, tests.size());
}

using namespace TTT;

auto make_tree(uint16_t depth) -> Heap_p {
	if (depth > 1) {

		Cons &cons = GC::getInstance().make<CONS>();
		cons.car = make_tree(depth-1);
		cons.cdr = make_tree(depth - 1);
		return &cons;
	}
	return &GC::getInstance().make<INT>(420);
   
}

constexpr auto tree_heap_cells(uint16_t depth) -> size_t {
	return (1<<depth)-1;
}

auto main(void) -> int {

	runTests({
		Test("RingBuffer",
			 [] {
				 TTT::Ring<int> r1(5);

				 r1.push(69);
				 EXPECT(r1.size(), 1ULL,
						"Size of queue after Object has been pushed!");
				 EXPECT(r1.read(), 69,
						"Read the Number that was pushed to the Queue!");
				 EXPECT(r1.size(), 0ULL,
						"Queue empty after all objects have benn read!");


				 for (int i = 0; i < 100; i++) {
					 r1.push(100 * i);
				 }

				 EXPECT(r1.size(), 100ULL,
						std::format("Write from same Thread! Got {} instead.",
									r1.size()));
				 r1.push(70);
				 EXPECT(r1.size(), 101ULL, "Write from seperate Thread!");

				 std::vector<int> v;
				 v.reserve(100);

				 for (auto e : r1) {
					 v.push_back(e);
				 }
				 for (int i = 0; i < 100; i++)
					 ASSERT(std::ranges::contains(v, 100 * i),
							std::format("Read back correctly! (full) Got {} expected {}",
										v, 100 * i));
				 ASSERT(std::ranges::contains(v, 70),
						"Read back correctly!");

				 for(int i=0; i<50; i++)
                     r1.read();
                 
				 v.clear();
				 for (auto e : r1) {
					 v.push_back(e);
				 }

				 std::vector<int> missing;
				 for (int i = 0; i < 100; i++)
					 if(!std::ranges::contains(v, 100 * i))
						 missing.push_back(100*i);
				 if (!std::ranges::contains(v, 70))
					 missing.push_back(70);

				 ASSERT(
						missing.size() == 50,
						std::format(
									"Read back correctly! (full) Got {}",
									missing)) ;
				 r1.clear();
				 v.clear();

				 // Test Concurrency
				 std::thread t([&r1] {
					 for (int i = 0; i < 100; i++) {
						 r1.push(100 * i);
					 }
				 });

				 t.detach();

				 while (v.size() != 100) {
					 if (r1.size()>0)
						 v.push_back(r1.read());
				 }

				 for (int i = 0; i < 100; i++)
					 ASSERT(std::ranges::contains(v, 100 * i),
							std::format("Read back correctly! Got {} expected {}",
										v, 100 * i));
				 SUCCESS;
			 }),
		Test("GC::full_deallocation::spread",
			 []() {
				 // Allocate shallow Trees
				 GC::init(100, 1024);
				 auto &stack = GC::getInstance().stack;
				 stack.beginFrame();
				 for (int i = 0; i < 50; i++) {

					 auto &arg = stack.push(make_tree(1));

					 // delete references to half of them
					 if (i % 2 == 0)
						 arg = GC::getInstance().alloc(NIL);
				 }
				 EXPECT(GC::getInstance().collectStats(),
						(long long)(25 * tree_heap_cells(1)),
						"Should free all unreachable Cells!");

				 stack.dropFrame();
				 EXPECT(GC::getInstance().collectStats(),
						(long long)(25 * tree_heap_cells(1)),
						"Should free all unreachable Cells!");

				 SUCCESS;
			 }),
		Test("GC::full_deallocation::deep", []() {
			GC::init(100, 1024);
			auto &stack = GC::getInstance().stack;
			stack.beginFrame();
			// Allocate deep Trees
			for (int i = 0; i < 8; i++) {
				auto &arg = stack.push(make_tree(4));

				// delete references to half of them
				if (i % 2 == 0)
					arg = GC::getInstance().alloc(NIL);
			}

			EXPECT(GC::getInstance().collectStats(),
				   (long long)(4 * tree_heap_cells(4)),std::format("Should free all unreachable Cells! Dups: {}", GC::instance->freeCells[0].duplicates()));

			stack.dropFrame();
			EXPECT(GC::getInstance().collectStats(),
				   (long long)(4 * tree_heap_cells(4)),
				   std::format("Should free all unreachable Cells! Dups: {}", GC::instance->freeCells[0].duplicates()));

			SUCCESS;
		})
	});
	
}
