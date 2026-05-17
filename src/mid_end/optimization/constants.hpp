#ifndef CONSTANTS_HPP
#define CONSTANTS_HPP

#include "../../shared/ast_builder.hpp"
#include "../analysis/semantics.hpp"

class ConstantOptimizer {
  public:
    ConstantOptimizer(program_ptr&& program);
    program_ptr&& optimize();

  private:
    ASTBuilder builder_;
    program_ptr program_;

    void optimize_stmt(statements_ptr_var& node);
    void optimize_return(return_ptr& _return);
    void optimize_if(if_ptr& _if);
    void optimize_while(while_ptr& _while);
    void optimize_declareassign(declareassign_ptr& _declareassign);
    void optimize_assign(assign_ptr& _assign);
    std::optional<expression_ptr_var&&> fold_constants(expression_ptr_var& node);
};

#endif // CONSTANTS_HPP
