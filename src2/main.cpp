#include "lexer.hpp"
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

bool errors_found = false;

void lex_source(const std::string_view source, std::vector<Token>& tokens)
{
    Lexer lex(source);
    Token tok = lex.next_token();
    while(!tok.is(TokenType::EOF_))
    {
        if(tok.is_error())
        {
            errors_found = true;
            std::cerr << "Lexing error at line " << tok.loc.line << ", column " << tok.loc.column
                      << ": unexpected character '" << tok.value << "'" << std::endl;
        }
        else if(!tok.is(TokenType::IGNORE))
        {
            tokens.push_back(tok);
        }
        tok = lex.next_token();
    }
}

int main(int argc, char* argv[])
{

    std::cout << "Rend Compiler v0.1.0\n";
    std::cout << "Using C++20\n";
    if(argc != 2)
    {
        std::cerr << "Error: Requires an input file." << std::endl;
        exit(EXIT_FAILURE);
    }

    std::string filename = argv[1];
    if(!filename.ends_with(".rd"))
    {
        std::cerr << "Error: file is not of type '.rd'" << std::endl;
        exit(EXIT_FAILURE);
    }

    std::string to_compile;

    std::stringstream cstream;
    std::fstream in(argv[1], std::ios::in);
    cstream << in.rdbuf();
    to_compile = cstream.str();
    in.close();

    std::vector<Token> tokens;
    lex_source(to_compile, tokens);

    int nasm_exitcode = system("nasm -felf64 -g rend.asm");
    std::cout << "nasm exited assembling with code " << nasm_exitcode << std::endl;
    int gcc_exitcode = system("gcc -g rend.o -o rend -lc");
    std::cout << "gcc exited linking with code " << gcc_exitcode << std::endl;
    return errors_found ? EXIT_FAILURE : EXIT_SUCCESS;
}
