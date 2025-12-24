#ifndef PARSER_H
#define PARSER_H

#include "Token.h"
#include "AST.h"
#include <vector>
#include <memory>
#include <stdexcept>

class ParseError : public std::runtime_error {
public:
    ParseError(const char* msg) : std::runtime_error(msg) {}
};

class Parser {
public:
    Parser(const std::vector<Token>& tokens);
    std::vector<std::unique_ptr<Stmt>> parse();

private:
    const std::vector<Token>& tokens;
    int current = 0;

    std::unique_ptr<Stmt> declaration();
    std::unique_ptr<Stmt> varDeclaration();
    std::unique_ptr<Stmt> functionDeclaration();
    std::unique_ptr<Stmt> statement();
    std::unique_ptr<Stmt> ifStatement();
    std::unique_ptr<Stmt> whileStatement();
    std::unique_ptr<Stmt> forStatement(); // Optional
    std::unique_ptr<Stmt> returnStatement();
    std::vector<std::unique_ptr<Stmt>> block();
    std::unique_ptr<Stmt> expressionStatement();

    std::unique_ptr<Expr> expression();
    std::unique_ptr<Expr> assignment();
    std::unique_ptr<Expr> orExpr();
    std::unique_ptr<Expr> andExpr();
    std::unique_ptr<Expr> equality();
    std::unique_ptr<Expr> comparison();
    std::unique_ptr<Expr> term();
    std::unique_ptr<Expr> factor();
    std::unique_ptr<Expr> unary();
    std::unique_ptr<Expr> call();
    std::unique_ptr<Expr> finishCall(std::unique_ptr<Expr> callee);
    std::unique_ptr<Expr> primary();

    bool match(const std::vector<TokenType>& types);
    bool check(TokenType type);
    bool isAtEnd();
    Token advance();
    Token peek();
    Token previous();
    Token consume(TokenType type, std::string message);
    void synchronize();
};

#endif // PARSER_H
