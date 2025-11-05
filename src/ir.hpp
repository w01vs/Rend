#ifndef IR_HPP
#define IR_HPP

#pragma once
#include "ast_def.hpp"
#include "type.hpp"




class IRInstruction {
    public:
        virtual ~IRInstruction() = default;
};

class BinaryOpInstruction : public IRInstruction {
    public:
        enum class Op {
            ADD,
            SUB,
            MUL,
            DIV,
            AND,
            OR,
            EQ,
            NE,
            LT,
            LE,
            GT,
            GE,
            XOR,
            AND,
            OR,
        };

        
};

#endif // IR_HPP
