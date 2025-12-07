#include "lir_generator.hpp"

LIRGenerator::LIRGenerator(std::vector<HIR>& hir_statements,
                           std::map<std::string_view, Var, std::less<>>& symbols)
    : hir_statements_(hir_statements), symbols_(symbols)
{
}

LIRGenerator::~LIRGenerator() {}

std::vector<LIRInstruction> LIRGenerator::generate()
{
    for(auto const&  [name, var] : symbols_) {
        // TODO: better offset alignment
        current_offset_ -= var.type->bytes;
        variable_offsets_[name] = current_offset_;
    }

    total_offset_ = ((-current_offset_  +15) / 16) * 16;


    int index = 0;
    for(auto& hir_stmt : hir_statements_)
    {
        std::visit(
            Overload{
                [&](HIRAssign& assign) -> void {
                    lir_statements_.emplace_back(OPCODE::MOV,
                                                assign.reg,
                                                lower_factor(assign.lhs),
                                                std::monostate{});
                },
                [&](HIRUnaryOp& unary_op) -> void {
                    switch(unary_op.op)
                    {
                    case Operator::NOT:
                        {
                            lir_statements_.emplace_back(OPCODE::NOT,
                                                        unary_op.reg,
                                                        lower_factor(unary_op.fac),
                                                        std::monostate{});
                            return;
                        }
                    default:
                        return;
                    }
                },
                [&](HIRBinaryOp& binary_op) {
                    cond_ctx_ = {binary_op.reg, binary_op.op, binary_op.rhs, binary_op.lhs, true};
                    switch(binary_op.op)
                    {
                    case Operator::ADD:
                        {
                            lir_statements_.emplace_back(OPCODE::ADD,
                                                        binary_op.reg,
                                                        lower_factor(binary_op.lhs),
                                                        lower_factor(binary_op.rhs));
                            return;
                        }
                    case Operator::SUB:
                        {
                            lir_statements_.emplace_back(OPCODE::SUB,
                                                        binary_op.reg,
                                                        lower_factor(binary_op.lhs),
                                                        lower_factor(binary_op.rhs));
                            return;
                        }
                    case Operator::MUL:
                        {
                            lir_statements_.emplace_back(OPCODE::MUL,
                                                        binary_op.reg,
                                                        lower_factor(binary_op.lhs),
                                                        lower_factor(binary_op.rhs));
                            return;
                        }
                    case Operator::DIV:
                        {
                            lir_statements_.emplace_back(OPCODE::DIV,
                                                        binary_op.reg,
                                                        lower_factor(binary_op.lhs),
                                                        lower_factor(binary_op.rhs));
                            return;
                        }
                    case Operator::MOD:
                        {
                            lir_statements_.emplace_back(OPCODE::MOD,
                                                        binary_op.reg,
                                                        lower_factor(binary_op.lhs),
                                                        lower_factor(binary_op.rhs));
                            return;
                        }
                    case Operator::LSH:
                        {
                            lir_statements_.emplace_back(OPCODE::LSH,
                                                        binary_op.reg,
                                                        lower_factor(binary_op.lhs),
                                                        lower_factor(binary_op.rhs));
                            return;
                        }
                    case Operator::RSH:
                        {
                            lir_statements_.emplace_back(OPCODE::RSH,
                                                        binary_op.reg,
                                                        lower_factor(binary_op.lhs),
                                                        lower_factor(binary_op.rhs));
                            return;
                        }
                    case Operator::XOR:
                        {
                            lir_statements_.emplace_back(OPCODE::XOR,
                                                        binary_op.reg,
                                                        lower_factor(binary_op.lhs),
                                                        lower_factor(binary_op.rhs));
                            return;
                        }
                    case Operator::OR:
                        {
                            lir_statements_.emplace_back(OPCODE::OR,
                                                        binary_op.reg,
                                                        lower_factor(binary_op.lhs),
                                                        lower_factor(binary_op.rhs));
                            return;
                        }
                    case Operator::AND:
                        {
                            lir_statements_.emplace_back(OPCODE::AND,
                                                        binary_op.reg,
                                                        lower_factor(binary_op.lhs),
                                                        lower_factor(binary_op.rhs));
                            return;
                        }
                    case Operator::EQ:
                        {
                            lir_statements_.emplace_back(OPCODE::CMP,
                                                        std::monostate{},
                                                        lower_factor(binary_op.lhs),
                                                        lower_factor(binary_op.rhs));
                            lir_statements_.emplace_back(OPCODE::SETE,
                                                        binary_op.reg,
                                                        std::monostate{},
                                                        std::monostate{});
                            return;
                        }
                    case Operator::NEQ:
                        {
                            lir_statements_.emplace_back(OPCODE::CMP,
                                                        std::monostate{},
                                                        lower_factor(binary_op.lhs),
                                                        lower_factor(binary_op.rhs));
                            lir_statements_.emplace_back(OPCODE::SETNE,
                                                        binary_op.reg,
                                                        std::monostate{},
                                                        std::monostate{});
                            return;
                        }
                    case Operator::LESS:
                        {
                            lir_statements_.emplace_back(OPCODE::CMP,
                                                        std::monostate{},
                                                        lower_factor(binary_op.lhs),
                                                        lower_factor(binary_op.rhs));
                            lir_statements_.emplace_back(OPCODE::SETL,
                                                        binary_op.reg,
                                                        std::monostate{},
                                                        std::monostate{});
                            return;
                        }
                    case Operator::LESSEQ:
                        {
                            lir_statements_.emplace_back(OPCODE::CMP,
                                                        std::monostate{},
                                                        lower_factor(binary_op.lhs),
                                                        lower_factor(binary_op.rhs));
                            lir_statements_.emplace_back(OPCODE::SETLE,
                                                        binary_op.reg,
                                                        std::monostate{},
                                                        std::monostate{});
                            return;
                        }
                    case Operator::GREATER:
                        {
                            lir_statements_.emplace_back(OPCODE::CMP,
                                                        std::monostate{},
                                                        lower_factor(binary_op.lhs),
                                                        lower_factor(binary_op.rhs));
                            lir_statements_.emplace_back(OPCODE::SETG,
                                                        binary_op.reg,
                                                        std::monostate{},
                                                        std::monostate{});
                            return;
                        }
                    case Operator::GREATEREQ:
                        {
                            lir_statements_.emplace_back(OPCODE::CMP,
                                                        std::monostate{},
                                                        lower_factor(binary_op.lhs),
                                                        lower_factor(binary_op.rhs));
                            lir_statements_.emplace_back(OPCODE::SETGE,
                                                        binary_op.reg,
                                                        std::monostate{},
                                                        std::monostate{});
                            return;
                        }
                    case Operator::BAND:
                        {
                            lir_statements_.emplace_back(OPCODE::AND,
                                                        binary_op.reg,
                                                        lower_factor(binary_op.lhs),
                                                        lower_factor(binary_op.rhs));
                            return;
                        }
                    case Operator::BOR:
                        {
                            lir_statements_.emplace_back(OPCODE::OR,
                                                        binary_op.reg,
                                                        lower_factor(binary_op.lhs),
                                                        lower_factor(binary_op.rhs));
                            return;
                        }
                    case Operator::RETURN:
                        {
                            lir_statements_.emplace_back(OPCODE::RET,
                                                        std::monostate{},
                                                        lower_factor(binary_op.lhs),
                                                        std::monostate{});
                            return;
                        }
                    default:
                        return;
                    }
                },
                [&](HIRJump& jump) {
                    lir_statements_.emplace_back(OPCODE::JMP,
                                                jump.label,
                                                std::monostate{},
                                                std::monostate{});
                },
                [&](HIRCondJump& cond_jump) {
                    if(!cond_ctx_.valid)
                        return;
                    if(index > 0 && std::holds_alternative<HIRBinaryOp>(hir_statements_[index - 1]))
                    {
                        // if previous statement is binary, remove setcc
                        lir_statements_.pop_back();
                    }
                    OPCODE jump_op;
                    switch(cond_ctx_.op)
                    {
                    case Operator::EQ:
                        jump_op = OPCODE::JE;
                        break;
                    case Operator::NEQ:
                        jump_op = OPCODE::JNE;
                        break;
                    case Operator::LESS:
                        jump_op = OPCODE::JL;
                        break;
                    case Operator::LESSEQ:
                        jump_op = OPCODE::JLE;
                        break;
                    case Operator::GREATER:
                        jump_op = OPCODE::JG;
                        break;
                    case Operator::GREATEREQ:
                        jump_op = OPCODE::JGE;
                        break;
                    default:
                        return; // Unsupported operator for conditional jump
                    }

                    // 2. Single emission point
                    lir_statements_.emplace_back(jump_op,
                                                cond_jump.label,
                                                std::monostate{},
                                                std::monostate{});
                },
                [&](LabelID label) {
                    lir_statements_.emplace_back(OPCODE::LABEL,
                                                label,
                                                std::monostate{},
                                                std::monostate{});
                },
                [&](HIRLoad& load) {
                    lir_statements_.emplace_back(OPCODE::MOV,
                                                load.reg,
                                                StackSlot{variable_offsets_[load.source], symbols_.at(load.source).type->bytes},
                                                std::monostate{});
                },
                [&](HIRStore& store) {
                    // use rbp offsets
                    lir_statements_.emplace_back(OPCODE::MOV,
                                                StackSlot{variable_offsets_[store.dest], symbols_.at(store.dest).type->bytes},
                                                lower_factor(store.reg),
                                                std::monostate{});
                },
                [&](HIRReturn& ret) {
                    lir_statements_.emplace_back(OPCODE::RET,
                                                std::monostate{},
                                                lower_factor(ret.value),
                                                std::monostate{});
                },
            },
            hir_stmt);
        index++;
    }

    return lir_statements_;
}

Operand LIRGenerator::lower_factor(HIRExprFactor& fac)
{
    return std::visit(Overload{[&](VirtualRegisterID& vreg) -> Operand {
                                   return vreg; 
                               },
                               [&](int& imm) -> Operand {
                                   return imm;
                               },
                            [&](auto& reg) -> Operand {
                                return -1;
                            }},
                      fac);
}

int LIRGenerator::final_stack_size()
{
    return total_offset_;
}