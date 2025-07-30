#include "Environment.hpp"
#include <cstdint>
#include <sys/types.h>

namespace TTT {

  class Loader {

    enum class Token: uint8_t {
      PO, PC, NUMBER, INTEGER, STRING, CHAR, SYMBOL, QUOTE, END, DOT
    };
    
    auto next_token() -> Loader::Token;
    bool error = false;
    const char *current, *end;
    
    Environment &env;

    auto parse() -> Reference;
    auto parse_cons() -> Reference;
    auto parse_symbol() -> Reference;
    auto parse_int() -> Reference;
    auto parse_number() -> Reference;
    auto parse_string() -> Reference;
    auto parse_char() -> Reference;

  public:
    Loader(Environment &env)
	: env(env)
    {}
    
    auto load(const char *text) -> ValRef;
  };
}
