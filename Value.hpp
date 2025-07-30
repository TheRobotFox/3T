#include <cstddef>
#include <cstdint>
#include "Def.hpp"
namespace TTT {
enum class Type : uint8_t { INTEGER, NUMBER, CHAR, STRING, SYMBOL, CONS, CLOSURE };

struct Value {
public:
  Type type;
  union {
      size_t symbol;
    struct {
      Value *car, *cdr;
    } cons;
    char character;
    const char *string;
    struct {
		
    } closure;
  };

  Value(){}

  template <class Fn> void apply_refs(Fn fn) {
    switch (type) {
    case Type::SYMBOL:
      fn(symbol);
      break;
    case Type::CONS:
      fn(cons.car);
      fn(cons.cdr);
      break;

    case Type::INTEGER:
    case Type::NUMBER:
    case Type::CHAR:
    case Type::STRING:
    case Type::FORM:

      // nop
    }
  }
};

} // namespace TTT
