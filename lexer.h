#ifndef MYTHON_LEXER_LEXER_H_
#define MYTHON_LEXER_LEXER_H_

#include <cctype>
#include <exception>
#include <iosfwd>
#include <iostream>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <variant>

class TestRunner;

namespace Parse {

namespace TokenType {
struct Number {
  int value;
};

struct Id {
  std::string value;
};

struct Char {
  char value;
};

struct String {
  std::string value;
};

struct Class {};
struct Return {};
struct If {};
struct Else {};
struct Def {};
struct Newline {};
struct Print {};
struct Indent {};
struct Dedent {};
struct Eof {};
struct And {};
struct Or {};
struct Not {};
struct Eq {};
struct NotEq {};
struct LessOrEq {};
struct GreaterOrEq {};
struct None {};
struct True {};
struct False {};
}

using TokenBase = std::variant<
    std::monostate,
    TokenType::Number,
    TokenType::Id,
    TokenType::Char,
    TokenType::String,
    TokenType::Class,
    TokenType::Return,
    TokenType::If,
    TokenType::Else,
    TokenType::Def,
    TokenType::Newline,
    TokenType::Print,
    TokenType::Indent,
    TokenType::Dedent,
    TokenType::And,
    TokenType::Or,
    TokenType::Not,
    TokenType::Eq,
    TokenType::NotEq,
    TokenType::LessOrEq,
    TokenType::GreaterOrEq,
    TokenType::None,
    TokenType::True,
    TokenType::False,
    TokenType::Eof
>;

struct Token : TokenBase {
  using TokenBase::TokenBase;

  template<typename T>
  bool Is() const {
    return std::holds_alternative<T>(*this);
  }

  template<typename T>
  const T &As() const {
    return std::get<T>(*this);
  }

  template<typename T>
  const T *TryAs() const {
    return std::get_if<T>(this);
  }
};

bool operator==(const Token &lhs, const Token &rhs);
std::ostream &operator<<(std::ostream &os, const Token &rhs);

class LexerError : public std::runtime_error {
 public:
  using std::runtime_error::runtime_error;
};

class Lexer {
 public:
  explicit Lexer(std::istream &input);

  const Token &CurrentToken() const;
  Token NextToken();

  template<typename T>
  const T &Expect() const {
    if (!CurrentToken().Is<T>()) {
      throw LexerError("Unexpected token");
    }
    return CurrentToken().As<T>();
  }

  template<typename T, typename U>
  void Expect(const U &value) const {
    if (!CurrentToken().Is<T>() || CurrentToken().As<T>().value != value) {
      throw LexerError("Unexpected token");
    }
  }

  template<typename T>
  const T &ExpectNext() {
    NextToken();
    return Expect<T>();
  }

  template<typename T, typename U>
  void ExpectNext(const U &value) {
    NextToken();
    Expect<T>(value);
  }

 private:
  Token ReadToken();

  Token ReadNewLine();

  void CountIndents();

  std::optional<Token> ReadIndentOrDedent();

  Token ReadNumber();

  Token ReadString();

  Token ReadChar();

  std::optional<Token> ReadComplexChar(char c);

  void Trim();

  int prev_indent_count = 0;
  int curr_indent_count = 0;
  std::istream &in_;
  Token current_token_;
  bool need_to_check = true;
};

void RunLexerTests(TestRunner &test_runner);

} /* namespace Parse */

#endif // MYTHON_LEXER_LEXER_H_
