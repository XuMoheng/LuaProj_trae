#ifndef COMPILER_H
#define COMPILER_H

#include "AST.h"
#include "Chunk.h"
#include <vector>
#include <memory>

class Compiler : public ExprVisitor, public StmtVisitor {
public:
    Compiler();
    bool compile(const std::vector<std::unique_ptr<Stmt>>& statements, Chunk* chunk);

    // Visitor methods
    void visitBinaryExpr(BinaryExpr* expr) override;
    void visitGroupingExpr(GroupingExpr* expr) override;
    void visitLiteralExpr(LiteralExpr* expr) override;
    void visitUnaryExpr(UnaryExpr* expr) override;
    void visitVariableExpr(VariableExpr* expr) override;
    void visitAssignmentExpr(AssignmentExpr* expr) override;
    void visitCallExpr(CallExpr* expr) override;

    void visitExpressionStmt(ExpressionStmt* stmt) override;
    void visitPrintStmt(PrintStmt* stmt) override;
    void visitVarDecl(VarDecl* stmt) override;
    void visitBlockStmt(BlockStmt* stmt) override;
    void visitIfStmt(IfStmt* stmt) override;
    void visitWhileStmt(WhileStmt* stmt) override;
    void visitFunctionStmt(FunctionStmt* stmt) override;
    void visitReturnStmt(ReturnStmt* stmt) override;

private:
    Chunk* currentChunk;
    
    void emitByte(uint8_t byte);
    void emitOp(OpCode op);
    void emitBytes(uint8_t byte1, uint8_t byte2);
    void emitLoop(int loopStart);
    int emitJump(uint8_t instruction);
    void patchJump(int offset);
    int makeConstant(Value value);
    void emitConstant(Value value);
};

#endif // COMPILER_H
