#ifndef AST_HPP
#define AST_HPP

#include <iostream>
#include <memory>
#include <string>
#include <optional>
#include <variant>
#include <vector>

namespace EntS {

enum class NodeType {
    Program,
    Function,
    VarDecl,
    VarDeclAssign,
    Assign,
    Return,
    Expression,
    If,
    While,
    For,
    Switch,
    Case,
    Default,
    Continue,
    Break,
    Block,
    Typedef,
    Struct,
    GlobalVarDecl,
    GlobalVarDeclAssign,
    Increment,
    Decrement,
    Header,
    Asm,
    FunctionPrototype,
    Call,
    Else,
    Elseif,
    Parameter
};

class ASTNode {
public:
    explicit ASTNode(NodeType type) : type(type) {}
    virtual ~ASTNode() = default;

    NodeType getType() const { return type; }

    virtual void print(int indent = 0) const = 0;

protected:
    void printIndent(int indent) const {
        for (int i = 0; i < indent; ++i) std::cout << "  ";
    }

private:
    NodeType type;
};

using ASTNodePtr = std::unique_ptr<ASTNode>;

class ProgramNode : public ASTNode {
public:
    ProgramNode(std::vector<ASTNodePtr> functions) : ASTNode(NodeType::Program), functions(std::move(functions)) {}

    void print(int indent = 0) const override {
        printIndent(indent);
        std::cout << "Program:" << std::endl;
        for (const auto& function : functions) {
            function->print(indent + 1);
        }
    }

private:
    std::vector<ASTNodePtr> functions;
};

class FunctionNode : public ASTNode {
public:
    FunctionNode(std::string_view name, std::string_view returnType, std::vector<ASTNodePtr> params, ASTNodePtr body)
        : ASTNode(NodeType::Function), name(name), returnType(returnType), params(std::move(params)), body(std::move(body)) {}

    void print(int indent = 0) const override {
        printIndent(indent);
        std::cout << "Function: " << name << std::endl;
        printIndent(indent + 1);
        std::cout << "return type: " << returnType << std::endl;
        printIndent(indent + 1);
        std::cout << "parameters:" << std::endl;
        for (const auto& param : params) {
            param->print(indent + 2);
        }
        printIndent(indent + 1);
        std::cout << "body:" << std::endl;
        body->print(indent + 2);
    }

private:
    std::string name;
    std::string returnType;
    std::vector<ASTNodePtr> params;
    ASTNodePtr body;
};

class VarDeclNode : public ASTNode {
public:
    VarDeclNode(std::string_view type, std::string_view name)
        : ASTNode(NodeType::VarDecl), type(type), name(name) {}

    void print(int indent = 0) const override {
        printIndent(indent);
        std::cout << "VarDecl: " << type << ": " << name << std::endl;
    }

private:
    std::string type;
    std::string name;
};

class VarDeclAssignNode : public ASTNode {
public:
    VarDeclAssignNode(std::string_view type, std::string_view name, ASTNodePtr expression)
        : ASTNode(NodeType::VarDeclAssign), type(type), name(name), expression(std::move(expression)) {}
    
    void print(int indent = 0) const override {
        printIndent(indent);
        std::cout << "VarDeclAssign: " << type << ": " << name << std::endl;
        expression->print(indent + 1);
    }

private:
    std::string type;
    std::string name;
    ASTNodePtr expression;
};

class AssignNode : public ASTNode {
public:
    AssignNode(std::string_view name, ASTNodePtr expression)
        : ASTNode(NodeType::Assign), name(name), expression(std::move(expression)) {}
    
    void print(int indent = 0) const override {
        printIndent(indent);
        std::cout << "Assign: " << name << std::endl;
        expression->print(indent + 1);
    }

private:
    std::string name;
    ASTNodePtr expression;
};

class ReturnNode : public ASTNode {
public:
    ReturnNode(ASTNodePtr expression)
        : ASTNode(NodeType::Return), expression(std::move(expression)) {}
    
    void print(int indent = 0) const override {
        printIndent(indent);
        std::cout << "Return" << std::endl;
        expression->print(indent + 1);
    }

private:
    ASTNodePtr expression;
};

class ExpressionNode : public ASTNode {
public:
    ExpressionNode(std::optional<ASTNodePtr> left, std::string_view op, std::optional<ASTNodePtr> right)
        : ASTNode(NodeType::Expression), left(std::move(left)), op(op), right(std::move(right)) {}

