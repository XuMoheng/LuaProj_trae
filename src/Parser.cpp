#include "Parser.h"
#include <iostream>

Parser::Parser(const std::vector<Token>& tokens) : tokens(tokens) {}

std::vector<std::unique_ptr<Stmt>> Parser::parse() {
    std::vector<std::unique_ptr<Stmt>> statements;
    while (!isAtEnd()) {
        statements.push_back(declaration());
    }
    return statements;
}

std::unique_ptr<Stmt> Parser::declaration() {
    try {
        if (match({TokenType::FUNCTION})) return functionDeclaration();
        if (match({TokenType::LOCAL})) return varDeclaration();
        return statement();
    } catch (ParseError& error) {
        synchronize();
        return nullptr;
    }
}

std::unique_ptr<Stmt> Parser::functionDeclaration() {
    Token name = consume(TokenType::IDENTIFIER, "Expect function name.");
    consume(TokenType::LEFT_PAREN, "Expect '(' after function name.");
    
    std::vector<Token> parameters;
    if (!check(TokenType::RIGHT_PAREN)) {
        do {
            if (parameters.size() >= 255) {
                // Warning: too many parameters
            }
            parameters.push_back(consume(TokenType::IDENTIFIER, "Expect parameter name."));
        } while (match({TokenType::COMMA}));
    }
    consume(TokenType::RIGHT_PAREN, "Expect ')' after parameters.");
    
    // Function body
    // In Lua, function body ends with 'end'
    std::vector<std::unique_ptr<Stmt>> body = block();
    consume(TokenType::END, "Expect 'end' after function body.");
    
    return std::make_unique<FunctionStmt>(name, parameters, std::move(body));
}

std::unique_ptr<Stmt> Parser::varDeclaration() {
    Token name = consume(TokenType::IDENTIFIER, "Expect variable name.");
    std::unique_ptr<Expr> initializer = nullptr;
    if (match({TokenType::EQUAL})) {
        initializer = expression();
    }
    // Lua doesn't strictly require semicolons, but we can consume if present
    // match({TokenType::SEMICOLON}); 
    return std::make_unique<VarDecl>(name, std::move(initializer));
}

std::unique_ptr<Stmt> Parser::statement() {
    if (match({TokenType::IF})) return ifStatement();
    if (match({TokenType::WHILE})) return whileStatement();
    if (match({TokenType::DO})) {
        std::vector<std::unique_ptr<Stmt>> stmts = block();
        consume(TokenType::END, "Expect 'end' after do block.");
        return std::make_unique<BlockStmt>(std::move(stmts));
    }
    if (match({TokenType::RETURN})) return returnStatement();
    
    return expressionStatement();
}

std::unique_ptr<Stmt> Parser::ifStatement() {
    std::unique_ptr<Expr> condition = expression();
    consume(TokenType::THEN, "Expect 'then' after if condition.");
    
    std::vector<std::unique_ptr<Stmt>> thenStmts = block();
    std::unique_ptr<Stmt> thenBranch = std::make_unique<BlockStmt>(std::move(thenStmts));
    std::unique_ptr<Stmt> elseBranch = nullptr;
    
    if (match({TokenType::ELSE})) {
        std::vector<std::unique_ptr<Stmt>> elseStmts = block();
        elseBranch = std::make_unique<BlockStmt>(std::move(elseStmts));
    }
    // TODO: Handle elseif
    
    consume(TokenType::END, "Expect 'end' after if statement.");
    return std::make_unique<IfStmt>(std::move(condition), std::move(thenBranch), std::move(elseBranch));
}

std::unique_ptr<Stmt> Parser::whileStatement() {
    std::unique_ptr<Expr> condition = expression();
    consume(TokenType::DO, "Expect 'do' after while condition.");
    std::vector<std::unique_ptr<Stmt>> bodyStmts = block();
    consume(TokenType::END, "Expect 'end' after while loop.");
    
    return std::make_unique<WhileStmt>(std::move(condition), std::make_unique<BlockStmt>(std::move(bodyStmts)));
}

std::unique_ptr<Stmt> Parser::returnStatement() {
    Token keyword = previous();
    std::unique_ptr<Expr> value = nullptr;
    if (!check(TokenType::END) && !check(TokenType::ELSE) && !check(TokenType::ELSEIF) && !check(TokenType::TOKEN_EOF)) {
         // Actually we should check if next token starts a statement or is expression start
         // Simple check: if not block end
         if (!check(TokenType::SEMICOLON)) {
             value = expression();
         }
    }
    match({TokenType::SEMICOLON});
    
    return std::make_unique<ReturnStmt>(keyword, std::move(value));
}

std::vector<std::unique_ptr<Stmt>> Parser::block() {
    std::vector<std::unique_ptr<Stmt>> statements;
    while (!check(TokenType::END) && !check(TokenType::ELSE) && !check(TokenType::ELSEIF) && !check(TokenType::UNTIL) && !isAtEnd()) {
        statements.push_back(declaration());
    }
    return statements;
}

std::unique_ptr<Stmt> Parser::expressionStatement() {
    std::unique_ptr<Expr> expr = expression();
    // match({TokenType::SEMICOLON});
    return std::make_unique<ExpressionStmt>(std::move(expr));
}

std::unique_ptr<Expr> Parser::expression() {
    return assignment();
}

