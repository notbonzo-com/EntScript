#ifndef LEXER_HPP
#define LEXER_HPP

#include <string>
#include <vector>
#include <unordered_map>
#include <optional>
#include "tokens.hpp"

namespace EntS {

class Lexer {
public:
    explicit Lexer(const std::string& source);
    std::vector<Token> tokenize();

private:
    char peek() const;
    char peekNext() const;
    char advance();
    bool match(char expected);
    void addToken(Token::TokenType type, const std::string& value = "");
    void error(const std::string& message);

    void handleIdentifier();
    void handleNumber();
    void handleString();
    void handleSlash();
    void skipLineComment();
    void skipBlockComment();
    void skipWhitespace();

    std::string source;
    std::vector<Token> tokens;
    size_t start;
    size_t current;
    int line;
    int column;
};

} // namespace EntS

#endif // LEXER_HPP
