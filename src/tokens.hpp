#ifndef TOKENS_HPP
#define TOKENS_HPP

#pragma once
#include <string>
enum class TokenType : char { 
    // Literals
    INT_LITERAL,     
    //FLOAT_LITERAL,   
    //STRING_LITERAL,
    BOOL_LITERAL,

    // Identifiers & Keywords
    IDENTIFIER,
    KW_IF,
    KW_ELSE,
    KW_WHILE,
    KW_FOR,
    KW_CONTINUE,
    KW_BREAK,
    KW_RETURN,
    KW_STRUCT,

    // Types
    TYPE_INT,        
    TYPE_BOOL,

    // Operators & Punctuation
    OP_ASSIGN,          // =
    OP_ADD,             // +
    OP_SUB,             // -
    OP_MUL,             // *
    OP_DIV,             // /
    OP_MOD,             // %
    OP_BITWISE_AND,     // &
    OP_BITWISE_OR,      // |
    OP_BITWISE_XOR,     // ^
    OP_LSH,             // <<
    OP_RSH,             // >>
    OP_GREATER_EQUAL,   // >=
    OP_LESS_EQUAL,      // <=
    OP_LESS,            // <
    OP_GREATER,         // >
    OP_LOGICAL_AND,     // &&
    OP_LOGICAL_OR,      // ||
    OP_EQUAL,           // ==
    OP_NOT_EQUAL,       // !=
    OP_NOT,             // !

    
    // Separators & Delimiters
    DELIMITER_SEMICOLON, // ;
    DELIMITER_COMMA,     // ,
    PAREN_L,             // (
    PAREN_R,             // )
    BRACE_L,             // {
    BRACE_R,             // }
    
    // Sentinel & Special
    IGNORE,              // For comments or whitespace if the lexer passes them through
    EOF_,                // End of File
    ERROR                // For tokens that could not be recognized
};

struct SourceLocation {
    size_t line;
    size_t column;
    bool valid = true;
};

struct Token {
    TokenType type;
    std::string_view value;
    SourceLocation loc;

    bool is(TokenType t) const { return type == t; }
    bool is_error() const { return type == TokenType::ERROR; }
};
#endif // TOKENS_HPP