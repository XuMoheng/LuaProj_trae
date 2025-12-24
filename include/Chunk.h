#ifndef CHUNK_H
#define CHUNK_H

#include <vector>
#include <cstdint>
#include "Value.h"

enum class OpCode : uint8_t {
    OP_CONSTANT,
    OP_NIL,
    OP_TRUE,
    OP_FALSE,
    OP_POP,
    OP_GET_GLOBAL,
    OP_SET_GLOBAL,
    OP_DEFINE_GLOBAL, // For var declarations
    OP_EQUAL,
    OP_GREATER,
    OP_LESS,
    OP_ADD,
    OP_SUBTRACT,
    OP_MULTIPLY,
    OP_DIVIDE,
    OP_NOT,
    OP_NEGATE,
    OP_PRINT,
    OP_JUMP,
    OP_JUMP_IF_FALSE,
    OP_LOOP,
    OP_RETURN
};

class Chunk {
public:
    std::vector<uint8_t> code;
    std::vector<Value> constants;
    std::vector<int> lines; // Line number for each byte (for debug)

    void write(uint8_t byte, int line) {
        code.push_back(byte);
        lines.push_back(line);
    }

    void writeOp(OpCode op, int line) {
        write(static_cast<uint8_t>(op), line);
    }

    int addConstant(Value value) {
        constants.push_back(value);
        return constants.size() - 1;
    }
};

#endif // CHUNK_H
