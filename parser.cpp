#include "parser.hpp"
#include <stdexcept>
#include <iostream>

namespace EntS {

Parser::Parser(const std::vector<Token>& tokens) : tokens(tokens), current(0) {}

std::unique_ptr<ASTNode> Parser::parse() {
    std::vector<ASTNodePtr> nodes;
    while (!check(Token::TokenType::EOF_TOKEN)) {
        nodes.push_back(parseStatement());
    }
    return std::make_unique<BlockNode>(std::move(nodes));
}

const Token& Parser::advance() {
    if (!check(Token::TokenType::EOF_TOKEN)) current++;
    return previous();
}

const Token& Parser::peek() const {
    return tokens[current];
}

const Token& Parser::previous() const {
    return tokens[current - 1];
}

bool Parser::match(std::initializer_list<Token::TokenType> types) {
    for (auto type : types) {
        if (check(type)) {
            advance();
            return true;
        }
    }
    return false;
}

bool Parser::check(Token::TokenType type) const {
    return current < tokens.size() && peek().type == type;
}

bool Parser::isType(const std::string& type) {
	for (std::string t : existing_types) {
		if (t == type) {
			return true;
		}
	}

	return false;
}

const Token& Parser::consume(Token::TokenType type, const std::string& message) {
    if (check(type)) {
        return advance();
    }
    error(peek(), message);
    return tokens[current]; // This line will never be reached, but avoids compiler warnings
}

void Parser::error(const Token& token, const std::string& message) {
    std::cerr << "Error at line " << token.line << ", column " << token.column << " got: " 
              << token.toString() << ": " << message << std::endl;
    throw std::runtime_error("Parse error");
}


std::unique_ptr<ASTNode> Parser::parseFunction() {
    std::string name = consume(Token::TokenType::IDENTIFIER, "Expect function name.").value;
    consume(Token::TokenType::LEFT_PAREN, "Expect '(' after function name.");

    std::vector<ASTNodePtr> params;
    if (!check(Token::TokenType::RIGHT_PAREN)) {
        do {
            // std::string paramType = consume(Token::TokenType::IDENTIFIER, "Expect parameter type.").value;
			std::string paramType;
			if (isType(peek().value)) {
				paramType = peek().value;
				advance();
			} else {
				error(peek(), "Expect parameter type.");
			}
            std::string paramName = consume(Token::TokenType::IDENTIFIER, "Expect parameter name.").value;
            params.push_back(std::make_unique<VarDeclNode>(paramType, paramName, nullptr));
        } while (match({Token::TokenType::COMMA}));
    }
    consume(Token::TokenType::RIGHT_PAREN, "Expect ')' after parameters.");
	consume(Token::TokenType::MINUS, "Expect '->' after function declaration.");
	consume(Token::TokenType::GREATER, "Expect '->' after function declaration.");
	std::string returnType;
	if (isType(peek().value)) {
		returnType = peek().value;
		advance();
	} else {
		error(peek(), "Expect parameter type.");
	}

    consume(Token::TokenType::LEFT_BRACE, "Expect '{' before function body.");

	std::vector<ASTNodePtr> body;
	while (!check(Token::TokenType::RIGHT_BRACE) && !check(Token::TokenType::EOF_TOKEN)) {
		body.push_back(parseStatement());
	}
	if (check(Token::TokenType::RIGHT_BRACE)) { advance(); }

	consume(Token::TokenType::SEMICOLON, "Expect ';' after function declaration.");
    return std::make_unique<FunctionNode>(name, std::move(params), std::move(body), returnType);
}

std::unique_ptr<ASTNode> Parser::parseVarDecl() {
    std::string type = previous().value;
    std::string name = consume(Token::TokenType::IDENTIFIER, "Expect variable name.").value;
    ASTNodePtr initializer = nullptr;

    if (match({Token::TokenType::ASSIGN})) {
        initializer = parseExpression();
    }
    consume(Token::TokenType::SEMICOLON, "Expect ';' after variable declaration.");

    return std::make_unique<VarDeclNode>(type, name, std::move(initializer));
}

std::unique_ptr<ASTNode> Parser::parseHeader() {
	consume(Token::TokenType::LEFT_BRACE, "Expect '{' after 'header' keyword.");
	// Parse the block
	std::vector<ASTNodePtr> statements;
	while (!check(Token::TokenType::RIGHT_BRACE) && !check(Token::TokenType::EOF_TOKEN)) {
		statements.push_back(parseStatement());
	}
	consume(Token::TokenType::RIGHT_BRACE, "Expect '}' after header block.");
	consume(Token::TokenType::SEMICOLON, "Expect ';' after header declaration.");
	return std::make_unique<HeaderNode>(std::move(statements));
}

std::unique_ptr<ASTNode> Parser::parseStructNode() {
	consume(Token::TokenType::LEFT_BRACE, "Expect '{' after 'struct' keyword.");
	std::vector<ASTNodePtr> fields;

	while (!check(Token::TokenType::RIGHT_BRACE) && !check(Token::TokenType::EOF_TOKEN)) {
		fields.push_back(parseVarDecl());
	}
	consume(Token::TokenType::RIGHT_BRACE, "Expect '}' after struct block.");
	
	return std::make_unique<StructNode>(previous().value, std::move(fields));
}

std::unique_ptr<ASTNode> Parser::parseTypeDef()
{
	std::string newType;
	if (peek().type == Token::TokenType::STRUCT)
	{
		consume(Token::TokenType::STRUCT, "Expect 'struct' keyword.");
		ASTNodePtr structNode = parseStructNode();
		newType = consume(Token::TokenType::IDENTIFIER, "Expect parameter name.").value;
		consume(Token::TokenType::SEMICOLON, "Expect ';' after struct declaration.");
		return std::make_unique<TypedefStructNode>(std::move(structNode), newType);
 	} else {
		std::string oldType;
		if (isType(peek().value)) {
			oldType = peek().value;
			advance();
		} else {
			error(peek(), "Expect parameter type.");
		}
		newType = consume(Token::TokenType::IDENTIFIER, "Expect parameter name.").value;
		consume(Token::TokenType::SEMICOLON, "Expect ';' after struct declaration.");
		existing_types.push_back(newType);
		return std::make_unique<TypedefNode>(oldType, newType);
	}
}

std::unique_ptr<ASTNode> Parser::parseCase()
{
	if (match({Token::TokenType::DEFAULT})) {
		consume(Token::TokenType::LEFT_BRACE, "Expect '{' after 'default' keyword.");
		std::vector<ASTNodePtr> statements;
		while (!check(Token::TokenType::RIGHT_BRACE) && !check(Token::TokenType::EOF_TOKEN)) {
			statements.push_back(parseStatement());
		}
		consume(Token::TokenType::RIGHT_BRACE, "Expect '}' after 'default' block.");
		consume(Token::TokenType::SEMICOLON, "Expect ';' after 'default' declaration.");
		return std::make_unique<DefaultNode>(std::move(statements));
	} else if (match({Token::TokenType::CASE})) {
		consume(Token::TokenType::LEFT_PAREN, "Expect '(' after 'case' keyword.");
		ASTNodePtr condition = parseExpression();
		consume(Token::TokenType::RIGHT_PAREN, "Expect ')' after 'case' condition.");
		consume(Token::TokenType::LEFT_BRACE, "Expect '{' after 'case' keyword.");
		std::vector<ASTNodePtr> statements;
		while (!check(Token::TokenType::RIGHT_BRACE) && !check(Token::TokenType::EOF_TOKEN)) {
			statements.push_back(parseStatement());
		}
		consume(Token::TokenType::RIGHT_BRACE, "Expect '}' after 'case' block.");
		consume(Token::TokenType::SEMICOLON, "Expect ';' after 'case' declaration.");
		return std::make_unique<CaseNode>(std::move(condition), std::move(statements));
	} else {
		error(peek(), "Expect 'case' or 'default' keyword.");
		return nullptr;
	}
}
std::unique_ptr<ASTNode> Parser::parseSwitch()
{
	consume(Token::TokenType::LEFT_PAREN, "Expect '(' after switch keyword.");
	ASTNodePtr condition = parseExpression();
	consume(Token::TokenType::RIGHT_PAREN, "Expect ')' after switch condition.");
	consume(Token::TokenType::LEFT_BRACE, "Expect '{' after switch condition.");
	std::vector<ASTNodePtr> cases;
	while (!check(Token::TokenType::RIGHT_BRACE) && !check(Token::TokenType::EOF_TOKEN)) {
		cases.push_back(parseCase());
	}
	consume(Token::TokenType::RIGHT_BRACE, "Expect '}' after switch block.");
	consume(Token::TokenType::SEMICOLON, "Expect ';' after switch declaration.");
	return std::make_unique<SwitchNode>(std::move(condition), std::move(cases));
}

std::unique_ptr<ASTNode> Parser::parseStatement() {
	if (match({Token::TokenType::INLINE_ASM})) {
        const auto& asmBlocks = proc->getAsmBlocks();
        size_t asmBlockIndex = asmIndex;
		asmIndex++;
        const auto& lines = asmBlocks[asmBlockIndex].lines;
        return std::make_unique<InlineAsmNode>(lines); 
    }
	if (match({Token::TokenType::HEADER})) {
		return parseHeader();
	}
    if (match({Token::TokenType::LEFT_BRACE})) {
        std::vector<ASTNodePtr> statements;
        while (!check(Token::TokenType::RIGHT_BRACE) && !check(Token::TokenType::EOF_TOKEN)) {
            statements.push_back(parseStatement());
        }
        consume(Token::TokenType::RIGHT_BRACE, "Expect '}' after block.");
        return std::make_unique<BlockNode>(std::move(statements));
    }
    if (match({Token::TokenType::IF})) {
        consume(Token::TokenType::LEFT_PAREN, "Expect '(' after 'if'.");
        auto condition = parseExpression();
        consume(Token::TokenType::RIGHT_PAREN, "Expect ')' after if condition.");
        std::vector<ASTNodePtr> thenBranch;
		consume(Token::TokenType::LEFT_BRACE, "Expect '{' after if condition.");
		while (!check(Token::TokenType::RIGHT_BRACE) && !check(Token::TokenType::EOF_TOKEN)) {
			thenBranch.push_back(parseStatement());
		}
		if (peek().type == Token::TokenType::RIGHT_BRACE) { advance(); }
        std::vector<ASTNodePtr> elseBranch;
        if (match({Token::TokenType::ELSE})) {
			while (!check(Token::TokenType::RIGHT_BRACE) && !check(Token::TokenType::EOF_TOKEN)) {
				elseBranch.push_back(parseStatement());
			}
			if (peek().type == Token::TokenType::RIGHT_BRACE) { advance(); }
        }
        return std::make_unique<IfNode>(std::move(condition), std::move(thenBranch), std::move(elseBranch));
    }
    if (match({Token::TokenType::WHILE})) {
        consume(Token::TokenType::LEFT_PAREN, "Expect '(' after 'while'.");
        auto condition = parseExpression();
        consume(Token::TokenType::RIGHT_PAREN, "Expect ')' after while condition.");
        auto body = parseStatement();
		consume(Token::TokenType::SEMICOLON, "Expect ';' after while body.");
        return std::make_unique<WhileNode>(std::move(condition), std::move(body));
    }
    if (match({Token::TokenType::RETURN})) {
        ASTNodePtr value = nullptr;
        if (!check(Token::TokenType::SEMICOLON)) {
            value = parseExpression();
        }
        consume(Token::TokenType::SEMICOLON, "Expect ';' after return value.");
        return std::make_unique<ReturnNode>(std::move(value));
    }
    if (match({Token::TokenType::CONTINUE})) {
        consume(Token::TokenType::SEMICOLON, "Expect ';' after 'continue'.");
        return std::make_unique<ContinueNode>();
    }
    if (match({Token::TokenType::BREAK})) {
        consume(Token::TokenType::SEMICOLON, "Expect ';' after 'break'.");
        return std::make_unique<BreakNode>();
    }
    if (match({Token::TokenType::IDENTIFIER})) {
		if (isType(previous().value)) {
			return parseVarDecl();
		}
		std::string name = previous().value;
		/* ++ or -- ?*/
		if (match({Token::TokenType::PLUS})) {
			if (match({Token::TokenType::PLUS})) {
				consume(Token::TokenType::SEMICOLON, "Expect ';' after '++'.");
				return std::make_unique<IncrementNode>(name);
			}
		}
		if (match({Token::TokenType::MINUS})) {
			if (match({Token::TokenType::MINUS})) {
				consume(Token::TokenType::SEMICOLON, "Expect ';' after '--'.");
				return std::make_unique<DecrementNode>(name);
			}
		}
        ASTNodePtr index = nullptr;
		if (match({Token::TokenType::LEFT_BRACKET})) {
			/* indexation */
			index = parseExpression();
			consume(Token::TokenType::RIGHT_BRACKET, "Expect ']' after indexation.");
		}
        consume(Token::TokenType::ASSIGN, "Expect '=' after identifier.");
        auto value = parseExpression();
        consume(Token::TokenType::SEMICOLON, "Expect ';' after assignment.");
        if (index) {
            return std::make_unique<AssignNode>(name, std::move(index), std::move(value));
        } else {
            return std::make_unique<AssignNode>(name, std::move(value));
        }
    }
	if (match({Token::TokenType::FUNCTION})) {
		// std::cout << peek().toString() << previous().toString() << std::endl;
		return parseFunction();
	}
	if (match({Token::TokenType::TYPEDEF})) {
		return parseTypeDef();
	}

	if (isType(peek().value)) {
		advance();
		return parseVarDecl();
	}
	if (match({Token::TokenType::SWITCH})) {
		return parseSwitch();
	}
	if (match({Token::TokenType::SEMICOLON})) { 
		fuckMe++;
		if (fuckMe == 2) {
			return nullptr;
		}
		current -= 2;
		return nullptr;
	}

	std::cout << "Token: " << peek().toString() << std::endl <<
	 previous().toString() << std::endl 
	 << tokens[current - 2].toString() << std::endl
	 << tokens[current - 3].toString() << std::endl;
    error(peek(), "Unexpected token.");
    return nullptr;
}

std::unique_ptr<ASTNode> Parser::parseExpression() {
    return parseAssignment();
}

std::unique_ptr<ASTNode> Parser::parseAssignment() {
    auto expr = parseOr();
    if (match({Token::TokenType::ASSIGN})) {
        auto equals = previous();
        auto value = parseAssignment();
        if (auto nameExpr = dynamic_cast<ExprNode*>(expr.get())) {
            if (auto varDecl = dynamic_cast<VarDeclNode*>(nameExpr->getLeft().get())) {
                return std::make_unique<AssignNode>(varDecl->getName(), std::move(value));
            }
        }
        error(equals, "Invalid assignment target.");
    }
    return expr;
}

std::unique_ptr<ASTNode> Parser::parseOr() {
    auto expr = parseAnd();

    while (match({Token::TokenType::PIPE})) {
        auto op = previous().value;
        auto right = parseAnd();
        expr = std::make_unique<ExprNode>(op, std::move(expr), std::move(right));
    }

    return expr;
}

std::unique_ptr<ASTNode> Parser::parseAnd() {
    auto expr = parseEquality();

    while (match({Token::TokenType::AMPERSAND})) {
        auto op = previous().value;
        auto right = parseEquality();
        expr = std::make_unique<ExprNode>(op, std::move(expr), std::move(right));
    }

    return expr;
}

std::unique_ptr<ASTNode> Parser::parseEquality() {
    auto expr = parseComparison();

    while (match({Token::TokenType::EQUAL, Token::TokenType::NOT_EQUAL})) {
        auto op = previous().value;
        auto right = parseComparison();
        expr = std::make_unique<ExprNode>(op, std::move(expr), std::move(right));
    }

    return expr;
}

std::unique_ptr<ASTNode> Parser::parseComparison() {
    auto expr = parseTerm();

    while (match({Token::TokenType::LESS, Token::TokenType::LESS_EQUAL, Token::TokenType::GREATER, Token::TokenType::GREATER_EQUAL})) {
        auto op = previous().value;
        auto right = parseTerm();
        expr = std::make_unique<ExprNode>(op, std::move(expr), std::move(right));
    }

    return expr;
}

std::unique_ptr<ASTNode> Parser::parseTerm() {
    auto expr = parseFactor();

    while (match({Token::TokenType::PLUS, Token::TokenType::MINUS})) {
        auto op = previous().value;
        auto right = parseFactor();
        expr = std::make_unique<ExprNode>(op, std::move(expr), std::move(right));
    }

    return expr;
}

std::unique_ptr<ASTNode> Parser::parseFactor() {
    auto expr = parseUnary();

    while (match({Token::TokenType::STAR, Token::TokenType::SLASH})) {
        auto op = previous().value;
        auto right = parseUnary();
        expr = std::make_unique<ExprNode>(op, std::move(expr), std::move(right));
    }

    return expr;
}

std::unique_ptr<ASTNode> Parser::parseUnary() {
    if (match({Token::TokenType::EXCLAMATION, Token::TokenType::MINUS})) {
        auto op = previous().value;
        auto right = parseUnary();
        return std::make_unique<ExprNode>(op, nullptr, std::move(right));
    }

    return parsePrimary();
}

std::unique_ptr<ASTNode> Parser::parsePrimary() {
    if (match({Token::TokenType::NUMBER})) {
        return std::make_unique<ExprNode>("literal", std::make_unique<VarDeclNode>("number", previous().value, nullptr), nullptr);
    }

    if (match({Token::TokenType::IDENTIFIER})) {
		/* Can be indexation or struct */
		if (match({Token::TokenType::LEFT_BRACKET})) {
			auto index = parseExpression();
			consume(Token::TokenType::RIGHT_BRACKET, "Expect ']' after indexation.");
			return std::make_unique<ExprNode>("indexation", std::make_unique<VarDeclNode>("variable", previous().value, nullptr), std::move(index));
		}
		if (match({Token::TokenType::MINUS})) {
			if (peek().type == Token::TokenType::GREATER)
			{
				if (match({Token::TokenType::IDENTIFIER})) {
                    return std::make_unique<ExprNode>("struct_member", 
                        std::make_unique<VarDeclNode>("variable", previous().value, nullptr), 
                        nullptr);
                } else {
                    error(peek(), "Expect member name after '->'.");
                }
			}
		}
        return std::make_unique<ExprNode>("variable", std::make_unique<VarDeclNode>("variable", previous().value, nullptr), nullptr);
    }

    if (match({Token::TokenType::LEFT_PAREN})) {
        auto expr = parseExpression();
        consume(Token::TokenType::RIGHT_PAREN, "Expect ')' after expression.");
        return expr;
    }

	/* Strings sir? */
	if (match({Token::TokenType::STRING})) {
		return std::make_unique<ExprNode>("literal", std::make_unique<VarDeclNode>("string", previous().value, nullptr), nullptr);
	}

    error(peek(), "Expect expression.");
    return nullptr;
}

} // namespace EntS
