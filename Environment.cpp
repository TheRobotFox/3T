#include "Environment.hpp"
#include <algorithm>
#include <execution>
#include <memory>
#include <utility>
#include <vector>

namespace TTT {

auto Environment::alloc() -> Reference {

  if (memory.available() == 0U) {

    // mark symbols
    memory.clear_marks();
    std::for_each(std::execution::par, sym_stack.begin(), sym_stack.end(),
                  [this](const std::pair<std::string, Reference> &pair) {
                    memory.mark(pair.second);
                  });
    // cleanup registered
    std::vector<std::shared_ptr<Reference>> registered;

    this->registered.erase(
        std::ranges::remove_if(this->registered,
                               [&registered](std::weak_ptr<Reference> &wptr) {
                                 if (auto sp = wptr.lock()) {
                                   registered.push_back(std::move(sp));
                                   return false;
                                 }
                                 return true;
                               })
            .begin(),
        this->registered.end());

    // mark registered
    std::for_each(std::execution::par, registered.begin(), registered.end(),
                  [this](std::shared_ptr<Reference> &p) { memory.mark(*p); });

    auto correction = memory.make_space();

    // sanitize
    std::for_each(std::execution::par, sym_stack.begin(), sym_stack.end(),
                  [&correction](std::pair<std::string, Reference> &pair) {
                    Memory::sanitize(pair.second, correction);
                  });
    std::for_each(std::execution::par, registered.begin(), registered.end(),
                  [&correction](std::shared_ptr<Reference> &p) {
                      Memory::sanitize(*p, correction);
                  });
  }

  return memory.alloc();
}
} // namespace TTT
