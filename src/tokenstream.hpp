#ifndef TOKENSTREAM_HPP
#define TOKENSTREAM_HPP

#pragma once
#include "tokens.hpp"
#include <optional>
#include <vector>

class TokenStream {
  public:
    TokenStream(std::vector<Token>& tokens) : tokens_(tokens) {}

    std::optional<Token> peek(size_t offset = 0) const; 

    std::optional<Token> consume();

    std::optional<Token> expect(const TokenType& type);

  private:
    std::vector<Token>& tokens_;
    size_t index_;
};

#endif // TOKENSTREAM_HPP
