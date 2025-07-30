#include <iostream>
#include "Parser.hpp"


auto main(int argc, const char **argv) -> int{
  TTT::Parser p;
  p.parse(argv[1]);
  
}
