#ifndef SEMANTICS_HPP
#define SEMANTICS_HPP

#include "shared/ast_def.hpp"
#include "shared/errors.hpp"
#include "shared/operator_matrix_index.hpp"
#include "shared/visit_overload.hpp"
#include <unordered_map>

class SemanticAnalyzer {
  public:
    SemanticAnalyzer(program_ptr&& program, ErrorReporter& reporter);

    program_ptr&& analyze();

    std::shared_ptr<type::BuiltinType> _typeof_(expression_ptr_var& node) const;

    std::map<std::pair<std::string_view, int>, Var, std::less<>>& variables()
    { return variables_; }

  private:
    static type::TypeRegistry& typeregistry_;
    program_ptr program_;
    int loop_depth_ = 0;
    ErrorReporter& reporter_;
    int scope_id_ = 0;
    std::vector<int> scope_stack_;

    void analyze_stmt(statements_ptr_var& node);

    void analyze_scope_var(scope_err_ptr_var& node);

    void analyze_else_var(else_ptr_var& node);

    void analyze_scope(std::vector<statements_ptr_var>& node);

    std::map<std::pair<std::string_view, int>, Var, std::less<>> variables_;

    static const std::unordered_map<OperatorMatrixIndex, OperatorResult> OPERATOR_MATRIX;

    bool declare_variable(identifier_ptr& ident, std::shared_ptr<type::BuiltinType> type);

    std::shared_ptr<type::BuiltinType> find_variable_type(identifier_ptr& ident) const;
    void fold_constants(expression_ptr_var& node);

    std::optional<long> get_constant_value(expression_ptr_var& node);
};

#endif // SEMANTICS_HPP
