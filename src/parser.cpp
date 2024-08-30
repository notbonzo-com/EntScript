#include "parser.hpp"
#include "ast.hpp"
#include <stdexcept>
#include <sstream>
#include <stack>
#include <set>

extern void printFatal(const char* str);
extern void printError(const char* str);

namespace EntS {

Parser::Parser(const std::vector<Token>& tokens) : tokens(tokens), current(0) {}

void Parser::enterScope() {
    scopedStack.push(std::set<std::string>());
}

void Parser::exitScope() {
    if (!scopedStack.empty()) {
        scopedStack.pop();
    } else {
        printError("Attempt to exit scope when no scope exists");
    }
}

void Parser::addScopedVariable(const std::string& name) {
    if (!scopedStack.empty()) {
        scopedStack.top().insert(name);
    } else {
        printError("No scope to add variable to");
    }
}

bool Parser::isVariableDeclared(const std::string& name) const {
	std::stack<std::set<std::string>> tempStack = scopedStack;

	while (!tempStack.empty()) {
		const std::set<std::string>& currentSet = tempStack.top();
		if (currentSet.find(name) != currentSet.end()) {
			return true;
		}
		tempStack.pop();
	}
	return false;
}

const Token& Parser::consume() {
    if (current >= tokens.size()) {
        printError("Unexpected end of input");
    }
    return tokens[current++];
}

const Token& Parser::peek(int offset) const {
    if (current + offset >= tokens.size()) {
        printError("Unexpected end of input");
    }
    return tokens[current + offset];
}

const Token& Parser::previous() const {
    if (current == 0) {
        printError("No previous token available");
    }
    return tokens[current - 1];
}

void Parser::expect(Token::TokenType type, const std::string& message) {
    if (!check(type)) {
        error(peek(), message);
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

bool Parser::isStructMember(const std::string& structName, const std::string& memberName) {
    auto it = structDefinitions.find(structName);
    if (it != structDefinitions.end()) {
        const auto& members = it->second;
        return std::find(members.begin(), members.end(), memberName) != members.end();
    }
    return false;
}

std::string Parser::resolveTypedef(const std::string& type) const {
    auto it = typedefs.find(type);
    if (it != typedefs.end()) {
        return resolveTypedef(it->second);
    }
    return type;
}

void Parser::error(const Token& token, const std::string& message) {
    std::stringstream ss;
    ss << "[" << token.line << "/" << token.column << ":" << token.toString() << "]: " << message << std::endl;
    printError(ss.str().c_str());
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
            } else if (tokens.size() > 2 && peek(2).type == Token::TokenType::ASSIGN) {
                statements.push_back(parseGlobalVarDecl());
            } else {
                error(peek(2), "Expect ';' or '=' after type declaration.");
            }
        } else {
            error(peek(), "Expect statement.");
        }
    }
    return std::make_shared<ProgramNode>(std::move(statements));
}

ASTNodePtr Parser::parseHeader() {
    std::vector<ASTNodePtr> prototypes;
    expect(Token::TokenType::LEFT_BRACE, "Expect '{' after 'header' keyword.");
    while (!check(Token::TokenType::EOF_TOKEN) && !check(Token::TokenType::RIGHT_BRACE)) {
        if (check(Token::TokenType::FUNCTION)) {
            prototypes.push_back(parseFunctionPrototype());
        } else if (check(Token::TokenType::TYPEDEF)) {
            prototypes.push_back(parseTypedef());
        } else if (tokens.size() > 2 && peek(2).type == Token::TokenType::ASSIGN) {
            error(peek(2), "Header does not allow for global variable initialization.");
        } else if (isType(peek().value)) {
            prototypes.push_back(parseGlobalVarDecl());
        } else {
            error(peek(), "Expect statement. (header)");
        }
    }
    expect(Token::TokenType::RIGHT_BRACE, "Expect '}' after header.");
    expect(Token::TokenType::SEMICOLON, "Expect ';' after header.");

    return std::make_shared<HeaderNode>(std::move(prototypes));
}

