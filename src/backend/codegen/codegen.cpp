#include "codegen.hpp"

CodeGenerator::CodeGenerator(std::vector<LIRInstruction>& lir_statements, int start_stack_size)
    : lir_statements_(lir_statements), code_(), start_stack_size_(start_stack_size)
{
}

const std::stringstream& CodeGenerator::generate()
{
    for(auto& lir_stmt : lir_statements_)
    {
        switch(lir_stmt.opcode)
        {
        case OPCODE::ADD:
        case OPCODE::SUB:
        case OPCODE::MUL:
        case OPCODE::DIV:
        case OPCODE::MOD:
        case OPCODE::LSH:
        case OPCODE::RSH:
        case OPCODE::XOR:
        case OPCODE::OR:
        case OPCODE::AND:
            {
                generate_binary_op(lir_stmt);
                break;
            }
        case OPCODE::NOT:
        case OPCODE::NEG:
            {
                generate_unary_op(lir_stmt);
                break;
            }
        case OPCODE::RET:
            {
                generate_ret(lir_stmt);
                break;
            }
        case OPCODE::JE:
        case OPCODE::JNE:
        case OPCODE::JG:
        case OPCODE::JGE:
        case OPCODE::JL:
        case OPCODE::JLE:
        case OPCODE::JZ:
        case OPCODE::JNZ:
        case OPCODE::JMP:
            {
                generate_jcc(lir_stmt);
                break;
            }
        case OPCODE::LABEL:
            {
                generate_label(lir_stmt);
                break;
            }
        case OPCODE::SETE:
        case OPCODE::SETNE:
        case OPCODE::SETG:
        case OPCODE::SETGE:
        case OPCODE::SETL:
        case OPCODE::SETLE:
            {
                generate_setcc(lir_stmt);
                break;
            }
        case OPCODE::CMP:
            {
                generate_cmp(lir_stmt);
                break;
            }
        case OPCODE::MOV:
            {
                generate_mov(lir_stmt);
                break;
            }
        case OPCODE::LEA:
            {
                generate_lea(lir_stmt);
                break;
            }
        default:
            break;
        }
    }

    return code_;
}

void CodeGenerator::generate_binary_op(LIRInstruction& instr)
{
    // temporary
    code_ << "mov rax, " << convert_operand(instr.left) << "\n";
    code_ << "mov rbx, " << convert_operand(instr.right) << "\n";

    switch(instr.opcode)
    {
    case OPCODE::ADD:
        {
            code_ << "add ";
            break;
        }
    case OPCODE::SUB:
        {
            code_ << "sub ";
            break;
        }
    case OPCODE::MUL:
        {
            code_ << "mul ";
            break;
        }
    case OPCODE::DIV:
        {
            code_ << "div ";
            break;
        }
    case OPCODE::MOD:
        {
            code_ << "mod ";
            break;
        }
    case OPCODE::XOR:
        {
            code_ << "xor ";
            break;
        }
    case OPCODE::OR:
        {
            code_ << "or ";
            break;
        }
    case OPCODE::AND:
        {
            code_ << "and ";
            break;
        }
    case OPCODE::RSH:
        {
            code_ << "shr ";
            break;
        }
    case OPCODE::LSH:
        {
            code_ << "shl ";
            break;
        }
    default:
        break;
    }
    // temporary
    code_ << "rax, rbx" << "\n";
    code_ << "mov " << convert_operand(instr.left) << ", rax\n";
    // keep
    // code << convert_operand(instr.left) << ", " << convert_operand(instr.right) << "\n";
}

std::string CodeGenerator::convert_operand(Operand& op)
{
    return std::visit(Overload{
                          [this](VirtualRegisterID& reg) -> std::string {
                              int offset = reg.value * 8 + start_stack_size_;
                              return "QWORD PTR[rbp - " + std::to_string(offset) + "]";
                          },
                          [](Register& reg) -> std::string {
                              return reg_to_string(reg);
                          },
                          [](int imm) -> std::string {
                              return std::to_string(imm);
                          },
                          [](LabelID label) -> std::string {
                              return ".L" + std::to_string(label.value) + ":";
                          },
                          [](StackSlot slot) -> std::string {
                              // TODO: StackSlot can have an arbitrary size composed of subtype
                              // sizes Structs are currently disabled and thus can be ignored
                              return "QWORD PTR [rbp - " + std::to_string(-slot.offset) + "]";
                          },
                          [](std::monostate) -> std::string {
                              return "MONOSTATE";
                          },
                          [](auto&& reg) -> std::string {
                              return "UNKNOWN REGISTER";
                          },
                      },
                      op);
}