std::unique_ptr<Expr> Parser::assignment() {
    std::unique_ptr<Expr> expr = orExpr();

    if (match({TokenType::EQUAL})) {
        Token equals = previous();
        std::unique_ptr<Expr> value = assignment();

        if (VariableExpr* v = dynamic_cast<VariableExpr*>(expr.get())) {
            Token name = v->name;
            return std::make_unique<AssignmentExpr>(name, std::move(value));
        }
        
        throw ParseError("Invalid assignment target.");
    }

    return expr;
}

std::unique_ptr<Expr> Parser::orExpr() {
    std::unique_ptr<Expr> expr = andExpr();

    while (match({TokenType::OR})) {
        Token op = previous();
        std::unique_ptr<Expr> right = andExpr();
        expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
    }

    return expr;
}

std::unique_ptr<Expr> Parser::andExpr() {
    std::unique_ptr<Expr> expr = equality();

    while (match({TokenType::AND})) {
        Token op = previous();
        std::unique_ptr<Expr> right = equality();
        expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
    }

    return expr;
}

std::unique_ptr<Expr> Parser::equality() {
    std::unique_ptr<Expr> expr = comparison();

    while (match({TokenType::BANG_EQUAL, TokenType::EQUAL_EQUAL})) {
        Token op = previous();
        std::unique_ptr<Expr> right = comparison();
        expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
    }

    return expr;
}

std::unique_ptr<Expr> Parser::comparison() {
    std::unique_ptr<Expr> expr = term();

    while (match({TokenType::GREATER, TokenType::GREATER_EQUAL, TokenType::LESS, TokenType::LESS_EQUAL})) {
        Token op = previous();
        std::unique_ptr<Expr> right = term();
        expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
    }

    return expr;
}

std::unique_ptr<Expr> Parser::term() {
    std::unique_ptr<Expr> expr = factor();

    while (match({TokenType::MINUS, TokenType::PLUS})) {
        Token op = previous();
        std::unique_ptr<Expr> right = factor();
        expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
    }

    return expr;
}

std::unique_ptr<Expr> Parser::factor() {
    std::unique_ptr<Expr> expr = unary();

    while (match({TokenType::SLASH, TokenType::STAR})) {
        Token op = previous();
        std::unique_ptr<Expr> right = unary();
        expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
    }

    return expr;
}

std::unique_ptr<Expr> Parser::unary() {
    if (match({TokenType::BANG, TokenType::MINUS, TokenType::NOT})) {
        Token op = previous();
        std::unique_ptr<Expr> right = unary();
        return std::make_unique<UnaryExpr>(op, std::move(right));
    }

    return call();
}

std::unique_ptr<Expr> Parser::call() {
    std::unique_ptr<Expr> expr = primary();

    while (true) {
        if (match({TokenType::LEFT_PAREN})) {
            expr = finishCall(std::move(expr));
        } else {
            break;
        }
    }

    return expr;
}

std::unique_ptr<Expr> Parser::finishCall(std::unique_ptr<Expr> callee) {
    std::vector<std::unique_ptr<Expr>> arguments;
    if (!check(TokenType::RIGHT_PAREN)) {
        do {
            if (arguments.size() >= 255) {
                // Error: Can't have more than 255 arguments.
            }
            arguments.push_back(expression());
        } while (match({TokenType::COMMA}));
    }

    Token paren = consume(TokenType::RIGHT_PAREN, "Expect ')' after arguments.");

    return std::make_unique<CallExpr>(std::move(callee), paren, std::move(arguments));
}

std::unique_ptr<Expr> Parser::primary() {
    if (match({TokenType::FALSE})) return std::make_unique<LiteralExpr>("false");
    if (match({TokenType::TRUE})) return std::make_unique<LiteralExpr>("true");
    if (match({TokenType::NIL})) return std::make_unique<LiteralExpr>("nil");

    if (match({TokenType::NUMBER, TokenType::STRING})) {
        return std::make_unique<LiteralExpr>(previous().lexeme);
    }

    if (match({TokenType::IDENTIFIER})) {
        return std::make_unique<VariableExpr>(previous());
    }

    if (match({TokenType::LEFT_PAREN})) {
        std::unique_ptr<Expr> expr = expression();
        consume(TokenType::RIGHT_PAREN, "Expect ')' after expression.");
        return std::make_unique<GroupingExpr>(std::move(expr));
    }

    throw ParseError("Expect expression.");
}

bool Parser::match(const std::vector<TokenType>& types) {
    for (TokenType type : types) {
        if (check(type)) {
            advance();
            return true;
        }
    }
    return false;
}

bool Parser::check(TokenType type) {
    if (isAtEnd()) return false;
    return peek().type == type;
}

bool Parser::isAtEnd() {
    return peek().type == TokenType::TOKEN_EOF;
}

Token Parser::advance() {
    if (!isAtEnd()) current++;
    return previous();
}

Token Parser::peek() {
    return tokens[current];
}

Token Parser::previous() {
    return tokens[current - 1];
}

Token Parser::consume(TokenType type, std::string message) {
    if (check(type)) return advance();
    throw ParseError(message.c_str());
}

void Parser::synchronize() {
    advance();
    while (!isAtEnd()) {
        if (previous().type == TokenType::SEMICOLON) return;
        switch (peek().type) {
            case TokenType::FUNCTION:
            case TokenType::LOCAL:
            case TokenType::FOR:
            case TokenType::IF:
            case TokenType::WHILE:
            case TokenType::RETURN:
            case TokenType::REPEAT:
                return;
            default:
                break;
        }
        advance();
    }
}
