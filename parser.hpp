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

    std::unique_ptr<ASTNode> parse();
	Preprocessor* proc;
private:
    const Token& advance();
    const Token& peek() const;
    const Token& previous() const;
    bool match(std::initializer_list<Token::TokenType> types);
    bool check(Token::TokenType type) const;
    const Token& consume(Token::TokenType type, const std::string& message);

    std::unique_ptr<ASTNode> parseFunction();
    std::unique_ptr<ASTNode> parseVarDecl();
    std::unique_ptr<ASTNode> parseStatement();

	std::unique_ptr<ASTNode> parseHeader();
    std::unique_ptr<ASTNode> parseExpression();
    std::unique_ptr<ASTNode> parseAssignment();
    std::unique_ptr<ASTNode> parseOr();
    std::unique_ptr<ASTNode> parseAnd();
    std::unique_ptr<ASTNode> parseEquality();
    std::unique_ptr<ASTNode> parseComparison();
    std::unique_ptr<ASTNode> parseTerm();
    std::unique_ptr<ASTNode> parseFactor();
    std::unique_ptr<ASTNode> parseUnary();
    std::unique_ptr<ASTNode> parsePrimary();
	std::unique_ptr<ASTNode> parseTypeDef();
	std::unique_ptr<ASTNode> parseBlock();
	std::unique_ptr<ASTNode> parseStructNode();
	std::unique_ptr<ASTNode> parseCase();
	std::unique_ptr<ASTNode> parseSwitch();

    void error(const Token& token, const std::string& message);

    const std::vector<Token>& tokens;
    size_t current;
	std::vector<std::string> existing_types = {
		"void", "char", "float", "bool", "int8", "int16", "int32", "int64", "uint8", "uint16", "uint32", "uint64"
	};
	bool isType(const std::string& name);
	uint32_t fuckMe;
	uint32_t asmIndex;
};

} // namespace EntS

#endif // PARSER_HPP