    void print(int indent = 0) const override {
        printIndent(indent);
        std::cout << "Expression: " << op << std::endl;
        if (left) {
            left->get()->print(indent + 1);
        }
        if (right) {
            right->get()->print(indent + 1);
        }
    }

private:
    std::optional<ASTNodePtr> left;
    std::string op;
    std::optional<ASTNodePtr> right;
};

class IfNode : public ASTNode {
public:
    IfNode(ASTNodePtr condition, ASTNodePtr body)
        : ASTNode(NodeType::If), condition(std::move(condition)), body(std::move(body)) {}

    void print(int indent = 0) const override {
        printIndent(indent);
        std::cout << "If" << std::endl;
        condition->print(indent + 1);
        body->print(indent + 1);
    }

private:
    ASTNodePtr condition;
    ASTNodePtr body;
};

class WhileNode : public ASTNode {
public:
    WhileNode(ASTNodePtr condition, ASTNodePtr body)
        : ASTNode(NodeType::While), condition(std::move(condition)), body(std::move(body)) {}

    void print(int indent = 0) const override {
        printIndent(indent);
        std::cout << "While" << std::endl;
        condition->print(indent + 1);
        body->print(indent + 1);
    }

private:
    ASTNodePtr condition;
    ASTNodePtr body;
};

class SwitchNode : public ASTNode {
public:
    SwitchNode(ASTNodePtr condition, std::vector<ASTNodePtr> cases)
        : ASTNode(NodeType::Switch), condition(std::move(condition)), cases(std::move(cases)) {}

    void print(int indent = 0) const override {
        printIndent(indent);
        std::cout << "Switch" << std::endl;
        condition->print(indent + 1);
        for (const auto& case_ : cases) {
            case_->print(indent + 1);
        }
    }

private:
    ASTNodePtr condition;
    std::vector<ASTNodePtr> cases;
};

class CaseNode : public ASTNode {
public:
    CaseNode(ASTNodePtr case_, ASTNodePtr body)
        : ASTNode(NodeType::Case), case_(std::move(case_)), body(std::move(body)) {}

    void print(int indent = 0) const override {
        printIndent(indent);
        std::cout << "Case" << std::endl;
        case_->print(indent + 1);
        body->print(indent + 1);
    }

private:
    ASTNodePtr case_;
    ASTNodePtr body;
};

class DefaultNode : public ASTNode {
public:
    DefaultNode(ASTNodePtr body)
        : ASTNode(NodeType::Default), body(std::move(body)) {}

    void print(int indent = 0) const override {
        printIndent(indent);
        std::cout << "Default" << std::endl;
        body->print(indent + 1);
    }

private:
    ASTNodePtr body;
};

class ContinueNode : public ASTNode {
public:
    void print(int indent = 0) const override {
        printIndent(indent);
        std::cout << "Continue" << std::endl;
    }
};

class BreakNode : public ASTNode {
public:
    void print(int indent = 0) const override {
        printIndent(indent);
        std::cout << "Break" << std::endl;
    }
};

class BlockNode : public ASTNode {
public:
    BlockNode(std::vector<ASTNodePtr> statements)
        : ASTNode(NodeType::Block), statements(std::move(statements)) {}

    void print(int indent = 0) const override {
        printIndent(indent);
        std::cout << "Block" << std::endl;
        for (const auto& statement : statements) {
            statement->print(indent + 1);
        }
    }

private:
    std::vector<ASTNodePtr> statements;
};

class TypedefNode : public ASTNode {
public:
    TypedefNode(std::string_view name, std::variant<ASTNodePtr, std::string_view> type)
        : ASTNode(NodeType::Typedef), name(name), type(std::move(type)) {}

    void print(int indent = 0) const override {
        printIndent(indent);
        std::cout << "Typedef: " << name << std::endl;
        if (std::holds_alternative<ASTNodePtr>(type)) {
            std::cout << "Type: ";
            std::get<ASTNodePtr>(type)->print(0);
        } else {
            std::cout << "Type: " << std::get<std::string_view>(type) << std::endl;
        }
    }

private:
    std::string name;
    std::variant<ASTNodePtr, std::string_view> type;
};

class StructNode : public ASTNode {
public:
    StructNode(std::vector<ASTNodePtr> members)
        : ASTNode(NodeType::Struct), members(std::move(members)) {}

    void print(int indent = 0) const override {
        printIndent(indent);
        std::cout << "Struct" << std::endl;
        for (const auto& member : members) {
            member->print(indent + 1);
        }
    }

private:
    std::vector<ASTNodePtr> members;
};

class GlobalVarDeclNode : public ASTNode {
public:
    GlobalVarDeclNode(std::string_view type, std::string_view name)
        : ASTNode(NodeType::GlobalVarDecl), type(type), name(name) {}

