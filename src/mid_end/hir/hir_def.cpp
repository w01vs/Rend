#include "hir_def.hpp"

std::string operator_print(Operator op)
{
    switch(op)
    {
    case Operator::ADD:
        return "+";
    case Operator::SUB:
        return "-";
    case Operator::MUL:
        return "*";
    case Operator::DIV:
        return "/";
    case Operator::MOD:
        return "%";
    case Operator::AND:
        return "&&";
    case Operator::OR:
        return "||";
    case Operator::BAND:
        return "&";
    case Operator::BOR:
        return "|";
    case Operator::XOR:
        return "^";
    case Operator::LSH:
        return "<<";
    case Operator::RSH:
        return ">>";
    case Operator::LESS:
        return "<";
    case Operator::GREATER:
        return ">";
    case Operator::LESSEQ:
        return "<=";
    case Operator::GREATEREQ:
        return ">=";
    case Operator::EQ:
        return "==";
    case Operator::NEQ:
        return "!=";
    case Operator::NOT:
        return "!";
    default:
        return "UNDEFINED";
    }
}

std::string hir_expr_print(HIRExprFactor& expr)
{
    return std::visit(Overload{[](VirtualRegisterID& vreg) -> std::string {
                                   return "T" + std::to_string(vreg.value);
                               },
                               [](int& i) -> std::string {
                                    return std::to_string(i);
                               },
                               [](auto&&) -> std::string {
                                   return "UNKNOWN EXPR";
                               }},
                      expr);
}

std::string hir_print(HIR& hir)
{
    return std::visit(Overload{[](HIRAssign& assign) -> std::string {
                                   std::string s = hir_expr_print(assign.lhs);
                                   return "T" + std::to_string(assign.reg.value) + " = " +
                                          hir_expr_print(assign.lhs);
                               },
                               [](HIRUnaryOp& unary) -> std::string{
                                   return "T" + std::to_string(unary.reg.value) + " = " +
                                          operator_print(unary.op) + " " +
                                          hir_expr_print(unary.fac);
                               },
                               [](HIRBinaryOp& binary) -> std::string {
                                   return "T" + std::to_string(binary.reg.value) + " = " +
                                          hir_expr_print(binary.lhs) + " " +
                                          operator_print(binary.op) + " " +
                                          hir_expr_print(binary.rhs);
                               },
                               [](HIRJump& jump) -> std::string {
                                   return "GOTO L" + std::to_string(jump.label.value);
                               },
                               [](HIRCondJump& condjump) -> std::string {
                                   return "IF_FALSE " + hir_expr_print(condjump.condition) +
                                          " GOTO L" + std::to_string(condjump.label.value);
                               },
                               [](LabelID& label) -> std::string {
                                   return "L" + std::to_string(label.value) + ":";
                               },
                               [](HIRLoad& load) -> std::string {
                                   return "T" + std::to_string(load.reg.value) + " = LD " +
                                          std::string(load.source);
                               },
                               [](HIRStore& store) -> std::string {
                                   if(std::holds_alternative<int>(store.reg))
                                   {
                                       int val = std::get<int>(store.reg);
                                       return "ST " + std::string(store.dest) + ", " +
                                              std::to_string(val);
                                   }
                                   else if(std::holds_alternative<VirtualRegisterID>(store.reg))
                                   {
                                       VirtualRegisterID vreg =
                                           std::get<VirtualRegisterID>(store.reg);
                                       return "ST " + std::string(store.dest) + ", " +
                                              "T" + std::to_string(vreg.value);
                                   }

                                   return "UNKNOWN HIR";
                               },
                               [](HIRReturn& ret) -> std::string {
                                   return "RETURN " + hir_expr_print(ret.value);
                               },
                               [](auto&&) -> std::string {
                                   return "UNKNOWN HIR";
                               }},
                      hir);
}
