#ifndef LIR_DEF_HPP
#define LIR_DEF_HPP

#include "../../mid_end/hir/hir_def.hpp"
#include <array>
#include <functional>
#include <string>
#include <unordered_map>
#include <utility>

enum class REGISTER : short {
    // COMMENTS ARE LINUX SPECIFIC
    RAX, // calleR saved | result register; used in idiv/imul
    RBX, // callee saved | misc
    RCX, // calleR saved | ARG 4
    RDX, // calleR saved | ARG 3, used in idiv/imul
    RSP, // calleR saved | stack pointer
    RBP, // callee saved | frame pointer
    RSI, // calleR saved | ARG 2
    RDI, // calleR saved | ARG 1
    R8,  // calleR saved | ARG 5
    R9,  // calleR saved | ARG 6
    R10, // calleR saved | misc
    R11, // calleR saved | misc
    R12, // callee saved | misc
    R13, // callee saved | misc
    R14, // callee saved | misc
    R15, // callee saved | misc
    STACK,
};

enum class REGSIZE : short {
    BYTE = 1,
    WORD = 2,
    DWORD = 4,
    QWORD = 8,
};

struct Register {
    REGISTER reg;
    REGSIZE size;

    bool operator==(const Register& other) const
    { return reg == other.reg && size == other.size; }
};

namespace std
{
    template <> struct hash<Register> {
        std::size_t operator()(const Register& reg) const
        {
            return std::hash<short>{}(static_cast<short>(reg.reg)) ^
                   std::hash<short>{}(static_cast<short>(reg.size)) << 1;
        }
    };
} // namespace std

