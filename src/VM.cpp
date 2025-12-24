#include "VM.h"
#include <iostream>
#include <cstdarg>
#include <cstring>

VM::VM() {
    // Reserve some stack space
    stack.reserve(256);
}

void VM::push(Value value) {
    stack.push_back(value);
}

Value VM::pop() {
    Value val = stack.back();
    stack.pop_back();
    return val;
}

Value VM::peek(int distance) {
    return stack[stack.size() - 1 - distance];
}

InterpretResult VM::interpret(Chunk* chunk) {
    this->chunk = chunk;
    this->ip = chunk->code.data();
    return run();
}

#define READ_BYTE() (*ip++)
#define READ_CONSTANT() (chunk->constants[READ_BYTE()])
#define READ_SHORT() (ip += 2, (uint16_t)((ip[-2] << 8) | ip[-1]))
#define READ_STRING() (std::get<std::string>(READ_CONSTANT()))

InterpretResult VM::run() {
    for (;;) {
        // Debug trace (optional)
        /*
        std::cout << "          ";
        for (const auto& v : stack) {
            std::cout << "[ ";
            printValue(v);
            std::cout << " ]";
        }
        std::cout << "\n";
        */

        uint8_t instruction;
        switch (instruction = READ_BYTE()) {
            case static_cast<uint8_t>(OpCode::OP_CONSTANT): {
                Value constant = READ_CONSTANT();
                push(constant);
                break;
            }
            case static_cast<uint8_t>(OpCode::OP_NIL): push(Nil{}); break;
            case static_cast<uint8_t>(OpCode::OP_TRUE): push(true); break;
            case static_cast<uint8_t>(OpCode::OP_FALSE): push(false); break;
            case static_cast<uint8_t>(OpCode::OP_POP): pop(); break;

            case static_cast<uint8_t>(OpCode::OP_GET_GLOBAL): {
                std::string name = READ_STRING();
                auto it = globals.find(name);
                if (it == globals.end()) {
                    // Lua returns nil for undefined globals, but for debugging let's warn or return nil
                    push(Nil{}); 
                    // Or runtimeError("Undefined variable '%s'.", name.c_str()); return InterpretResult::RUNTIME_ERROR;
                } else {
                    push(it->second);
                }
                break;
            }
            case static_cast<uint8_t>(OpCode::OP_DEFINE_GLOBAL): {
                std::string name = READ_STRING();
                globals[name] = peek(0);
                pop();
                break;
            }
            case static_cast<uint8_t>(OpCode::OP_SET_GLOBAL): {
                std::string name = READ_STRING();
                auto it = globals.find(name);
                if (it == globals.end()) {
                    // Implicit global declaration in Lua if assignment
                    globals[name] = peek(0);
                } else {
                    globals[name] = peek(0);
                }
                // Assignment expression evaluates to the value, so we don't pop?
                // But in statement context we might pop. 
                // Let's assume assignment expression keeps value on stack.
                break;
            }

            case static_cast<uint8_t>(OpCode::OP_EQUAL): {
                Value b = pop();
                Value a = pop();
                push(valuesEqual(a, b));
                break;
            }
            case static_cast<uint8_t>(OpCode::OP_GREATER): {
                // Simplified: assume numbers
                if (!std::holds_alternative<double>(peek(0)) || !std::holds_alternative<double>(peek(1))) {
                     runtimeError("Operands must be numbers.");
                     return InterpretResult::RUNTIME_ERROR;
                }
                double b = std::get<double>(pop());
                double a = std::get<double>(pop());
                push(a > b);
                break;
            }
            case static_cast<uint8_t>(OpCode::OP_LESS): {
                 if (!std::holds_alternative<double>(peek(0)) || !std::holds_alternative<double>(peek(1))) {
                     runtimeError("Operands must be numbers.");
                     return InterpretResult::RUNTIME_ERROR;
                }
                double b = std::get<double>(pop());
                double a = std::get<double>(pop());
                push(a < b);
                break;
            }
            case static_cast<uint8_t>(OpCode::OP_ADD): binaryOp(OpCode::OP_ADD); break;
            case static_cast<uint8_t>(OpCode::OP_SUBTRACT): binaryOp(OpCode::OP_SUBTRACT); break;
            case static_cast<uint8_t>(OpCode::OP_MULTIPLY): binaryOp(OpCode::OP_MULTIPLY); break;
            case static_cast<uint8_t>(OpCode::OP_DIVIDE): binaryOp(OpCode::OP_DIVIDE); break;
            case static_cast<uint8_t>(OpCode::OP_NOT): {
                push(isFalsey(pop()));
                break;
            }
            case static_cast<uint8_t>(OpCode::OP_NEGATE): {
                if (!std::holds_alternative<double>(peek(0))) {
                    runtimeError("Operand must be a number.");
                    return InterpretResult::RUNTIME_ERROR;
                }
                double val = std::get<double>(pop());
                push(-val);
                break;
            }
            case static_cast<uint8_t>(OpCode::OP_PRINT): {
                printValue(pop());
                std::cout << std::endl;
                break;
            }
            case static_cast<uint8_t>(OpCode::OP_JUMP): {
                uint16_t offset = READ_SHORT();
                ip += offset;
                break;
            }
            case static_cast<uint8_t>(OpCode::OP_JUMP_IF_FALSE): {
                uint16_t offset = READ_SHORT();
                if (isFalsey(peek(0))) {
                    ip += offset;
                }
                break;
            }
            case static_cast<uint8_t>(OpCode::OP_LOOP): {
                uint16_t offset = READ_SHORT();
                ip -= offset;
                break;
            }
            case static_cast<uint8_t>(OpCode::OP_RETURN): {
                // Exit interpreter
                return InterpretResult::OK;
            }
        }
    }
}

void VM::binaryOp(OpCode op) {
    if (!std::holds_alternative<double>(peek(0)) || !std::holds_alternative<double>(peek(1))) {
        runtimeError("Operands must be numbers.");
        return;
    }
    double b = std::get<double>(pop());
    double a = std::get<double>(pop());
    
    switch (op) {
        case OpCode::OP_ADD: push(a + b); break;
        case OpCode::OP_SUBTRACT: push(a - b); break;
        case OpCode::OP_MULTIPLY: push(a * b); break;
        case OpCode::OP_DIVIDE: push(a / b); break;
        default: break; // Unreachable
    }
}

bool VM::valuesEqual(Value a, Value b) {
    if (a.index() != b.index()) return false;
    if (std::holds_alternative<Nil>(a)) return true;
    if (std::holds_alternative<bool>(a)) return std::get<bool>(a) == std::get<bool>(b);
    if (std::holds_alternative<double>(a)) return std::get<double>(a) == std::get<double>(b);
    if (std::holds_alternative<std::string>(a)) return std::get<std::string>(a) == std::get<std::string>(b);
    return false;
}

void VM::runtimeError(const char* format, ...) {
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fputs("\n", stderr);
    
    size_t instruction = ip - chunk->code.data() - 1;
    int line = chunk->lines[instruction];
    fprintf(stderr, "[line %d] in script\n", line);
}
