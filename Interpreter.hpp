#include "Environment.hpp"


namespace TTT {
  class Interpreter {
    Environment &env;
  public:
    Interpreter(Environment &env)
	: env(env)
    {}

    auto run(std::string) -> Atom*;
  };
}
