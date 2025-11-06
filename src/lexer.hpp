#ifndef LEXER_HPP
#define LEXER_HPP

#pragma once
#include "tokens.hpp"
#include <iostream>
#include <vector>
#include <unordered_map>
class Lexer {
  public:
    Lexer(std::string_view view);
    Token next_token();

  private:
    const static std::unordered_map<std::string_view, TokenType> KEYWORDS;
    std::string_view view_;
    size_t index_;
    size_t line_;
    size_t column_;
    size_t errors_;

    Token identifier_or_keyword();
    Token number_literal();
    Token excl_mark();
    Token caret_left();
    Token caret_right();
    Token ampersand();
    Token pipe();
    bool is_eof();
    void skip_insignificant();
    bool expect(char expected);
    void advance();
};

#endif // LEXER_HPP