const std::unordered_map<Register, std::string> register_map = {
    {Register{REGISTER::RAX, {REGSIZE::QWORD}}, "rax"},
    {Register{REGISTER::RBX, {REGSIZE::QWORD}}, "rbx"},
    {Register{REGISTER::RCX, {REGSIZE::QWORD}}, "rcx"},
    {Register{REGISTER::RDX, {REGSIZE::QWORD}}, "rdx"},
    {Register{REGISTER::RSP, {REGSIZE::QWORD}}, "rsp"},
    {Register{REGISTER::RBP, {REGSIZE::QWORD}}, "rbp"},
    {Register{REGISTER::RSI, {REGSIZE::QWORD}}, "rsi"},
    {Register{REGISTER::RDI, {REGSIZE::QWORD}}, "rdi"},
    {Register{REGISTER::R8, {REGSIZE::QWORD}}, "r8"},
    {Register{REGISTER::R9, {REGSIZE::QWORD}}, "r9"},
    {Register{REGISTER::R10, {REGSIZE::QWORD}}, "r10"},
    {Register{REGISTER::R11, {REGSIZE::QWORD}}, "r11"},
    {Register{REGISTER::R12, {REGSIZE::QWORD}}, "r12"},
    {Register{REGISTER::R13, {REGSIZE::QWORD}}, "r13"},
    {Register{REGISTER::R14, {REGSIZE::QWORD}}, "r14"},
    {Register{REGISTER::R15, {REGSIZE::QWORD}}, "r15"},
    {Register{REGISTER::RAX, {REGSIZE::DWORD}}, "eax"},
    {Register{REGISTER::RBX, {REGSIZE::DWORD}}, "ebx"},
    {Register{REGISTER::RCX, {REGSIZE::DWORD}}, "ecx"},
    {Register{REGISTER::RDX, {REGSIZE::DWORD}}, "edx"},
    {Register{REGISTER::RSP, {REGSIZE::DWORD}}, "esp"},
    {Register{REGISTER::RBP, {REGSIZE::DWORD}}, "ebp"},
    {Register{REGISTER::RSI, {REGSIZE::DWORD}}, "esi"},
    {Register{REGISTER::RDI, {REGSIZE::DWORD}}, "edi"},
    {Register{REGISTER::R8, {REGSIZE::DWORD}}, "r8d"},
    {Register{REGISTER::R9, {REGSIZE::DWORD}}, "r9d"},
    {Register{REGISTER::R10, {REGSIZE::DWORD}}, "r10d"},
    {Register{REGISTER::R11, {REGSIZE::DWORD}}, "r11d"},
    {Register{REGISTER::R12, {REGSIZE::DWORD}}, "r12d"},
    {Register{REGISTER::R13, {REGSIZE::DWORD}}, "r13d"},
    {Register{REGISTER::R14, {REGSIZE::DWORD}}, "r14d"},
    {Register{REGISTER::R15, {REGSIZE::DWORD}}, "r15d"},
    {Register{REGISTER::RAX, {REGSIZE::WORD}}, "ax"},
    {Register{REGISTER::RBX, {REGSIZE::WORD}}, "bx"},
    {Register{REGISTER::RCX, {REGSIZE::WORD}}, "cx"},
    {Register{REGISTER::RDX, {REGSIZE::WORD}}, "dx"},
    {Register{REGISTER::RSP, {REGSIZE::WORD}}, "sp"},
    {Register{REGISTER::RBP, {REGSIZE::WORD}}, "bp"},
    {Register{REGISTER::RSI, {REGSIZE::WORD}}, "si"},
    {Register{REGISTER::RDI, {REGSIZE::WORD}}, "di"},
    {Register{REGISTER::R8, {REGSIZE::WORD}}, "r8w"},
    {Register{REGISTER::R9, {REGSIZE::WORD}}, "r9w"},
    {Register{REGISTER::R10, {REGSIZE::WORD}}, "r10w"},
    {Register{REGISTER::R11, {REGSIZE::WORD}}, "r11w"},
    {Register{REGISTER::R12, {REGSIZE::WORD}}, "r12w"},
    {Register{REGISTER::R13, {REGSIZE::WORD}}, "r13w"},
    {Register{REGISTER::R14, {REGSIZE::WORD}}, "r14w"},
    {Register{REGISTER::R15, {REGSIZE::WORD}}, "r15w"},
    {Register{REGISTER::RAX, {REGSIZE::BYTE}}, "al"},
    {Register{REGISTER::RBX, {REGSIZE::BYTE}}, "bl"},
    {Register{REGISTER::RCX, {REGSIZE::BYTE}}, "cl"},
    {Register{REGISTER::RDX, {REGSIZE::BYTE}}, "dl"},
    {Register{REGISTER::RSP, {REGSIZE::BYTE}}, "spl"},
    {Register{REGISTER::RBP, {REGSIZE::BYTE}}, "bpl"},
    {Register{REGISTER::RSI, {REGSIZE::BYTE}}, "sil"},
    {Register{REGISTER::RDI, {REGSIZE::BYTE}}, "dil"},
    {Register{REGISTER::R8, {REGSIZE::BYTE}}, "r8b"},
    {Register{REGISTER::R9, {REGSIZE::BYTE}}, "r9b"},
    {Register{REGISTER::R10, {REGSIZE::BYTE}}, "r10b"},
    {Register{REGISTER::R11, {REGSIZE::BYTE}}, "r11b"},
    {Register{REGISTER::R12, {REGSIZE::BYTE}}, "r12b"},
    {Register{REGISTER::R13, {REGSIZE::BYTE}}, "r13b"},
    {Register{REGISTER::R14, {REGSIZE::BYTE}}, "r14b"},
    {Register{REGISTER::R15, {REGSIZE::BYTE}}, "r15b"},
};

enum OPCODE {
    // Binary operations
    ADD,
    SUB,
    MUL,
    DIV,
    MOD,
    LSH,
    RSH,
    XOR,
    OR,
    AND,
    // Unary Operations
    NOT,
    NEG,
    RET,
    // Jcc
    JE,
    JNE,
    JG,
    JGE,
    JL,
    JLE,
    JZ,
    JNZ,
    JMP,
    LABEL,
    // SETcc
    SETE,
    SETNE,
    SETG,
    SETGE,
    SETL,
    SETLE,
    //
    CMP,
    MOV,
    LEA,
};

struct StackSlot {
    int offset;
    int size;
};

using Operand = std::variant<VirtualRegisterID, int, Register, LabelID, std::monostate, StackSlot>;

struct LIRInstruction {
    OPCODE opcode;
    Operand dst;
    Operand left;
    Operand right;

    LIRInstruction(OPCODE op, Operand dst, Operand left, Operand right)
        : opcode(op), dst(dst), left(left), right(right)
    {
    }
};

std::string to_string(LIRInstruction& instr);

std::string to_string(Operand& operand);

const std::string& reg_to_string(Register& reg);

#endif // LIR_DEF_HPP
