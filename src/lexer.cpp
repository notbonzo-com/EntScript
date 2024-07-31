#include "lexer.hpp"
#include <cctype>
#include <unordered_map>
#include <iostream>
#include <variant>

namespace EntS {

static const std::unordered_map<std::string, Token::TokenType> keywords = {
    {"function", Token::TokenType::FUNCTION},
    {"return", Token::TokenType::RETURN},
    {"void", Token::TokenType::VOID},
    {"typedef", Token::TokenType::TYPEDEF},
    {"struct", Token::TokenType::STRUCT},
    {"if", Token::TokenType::IF},
    {"else", Token::TokenType::ELSE},
    {"while", Token::TokenType::WHILE},
    {"switch", Token::TokenType::SWITCH},
    {"case", Token::TokenType::CASE},
    {"default", Token::TokenType::DEFAULT},
    {"break", Token::TokenType::BREAK},
    {"continue", Token::TokenType::CONTINUE},
    {"header", Token::TokenType::HEADER},
    {"int8", Token::TokenType::INT8},
    {"int16", Token::TokenType::INT16},
    {"int32", Token::TokenType::INT32},
    {"int64", Token::TokenType::INT64},
    {"uint8", Token::TokenType::UINT8},
    {"uint16", Token::TokenType::UINT16},
    {"uint32", Token::TokenType::UINT32},
    {"uint64", Token::TokenType::UINT64},
    {"float", Token::TokenType::FLOAT},
    {"char", Token::TokenType::CHAR},
    {"bool", Token::TokenType::BOOL},
    {"asm", Token::TokenType::INLINE_ASM}
};

Lexer::Lexer(const std::string& source)
    : source(source), start(0), current(0), line(1), column(1) {
    tokens.reserve(source.size() / 4); // Estimate, to minimise resizing
}

std::vector<Token> Lexer::tokenize() {
    while (current < source.length()) {
        skipWhitespace();
        start = current;
        if (current >= source.length()) break;
        char c = advance();
        switch (c) {
            case '(': addToken(Token::TokenType::LEFT_PAREN); break;
            case ')': addToken(Token::TokenType::RIGHT_PAREN); break;
            case '{': addToken(Token::TokenType::LEFT_BRACE); break;
            case '}': addToken(Token::TokenType::RIGHT_BRACE); break;
            case '[': addToken(Token::TokenType::LEFT_BRACKET); break;
            case ']': addToken(Token::TokenType::RIGHT_BRACKET); break;
            case ';': addToken(Token::TokenType::SEMICOLON); break;
            case ',': addToken(Token::TokenType::COMMA); break;
            case '=':
                addToken(match('=') ? Token::TokenType::EQUAL : Token::TokenType::ASSIGN);
                break;
            case '!':
                addToken(match('=') ? Token::TokenType::NOT_EQUAL : Token::TokenType::EXCLAMATION);
                break;
            case '<':
                addToken(match('=') ? Token::TokenType::LESS_EQUAL : Token::TokenType::LESS);
                break;
            case '>':
                addToken(match('=') ? Token::TokenType::GREATER_EQUAL : Token::TokenType::GREATER);
                break;
            case '+': addToken(Token::TokenType::PLUS); break;
            case '-': addToken(Token::TokenType::MINUS); break;
            case '*': addToken(Token::TokenType::STAR); break;
            case '/':
                handleSlash();
                break;
            case '%': addToken(Token::TokenType::PERCENT); break;
            case '&': addToken(Token::TokenType::AMPERSAND); break;
            case '|': addToken(Token::TokenType::PIPE); break;
            case '"': handleString(); break;
            default:
                if (std::isdigit(c)) {
                    handleNumber();
                } else if (std::isalpha(c) || c == '_') {
                    handleIdentifier();
                } else {
                    error("Unexpected character: " + std::string(1, c));
                }
                break;
        }
    }
    tokens.emplace_back(Token::TokenType::EOF_TOKEN, "", line, column);
    return tokens;
}

char Lexer::peek() const {
    return current < source.length() ? source[current] : '\0';
}

char Lexer::peekNext() const {
    return (current + 1) < source.length() ? source[current + 1] : '\0';
}

char Lexer::advance() {
    char c = source[current++];
    column++;
    if (c == '\n') {
        line++;
        column = 1;
    }
    return c;
}

bool Lexer::match(char expected) {
    if (peek() != expected) return false;
    advance();
    return true;
}

void Lexer::addToken(Token::TokenType type, const std::string& value) {
    tokens.emplace_back(type, value, line, column - (current - start));
}

void Lexer::error(const std::string& message) {
    std::cerr << "Error at line " << line << ", column " << column << ": " << message << std::endl;
}

void Lexer::handleIdentifier() {
    while (std::isalnum(peek()) || peek() == '_') {
        advance();
    }
    std::string text = source.substr(start, current - start);
    Token::TokenType type = keywords.count(text) ? keywords.at(text) : Token::TokenType::IDENTIFIER;
    addToken(type, text);
}

void Lexer::handleNumber() {
    while (std::isdigit(peek())) {
        advance();
    }
    if (peek() == '.' && std::isdigit(peekNext())) {
        advance(); // consume the '.'
        while (std::isdigit(peek())) {
            advance();
        }
    }
    addToken(Token::TokenType::NUMBER, source.substr(start, current - start));
}

void Lexer::handleString() {
    while (peek() != '"' && current < source.length()) {
        if (peek() == '\n') line++;
        advance();
    }
    if (current >= source.length()) {
        error("Unterminated string.");
        return;
    }
    advance(); // closing "
    addToken(Token::TokenType::STRING, source.substr(start + 1, current - start - 2));
}

void Lexer::handleSlash() {
    if (match('/')) {
        skipLineComment();
    } else if (match('*')) {
        skipBlockComment();
    } else {
        addToken(Token::TokenType::SLASH);
    }
}

void Lexer::skipLineComment() {
    while (peek() != '\n' && current < source.length()) {
        advance();
    }
}

void Lexer::skipBlockComment() {
    while (peek() != '\0' && !(peek() == '*' && peekNext() == '/')) {
        if (peek() == '\n') line++;
        advance();
    }
    if (current >= source.length()) {
        error("Unterminated block comment.");
        return;
    }
    advance(); // consume '*'
    advance(); // consume '/'
}

void Lexer::skipWhitespace() {
    while (true) {
        char c = peek();
        switch (c) {
            case ' ':
            case '\r':
            case '\t':
                advance();
                break;
            case '\n':
                advance();
                break;
            default:
                start = current;
                return;
        }
    }
}

} // namespace EntS
