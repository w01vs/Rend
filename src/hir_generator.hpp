#ifndef HIR_GENERATOR
#define HIR_GENERATOR

#include "hir_def.hpp"
#include "semantics.hpp"
#include <unordered_map>

class HIRGen {
  public:
    HIRGen(program_ptr& program, std::map<std::string_view, Var, std::less<>>& symbols);

    program_ptr&& analyze();

  private:
    program_ptr& program_;
    int loop_depth_ = 0;

    std::vector<HIR> hir_stmt_;

    LabelID current_label_;

    VirtualRegisterID current_register_;

    void visit_hirexpr(HIRExprFactor& fac);

    using FlattenedExpr = std::variant<VirtualRegisterID, int, expression_ptr, std::string_view>;

    HIRExprFactor handle_flattened_expr(std::vector<FlattenedExpr>& exprs);

    void fold_flat_constants(std::vector<FlattenedExpr>& operands, Operator op) const;

    void collect_operands(expression_ptr_var&, std::vector<FlattenedExpr>&, Operator op) const;

    HIRExprFactor unfold_expression(expression_ptr_var& expr);

    void analyze_stmt(statements_ptr_var& node);

    void analyze_scope_var(scope_err_ptr_var& node);

    void analyze_else_var(else_ptr_var& node);

    void analyze_scope(std::vector<statements_ptr_var>& node);

    std::map<std::string_view, Var, std::less<>>& symbols_;

    std::shared_ptr<type::BuiltinType> find_variable_type(std::string_view name) const;
};

#endif // HIR_GENERATOR
