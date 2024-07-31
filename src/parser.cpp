#include "parser.hpp"
#include "ast.hpp"
#include <stdexcept>

namespace EntS {

Parser::Parser(const std::vector<Token>& tokens) : tokens(tokens), current(0), proc(nullptr) {}

const Token& Parser::consume() {
    if (current >= tokens.size()) {
        throw std::runtime_error("Unexpected end of input");
    }
    return tokens[current++];
}

const Token& Parser::peek(int offset) const {
    if (current + offset >= tokens.size()) {
        throw std::runtime_error("Unexpected end of input");
    }
    return tokens[current + offset];
}

const Token& Parser::previous() const {
    if (current == 0) {
        throw std::runtime_error("No previous token available");
    }
    return tokens[current - 1];
}

void Parser::expect(Token::TokenType type, const std::string& message) {
    if (!check(type)) {
        throw std::runtime_error(message);
    }
    consume();
}

bool Parser::check(Token::TokenType type) const {
    if (current >= tokens.size()) {
        return false;
    }
    return tokens[current].type == type;
}

bool Parser::match(std::initializer_list<Token::TokenType> types) {
    for (Token::TokenType type : types) {
        if (check(type)) {
            consume();
            return true;
        }
    }
    return false;
}

bool Parser::isType(const std::string& name) {
    return std::find(existing_types.begin(), existing_types.end(), name) != existing_types.end();
}

void Parser::error(const Token& token, const std::string& message) {
    std::cerr << "Error at line " << token.line << ", column " << token.column << " got: " 
              << token.toString() << ": " << message << std::endl;
    throw std::runtime_error("Parse error");
}

ASTNodePtr Parser::parse() {
	std::vector<ASTNodePtr> statements;
	while (!check(Token::TokenType::EOF_TOKEN)) {
		if (match({Token::TokenType::HEADER})) {
			statements.push_back(parseHeader());
		} else if (match({Token::TokenType::FUNCTION})) {
			statements.push_back(parseFunction());
		} else if (match({Token::TokenType::TYPEDEF})) {
			statements.push_back(parseTypedef());
		} else if (isType(peek().value)) {
			if (peek(2).type == Token::TokenType::SEMICOLON) {
				statements.push_back(parseGlobalVarDecl());
			} else if (peek(2).type == Token::TokenType::ASSIGN) {
				statements.push_back(parseGlobalVarDecl());
			} else {
				error(peek(2), "Expect ';' or '=' after type declaration.");
			}
		} else if (match({Token::TokenType::INLINE_ASM})) {
			statements.push_back(parseAsm());
		} else {
			error(peek(), "Expect statement.");
		}
	}
	return std::make_unique<ProgramNode>(std::move(statements));
}

ASTNodePtr Parser::parseHeader() {
	std::vector<ASTNodePtr> prototypes;
	while (!check(Token::TokenType::EOF_TOKEN) || !check(Token::TokenType::RIGHT_BRACE)) {

		if (match({Token::TokenType::FUNCTION})) {
			prototypes.push_back(parseFunctionPrototype());
		} else if (match({Token::TokenType::TYPEDEF})) {
			prototypes.push_back(parseTypedef());
		} else if (peek(2).type == Token::TokenType::ASSIGN) {
			error(peek(2), "Header does not allow for global variable initialization.");
		} else if (isType(peek().value)) {
			prototypes.push_back(parseGlobalVarDecl());
		} else {
			error(peek(), "Expect statement.");
		}

	}
	expect(Token::TokenType::RIGHT_BRACE, "Expect '}' after header.");
	expect(Token::TokenType::SEMICOLON, "Expect ';' after header.");

	return std::make_unique<HeaderNode>(std::move(prototypes));
}

ASTNodePtr Parser::parseFunctionPrototype() {
	std::string name;
	std::string return_value;
	std::vector<ASTNodePtr> parameters;

	expect(Token::TokenType::FUNCTION, "Expect 'function' keyword.");
	name = consume().value;
	existing_functions.push_back(name);

	expect(Token::TokenType::LEFT_PAREN, "Expect '(' after function name.");
	if (!check(Token::TokenType::RIGHT_PAREN)) {
		std::string type;
		type = consume().value;
		if (!isType(type)) {
			error(peek(), "Expect function parameter type.");
		}
		std::string name = consume().value;
		parameters.push_back(std::make_unique<ParameterNode>(type, name));
		while (match({Token::TokenType::COMMA})) {
			type = consume().value;
			if (!isType(type)) {
				error(peek(), "Expect function parameter type.");
			}
			name = consume().value;
			parameters.push_back(std::make_unique<ParameterNode>(type, name));
		}
	}
	expect(Token::TokenType::RIGHT_PAREN, "Expect ')' after function parameters.");
	expect(Token::TokenType::MINUS, "Expect '->' after function parameters.");
	expect(Token::TokenType::GREATER, "Expect '>' after function return type.");

	return_value = consume().value;
	if (!isType(return_value)) {
		error(peek(), "Expect function return type.");
	}

	expect(Token::TokenType::SEMICOLON, "Expect ';' after function prototype.");

	return std::make_unique<FunctionPrototypeNode>(name, return_value, std::move(parameters));
}

} // namespace EntS