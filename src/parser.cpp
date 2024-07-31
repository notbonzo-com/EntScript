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
		} else if (check(Token::TokenType::FUNCTION)) {
			statements.push_back(parseFunction());
		} else if (check(Token::TokenType::TYPEDEF)) {
			statements.push_back(parseTypedef());
		} else if (isType(peek().value)) {
			if (peek(2).type == Token::TokenType::SEMICOLON) {
				statements.push_back(parseGlobalVarDecl());
			} else if (peek(2).type == Token::TokenType::ASSIGN) {
				statements.push_back(parseGlobalVarDecl());
			} else {
				error(peek(2), "Expect ';' or '=' after type declaration.");
			}
		} else if (peek().type == Token::TokenType::INLINE_ASM) {
			statements.push_back(parseAsm());
		} else {
			error(peek(), "Expect statement.");
		}
	}
	return std::make_unique<ProgramNode>(std::move(statements));
}

ASTNodePtr Parser::parseHeader() {
	std::vector<ASTNodePtr> prototypes;
	while (!check(Token::TokenType::EOF_TOKEN) && !check(Token::TokenType::RIGHT_BRACE)) {
		if (check(Token::TokenType::FUNCTION)) {
			prototypes.push_back(parseFunctionPrototype());
		} else if (check(Token::TokenType::TYPEDEF)) {
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

ASTNodePtr Parser::parseTypedef() {
	std::string new_type;
	std::variant<ASTNodePtr, std::string> old_type;

	expect(Token::TokenType::TYPEDEF, "Expect 'typedef' keyword.");
	if (check(Token::TokenType::STRUCT)) {
		old_type = parseStruct();
	} else {
		std::string old_typel = consume().value;
		if (!isType(old_typel)) {
			error(peek(), "Expect typedef type.");
		}
		old_type = old_typel;
	}

	new_type = consume().value;
	if (isType(new_type)) {
		error(previous(), "Can not redefine type");
	}
	expect(Token::TokenType::SEMICOLON, "Expect ';' after typedef.");
	return std::make_unique<TypedefNode>(new_type, std::move(old_type));
}

ASTNodePtr Parser::parseStruct() {
	std::vector<ASTNodePtr> members;
	expect(Token::TokenType::STRUCT, "Expect 'struct' keyword.");
	expect(Token::TokenType::LEFT_BRACE, "Expect '{' after 'struct' keyword.");

	std::vector<std::string> used_names;
	while (!check(Token::TokenType::RIGHT_BRACE) && !check(Token::TokenType::EOF_TOKEN)) {
		std::string type;
		std::string name;

		type = consume().value;
		if (!isType(type)) {
			error(previous(), "Expect struct member type.");
		}

		name = consume().value;
		if (used_names.end() != std::find(used_names.begin(), used_names.end(), name)) {
			error(previous(), "Duplicated struct member name.");
		}
		used_names.push_back(name);
		members.push_back(std::make_unique<ParameterNode>(type, name));
	}
	expect(Token::TokenType::RIGHT_BRACE, "Expect '}' after struct members.");
	return std::make_unique<StructNode>(std::move(members));
}

ASTNodePtr Parser::parseGlobalVarDecl() {
	std::string type;
	std::string name;

	type = consume().value;
	if (!isType(type)) {
		error(previous(), "Expect global variable type.");
	}

	name = consume().value;
	if (existing_variables.end() != std::find(existing_variables.begin(), existing_variables.end(), name)) {
		error(previous(), "Duplicated global variable name.");
	}
	existing_variables.push_back(name);
	expect(Token::TokenType::SEMICOLON, "Expect ';' after global variable declaration.");
	return std::make_unique<GlobalVarDeclNode>(type, name);
}

ASTNodePtr Parser::parseGlobalVarDeclAssign() {
	std::string type;
	std::string name;
	ASTNodePtr initializer;

	type = consume().value;
	if (!isType(type)) {
		error(previous(), "Expect global variable type.");
	}

	name = consume().value;
	if (existing_variables.end() != std::find(existing_variables.begin(), existing_variables.end(), name)) {
		error(previous(), "Duplicated global variable name.");
	}
	existing_variables.push_back(name);
	initializer = parseExpression();
	if (initializer == nullptr) {
		error(peek(), "ParseExpression returned nullptr.");
	}
	expect(Token::TokenType::SEMICOLON, "Expect ';' after global variable declaration.");
	return std::make_unique<GlobalVarDeclAssignNode>(type, name, std::move(initializer));
}

ASTNodePtr Parser::parseFunction() {
	std::string name;
	std::vector<ASTNodePtr> parameters;
	std::string return_value;
	ASTNodePtr body;

	expect(Token::TokenType::FUNCTION, "Expect 'function' keyword.");
	name = consume().value;
	if (existing_functions.end() != std::find(existing_functions.begin(), existing_functions.end(), name)) {
		error(previous(), "Duplicated function name.");
	}
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

	expect(Token::TokenType::MINUS, "Expect '->' after function declaration.");
	expect(Token::TokenType::GREATER, "Expect '->' after function declaration.");

	return_value = consume().value;
	if (!isType(return_value)) {
		error(peek(), "Expect function return type.");
	}

	expect(Token::TokenType::LEFT_BRACE, "Expect '{' after function declaration.");
	body = parseBlock();
	expect(Token::TokenType::RIGHT_BRACE, "Expect '}' after function body.");
	expect(Token::TokenType::SEMICOLON, "Expect ';' after function declaration.");
	return std::make_unique<FunctionNode>(name, return_value, std::move(parameters), std::move(body));
}

ASTNodePtr Parser::parseAsm() {
	std::string asm_source;

	expect(Token::TokenType::INLINE_ASM, "Expect 'asm' keyword.");
	expect(Token::TokenType::LEFT_BRACE, "Expect '{' after 'asm' keyword.");
	int depth = 0;
	while (!check(Token::TokenType::RIGHT_BRACE) || !depth == 0) {
		if (check(Token::TokenType::LEFT_BRACE)) {
			depth++;
			asm_source += '{';
			continue;
		} else if (check(Token::TokenType::RIGHT_BRACE)) {
			depth--;
			asm_source += '}';
			continue;
		}
		asm_source += consume().value;
	}

	expect(Token::TokenType::RIGHT_BRACE, "Expect '}' after inline asm block.");
	expect(Token::TokenType::SEMICOLON, "Expect ';' after inline asm block.");
	return std::make_unique<AsmNode>(asm_source);
}

ASTNodePtr Parser::parseBlock() {
    std::vector<ASTNodePtr> statements;

    while (!check(Token::TokenType::RIGHT_BRACE) && !check(Token::TokenType::EOF_TOKEN)) {
        
        // variable decl
        if (isType(peek().value) && peek(1).type == Token::TokenType::IDENTIFIER) {
            if (peek(2).type == Token::TokenType::SEMICOLON) {
                statements.push_back(parseVarDecl());
                consume();
            } else if (peek(2).type == Token::TokenType::ASSIGN) {
                statements.push_back(parseVarDeclAssign());
                expect(Token::TokenType::SEMICOLON, "Expect ';' after variable declaration.");
            } else {
                error(peek(2), "Expect ';' or '=' after variable declaration.");
            }
        }

        // while
        else if (check(Token::TokenType::WHILE)) {
            statements.push_back(parseWhile());
        }

        // if
        else if (check(Token::TokenType::IF)) {
            statements.push_back(parseIf());
        }

        // return
        else if (match({Token::TokenType::RETURN})) {
            ASTNodePtr expr = parseExpression();
            expect(Token::TokenType::SEMICOLON, "Expect ';' after return statement.");
            statements.push_back(std::make_unique<ReturnNode>(std::move(expr)));
        }

        // continue
        else if (match({Token::TokenType::CONTINUE})) {
            statements.push_back(std::make_unique<ContinueNode>());
            expect(Token::TokenType::SEMICOLON, "Expect ';' after continue statement.");
        }

        // break
        else if (match({Token::TokenType::BREAK})) {
            statements.push_back(std::make_unique<BreakNode>());
            expect(Token::TokenType::SEMICOLON, "Expect ';' after break statement.");
        }

        // identifier
        else if (check(Token::TokenType::IDENTIFIER)) {
            if (existing_variables.end() != std::find(existing_variables.begin(), existing_variables.end(), peek().value)) {
                
                // increment / decrement
                if (peek(1).type == Token::TokenType::PLUS && peek(2).type == Token::TokenType::PLUS) {
                    statements.push_back(std::make_unique<IncrementNode>(peek().value));
                    consume(); consume(); consume(); // consume identifier and '++'
                    expect(Token::TokenType::SEMICOLON, "Expect ';' after increment statement.");
                }
                else if (peek(1).type == Token::TokenType::MINUS && peek(2).type == Token::TokenType::MINUS) {
                    statements.push_back(std::make_unique<DecrementNode>(peek().value));
                    consume(); consume(); consume(); // consume identifier and '--'
                    expect(Token::TokenType::SEMICOLON, "Expect ';' after decrement statement.");
                }
                // normal assign
                else if (peek(1).type == Token::TokenType::ASSIGN) {
                    std::string name = consume().value;
                    expect(Token::TokenType::ASSIGN, "Expect '=' after variable name.");
                    ASTNodePtr expr = parseExpression();
                    expect(Token::TokenType::SEMICOLON, "Expect ';' after assignment.");
                    statements.push_back(std::make_unique<AssignNode>(name, std::move(expr)));
                }
                // indexation assign
                else if (peek(1).type == Token::TokenType::LEFT_BRACKET) {
                    std::string name = consume().value;
                    expect(Token::TokenType::LEFT_BRACKET, "Expect '[' after variable name.");
                    ASTNodePtr index = parseExpression();
                    expect(Token::TokenType::RIGHT_BRACKET, "Expect ']' after index.");
                    expect(Token::TokenType::ASSIGN, "Expect '=' after index.");
                    ASTNodePtr value = parseExpression();
                    expect(Token::TokenType::SEMICOLON, "Expect ';' after indexation assignment.");
                    statements.push_back(std::make_unique<IndexationAssignNode>(name, std::move(index), std::move(value)));
                } else {
                    error(peek(1), "Unexpected token after identifier.");
                }
            } 
            else if (existing_functions.end() != std::find(existing_functions.begin(), existing_functions.end(), peek().value)) {
                statements.push_back(parseFunctionCall());
                expect(Token::TokenType::SEMICOLON, "Expect ';' after function call.");
            } 
            else {
                error(peek(), "Undefined variable or function name.");
            }
        } else if (match({Token::TokenType::LEFT_BRACKET})) {
            std::string name = consume().value;
            expect(Token::TokenType::RIGHT_BRACKET, "Expect ']' after variable name.");
            expect(Token::TokenType::ASSIGN, "Expect '=' after index.");
            ASTNodePtr value = parseExpression();
            expect(Token::TokenType::SEMICOLON, "Expect ';' after memory assignment.");
            statements.push_back(std::make_unique<MemoryAssignNode>(name, std::move(value)));
        } else {
            ASTNodePtr expr = parseExpression();
            statements.push_back(std::move(expr));
            expect(Token::TokenType::SEMICOLON, "Expect ';' after expression.");
        }

    }

    return std::make_unique<BlockNode>(std::move(statements));
}

ASTNodePtr Parser::parseVarDecl() {
	std::string type;
	std::string name;
	
	type = consume().value;
	if (!isType(type)) {
		error(peek(), "Expect variable type.");
	}

	name = consume().value;
	if (existing_variables.end() != std::find(existing_variables.begin(), existing_variables.end(), name)) {
		error(peek(), "Duplicated variable name.");
	}

	existing_variables.push_back(name);
	expect(Token::TokenType::SEMICOLON, "Expect ';' after variable declaration.");
	return std::make_unique<VarDeclNode>(type, name);
}

ASTNodePtr Parser::parseVarDeclAssign() {
	std::string type;
	std::string name;
	ASTNodePtr initializer;

	type = consume().value;
	if (!isType(type)) {
		error(peek(), "Expect variable type.");
	}

	name = consume().value;
	if (existing_variables.end() != std::find(existing_variables.begin(), existing_variables.end(), name)) {
		error(peek(), "Duplicated variable name.");
	}

	existing_variables.push_back(name);

	expect(Token::TokenType::ASSIGN, "Expect '=' after variable name.");
	initializer = parseExpression();

	expect(Token::TokenType::SEMICOLON, "Expect ';' after variable declaration.");
	return std::make_unique<VarDeclAssignNode>(type, name, std::move(initializer));
}

} // namespace EntS