#include "tokenstream.hpp"

std::optional<Token> TokenStream::peek(size_t offset) const
{
    if(index_ < tokens_.size())
        return {tokens_.at(index_ + offset)};
    return std::nullopt;
}

std::optional<Token> TokenStream::consume()
{
    if(index_ < tokens_.size())
        return tokens_.at(index_++);
    return std::nullopt;
}

std::optional<Token> TokenStream::expect(const TokenType& type)
{
    if(index_ < tokens_.size())
    {
        Token token = tokens_.at(index_);
        if(token.type == type)
        {
            index_++;
            return token;
        }
    }

    return std::nullopt;
}
