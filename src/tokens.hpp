#ifndef TOKENS_HPP
#define TOKENS_HPP

#include <string>

namespace EntS {

    struct Token {
        enum class TokenType {
            FUNCTION, RETURN, VOID, TYPEDEF, STRUCT,
            IF, ELSE, WHILE, SWITCH, CASE, DEFAULT, BREAK, CONTINUE,
            HEADER, INLINE_ASM,
            INT8, INT16, INT32, INT64, UINT8, UINT16, UINT32, UINT64, FLOAT, CHAR, BOOL,
            LEFT_PAREN, RIGHT_PAREN, LEFT_BRACE, RIGHT_BRACE, LEFT_BRACKET, RIGHT_BRACKET,
            SEMICOLON, COMMA, ASSIGN, EQUAL, NOT_EQUAL, LESS, LESS_EQUAL, GREATER, GREATER_EQUAL,
            PLUS, MINUS, STAR, SLASH, PERCENT, AMPERSAND, PIPE, EXCLAMATION,
            IDENTIFIER, NUMBER, STRING, EOF_TOKEN,
        };
        TokenType type;
        std::string value;
        int line;
        int column;

        Token(TokenType t, std::string v, int l, int c)
            : type(t), value(v), line(l), column(c) {}
        
        std::string toString() const
        {
            std::string result;
            switch (type) {
                case TokenType::FUNCTION: result = "FUNCTION"; break;
                case TokenType::RETURN: result = "RETURN"; break;
                case TokenType::VOID: result = "VOID"; break;
                case TokenType::TYPEDEF: result = "TYPEDEF"; break;
                case TokenType::STRUCT: result = "STRUCT"; break;
                case TokenType::IF: result = "IF"; break;
                case TokenType::ELSE: result = "ELSE"; break;
                case TokenType::WHILE: result = "WHILE"; break;
                case TokenType::SWITCH: result = "SWITCH"; break;
                case TokenType::CASE: result = "CASE"; break;
                case TokenType::DEFAULT: result = "DEFAULT"; break;
                case TokenType::BREAK: result = "BREAK"; break;
                case TokenType::CONTINUE: result = "CONTINUE"; break;
                case TokenType::HEADER: result = "HEADER"; break;
                case TokenType::INT8: result = "INT8"; break;
                case TokenType::INT16: result = "INT16"; break;
                case TokenType::INT32: result = "INT32"; break;
                case TokenType::INT64: result = "INT64"; break;
                case TokenType::UINT8: result = "UINT8"; break;
                case TokenType::UINT16: result = "UINT16"; break;
                case TokenType::UINT32: result = "UINT32"; break;
                case TokenType::UINT64: result = "UINT64"; break;
                case TokenType::FLOAT: result = "FLOAT"; break;
                case TokenType::CHAR: result = "CHAR"; break;
                case TokenType::BOOL: result = "BOOL"; break;
                case TokenType::LEFT_PAREN: result = "LEFT_PAREN"; break;
                case TokenType::RIGHT_PAREN: result = "RIGHT_PAREN"; break;
                case TokenType::LEFT_BRACE: result = "LEFT_BRACE"; break;
                case TokenType::RIGHT_BRACE: result = "RIGHT_BRACE"; break;
                case TokenType::LEFT_BRACKET: result = "LEFT_BRACKET"; break;
                case TokenType::RIGHT_BRACKET: result = "RIGHT_BRACKET"; break;
                case TokenType::SEMICOLON: result = "SEMICOLON"; break;
                case TokenType::COMMA: result = "COMMA"; break;
                case TokenType::ASSIGN: result = "ASSIGN"; break;
                case TokenType::EQUAL: result = "EQUAL"; break;
                case TokenType::NOT_EQUAL: result = "NOT_EQUAL"; break;
                case TokenType::LESS: result = "LESS"; break;
                case TokenType::LESS_EQUAL: result = "LESS_EQUAL"; break;
                case TokenType::GREATER: result = "GREATER"; break;
                case TokenType::GREATER_EQUAL: result = "GREATER_EQUAL"; break;
                case TokenType::PLUS: result = "PLUS"; break;
                case TokenType::MINUS: result = "MINUS"; break;
                case TokenType::STAR: result = "STAR"; break;
                case TokenType::SLASH: result = "SLASH"; break;
                case TokenType::PERCENT: result = "PERCENT"; break;
                case TokenType::AMPERSAND: result = "AMPERSAND"; break;
                case TokenType::PIPE: result = "PIPE"; break;
                case TokenType::EXCLAMATION: result = "EXCLAMATION"; break;
                case TokenType::IDENTIFIER: result = "IDENTIFIER"; break;
                case TokenType::NUMBER: result = "NUMBER"; break;
                case TokenType::STRING: result = "STRING"; break;
                case TokenType::EOF_TOKEN: result = "EOF_TOKEN"; break;
            }

            return result + " " + value + " " + std::to_string(line) + " " + std::to_string(column);
        }
    };

} // namespace EntS

#endif // TOKENS_HPP