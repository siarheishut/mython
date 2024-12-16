#include "lexer.h"

#include <charconv>
#include <unordered_map>

using namespace std;

namespace {
using namespace Parse;

std::unordered_map<std::string, Token> str_to_token{
    {"and", TokenType::And{}}, {"or", TokenType::Or{}},
    {"not", TokenType::Not{}}, {"None", TokenType::None{}},
    {"def", TokenType::Def{}}, {"class", TokenType::Class{}},
    {"print", TokenType::Print{}}, {"return", TokenType::Return{}},
    {"if", TokenType::If{}}, {"else", TokenType::Else{}},
    {"True", TokenType::True{}}, {"False", TokenType::False{}},
    {">=", TokenType::GreaterOrEq{}}, {"<=", TokenType::LessOrEq{}},
    {"==", TokenType::Eq{}}, {"!=", TokenType::NotEq{}}
};
}

namespace Parse {

bool operator==(const Token &lhs, const Token &rhs) {
  using namespace TokenType;

  if (lhs.index() != rhs.index()) {
    return false;
  }
  if (lhs.Is<Char>()) {
    return lhs.As<Char>().value == rhs.As<Char>().value;
  } else if (lhs.Is<Number>()) {
    return lhs.As<Number>().value == rhs.As<Number>().value;
  } else if (lhs.Is<String>()) {
    return lhs.As<String>().value == rhs.As<String>().value;
  } else if (lhs.Is<Id>()) {
    return lhs.As<Id>().value == rhs.As<Id>().value;
  } else {
    return true;
  }
}

std::ostream &operator<<(std::ostream &os, const Token &rhs) {
  using namespace TokenType;

#define VALUED_OUTPUT(type) \
  if (auto p = rhs.TryAs<type>()) return os << #type << '{' << p->value << '}';

  VALUED_OUTPUT(Number);
  VALUED_OUTPUT(Id);
  VALUED_OUTPUT(String);
  VALUED_OUTPUT(Char);

#undef VALUED_OUTPUT

#define UNVALUED_OUTPUT(type) \
    if (rhs.Is<type>()) return os << #type;

  UNVALUED_OUTPUT(Class);
  UNVALUED_OUTPUT(Return);
  UNVALUED_OUTPUT(If);
  UNVALUED_OUTPUT(Else);
  UNVALUED_OUTPUT(Def);
  UNVALUED_OUTPUT(Newline);
  UNVALUED_OUTPUT(Print);
  UNVALUED_OUTPUT(Indent);
  UNVALUED_OUTPUT(Dedent);
  UNVALUED_OUTPUT(And);
  UNVALUED_OUTPUT(Or);
  UNVALUED_OUTPUT(Not);
  UNVALUED_OUTPUT(Eq);
  UNVALUED_OUTPUT(NotEq);
  UNVALUED_OUTPUT(LessOrEq);
  UNVALUED_OUTPUT(GreaterOrEq);
  UNVALUED_OUTPUT(None);
  UNVALUED_OUTPUT(True);
  UNVALUED_OUTPUT(False);
  UNVALUED_OUTPUT(Eof);

#undef UNVALUED_OUTPUT

  return os << "Unknown token :(";
}

Lexer::Lexer(std::istream &input) : in_(input) {
  while (isspace(in_.peek())) in_.ignore();
  NextToken();
}

const Token &Lexer::CurrentToken() const {
  return current_token_;
}

Token Lexer::NextToken() {
  return ReadToken();
}

Token Lexer::ReadToken() {
  if (need_to_check)
    CountIndents();
  if (auto opt = ReadIndentOrDedent()) {
    return *opt;
  }

  if ((in_.eof() || in_.peek() == char_traits<char>::eof())) {
    if (current_token_.Is<TokenType::Newline>() ||
        current_token_.Is<TokenType::Eof>() ||
        current_token_.Is<TokenType::Dedent>()) {
      return current_token_ = TokenType::Eof{};
    }
    return current_token_ = TokenType::Newline{};
  }

  char c = in_.peek();
  if (c == '\n') {
    ReadNewLine();
    return current_token_;
  }
  Trim();
  c = in_.peek();
  if (isdigit(c)) {
    ReadNumber();
  } else if (isalpha(c) || c == '_' || c == '\"' || c == '\'') {
    ReadString();
  } else {
    ReadChar();
  }
  return current_token_;
}

Token Lexer::ReadNewLine() {
  in_.ignore();
  need_to_check = true;
  return current_token_ = TokenType::Newline{};
}

void Lexer::CountIndents() {
  int count = 0;

  while (in_.peek() == ' ' && !in_.eof()) {
    in_.get();
    ++count;
  }
  if (in_.peek() == '\n') {
    in_.ignore();
    CountIndents();
  } else {
    curr_indent_count = count / 2;
    need_to_check = false;
  }
}

std::optional<Token> Lexer::ReadIndentOrDedent() {
  if (prev_indent_count > curr_indent_count) {
    --prev_indent_count;
    return current_token_ = TokenType::Dedent{};
  } else if (prev_indent_count < curr_indent_count) {
    ++prev_indent_count;
    return current_token_ = TokenType::Indent{};
  }
  return std::nullopt;
};

Token Lexer::ReadNumber() {
  std::string num;
  while (isdigit(in_.peek()) && !in_.eof()) {
    num.push_back(in_.get());
  }
  return current_token_ = TokenType::Number{std::stoi(num)};
}

Token Lexer::ReadString() {
  char c = in_.peek();
  if (c == '\'' || c == '\"') {
    in_.ignore();
    std::string str;
    while (in_.peek() != c) {
      str.push_back(in_.get());
    }
    in_.ignore();

    return current_token_ = TokenType::String{std::move(str)};
  }

  std::string str;
  while ((isalnum(in_.peek()) || in_.peek() == '_') && !in_.eof()) {
    str.push_back(in_.get());
  }
  if (auto found = str_to_token.find(str); found != str_to_token.end()) {
    current_token_ = found->second;
  } else {
    current_token_ = TokenType::Id{str};
  }

  return current_token_;
}

Token Lexer::ReadChar() {
  char c = in_.get();
  if ((c == '!' || c == '=' || c == '>' || c == '<') && !in_.eof()) {
    if (auto opt = ReadComplexChar(c)) {
      return *opt;
    }
  }
  return current_token_ = TokenType::Char{c};

}

std::optional<Token> Lexer::ReadComplexChar(char c) {
  std::string complex;
  complex.push_back(c);

  char next = in_.peek();
  if (next == '=') {
    complex.push_back(in_.get());
  }

  if (complex.size() == 1) {
    return std::nullopt;
  }
  return current_token_ = str_to_token[complex];
}

void Lexer::Trim() {
  while (isspace(in_.peek())) {
    in_.ignore();
  }
}
} /* namespace Parse */
