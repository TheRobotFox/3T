#include <algorithm>
#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>
#include "Memory.hpp"

namespace TTT {

class ValRef;
class Environment {
  friend ValRef;

  // used for interpreter, linking and debug
  // symbol references should be constant
  std::vector<std::pair<std::string, Reference>> sym_stack;

  std::vector<std::weak_ptr<Reference>> registered;
  // simulated heap
  Memory memory;

public:
  auto alloc() -> Reference;

  auto get(Reference ref) -> Value * {return memory.get(ref);}

  auto serialize() -> std::vector<uint8_t>;
  void deserialie(std::vector<uint8_t>);
};

/*
 * Allow to Register Handle to Value to avoid GC
 */
class ValRef {
  Environment &env;
  std::shared_ptr<Reference> p_ref;

public:
  ValRef(Environment &env, Reference ref) : env(env) {
    auto it = std::ranges::find_if(
        env.registered, [&ref](std::weak_ptr<Reference> &w) {
          if (std::shared_ptr<Reference> p = w.lock())
            return *p == ref;
          return false;
        });
    if (it == env.registered.end()) {
      p_ref = std::make_shared<Reference>(ref);
      env.registered.push_back(p_ref);
    } else {
      p_ref = it->lock();
    }
  }

  auto get() -> Value & { return *env.memory.get(*p_ref); }
};

} // namespace TTT
