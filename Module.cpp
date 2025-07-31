#include "Module.hpp"
#include "Expr.hpp"

using namespace TTT;


Module::Module()
: memory(1024 * 1024 * 100) // 100Mb
{
	// Initilize ReadTable

	SymbolExpr s_closing = intern("closing"),
			   s_char	 = intern("char"),
			   s_stream  = intern("stream");
		
	Sexp *closing_paren = memory.alloc();
	closing_paren->body = CharExpr{.value = ')'};


	readtable.value[CharExpr{.value = '('}] = CallableExpr{
		.name = "read-delimeter-paren",
		.doc = "Internal-function for reading parentethised delimeters.",
		.env = {.env = { {s_closing, closing_paren} }},
		.args = {s_stream, s_char},
		.value = Special{.func = read_delimeter{}()}
		
	}
}

auto isTokenSeperator(char c) {
  return c == '\'' || c == '\"' || (std::isspace(c) != 0) || c==0 || c=='(' || c==')';
}

enum class Token : uint8_t {
  PO,
  PC,
  NUMBER,
  INTEGER,
  STRING,
  CHAR,
  SYMBOL,
  QUOTE,
  END,
  DOT
};



auto next_token(std::istream stream) -> std::pair<std::string, Token {
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


auto Module::load(std::istream text) -> bool
{
	
}
