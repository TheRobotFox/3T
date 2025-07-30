#include "Parser.hpp"
#include <cctype>
#include <iostream>
#include <string_view>

namespace TTT {

auto isTokenSeperator(char c) {
  return c == '\'' || c == '\"' || (std::isspace(c) != 0) || c==0 || c=='(' || c==')';
}

auto Loader::next_token() -> Loader::Token {
  using enum Loader::Token;

  current = end;
  while (std::isspace(*current) != 0)
    current++;

  if (*current == 0)
    return END;

  Token t = END;

  end = current + 1;

  switch (*current) {
  case '\'':
    return QUOTE;
  case '.':
    return DOT;
  case '(':
    return PO;
  case ')':
    return PC;
  case '\"':
    t = STRING;
    break;
  case '?':
    current++;
    end = current + 1;
    return CHAR;
    break;
  default:

    if (std::isdigit(*current) != 0) {
      t = NUMBER;
    } else {
      t = SYMBOL;
    }
  }

  end = current;
  do {
    while (!isTokenSeperator(*++end))
      ;
  } while (t == STRING && (*end != '\"' || *(end - 1) == '\\') && *end != 0);

  if (t == STRING) {
    if (*end == 0) {
      error = true;
      return END;
    }
    end++;
  }

  return t;
}
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
