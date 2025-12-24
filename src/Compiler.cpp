#include "Compiler.h"
#include <iostream>

Compiler::Compiler() : currentChunk(nullptr) {}

bool Compiler::compile(const std::vector<std::unique_ptr<Stmt>>& statements, Chunk* chunk) {
    currentChunk = chunk;
    for (const auto& stmt : statements) {
        stmt->accept(this);
    }
    emitOp(OpCode::OP_RETURN);
    return true;
}

void Compiler::emitByte(uint8_t byte) {
    currentChunk->write(byte, 0); // TODO: Pass line number
}

void Compiler::emitOp(OpCode op) {
    emitByte(static_cast<uint8_t>(op));
}

void Compiler::emitBytes(uint8_t byte1, uint8_t byte2) {
    emitByte(byte1);
    emitByte(byte2);
}

void Compiler::emitLoop(int loopStart) {
    emitOp(OpCode::OP_LOOP);

    int offset = currentChunk->code.size() - loopStart + 2;
    if (offset > UINT16_MAX) {
        // Error: Loop body too large
    }

    emitByte((offset >> 8) & 0xff);
    emitByte(offset & 0xff);
}

int Compiler::emitJump(uint8_t instruction) {
    emitByte(instruction);
    emitByte(0xff);
    emitByte(0xff);
    return currentChunk->code.size() - 2;
}

void Compiler::patchJump(int offset) {
    // -2 to adjust for the jump offset itself
    int jump = currentChunk->code.size() - offset - 2;

    if (jump > UINT16_MAX) {
        // Error: Too much code to jump over
    }

    currentChunk->code[offset] = (jump >> 8) & 0xff;
    currentChunk->code[offset + 1] = jump & 0xff;
}

int Compiler::makeConstant(Value value) {
    return currentChunk->addConstant(value);
}

void Compiler::emitConstant(Value value) {
    emitBytes(static_cast<uint8_t>(OpCode::OP_CONSTANT), makeConstant(value));
}

// --- Visitors ---

void Compiler::visitBinaryExpr(BinaryExpr* expr) {
    expr->left->accept(this);
    expr->right->accept(this);

    TokenType type = expr->op.type;
    switch (type) {
        case TokenType::PLUS:          emitOp(OpCode::OP_ADD); break;
        case TokenType::MINUS:         emitOp(OpCode::OP_SUBTRACT); break;
        case TokenType::STAR:          emitOp(OpCode::OP_MULTIPLY); break;
        case TokenType::SLASH:         emitOp(OpCode::OP_DIVIDE); break;
        case TokenType::EQUAL_EQUAL:   emitOp(OpCode::OP_EQUAL); break;
        case TokenType::GREATER:       emitOp(OpCode::OP_GREATER); break;
        case TokenType::LESS:          emitOp(OpCode::OP_LESS); break;
        // Handle other operators like >=, <=, != by combining logic or adding opcodes
        // For now simplified
        default: break;
    }
}

void Compiler::visitGroupingExpr(GroupingExpr* expr) {
    expr->expression->accept(this);
}

void Compiler::visitLiteralExpr(LiteralExpr* expr) {
    std::string& val = expr->value;
    // Check if it's a number, boolean, or nil
    if (val == "nil") {
        emitOp(OpCode::OP_NIL);
    } else if (val == "true") {
        emitOp(OpCode::OP_TRUE);
    } else if (val == "false") {
        emitOp(OpCode::OP_FALSE);
    } else {
        // Try parsing as double
        try {
            size_t idx;
            double num = std::stod(val, &idx);
            if (idx == val.length()) {
                emitConstant(num);
                return;
            }
        } catch (...) {}
        
        // It's a string
        // Remove quotes if present
        if (val.length() >= 2 && val.front() == '"' && val.back() == '"') {
            emitConstant(val.substr(1, val.length() - 2));
        } else {
             emitConstant(val);
        }
    }
}

void Compiler::visitUnaryExpr(UnaryExpr* expr) {
    expr->right->accept(this);

    switch (expr->op.type) {
        case TokenType::MINUS: emitOp(OpCode::OP_NEGATE); break;
        case TokenType::NOT: emitOp(OpCode::OP_NOT); break;
        default: break;
    }
}

void Compiler::visitVariableExpr(VariableExpr* expr) {
    emitBytes(static_cast<uint8_t>(OpCode::OP_GET_GLOBAL), makeConstant(expr->name.lexeme));
}

void Compiler::visitAssignmentExpr(AssignmentExpr* expr) {
    expr->value->accept(this);
    emitBytes(static_cast<uint8_t>(OpCode::OP_SET_GLOBAL), makeConstant(expr->name.lexeme));
}

