#include "Parser.hpp"
#include <cctype>
#include <iostream>
#include <string_view>

namespace TTT {

auto Loader::parse() -> Reference {
  switch (this->next_token()) {
  case Token::PO:
    return parse_cons();
    break;
  case Token::QUOTE:
    
  }
}
auto Loader::parse_cons() -> Reference {}
auto Loader::parse_symbol() -> Reference {}
auto Loader::parse_int() -> Reference {}
auto Loader::parse_number() -> Reference {}
auto Loader::parse_string() -> Reference {}
auto Loader::parse_char() -> Reference {}

auto Loader::load(const char *text) -> ValRef {
  end = text;

  Token t;
  while ((t = this->next_token()) != Token::END) {
    std::cout << (int)t << " ";
    std::cout << std::string_view(current, end - current) << '\n';
  }
  return {};
}

} // namespace TTT
