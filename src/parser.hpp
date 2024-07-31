#ifndef PARSER_HPP
#define PARSER_HPP

#include <vector>
#include <memory>
#include <unordered_map>
#include "tokens.hpp"
#include "ast.hpp"
#include "preprocessor.hpp"

namespace EntS {

class Parser {
public:
    explicit Parser(const std::vector<Token>& tokens);

    ASTNodePtr parse();
    Preprocessor* proc;

private:
    const Token& consume(); // returns reference to token, and increments
    const Token& peek(int offset = 0) const; // returns reference to token at offset
    const Token& previous() const; // returns reference to previous token
    void expect(Token::TokenType type, const std::string& message); // errors if token is not of type
    bool check(Token::TokenType type) const; // returns true if current token is of type
    bool match(std::initializer_list<Token::TokenType> types); // returns true if current token is one of types

    ASTNodePtr parseFunction();
    ASTNodePtr parseCall();
    ASTNodePtr parseVarDecl();
    ASTNodePtr parseVarDeclAssign();
    ASTNodePtr parseIf();
    ASTNodePtr parseWhile();
    ASTNodePtr parseSwitch();
    ASTNodePtr parseCase();
    ASTNodePtr parseDefault();

    ASTNodePtr parseBlock();
    ASTNodePtr parseHeader();

    ASTNodePtr parseExpression();
    ASTNodePtr parseParenExpression();
    ASTNodePtr parseLogicalOr();
    ASTNodePtr parseLogicalAnd();
    ASTNodePtr parseEquality();
    ASTNodePtr parseRelational();
    ASTNodePtr parseBitwise();
    ASTNodePtr parseAdditive();
    ASTNodePtr parseMultiplicative();
    ASTNodePtr parseUnary();
    ASTNodePtr parseAddr();
    ASTNodePtr parseMemberAccess();
    ASTNodePtr parseIndexation();
    ASTNodePtr parsePrimary();
    ASTNodePtr parseFunctionCall();
    ASTNodePtr parseCharLiteral();
    ASTNodePtr parseStringLiteral();
    ASTNodePtr parseTernary();
    ASTNodePtr parseIdentifier();
    ASTNodePtr parseTypeCast();

    ASTNodePtr parseFunctionPrototype();
    ASTNodePtr parseReturn();

    ASTNodePtr parseTypedef();
    ASTNodePtr parseStruct();
    ASTNodePtr parseGlobalVarDecl();
    ASTNodePtr parseGlobalVarDeclAssign();

    ASTNodePtr parseIncrement();
    ASTNodePtr parseDecrement();
    ASTNodePtr parseContinue();
    ASTNodePtr parseBreak();
    ASTNodePtr parseAsm();

    ASTNodePtr parseElse();
    ASTNodePtr parseElseIf();

    void error(const Token& token, const std::string& message);

    const std::vector<Token>& tokens;
    size_t current = 0;
    std::vector<std::string> existing_types = {
        "void", "char", "float", "bool", "int8", "int16", "int32", "int64", "uint8", "uint16", "uint32", "uint64"
    };
    std::vector<std::string> existing_functions;
    std::vector<std::string> existing_variables;

    bool isType(const std::string& name);
};

} // namespace EntS

#endif // PARSER_HPP
