#include "lexer.hpp"

const std::unordered_map<std::string_view, TokenType> Lexer::KEYWORDS = {
    {"if", TokenType::KW_IF},
    {"else", TokenType::KW_ELSE},
    {"while", TokenType::KW_WHILE},
    {"for", TokenType::KW_FOR},
    {"return", TokenType::KW_RETURN},
    {"break", TokenType::KW_BREAK},
    {"continue", TokenType::KW_CONTINUE},
    {"int", TokenType::TYPE_INT},
    {"bool", TokenType::TYPE_BOOL},
    {"true", TokenType::BOOL_LITERAL},
    {"false", TokenType::BOOL_LITERAL},
};

Lexer::Lexer(std::string_view view) : view_(view), index_(0), line_(0), column_(0), errors_(0) {}

Token Lexer::next_token()
{
    skip_insignificant();
    if(is_eof())
    {
        return {TokenType::EOF_, "", line_, column_};
    }

    char c = view_.at(index_);

    switch(c)
    {
    case 'a' ... 'z':
        [[fallthrough]];
    case 'A' ... 'Z':
        return identifier_or_keyword();
    case '0' ... '9':
        return number_literal();
    case '+':
        advance();
        return {TokenType::OP_ADD, "+",{line_, column_ - 1}};
    case '-':
        advance();
        return {TokenType::OP_SUB, "-", {line_, column_ - 1}};
    case '*':
        advance();
        return {TokenType::OP_MUL, "*", {line_, column_ - 1}};
    case '/':
        advance();
        return {TokenType::OP_DIV, "/", {line_, column_ - 1}};
    case '%':
        advance();
        return {TokenType::OP_MOD, "%", {line_, column_ - 1}};
    case '!':
        return excl_mark();
    case '<':
        return caret_left();
    case '>':
        return caret_right();
    case '&':
        return ampersand();
    case '|':
        return pipe();
    case '^':
        advance();
        return {TokenType::OP_BITWISE_XOR, "^", {line_, column_ - 1}};
    case ';':
        advance();
        return {TokenType::DELIMITER_SEMICOLON, ";", {line_, column_ - 1}};
    case ',':
        advance();
        return {TokenType::DELIMITER_COMMA, ",", {line_, column_ - 1}};
    case '(':
        advance();
        return {TokenType::PAREN_L, "(", {line_, column_ - 1}};
    case ')':
        advance();
        return {TokenType::PAREN_R, ")", {line_, column_ - 1}};
    case '{':
        advance();
        return {TokenType::BRACE_L, "{", {line_, column_ - 1}};
    case '}':
        advance();
        return {TokenType::BRACE_R, "}", {line_, column_ - 1}};
    case '=':
        advance();
        return {TokenType::OP_ASSIGN, "=", {line_, column_ - 1}};
    default:
        std::cerr << "Unexpected character: " << c << " on line " << line_ << ", column " << column_ << std::endl;
        advance();
        errors_++;
        return {TokenType::ERROR, std::string_view(&c, 1), {line_, column_ - 1}};
    }
}

Token Lexer::identifier_or_keyword()
{
    size_t start_index = index_;
    size_t start_column = column_;
    while(!is_eof() && (std::isalnum(view_.at(index_)) || view_.at(index_) == '_')) { advance(); }
    std::string_view ident_str = view_.substr(start_index, index_ - start_index);

    auto it = KEYWORDS.find(ident_str);

    if(it != KEYWORDS.end())
    {
        return {it->second, ident_str, {line_, column_}};
    }

    return {TokenType::IDENTIFIER, ident_str, {line_, column_}};
}

Token Lexer::number_literal()
{
    size_t start_index = index_;
    size_t start_column = column_;
    while(!is_eof() && std::isdigit(view_.at(index_))) { advance(); }
    std::string_view num_str = view_.substr(start_index, index_ - start_index);
    return {TokenType::INT_LITERAL, num_str, {line_, start_column}};
}

Token Lexer::excl_mark()
{
    advance();
    if(!is_eof() && expect('='))
    {
        advance();
        return {TokenType::OP_NOT_EQUAL, "!=", {line_, column_ - 2}};
    }
    else
    {
        return {TokenType::OP_NOT, "!", {line_, column_ - 1}};
    }
}

Token Lexer::caret_left()
{
    advance();
    if(!is_eof() && expect('<'))
    {
        advance();
        return {TokenType::OP_LSH, "<<", {line_, column_ - 2}};
    }
    else if(!is_eof() && expect('='))
    {
        advance();
        return {TokenType::OP_LESS_EQUAL, "<=", {line_, column_ - 2}};
    }
    else
    {
        return {TokenType::OP_LESS, "<", {line_, column_ - 1}};
    }
}

Token Lexer::caret_right()
{
    advance();
    if(!is_eof() && expect('>'))
    {
        advance();
        return {TokenType::OP_RSH, ">>", {line_, column_ - 2}};
    }
    else if(!is_eof() && expect('='))
    {
        advance();
        return {TokenType::OP_GREATER_EQUAL, ">=", {line_, column_ - 2}};
    }
    else
    {
        return {TokenType::OP_GREATER, ">", {line_, column_ - 1}};
    }
}

Token Lexer::ampersand()
{
    advance();
    if(!is_eof() && expect('&'))
    {
        advance();
        return {TokenType::OP_LOGICAL_AND, "&&", {line_, column_ - 2}};
    }
    else
    {
        return {TokenType::OP_BITWISE_AND, "&", {line_, column_ - 1}};
    }
}

Token Lexer::pipe()
{
    advance();
    if(!is_eof() && expect('|'))
    {
        advance();
        return {TokenType::OP_LOGICAL_OR, "||", {line_, column_ - 2}};
    }
    else
    {
        return {TokenType::OP_BITWISE_OR, "|", {line_, column_ - 1}};
    }
}

bool Lexer::is_eof()
{
    return index_ == view_.length();
}

void Lexer::skip_insignificant()
{
    switch(view_.at(index_))
    {
    case '\t':
    case '\n':
    case ' ':
        advance();
        return;
    default:
        return;
    }
}

bool Lexer::expect(char expected)
{
    if(is_eof() || view_.at(index_) != expected)
    {
        return false;
    }
    return true;
}

void Lexer::advance()
{
    if(view_.at(index_) == '\n')
    {
        line_++;
        column_ = 0;
    }
    else
    {
        column_++;
    }
    index_++;
}
