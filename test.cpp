#include "Memory.hpp"
#include <cstddef>
#include <functional>
#include <iostream>
#include <optional>
#include <print>
#include <vector>

#define FAIL(REASON)                                                           \
  return std::optional{Failure{REASON, __LINE__, __FILE__}}
#define ASSERT(COND, REASON)                                                   \
  if (!(COND)) {                                                               \
FAIL(std::string("Assertion Failed: ") +  std::string(#COND) + std::string(" -- ") + std::string(REASON));                               \
	}

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
	int counter = 1;
	for (Test &t : tests) {
        std::print("[{}/{}] {}", counter, tests.size(), t.name);
		std::cout.flush();
		if (auto f = t.run()) {
			std::print(" at {}:{} has Failed!\nReason:\n{}\n", f->file, f->line, f->error);
			failing++;
		} else std::print(" Succeeded!\n");
    }
    if(failing>0) std::print("{}/{} Tests have failed!", failing, tests.size());
}


void make_tree(GC &gc, uint16_t depth, Atom *out) {
	if (depth == 0) return;
		
	Atom *car = gc.alloc(), *cdr = gc.alloc();
    *out = Cons{car, cdr};
    make_tree(gc, depth-1, car);
    make_tree(gc, depth-1, cdr);
}

auto main() -> int {
	runTests(
			 {Test("GC::full_deallocation::spread", []() -> std::optional<Failure> {
				 GC gc({100}, {100});

				 // Allocate shallow Trees
				 for (int i = 0; i < 30; i++) {
					 Atom &head = gc.stack.push();
					 make_tree(gc, 1, &head);

					 // delete references to half of them

                     if (i % 2 == 0)
						 head = nil{};
                 }
                 gc.stack.end_frame();		  

				 ASSERT(gc.heap.available() == 40,
						std::format("Allocated {} Bytes instead of 60 Bytes", 100-gc.heap.available()));

				 ASSERT(gc.collect()==30, std::format("GC has reclaimed {} Bytes!", gc.heap.available()-40));
				 return std::nullopt;
			 }),
			  Test("GC::full_deallocation::deep", []() -> std::optional<Failure> {
				  GC gc({2 * ( (1 << 8) - 2)}, {100});
                  
				  // Allocate shallow Trees
				  for (int i = 0; i < 2; i++) {
					  Atom &head = gc.stack.push();
					  make_tree(gc, 8, &head);

					  // delete references to half of them
					  if (i % 2 == 0)
						  head = nil{};
                  }
                  gc.stack.end_frame();

				  ASSERT(gc.heap.available() == 0,
						 "Trees should exactly occupy Heap!");

				  gc.alloc();
				  ASSERT(gc.heap.available() == (1<<8)-1,
						 "First Tree Should have been reclaimed to allocate new Atom");

				  return std::nullopt;
			  })
			 });
}
