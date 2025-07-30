#include <unordered_map>
#include "Value.hpp"

namespace TTT {

class Module {
  const std::unordered_map<std::string, Value *> symbols;
  const Value *static_data;
  // Bytecode ...
};

} // namespace TTT