void Compiler::visitCallExpr(CallExpr* expr) {
    // For now, only support 'print' specially
    if (VariableExpr* v = dynamic_cast<VariableExpr*>(expr->callee.get())) {
        if (v->name.lexeme == "print") {
            // Evaluate arguments
            for (const auto& arg : expr->arguments) {
                arg->accept(this);
                emitOp(OpCode::OP_PRINT);
            }
            // Push nil as return value of print
            emitOp(OpCode::OP_NIL); 
            return;
        }
    }
    // TODO: Generic function call
}

void Compiler::visitExpressionStmt(ExpressionStmt* stmt) {
    stmt->expression->accept(this);
    emitOp(OpCode::OP_POP);
}

void Compiler::visitPrintStmt(PrintStmt* stmt) {
    // Not used in our parser (handled as call)
}

void Compiler::visitVarDecl(VarDecl* stmt) {
    if (stmt->initializer) {
        stmt->initializer->accept(this);
    } else {
        emitOp(OpCode::OP_NIL);
    }
    emitBytes(static_cast<uint8_t>(OpCode::OP_DEFINE_GLOBAL), makeConstant(stmt->name.lexeme));
}

void Compiler::visitBlockStmt(BlockStmt* stmt) {
    for (const auto& s : stmt->statements) {
        s->accept(this);
    }
}

void Compiler::visitIfStmt(IfStmt* stmt) {
    stmt->condition->accept(this);

    int thenJump = emitJump(static_cast<uint8_t>(OpCode::OP_JUMP_IF_FALSE));
    emitOp(OpCode::OP_POP); // Pop condition

    stmt->thenBranch->accept(this);

    int elseJump = emitJump(static_cast<uint8_t>(OpCode::OP_JUMP));

    patchJump(thenJump);
    emitOp(OpCode::OP_POP); // Pop condition if we jumped here (wait, jump_if_false doesn't pop in some VMs, but here we popped in then block. 
    // Actually: JUMP_IF_FALSE usually leaves value on stack if false? 
    // Let's assume JUMP_IF_FALSE does NOT pop.
    // So if true: fall through, we should pop.
    // If false: jump, we should pop at target.
    // My implementation:
    //   condition
    //   JUMP_IF_FALSE else_label
    //   POP (pop true condition)
    //   then_block
    //   JUMP end_label
    // else_label:
    //   POP (pop false condition)
    //   else_block
    // end_label:
    
    // Correct logic:
    // stmt->condition->accept(this);
    // int thenJump = emitJump(OP_JUMP_IF_FALSE);
    // emitOp(OP_POP); // Pop condition (true path)
    // stmt->thenBranch->accept(this);
    // int elseJump = emitJump(OP_JUMP);
    // patchJump(thenJump);
    // emitOp(OP_POP); // Pop condition (false path)
    // if (stmt->elseBranch) stmt->elseBranch->accept(this);
    // patchJump(elseJump);

    // Let's re-implement strictly
    // My previous logic was slightly off in the manual code above.
    
    // Correction:
    //   condition
    //   JUMP_IF_FALSE -> jump to ELSE
    //   POP (condition was true, discard it)
    //   THEN branch
    //   JUMP -> jump to END
    // ELSE:
    //   POP (condition was false, discard it)
    //   ELSE branch
    // END:
    
    // But wait, my VM implementation of OP_JUMP_IF_FALSE does NOT pop.
    // So yes, we need explicit POPs.
    
    // Wait, I already wrote `emitOp(OpCode::OP_POP); // Pop condition` after jump.
    // That handles the TRUE case.
    // Now I need to handle the FALSE case (target of jump).
    
    if (stmt->elseBranch) {
        stmt->elseBranch->accept(this);
    }
    patchJump(elseJump);
}

void Compiler::visitWhileStmt(WhileStmt* stmt) {
    int loopStart = currentChunk->code.size();
    
    stmt->condition->accept(this);
    int exitJump = emitJump(static_cast<uint8_t>(OpCode::OP_JUMP_IF_FALSE));
    emitOp(OpCode::OP_POP);
    
    stmt->body->accept(this);
    emitLoop(loopStart);
    
    patchJump(exitJump);
    emitOp(OpCode::OP_POP);
}

void Compiler::visitFunctionStmt(FunctionStmt* stmt) {
    // Not implemented yet
}

void Compiler::visitReturnStmt(ReturnStmt* stmt) {
    if (stmt->value) {
        stmt->value->accept(this);
    } else {
        emitOp(OpCode::OP_NIL);
    }
    emitOp(OpCode::OP_RETURN);
}
