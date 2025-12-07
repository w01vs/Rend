
#ifndef LIR_GENERATOR_HPP
#define LIR_GENERATOR_HPP

#include "../../shared/operator_matrix_index.hpp"
#include "lir_def.hpp"

struct ConditionContext {
    VirtualRegisterID reg = VirtualRegisterID(-1);
    Operator op = Operator::UNDEFINED;
    HIRExprFactor rhs = VirtualRegisterID(-1);
    HIRExprFactor lhs = VirtualRegisterID(-1);
    bool valid = false;
    ConditionContext() = default;
    ConditionContext(VirtualRegisterID reg, Operator op, HIRExprFactor rhs, HIRExprFactor lhs,
                     bool valid)
        : reg(reg), op(op), rhs(rhs), lhs(lhs), valid(valid)
    {
    }
};

class LIRGenerator {
  public:
    LIRGenerator(std::vector<HIR>& hir_statements,
                 std::map<std::string_view, Var, std::less<>>& symbols);
    ~LIRGenerator();

    std::vector<LIRInstruction> generate();
    int final_stack_size();

  private:
    std::vector<HIR>& hir_statements_;
    std::vector<LIRInstruction> lir_statements_;
    int current_register_ = 0;
    int current_label_ = 0;
    int current_offset_ = 0;
    int total_offset_ = 0;
    ConditionContext cond_ctx_;
    std::map<std::string_view, Var, std::less<>>& symbols_;
    std::map<std::string_view, int> variable_offsets_;

    Operand lower_factor(HIRExprFactor& fac);
};

#endif // LIR_GENERATOR_HPP
