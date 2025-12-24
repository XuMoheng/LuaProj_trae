#include "Lexer.h"
#include <unordered_map>
#include <cctype>

Lexer::Lexer(const std::string& source) : source(source) {}

std::vector<Token> Lexer::scanTokens() {
    while (!isAtEnd()) {
        start = current;
        scanToken();
    }
    tokens.emplace_back(TokenType::TOKEN_EOF, "", line, column);
    return tokens;
}

bool Lexer::isAtEnd() {
    return current >= source.length();
}

char Lexer::advance() {
    column++;
    return source[current++];
}

char Lexer::peek() {
    if (isAtEnd()) return '\0';
    return source[current];
}

char Lexer::peekNext() {
    if (current + 1 >= source.length()) return '\0';
    return source[current + 1];
}

bool Lexer::match(char expected) {
    if (isAtEnd()) return false;
    if (source[current] != expected) return false;
    current++;
    column++;
    return true;
}

void Lexer::addToken(TokenType type) {
    addToken(type, source.substr(start, current - start));
}

void Lexer::addToken(TokenType type, std::string literal) {
    tokens.emplace_back(type, literal, line, column);
}

void Lexer::scanToken() {
    char c = advance();
    switch (c) {
        case '(': addToken(TokenType::LEFT_PAREN); break;
        case ')': addToken(TokenType::RIGHT_PAREN); break;
        case '{': addToken(TokenType::LEFT_BRACE); break;
        case '}': addToken(TokenType::RIGHT_BRACE); break;
        case ',': addToken(TokenType::COMMA); break;
        case '.': addToken(TokenType::DOT); break;
        case '-': 
            if (match('-')) {
                // Comment
                while (peek() != '\n' && !isAtEnd()) advance();
            } else {
                addToken(TokenType::MINUS);
            }
            break;
        case '+': addToken(TokenType::PLUS); break;
        case ';': addToken(TokenType::SEMICOLON); break;
        case '*': addToken(TokenType::STAR); break;
        case '!': addToken(match('=') ? TokenType::BANG_EQUAL : TokenType::BANG); break;
        case '=': addToken(match('=') ? TokenType::EQUAL_EQUAL : TokenType::EQUAL); break;
        case '<': addToken(match('=') ? TokenType::LESS_EQUAL : TokenType::LESS); break;
        case '>': addToken(match('=') ? TokenType::GREATER_EQUAL : TokenType::GREATER); break;
        case '/': addToken(TokenType::SLASH); break;
        
        case ' ':
        case '\r':
        case '\t':
            // Ignore whitespace
            break;
        case '\n':
            line++;
            column = 1;
            break;
        case '"': string(); break;
        default:
            if (isdigit(c)) {
                number();
            } else if (isalpha(c) || c == '_') {
                identifier();
            } else {
                // Error: Unexpected character
                std::cerr << "Unexpected character at line " << line << ": " << c << std::endl;
            }
            break;
    }
}

void Lexer::string() {
    while (peek() != '"' && !isAtEnd()) {
        if (peek() == '\n') {
            line++;
            column = 1;
        }
        advance();
    }

    if (isAtEnd()) {
        std::cerr << "Unterminated string at line " << line << std::endl;
        return;
    }

    advance(); // The closing "

    // Trim the surrounding quotes
    std::string value = source.substr(start + 1, current - start - 2);
    addToken(TokenType::STRING, value);
}

void Lexer::number() {
    while (isdigit(peek())) advance();

    if (peek() == '.' && isdigit(peekNext())) {
        advance(); // Consume the "."
        while (isdigit(peek())) advance();
    }

    addToken(TokenType::NUMBER);
}

void Lexer::identifier() {
    while (isalnum(peek()) || peek() == '_') advance();

    std::string text = source.substr(start, current - start);
    TokenType type = TokenType::IDENTIFIER;
    
    static const std::unordered_map<std::string, TokenType> keywords = {
        {"and", TokenType::AND},
        {"break", TokenType::BREAK},
        {"do", TokenType::DO},
        {"else", TokenType::ELSE},
        {"elseif", TokenType::ELSEIF},
        {"end", TokenType::END},
        {"false", TokenType::FALSE},
        {"for", TokenType::FOR},
        {"function", TokenType::FUNCTION},
        {"if", TokenType::IF},
        {"in", TokenType::IN},
        {"local", TokenType::LOCAL},
        {"nil", TokenType::NIL},
        {"not", TokenType::NOT},
        {"or", TokenType::OR},
        {"repeat", TokenType::REPEAT},
        {"return", TokenType::RETURN},
        {"then", TokenType::THEN},
        {"true", TokenType::TRUE},
        {"until", TokenType::UNTIL},
        {"while", TokenType::WHILE}
    };

    auto it = keywords.find(text);
    if (it != keywords.end()) {
        type = it->second;
    }

    addToken(type);
}