ASTNodePtr Parser::parseFunctionPrototype() {
    std::string name;
    std::string return_value;
    std::vector<ASTNodePtr> parameters;

    expect(Token::TokenType::FUNCTION, "Expect 'function' keyword.");
    name = consume().value;
    existing_functions.push_back(name);
    prototypes.push_back(name);

    expect(Token::TokenType::LEFT_PAREN, "Expect '(' after function name.");
    if (!check(Token::TokenType::RIGHT_PAREN)) {
        std::string type;
        type = consume().value;
        if (!isType(type)) {
            error(peek(), "Expect function parameter type.");
        }
        std::string paramName = consume().value;
        parameters.push_back(std::make_shared<ParameterNode>(type, paramName));
        while (match({Token::TokenType::COMMA})) {
            type = consume().value;
            if (!isType(type)) {
                error(peek(), "Expect function parameter type.");
            }
            paramName = consume().value;
            parameters.push_back(std::make_shared<ParameterNode>(type, paramName));
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

    return std::make_shared<FunctionPrototypeNode>(name, return_value, std::move(parameters));
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
    if (peek().type != Token::TokenType::IDENTIFIER) {
        error(peek(), "Anonymous structs are not supported.");
    }
    new_type = consume().value;
    if (isType(new_type)) {
        error(previous(), "Cannot redefine type");
    }
    expect(Token::TokenType::SEMICOLON, "Expect ';' after typedef.");

    if (std::holds_alternative<std::string>(old_type)) {
        typedefs[new_type] = resolveTypedef(std::get<std::string>(old_type));
    } else {
        typedefs[new_type] = "struct";
    }

    existing_types.push_back(new_type);
    return std::make_shared<TypedefNode>(new_type, std::move(old_type));
}

ASTNodePtr Parser::parseStruct() {
    std::vector<ASTNodePtr> members;
    std::vector<std::string> memberNames;

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
        members.push_back(std::make_shared<ParameterNode>(type, name));
        memberNames.push_back(name);
        expect(Token::TokenType::SEMICOLON, "Expect ';' after struct member.");
    }
    expect(Token::TokenType::RIGHT_BRACE, "Expect '}' after struct members.");

    if (peek().type != Token::TokenType::IDENTIFIER) {
        error(peek(), "Anonymous structs are not supported.");
    }

    structDefinitions[peek().value] = memberNames;

    return std::make_shared<StructNode>(std::move(members));
}

ASTNodePtr Parser::parseFunction() {
    std::string name;
    std::vector<ASTNodePtr> parameters;
    std::string return_value;
    ASTNodePtr body;

    expect(Token::TokenType::FUNCTION, "Expect 'function' keyword.");
    name = consume().value;
    if (existing_functions.end() != std::find(existing_functions.begin(), existing_functions.end(), name)) {
        if (prototypes.end() == std::find(prototypes.begin(), prototypes.end(), name)) {
            error(previous(), "Duplicated function name.");
        }
    }
    existing_functions.push_back(name);

    // Enter a new scope for function parameters
    enterScope();

    expect(Token::TokenType::LEFT_PAREN, "Expect '(' after function name.");
    if (!check(Token::TokenType::RIGHT_PAREN)) {
        std::string type;
        type = consume().value;
        if (!isType(type)) {
            error(peek(), "Expect function parameter type.");
        }
        std::string paramName = consume().value;
        addScopedVariable(paramName);
        parameters.push_back(std::make_shared<ParameterNode>(type, paramName));
        while (match({Token::TokenType::COMMA})) {
            type = consume().value;
            if (!isType(type)) {
                error(peek(), "Expect function parameter type.");
            }
            paramName = consume().value;
            addScopedVariable(paramName);
            parameters.push_back(std::make_shared<ParameterNode>(type, paramName));
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

    // Exit the scope at the end of the function
    exitScope();

    expect(Token::TokenType::SEMICOLON, "Expect ';' after function declaration.");
    return std::make_shared<FunctionNode>(name, return_value, std::move(parameters), std::move(body));
}

ASTNodePtr Parser::parseBlock() {
    std::vector<ASTNodePtr> statements;

    // Enter a new scope for the block
    enterScope();

    while (!check(Token::TokenType::RIGHT_BRACE) && !check(Token::TokenType::EOF_TOKEN)) {
        if (isType(peek().value) && peek(1).type == Token::TokenType::IDENTIFIER) {
            if (peek(2).type == Token::TokenType::SEMICOLON) {
                statements.push_back(parseVarDecl());
            } else if (tokens.size() > 2 && peek(2).type == Token::TokenType::ASSIGN) {
                statements.push_back(parseVarDeclAssign());
            } else {
                error(peek(2), "Expect ';' or '=' after variable declaration.");
            }
        }

        else if (check(Token::TokenType::WHILE)) {
            statements.push_back(parseWhile());
        }

        else if (check(Token::TokenType::IF)) {
            statements.push_back(parseIf());
        }

        else if (match({Token::TokenType::RETURN})) {
            ASTNodePtr expr = parseExpression();
            expect(Token::TokenType::SEMICOLON, "Expect ';' after return statement.");
            statements.push_back(std::make_shared<ReturnNode>(std::move(expr)));
        }

        else if (match({Token::TokenType::CONTINUE})) {
            statements.push_back(std::make_shared<ContinueNode>());
            expect(Token::TokenType::SEMICOLON, "Expect ';' after continue statement.");
        }

        else if (match({Token::TokenType::BREAK})) {
            statements.push_back(std::make_shared<BreakNode>());
            expect(Token::TokenType::SEMICOLON, "Expect ';' after break statement.");
        }

        else if (check(Token::TokenType::SWITCH)) {
            statements.push_back(parseSwitch());
        }

        else if (check(Token::TokenType::IDENTIFIER)) {
            if (isVariableDeclared(peek().value)) {
                if (peek(1).type == Token::TokenType::PLUS && peek(2).type == Token::TokenType::PLUS) {
                    statements.push_back(std::make_shared<IncrementNode>(peek().value));
                    consume(); consume(); consume(); // consume identifier and '++'
                    expect(Token::TokenType::SEMICOLON, "Expect ';' after increment statement.");
                }
                else if (peek(1).type == Token::TokenType::MINUS && peek(2).type == Token::TokenType::MINUS) {
                    statements.push_back(std::make_shared<DecrementNode>(peek().value));
                    consume(); consume(); consume(); // consume identifier and '--'
                    expect(Token::TokenType::SEMICOLON, "Expect ';' after decrement statement.");
                }
                else if (peek(1).type == Token::TokenType::ASSIGN) {
                    std::string name = consume().value;
                    expect(Token::TokenType::ASSIGN, "Expect '=' after variable name.");
                    ASTNodePtr expr = parseExpression();
                    expect(Token::TokenType::SEMICOLON, "Expect ';' after assignment.");
                    statements.push_back(std::make_shared<AssignNode>(name, std::move(expr)));
                }
                else if (peek(1).type == Token::TokenType::LEFT_BRACKET) {
                    std::string name = consume().value;
                    expect(Token::TokenType::LEFT_BRACKET, "Expect '[' after variable name.");
                    ASTNodePtr index = parseExpression();
                    expect(Token::TokenType::RIGHT_BRACKET, "Expect ']' after index.");
                    expect(Token::TokenType::ASSIGN, "Expect '=' after index.");
                    ASTNodePtr value = parseExpression();
                    expect(Token::TokenType::SEMICOLON, "Expect ';' after indexation assignment.");
                    statements.push_back(std::make_shared<IndexationAssignNode>(name, std::move(index), std::move(value)));
                }
                else if (peek(1).type == Token::TokenType::MINUS && peek(2).type == Token::TokenType::GREATER) {
                    std::string name = consume().value;
                    ASTNodePtr current = std::make_shared<IdentifierNode>(name);
                    expect(Token::TokenType::MINUS, "Expect '->' after parent name.");
                    expect(Token::TokenType::GREATER, "Expect '->' after parent name.");
                    std::string memberName = consume().value;
                    current = std::make_shared<StructMemberAccessNode>(std::move(current), memberName);

                    while (match({Token::TokenType::MINUS}) && match({Token::TokenType::GREATER})) {
                        memberName = consume().value;
                        current = std::make_shared<StructMemberAccessNode>(std::move(current), memberName);
                    }

                    expect(Token::TokenType::ASSIGN, "Expect '=' after struct member name.");
                    ASTNodePtr expr = parseExpression();
                    expect(Token::TokenType::SEMICOLON, "Expect ';' after struct member assignment.");

                    statements.push_back(std::make_shared<StructMemberAssignNode>(std::move(current), std::move(expr)));
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
            statements.push_back(std::make_shared<MemoryAssignNode>(name, std::move(value)));
        } else {
            ASTNodePtr expr = parseExpression();
            statements.push_back(std::move(expr));
            expect(Token::TokenType::SEMICOLON, "Expect ';' after expression.");
        }
    }

    // Exit the scope when the block ends
    exitScope();

    return std::make_shared<BlockNode>(std::move(statements));
}

ASTNodePtr Parser::parseSwitch() {
    ASTNodePtr condition;
    std::vector<ASTNodePtr> cases;
    expect(Token::TokenType::SWITCH, "Expect 'switch' keyword.");
    expect(Token::TokenType::LEFT_PAREN, "Expect '(' after 'switch' keyword.");
    condition = parseExpression();
    expect(Token::TokenType::RIGHT_PAREN, "Expect ')' after 'switch' condition.");
    expect(Token::TokenType::LEFT_BRACE, "Expect '{' after 'switch' condition.");

    while (!check(Token::TokenType::RIGHT_BRACE)) {
        if (check(Token::TokenType::DEFAULT)) {
            cases.push_back(parseDefault());
            break;
        }
        cases.push_back(parseCase());
    }

    expect(Token::TokenType::RIGHT_BRACE, "Expect '}' after 'switch' body.");
    expect(Token::TokenType::SEMICOLON, "Expect ';' after 'switch' body.");
    return std::make_shared<SwitchNode>(std::move(condition), std::move(cases));
}

ASTNodePtr Parser::parseCase() {
    expect(Token::TokenType::CASE, "Expect 'case' keyword.");
    expect(Token::TokenType::LEFT_PAREN, "Expect '(' after 'case' keyword.");
    ASTNodePtr condition = parseExpression();
    expect(Token::TokenType::RIGHT_PAREN, "Expect ')' after 'case' condition.");
    expect(Token::TokenType::LEFT_BRACE, "Expect '{' after 'case' condition.");
    ASTNodePtr body = parseBlock();
    expect(Token::TokenType::RIGHT_BRACE, "Expect '}' after 'case' block.");
    expect(Token::TokenType::SEMICOLON, "Expect ';' after 'case' block.");
    return std::make_shared<CaseNode>(std::move(condition), std::move(body));
}

ASTNodePtr Parser::parseDefault() {
    expect(Token::TokenType::DEFAULT, "Expect 'default' keyword.");
    expect(Token::TokenType::LEFT_BRACE, "Expect '{' after 'default' keyword.");
    ASTNodePtr body = parseBlock();
    expect(Token::TokenType::RIGHT_BRACE, "Expect '}' after 'default' block.");
    expect(Token::TokenType::SEMICOLON, "Expect ';' after 'default' block.");
    return std::make_shared<DefaultNode>(std::move(body));
}

ASTNodePtr Parser::parseVarDecl() {
    std::string type;
    std::string name;
    
    type = consume().value;
    if (!isType(type)) {
        error(peek(), "Expect variable type.");
    }
    bool initByAddr = false;
    if (check(Token::TokenType::LEFT_BRACKET)) {
        expect(Token::TokenType::LEFT_BRACKET, "Expect '[' after variable type.");
        initByAddr = true;
    }

    name = consume().value;
    if (isVariableDeclared(name)) {
        error(peek(), "Duplicated variable name.");
    }
    if (initByAddr) {
        expect(Token::TokenType::RIGHT_BRACKET, "Expect ']' after variable name.");
    }

    addScopedVariable(name);
    expect(Token::TokenType::SEMICOLON, "Expect ';' after variable declaration.");
    return std::make_shared<VarDeclNode>(type, name, initByAddr);
}

ASTNodePtr Parser::parseVarDeclAssign() {
    std::string type;
    std::string name;
    ASTNodePtr initializer;

    type = consume().value;
    if (!isType(type)) {
        error(peek(), "Expect variable type.");
    }

    bool initByAddr = false;
    if (check(Token::TokenType::LEFT_BRACKET)) {
        expect(Token::TokenType::LEFT_BRACKET, "Expect '[' after variable type.");
        initByAddr = true;
    }

    name = consume().value;
    if (isVariableDeclared(name)) {
        error(peek(), "Duplicated variable name.");
    }

    if (initByAddr) {
        expect(Token::TokenType::RIGHT_BRACKET, "Expect ']' after variable name.");
    }

    addScopedVariable(name);

    expect(Token::TokenType::ASSIGN, "Expect '=' after variable name.");
    initializer = parseExpression();
    if (initializer == nullptr) {
        error(peek(), "ParseExpression returned nullptr.");
    }

    expect(Token::TokenType::SEMICOLON, "Expect ';' after variable declaration.");
    return std::make_shared<VarDeclAssignNode>(type, name, std::move(initializer), initByAddr);
}

ASTNodePtr Parser::parseGlobalVarDecl() {
    std::string type;
    std::string name;

    type = consume().value;
    if (!isType(type)) {
        error(previous(), "Expect global variable type.");
    }

    bool initByAddr = false;
    if (check(Token::TokenType::LEFT_BRACKET)) {
        expect(Token::TokenType::LEFT_BRACKET, "Expect '[' after variable type.");
        initByAddr = true;
    }

    name = consume().value;
    if (isVariableDeclared(name)) {
        error(previous(), "Duplicated global variable name.");
    }

    if (initByAddr) {
        expect(Token::TokenType::RIGHT_BRACKET, "Expect ']' after variable name.");
    }

    addScopedVariable(name);
    expect(Token::TokenType::SEMICOLON, "Expect ';' after global variable declaration.");
    return std::make_shared<GlobalVarDeclNode>(type, name, initByAddr);
}

ASTNodePtr Parser::parseGlobalVarDeclAssign() {
    std::string type;
    std::string name;
    ASTNodePtr initializer;

    type = consume().value;
    if (!isType(type)) {
        error(previous(), "Expect global variable type.");
    }

    bool initByAddr = false;
    if (check(Token::TokenType::LEFT_BRACKET)) {
        expect(Token::TokenType::LEFT_BRACKET, "Expect '[' after variable type.");
        initByAddr = true;
    }

    name = consume().value;
    if (isVariableDeclared(name)) {
        error(previous(), "Duplicated global variable name.");
    }
    if (initByAddr) {
        expect(Token::TokenType::RIGHT_BRACKET, "Expect ']' after variable name.");
    }

    addScopedVariable(name);

    expect(Token::TokenType::ASSIGN, "Expect '=' after variable name.");
    initializer = parseExpression();
    if (initializer == nullptr) {
        error(peek(), "ParseExpression returned nullptr.");
    }

    expect(Token::TokenType::SEMICOLON, "Expect ';' after global variable declaration.");
    return std::make_shared<GlobalVarDeclAssignNode>(type, name, std::move(initializer), initByAddr);
}

ASTNodePtr Parser::parseWhile() {
    expect(Token::TokenType::WHILE, "Expect 'while' keyword.");
    expect(Token::TokenType::LEFT_PAREN, "Expect '(' after 'while' keyword.");
    ASTNodePtr condition = parseExpression();
    expect(Token::TokenType::RIGHT_PAREN, "Expect ')' after 'while' condition.");
    expect(Token::TokenType::LEFT_BRACE, "Expect '{' after 'while' condition.");
    ASTNodePtr body = parseBlock();
    expect(Token::TokenType::RIGHT_BRACE, "Expect '}' after 'while' block.");
    expect(Token::TokenType::SEMICOLON, "Expect ';' after 'while' block.");
    return std::make_shared<WhileNode>(std::move(condition), std::move(body));
}

ASTNodePtr Parser::parseIf() {
    expect(Token::TokenType::IF, "Expect 'if' keyword.");
    expect(Token::TokenType::LEFT_PAREN, "Expect '(' after 'if' keyword.");
    ASTNodePtr condition = parseExpression();
    expect(Token::TokenType::RIGHT_PAREN, "Expect ')' after 'if' condition.");
    expect(Token::TokenType::LEFT_BRACE, "Expect '{' after 'if' condition.");
    ASTNodePtr then_branch = parseBlock();
    expect(Token::TokenType::RIGHT_BRACE, "Expect '}' after 'if' block.");
    ASTNodePtr else_branch = nullptr;
    if (match({Token::TokenType::ELSE})) {
        if (check(Token::TokenType::IF)) {
            else_branch = parseIf();
        } else {
            expect(Token::TokenType::LEFT_BRACE, "Expect '{' after 'else' keyword.");
            else_branch = parseBlock();
            expect(Token::TokenType::RIGHT_BRACE, "Expect '}' after 'else' block.");
        }
    }
    expect(Token::TokenType::SEMICOLON, "Expect ';' after 'if' block.");
    return std::make_shared<IfNode>(std::move(condition), std::move(then_branch), std::move(else_branch));
}

ASTNodePtr Parser::parseFunctionCall() {
    std::string name = consume().value;
    expect(Token::TokenType::LEFT_PAREN, "Expect '(' after function name.");
    std::vector<ASTNodePtr> args;
    if (!check(Token::TokenType::RIGHT_PAREN)) {
        args.push_back(parseExpression());
        while (match({Token::TokenType::COMMA})) {
            args.push_back(parseExpression());
        }
    }
    expect(Token::TokenType::RIGHT_PAREN, "Expect ')' after function arguments.");
    return std::make_shared<FunctionCallNode>(name, std::move(args));
}

ASTNodePtr Parser::parseExpression() {
    return parseLogicalOr();
}

ASTNodePtr Parser::parseParenExpression() {
    expect(Token::TokenType::LEFT_PAREN, "Expect '(' at start of parenthesized expression.");
    ASTNodePtr expr = parseExpression();
    expect(Token::TokenType::RIGHT_PAREN, "Expect ')' after parenthesized expression.");
    return expr;
}

ASTNodePtr Parser::parseLogicalOr() {
    ASTNodePtr left = parseLogicalAnd();

    while (match({Token::TokenType::PIPE})) {
        std::string op = previous().toSymbol();
        ASTNodePtr right = parseLogicalAnd();
        left = std::make_shared<ExpressionNode>(std::move(left), op, std::move(right));
    }

    return left;
}

ASTNodePtr Parser::parseLogicalAnd() {
    ASTNodePtr left = parseEquality();

    while (match({Token::TokenType::AMPERSAND})) {
        std::string op = previous().toSymbol();
        ASTNodePtr right = parseEquality();
        left = std::make_shared<ExpressionNode>(std::move(left), op, std::move(right));
    }

    return left;
}

ASTNodePtr Parser::parseEquality() {
    ASTNodePtr left = parseRelational();

    while (match({Token::TokenType::EQUAL, Token::TokenType::NOT_EQUAL})) {
        std::string op = previous().toSymbol();
        ASTNodePtr right = parseRelational();
        left = std::make_shared<ExpressionNode>(std::move(left), op, std::move(right));
    }

    return left;
}

ASTNodePtr Parser::parseRelational() {
    ASTNodePtr left = parseBitwise();

    while (match({Token::TokenType::GREATER, Token::TokenType::GREATER_EQUAL, Token::TokenType::LESS, Token::TokenType::LESS_EQUAL})) {
        std::string op = previous().toSymbol();
        ASTNodePtr right = parseBitwise();
        left = std::make_shared<ExpressionNode>(std::move(left), op, std::move(right));
    }

    return left;
}

ASTNodePtr Parser::parseBitwise() {
    ASTNodePtr left = parseAdditive();

    while ((peek().type == Token::TokenType::AMPERSAND && peek(1).type == Token::TokenType::AMPERSAND) ||
            peek().type == Token::TokenType::PIPE && peek(1).type == Token::TokenType::PIPE) {
        std::string op = previous().toSymbol();
        consume();
        ASTNodePtr right = parseAdditive();
        left = std::make_shared<ExpressionNode>(std::move(left), op, std::move(right));
    }

    return left;
}

ASTNodePtr Parser::parseAdditive() {
    ASTNodePtr left = parseMultiplicative();

    while (match({Token::TokenType::PLUS, Token::TokenType::MINUS})) {
        std::string op = previous().toSymbol();
        ASTNodePtr right = parseMultiplicative();
        left = std::make_shared<ExpressionNode>(std::move(left), op, std::move(right));
    }

    return left;
}

ASTNodePtr Parser::parseMultiplicative() {
    ASTNodePtr left = parseUnary();

    while (match({Token::TokenType::STAR, Token::TokenType::SLASH})) {
        std::string op = previous().toSymbol();
        ASTNodePtr right = parseUnary();
        left = std::make_shared<ExpressionNode>(std::move(left), op, std::move(right));
    }

    return left;
}

ASTNodePtr Parser::parseUnary() {
    if (match({Token::TokenType::EXCLAMATION, Token::TokenType::MINUS})) {
        std::string op = previous().toSymbol();
        ASTNodePtr right = parseUnary();
        return std::make_shared<ExpressionNode>(nullptr, op, std::move(right));
    }

    return parsePrimary();
}

ASTNodePtr Parser::parsePrimary() {
    if (match({Token::TokenType::NUMBER})) {
        return parseLiteral();
    }

    if (check(Token::TokenType::LEFT_PAREN)) {
        return parseParenExpression();
    }

    if (match({Token::TokenType::IDENTIFIER})) {
        return parseIdentifier();
    }

    if (match({Token::TokenType::STRING})) {
        return parseStringLiteral();
    }

    if (match({Token::TokenType::LEFT_BRACKET})) {  // For memory addressing `[variable]`
        return parseMemoryAddressing();
    }

    error(peek(), "Expect expression.");
    __builtin_unreachable();
}

ASTNodePtr Parser::parseIdentifier() {
    std::string name = previous().value;
    if (match({Token::TokenType::LEFT_BRACKET})) {
        return parseIndexing(name);
    } else if (match({Token::TokenType::MINUS}) && match({Token::TokenType::GREATER})) {
        return parseStructMemberAccess(name);
    } else if (isVariableDeclared(name)) {
        return std::make_shared<IdentifierNode>(name);
    } else if (existing_functions.end() != std::find(existing_functions.begin(), existing_functions.end(), name)) {
        current--;
        return parseFunctionCall();
    } else {
        error(peek(), "Undefined variable or function name.");
    }
    __builtin_unreachable();
}

ASTNodePtr Parser::parseLiteral() {
    Token token = previous();
    return std::make_shared<LiteralNode>(token.value);
}

ASTNodePtr Parser::parseStringLiteral() {
    Token token = previous();
    return std::make_shared<StringLiteralNode>(token.value);
}

ASTNodePtr Parser::parseIndexing(const std::string& name) {
    ASTNodePtr index = parseExpression();
    if (!isVariableDeclared(tokens[current - 3].value)) {
        error(tokens[current - 3], "Undefined variable name.");
    }
    expect(Token::TokenType::RIGHT_BRACKET, "Expect ']' after array index.");
    return std::make_shared<IndexNode>(name, std::move(index));
}

ASTNodePtr Parser::parseMemoryAddressing() {
    std::string name = consume().value;
    if (!isVariableDeclared(name)) {
        error(previous(), "Undefined variable name.");
    }
    expect(Token::TokenType::RIGHT_BRACKET, "Expect ']' after variable name.");
    return std::make_shared<MemoryAddressNode>(name);
}

ASTNodePtr Parser::parseStructMemberAccess(const std::string& structName) {
    ASTNodePtr current = std::make_shared<IdentifierNode>(structName);

    std::string memberName = consume().value;
    current = std::make_shared<StructMemberAccessNode>(std::move(current), memberName);

    while (match({Token::TokenType::MINUS}) && match({Token::TokenType::GREATER})) {
        memberName = consume().value;

        if (!isStructMember(structName, memberName)) {
            error(previous(), "Undefined struct member.");
        }

        current = std::make_shared<StructMemberAccessNode>(std::move(current), memberName);
    }

    return current;
}

} // namespace EntS