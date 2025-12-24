#ifndef AST_H
#define AST_H

#include "Token.h"
#include <vector>
#include <memory>
#include <string>

// Forward declarations
class BinaryExpr;
class GroupingExpr;
class LiteralExpr;
class UnaryExpr;
class VariableExpr;
class AssignmentExpr;
class CallExpr;

class ExpressionStmt;
class PrintStmt; // Using 'print' as a statement for simplicity in this basic version
class VarDecl;
class BlockStmt;
class IfStmt;
class WhileStmt;
class FunctionStmt;
class ReturnStmt;

class ExprVisitor {
public:
    virtual ~ExprVisitor() = default;
    virtual void visitBinaryExpr(BinaryExpr* expr) = 0;
    virtual void visitGroupingExpr(GroupingExpr* expr) = 0;
    virtual void visitLiteralExpr(LiteralExpr* expr) = 0;
    virtual void visitUnaryExpr(UnaryExpr* expr) = 0;
    virtual void visitVariableExpr(VariableExpr* expr) = 0;
    virtual void visitAssignmentExpr(AssignmentExpr* expr) = 0;
    virtual void visitCallExpr(CallExpr* expr) = 0;
};

class StmtVisitor {
public:
    virtual ~StmtVisitor() = default;
    virtual void visitExpressionStmt(ExpressionStmt* stmt) = 0;
    virtual void visitPrintStmt(PrintStmt* stmt) = 0;
    virtual void visitVarDecl(VarDecl* stmt) = 0;
    virtual void visitBlockStmt(BlockStmt* stmt) = 0;
    virtual void visitIfStmt(IfStmt* stmt) = 0;
    virtual void visitWhileStmt(WhileStmt* stmt) = 0;
    virtual void visitFunctionStmt(FunctionStmt* stmt) = 0;
    virtual void visitReturnStmt(ReturnStmt* stmt) = 0;
};

class Expr {
public:
    virtual ~Expr() = default;
    virtual void accept(ExprVisitor* visitor) = 0;
};

class Stmt {
public:
    virtual ~Stmt() = default;
    virtual void accept(StmtVisitor* visitor) = 0;
};

// --- Expressions ---

class BinaryExpr : public Expr {
public:
    std::unique_ptr<Expr> left;
    Token op;
    std::unique_ptr<Expr> right;
    BinaryExpr(std::unique_ptr<Expr> left, Token op, std::unique_ptr<Expr> right)
        : left(std::move(left)), op(op), right(std::move(right)) {}
    void accept(ExprVisitor* visitor) override { visitor->visitBinaryExpr(this); }
};

class GroupingExpr : public Expr {
public:
    std::unique_ptr<Expr> expression;
    GroupingExpr(std::unique_ptr<Expr> expression) : expression(std::move(expression)) {}
    void accept(ExprVisitor* visitor) override { visitor->visitGroupingExpr(this); }
};

class LiteralExpr : public Expr {
public:
    std::string value; // Storing as string for simplicity
    LiteralExpr(std::string value) : value(value) {}
    void accept(ExprVisitor* visitor) override { visitor->visitLiteralExpr(this); }
};

class UnaryExpr : public Expr {
public:
    Token op;
    std::unique_ptr<Expr> right;
    UnaryExpr(Token op, std::unique_ptr<Expr> right) : op(op), right(std::move(right)) {}
    void accept(ExprVisitor* visitor) override { visitor->visitUnaryExpr(this); }
};

class VariableExpr : public Expr {
public:
    Token name;
    VariableExpr(Token name) : name(name) {}
    void accept(ExprVisitor* visitor) override { visitor->visitVariableExpr(this); }
};

class AssignmentExpr : public Expr {
public:
    Token name;
    std::unique_ptr<Expr> value;
    AssignmentExpr(Token name, std::unique_ptr<Expr> value)
        : name(name), value(std::move(value)) {}
    void accept(ExprVisitor* visitor) override { visitor->visitAssignmentExpr(this); }
};

class CallExpr : public Expr {
public:
    std::unique_ptr<Expr> callee;
    Token paren; // For error reporting
    std::vector<std::unique_ptr<Expr>> arguments;
    CallExpr(std::unique_ptr<Expr> callee, Token paren, std::vector<std::unique_ptr<Expr>> arguments)
        : callee(std::move(callee)), paren(paren), arguments(std::move(arguments)) {}
    void accept(ExprVisitor* visitor) override { visitor->visitCallExpr(this); }
};

// --- Statements ---

class ExpressionStmt : public Stmt {
public:
    std::unique_ptr<Expr> expression;
    ExpressionStmt(std::unique_ptr<Expr> expression) : expression(std::move(expression)) {}
    void accept(StmtVisitor* visitor) override { visitor->visitExpressionStmt(this); }
};

class PrintStmt : public Stmt {
public:
    std::unique_ptr<Expr> expression;
    PrintStmt(std::unique_ptr<Expr> expression) : expression(std::move(expression)) {}
    void accept(StmtVisitor* visitor) override { visitor->visitPrintStmt(this); }
};

class VarDecl : public Stmt {
public:
    Token name;
    std::unique_ptr<Expr> initializer;
    VarDecl(Token name, std::unique_ptr<Expr> initializer)
        : name(name), initializer(std::move(initializer)) {}
    void accept(StmtVisitor* visitor) override { visitor->visitVarDecl(this); }
};

class BlockStmt : public Stmt {
public:
    std::vector<std::unique_ptr<Stmt>> statements;
    BlockStmt(std::vector<std::unique_ptr<Stmt>> statements)
        : statements(std::move(statements)) {}
    void accept(StmtVisitor* visitor) override { visitor->visitBlockStmt(this); }
};

class IfStmt : public Stmt {
public:
    std::unique_ptr<Expr> condition;
    std::unique_ptr<Stmt> thenBranch;
    std::unique_ptr<Stmt> elseBranch;
    IfStmt(std::unique_ptr<Expr> condition, std::unique_ptr<Stmt> thenBranch, std::unique_ptr<Stmt> elseBranch)
        : condition(std::move(condition)), thenBranch(std::move(thenBranch)), elseBranch(std::move(elseBranch)) {}
    void accept(StmtVisitor* visitor) override { visitor->visitIfStmt(this); }
};

class WhileStmt : public Stmt {
public:
    std::unique_ptr<Expr> condition;
    std::unique_ptr<Stmt> body;
    WhileStmt(std::unique_ptr<Expr> condition, std::unique_ptr<Stmt> body)
        : condition(std::move(condition)), body(std::move(body)) {}
    void accept(StmtVisitor* visitor) override { visitor->visitWhileStmt(this); }
};

class FunctionStmt : public Stmt {
public:
    Token name;
    std::vector<Token> params;
    std::vector<std::unique_ptr<Stmt>> body;
    FunctionStmt(Token name, std::vector<Token> params, std::vector<std::unique_ptr<Stmt>> body)
        : name(name), params(params), body(std::move(body)) {}
    void accept(StmtVisitor* visitor) override { visitor->visitFunctionStmt(this); }
};

class ReturnStmt : public Stmt {
public:
    Token keyword;
    std::unique_ptr<Expr> value;
    ReturnStmt(Token keyword, std::unique_ptr<Expr> value)
        : keyword(keyword), value(std::move(value)) {}
    void accept(StmtVisitor* visitor) override { visitor->visitReturnStmt(this); }
};

#endif // AST_H
