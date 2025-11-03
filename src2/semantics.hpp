#ifndef SEMANTICS_HPP
#define SEMANTICS_HPP

#include "ast_def.hpp"
#include "operator_matrix_index.hpp"
#include "errors.hpp"
#include <unordered_map>
using semantics::OperatorMatrixIndex;


template<class... Ts> struct Overload : Ts... { using Ts::operator()...; };
template<class... Ts> Overload(Ts...) -> Overload<Ts...>;

class SemanticAnalyzer {
  public:
    SemanticAnalyzer(program_ptr&& program, ErrorReporter& reporter);

    program_ptr&& analyze();

    std::shared_ptr<type::BuiltinType>
    _typeof_(expression_ptr_var& node) const;

    struct OperatorResult {
        std::shared_ptr<type::BuiltinType> result;
        // coerce rules here, e.g. type promotion (int -> float) or division type (integer div)
    };

  private:
    static type::TypeRegistry& typeregistry_;
    program_ptr program_;
    int loop_depth_ = 0;
    ErrorReporter& reporter_;
    
    struct Var {
        std::string_view name;
        std::shared_ptr<type::BuiltinType> type;
    };

    void analyze_stmt(statements_ptr_var& node);

    void analyze_scope_var(scope_err_ptr_var& node);

    void analyze_else_var(else_ptr_var& node);

    void analyze_scope(std::vector<statements_ptr_var>& node);
    
    std::map<std::string_view, Var, std::less<>> variables;

    static const std::unordered_map<OperatorMatrixIndex, OperatorResult> OPERATOR_MATRIX;

    bool declare_variable(std::string_view name, std::shared_ptr<type::BuiltinType> type);

    std::shared_ptr<type::BuiltinType> find_variable_type(std::string_view name) const;

};

#endif // SEMANTICS_HPP
