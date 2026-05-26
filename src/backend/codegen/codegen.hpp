#ifndef CODEGEN_H
#define CODEGEN_H

#include <sstream>
#include <vector>
#include "../lir/lir_def.hpp"

class CodeGenerator {
  public:
    CodeGenerator(std::vector<LIRInstruction>& lir_statements, int start_stack_size, int start_variable_stack_size);

    const std::stringstream& generate();

  private:
    std::stringstream code_;
    std::stringstream prefix_;
    std::stringstream data_;
    std::stringstream final_;
    std::vector<LIRInstruction>& lir_statements_;
    int start_stack_size_;
    int start_variable_stack_size_;

    void generate_binary_op(LIRInstruction& instr);
    void generate_unary_op(LIRInstruction& instr);
    void generate_jcc(LIRInstruction& instr);
    void generate_setcc(LIRInstruction& instr);
    void generate_cmp(LIRInstruction& instr);
    void generate_mov(LIRInstruction& instr);
    void generate_lea(LIRInstruction& instr);
    void generate_ret(LIRInstruction& instr);
    void generate_label(LIRInstruction& instr);
    
    void generate_code();

    void generate_prefix();

    void generate_data();
    void add_indent(std::stringstream& stream, int indent = 1);

    void open_stackframe();

    std::string generate_label_name(LabelID label);

    std::string convert_operand(Operand& op);
};

#endif // CODEGEN_H
