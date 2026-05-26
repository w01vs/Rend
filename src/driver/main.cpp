#define DEBUG true

#include "frontend/lexer/lexer.hpp"
#include "frontend/parser/parser.hpp"
#include "mid_end/analysis/semantics.hpp"
#include "frontend/parser/tokenstream.hpp"
#include "mid_end/hir/hir_generator.hpp"
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include "backend/lir/lir_generator.hpp"
#include "backend/codegen/codegen.hpp"

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

    TokenStream tstream{tokens};
    ErrorReporter reporter{};

    Parser parser(tstream, reporter);
    program_ptr program = parser.parse();

    SemanticAnalyzer semantic{std::move(program), reporter};
    program = semantic.analyze();
    if(reporter.has_errors())
    {
        errors_found = true;
        reporter.print_diagnostics();
        return EXIT_FAILURE;
    }

    HIRGen hirgen(program, semantic.variables());
    std::vector<HIR>& hir_stmts = hirgen.generate();

    std::ofstream hir_out("hir_output.txt", std::ios::out);
    for(auto& stmt : hir_stmts)
    {
        std::string hir_str = hir_print(stmt).data();
        hir_out << hir_str << std::endl;
    }
    hir_out.close();


    LIRGenerator lirgen{hir_stmts, semantic.variables()};
    auto lir_stmts = lirgen.generate();

    std::ofstream lir_out("lir_output.txt", std::ios::out);
    for(auto& stmt : lir_stmts)
    {
        lir_out << to_string(stmt) << std::endl;
    }

    // following code is for generating assembly
    CodeGenerator codegen{lir_stmts, lirgen.final_stack_size(), lirgen.variable_stack_size()};
    const std::stringstream& assembly = codegen.generate();
    std::ofstream asm_out("rend.s", std::ios::out);
    asm_out << assembly.str();
    asm_out.close();


    // Following code is for assembling and linking the generated assembly
    // but HIR generation is the current focus. LIR/assembly generation is pending.
    int gcc_obj_exitcode = system("gcc -c rend.s -o rend.o");
    std::cout << "gcc exited assembling with code " << gcc_obj_exitcode << std::endl;
    int gcc_link_exitcode = system("gcc -g rend.o -o rendout -lc");
    std::cout << "gcc exited linking with code " << gcc_link_exitcode << std::endl;

    if(gcc_link_exitcode != 0 || gcc_obj_exitcode != 0){
        std::cerr << "Failed to compile or link" << std::endl;
        return EXIT_FAILURE;
    }
    std::cout << "Successfully compiled and linked" << std::endl;
    return EXIT_SUCCESS;
}
