#ifndef VM_H
#define VM_H

#include "Chunk.h"
#include <vector>
#include <stack>
#include <unordered_map>
#include <string>

enum class InterpretResult {
    OK,
    COMPILE_ERROR,
    RUNTIME_ERROR
};

class VM {
public:
    VM();
    InterpretResult interpret(Chunk* chunk);

private:
    Chunk* chunk;
    uint8_t* ip; // Instruction pointer
    std::vector<Value> stack; // Stack
    std::unordered_map<std::string, Value> globals;

    void push(Value value);
    Value pop();
    Value peek(int distance);
    InterpretResult run();

    void runtimeError(const char* format, ...);

    // Helpers for operations
    void binaryOp(OpCode op);
    bool valuesEqual(Value a, Value b);
};

#endif // VM_H