void CodeGenerator::generate_unary_op(LIRInstruction& instr)
{
    // temporary
    code_ << "mov rax, " << convert_operand(instr.left) << "\n";

    switch(instr.opcode)
    {
    case OPCODE::NOT:
        {
            code_ << "not ";
            break;
        }
    case OPCODE::NEG:
        {
            code_ << "neg ";
            break;
        }
    default:
        break;
    }

    // temporary
    code_ << "rax, rbx" << "\n";
    code_ << "mov " << convert_operand(instr.left) << ", rax\n";
    // keep
    // code_ << convert_operand(instr.left) << "\n";
}

void CodeGenerator::generate_jcc(LIRInstruction& instr)
{
    switch(instr.opcode)
    {
    case OPCODE::JE:
        {
            code_ << "je ";
            break;
        }
    case OPCODE::JNE:
        {
            code_ << "jne ";
            break;
        }
    case OPCODE::JG:
        {
            code_ << "jg ";
            break;
        }
    case OPCODE::JGE:
        {
            code_ << "jge ";
            break;
        }
    case OPCODE::JL:
        {
            code_ << "jl ";
            break;
        }
    case OPCODE::JLE:
        {
            code_ << "jle ";
            break;
        }
    case OPCODE::JZ:
        {
            code_ << "jz ";
            break;
        }
    case OPCODE::JNZ:
        {
            code_ << "jnz ";
            break;
        }
    case OPCODE::JMP:
        {
            code_ << "jmp ";
            break;
        }
    default:
        break;
    }
    LabelID label = std::get<LabelID>(instr.dst);
    code_ << generate_label_name(label) << "\n";
}

// TODO: implement register shortening
void CodeGenerator::generate_setcc(LIRInstruction& instr)
{
    code_ << "xor " << "rax" << ", " << "rax" << "\n";
    switch(instr.opcode)
    {
    case OPCODE::SETE:
        {
            code_ << "sete ";
            break;
        }
    case OPCODE::SETNE:
        {
            code_ << "setne ";
            break;
        }
    case OPCODE::SETG:
        {
            code_ << "setg ";
            break;
        }
    case OPCODE::SETGE:
        {
            code_ << "setge ";
            break;
        }
    case OPCODE::SETL:
        {
            code_ << "setl ";
            break;
        }
    case OPCODE::SETLE:
        {
            code_ << "setle ";
            break;
        }
    default:
        break;
    }
    // temporary
    code_ << "al" << "\n";
    code_ << "mov " << convert_operand(instr.left) << ", al\n";

    // later
    // requires single byte register
    // need to convert register to byte version
}

void CodeGenerator::generate_cmp(LIRInstruction& instr)
{
    // temporary
    code_ << "mov rax, " << convert_operand(instr.left) << "\n";
    code_ << "mov rbx, " << convert_operand(instr.right) << "\n";
    // keep
    code_ << "cmp ";
    // temporary
    code_ << "rax, rbx" << "\n";

    // keep
    // code_ << convert_operand(instr.left) << ", " << convert_operand(instr.right) << "\n";
}

void CodeGenerator::generate_mov(LIRInstruction& instr)
{
    if(!std::holds_alternative<Register>(instr.dst))
    {
        code_ << "mov rax, " << convert_operand(instr.left) << "\n";
        code_ << "mov " << convert_operand(instr.dst) << ", rax\n";
    }
    else
    {
        code_ << "mov ";
        code_ << convert_operand(instr.dst) << ", " << convert_operand(instr.left) << "\n";
    }
}

void CodeGenerator::generate_lea(LIRInstruction& instr)
{
    code_ << "lea ";
    code_ << convert_operand(instr.left) << ", " << convert_operand(instr.right) << "\n";
}

void CodeGenerator::generate_ret(LIRInstruction& instr)
{
    code_ << "ret ";
    code_ << convert_operand(instr.left) << "\n";
}

void CodeGenerator::generate_label(LIRInstruction& instr)
{
    LabelID label = std::get<LabelID>(instr.dst);
    code_ << "." << generate_label_name(label) << ":" << "\n";
}

std::string CodeGenerator::generate_label_name(LabelID label)
{ return "L" + std::to_string(label.value); }