    void print(int indent = 0) const override {
        printIndent(indent);
        std::cout << "GlobalVarDecl: " << type << ": " << name << std::endl;
    }

private:
    std::string type;
    std::string name;
};

class GlobalVarDeclAssignNode : public ASTNode {
public:
    GlobalVarDeclAssignNode(std::string_view type, std::string_view name, ASTNodePtr expression)
        : ASTNode(NodeType::GlobalVarDeclAssign), type(type), name(name), expression(std::move(expression)) {}

    void print(int indent = 0) const override {
        printIndent(indent);
        std::cout << "GlobalVarDeclAssign: " << type << ": " << name << std::endl;
        expression->print(indent + 1);
    }

private:
    std::string type;
    std::string name;
    ASTNodePtr expression;
};

class IncrementNode : public ASTNode {
public:
    IncrementNode(std::string_view variable)
        : ASTNode(NodeType::Increment), variable(variable) {}

    void print(int indent = 0) const override {
        printIndent(indent);
        std::cout << "Increment: " << variable << std::endl;
    }

private:
    std::string variable;
};

class DecrementNode : public ASTNode {
public:
    DecrementNode(std::string_view variable)
        : ASTNode(NodeType::Decrement), variable(variable) {}

    void print(int indent = 0) const override {
        printIndent(indent);
        std::cout << "Decrement: " << variable << std::endl;
    }

private:
    std::string variable;
};

class HeaderNode : public ASTNode {
public:
    HeaderNode(std::vector<ASTNodePtr> prototypes)
        : ASTNode(NodeType::Header), prototypes(std::move(prototypes)) {}

    void print(int indent = 0) const override {
        printIndent(indent);
        std::cout << "Header" << std::endl;
        for (const auto& prototype : prototypes) {
            prototype->print(indent + 1);
        }
    }

private:
    std::vector<ASTNodePtr> prototypes;
};

class AsmNode : public ASTNode {
public:
    AsmNode(std::string_view asmCode)
        : ASTNode(NodeType::Asm), asmCode(asmCode) {}

    void print(int indent = 0) const override {
        printIndent(indent);
        std::cout << "Asm: " << asmCode << std::endl;
    }

private:
    std::string asmCode;
};

class FunctionPrototypeNode : public ASTNode {
public:
    FunctionPrototypeNode(std::string_view returnType, std::string_view name, std::vector<ASTNodePtr> parameters)
        : ASTNode(NodeType::FunctionPrototype), returnType(returnType), name(name), parameters(std::move(parameters)) {}

    void print(int indent = 0) const override {
        printIndent(indent);
        std::cout << "FunctionPrototype: " << returnType << " " << name << std::endl;
        printIndent(indent + 1);
        std::cout << "parameters:" << std::endl;
        for (const auto& param : parameters) {
            param->print(indent + 2);
        }
    }

private:
    std::string returnType;
    std::string name;
    std::vector<ASTNodePtr> parameters;
};

class ParameterNode : public ASTNode {
public:
    ParameterNode(std::string_view type, std::string_view name)
        : ASTNode(NodeType::Parameter), type(type), name(name) {}

    void print(int indent = 0) const override {
        printIndent(indent);
        std::cout << "Parameter: " << type << " " << name << std::endl;
    }   

private:
    std::string type;
    std::string name;
};

class CallNode : public ASTNode {
public:
    CallNode(std::string_view name, std::vector<ASTNodePtr> arguments)
        : ASTNode(NodeType::Call), name(name), arguments(std::move(arguments)) {}

    void print(int indent = 0) const override {
        printIndent(indent);
        std::cout << "Call: " << name << std::endl;
        printIndent(indent + 1);
        std::cout << "arguments:" << std::endl;
        for (const auto& arg : arguments) {
            arg->print(indent + 2);
        }
    }

private:
    std::string name;
    std::vector<ASTNodePtr> arguments;
};

class ElseNode : public ASTNode {
public:
    ElseNode(ASTNodePtr body)
        : ASTNode(NodeType::Else), body(std::move(body)) {}

    void print(int indent = 0) const override {
        printIndent(indent);
        std::cout << "Else" << std::endl;
        body->print(indent + 1);
    }

private:
    ASTNodePtr body;
};

class ElseIfNode : public ASTNode {
public:
    ElseIfNode(ASTNodePtr ifNode)
        : ASTNode(NodeType::Elseif), ifNode(std::move(ifNode)) {}

    void print(int indent = 0) const override {
        printIndent(indent);
        std::cout << "ElseIf" << std::endl;
        ifNode->print(indent + 1);
    }

private:
    ASTNodePtr ifNode;
};


} // namespace EntS

#endif // AST_HPP
