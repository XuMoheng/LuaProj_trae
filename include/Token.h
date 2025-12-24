#ifndef TOKEN_H
#define TOKEN_H

#include <string>
#include <iostream>

enum class TokenType {
    // Single-character tokens
    LEFT_PAREN, RIGHT_PAREN, LEFT_BRACE, RIGHT_BRACE,
    COMMA, DOT, MINUS, PLUS, SEMICOLON, SLASH, STAR,
    
    // One or two character tokens
    BANG, BANG_EQUAL,
    EQUAL, EQUAL_EQUAL,
    GREATER, GREATER_EQUAL,
    LESS, LESS_EQUAL,
    
    // Literals
    IDENTIFIER, STRING, NUMBER,
    
    // Keywords
    AND, BREAK, DO, ELSE, ELSEIF, END, FALSE, FOR, FUNCTION,
    IF, IN, LOCAL, NIL, NOT, OR, REPEAT, RETURN, THEN,
    TRUE, UNTIL, WHILE,
    
    TOKEN_EOF
};

struct Token {
    TokenType type;
    std::string lexeme;
    int line;
    int column;

    Token(TokenType type, std::string lexeme, int line, int column)
        : type(type), lexeme(lexeme), line(line), column(column) {}
        
    std::string toString() const {
        return "Token(" + std::to_string(static_cast<int>(type)) + ", '" + lexeme + "')";
    }
};

#endif // TOKEN_H
