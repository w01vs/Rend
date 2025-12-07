#ifndef HIR_GENERATOR
#define HIR_GENERATOR

#include "hir_def.hpp"
#include "mid_end/analysis/semantics.hpp"
#include <unordered_map>
#include <algorithm>

class HIRGen {
  public:
    HIRGen(program_ptr& program, std::map<std::string_view, Var, std::less<>>& symbols);

    std::vector<HIR>& generate();

  private:
    program_ptr& program_;
    LabelManager label_manager_;

    std::vector<HIR> hir_stmt_;

    VirtualRegisterID current_register_;

    void visit_hirexpr(HIRExprFactor& fac);

    using FlattenedExpr = std::variant<VirtualRegisterID, int, expression_ptr, std::string_view>;

    int sethi_ulmann(expression_ptr_var& expr);
    int sethi_ulmann(const expression_ptr& expr);

    HIRExprFactor handle_flattened_expr(std::vector<FlattenedExpr>& exprs, Operator op);

    void fold_comm_assoc(std::vector<FlattenedExpr>& operands, Operator op) const;

    HIRExprFactor fold_other(int lhs, int rhs, Operator op) const;

    void collect_operands(expression_ptr_var&, std::vector<FlattenedExpr>&, Operator op) const;

    HIRExprFactor unfold_expression(expression_ptr_var& expr);

    void generate_stmt(statements_ptr_var& node);

    void generate_scope_var(scope_err_ptr_var& node);

    void generate_else_var(else_ptr_var& node, LabelID L_END);

    void generate_inner_scope(stmt_vec_err_ptr& node);

    std::map<std::string_view, Var, std::less<>>& symbols_;

    std::shared_ptr<type::BuiltinType> find_variable_type(std::string_view name) const;
};

#endif // HIR_GENERATOR
