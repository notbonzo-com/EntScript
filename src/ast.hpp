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
    IndexationAssign,
    MemoryAssign,
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
    FunctionPrototype,
    Call,
    Else,
    Elseif,
    Parameter,
    FunctionCall,
    Identifier,
    Literal,
    StringLiteral,
    Index,
    MemoryAddress,
    StructMemberAccess,
    StructMemberAssign,
};

static inline std::string toString(NodeType type) {
    switch (type) {
        case NodeType::Program: return "Program";
        case NodeType::Function: return "Function";
        case NodeType::VarDecl: return "VarDecl";
        case NodeType::VarDeclAssign: return "VarDeclAssign";
        case NodeType::Assign: return "Assign";
        case NodeType::IndexationAssign: return "IndexationAssign";
        case NodeType::MemoryAssign: return "MemoryAssign";
        case NodeType::Return: return "Return";
        case NodeType::Expression: return "Expression";
        case NodeType::If: return "If";
        case NodeType::While: return "While";
        case NodeType::For: return "For";
        case NodeType::Switch: return "Switch";
        case NodeType::Case: return "Case";
        case NodeType::Default: return "Default";
        case NodeType::Continue: return "Continue";
        case NodeType::Break: return "Break";
        case NodeType::Block: return "Block";
        case NodeType::Typedef: return "Typedef";
        case NodeType::Struct: return "Struct";
        case NodeType::GlobalVarDecl: return "GlobalVarDecl";
        case NodeType::GlobalVarDeclAssign: return "GlobalVarDeclAssign";
        case NodeType::Increment: return "Increment";
        case NodeType::Decrement: return "Decrement";
        case NodeType::Header: return "Header";
        case NodeType::FunctionPrototype: return "FunctionPrototype";
        case NodeType::Call: return "Call";
        case NodeType::Else: return "Else";
        case NodeType::Elseif: return "Elseif";
        case NodeType::Parameter: return "Parameter";
        case NodeType::FunctionCall: return "FunctionCall";
        case NodeType::Identifier: return "Identifier";
        case NodeType::Literal: return "Literal";
        case NodeType::StringLiteral: return "StringLiteral";
        case NodeType::Index: return "Index";
        case NodeType::MemoryAddress: return "MemoryAddress";
        case NodeType::StructMemberAccess: return "StructMemberAccess";
        case NodeType::StructMemberAssign: return "StructMemberAssign";
    }
    return "";
}

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


    NodeType type;
};

using ASTNodePtr = std::shared_ptr<ASTNode>;

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


    std::string name;
    std::string returnType;
    std::vector<ASTNodePtr> params;
    ASTNodePtr body;
};

class VarDeclNode : public ASTNode {
public:
    VarDeclNode(std::string_view type, std::string_view name, bool initByAddr = false)
        : ASTNode(NodeType::VarDecl), type(type), name(name), initByAddr(initByAddr) {}

    void print(int indent = 0) const override {
        printIndent(indent);
        std::cout << "VarDecl: " << type << ": " << name << (initByAddr ? " (Address initialised)" : "") << std::endl;
    }


    std::string type;
    std::string name;
    bool initByAddr;
};

class VarDeclAssignNode : public ASTNode {
public:
    VarDeclAssignNode(std::string_view type, std::string_view name, ASTNodePtr expression, bool initByAddr = false)
        : ASTNode(NodeType::VarDeclAssign), type(type), name(name), expression(std::move(expression)), initByAddr(initByAddr) {}

    void print(int indent = 0) const override {
        printIndent(indent);
        std::cout << "VarDeclAssign: " << type << ": " << name << (initByAddr ? " (Address initialised)" : "") << std::endl;
        expression->print(indent + 1);
    }


    std::string type;
    std::string name;
    ASTNodePtr expression;
    bool initByAddr;
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


    std::string name;
    ASTNodePtr expression;
};

class IndexationAssignNode : public ASTNode {
public:
    IndexationAssignNode(std::string_view name, ASTNodePtr index, ASTNodePtr expression)
        : ASTNode(NodeType::IndexationAssign), name(name), index(std::move(index)), expression(std::move(expression)) {}

    void print(int indent = 0) const override {
        printIndent(indent);
        std::cout << "IndexationAssign: " << name << std::endl;
        index->print(indent + 1);
        expression->print(indent + 1);
    }


    std::string name;
    ASTNodePtr index;
    ASTNodePtr expression;
};

class MemoryAssignNode : public ASTNode {
public:
    MemoryAssignNode(std::string_view name, ASTNodePtr expression)
        : ASTNode(NodeType::MemoryAssign), name(name), expression(std::move(expression)) {}

    void print(int indent = 0) const override {
        printIndent(indent);
        std::cout << "MemoryAssign: " << name << std::endl;
        expression->print(indent + 1);
    }


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


    std::optional<ASTNodePtr> left;
    std::string op;
    std::optional<ASTNodePtr> right;
};

class IdentifierNode : public ASTNode {
public:
    IdentifierNode(std::string_view name)
        : ASTNode(NodeType::Identifier), name(name) {}

    void print(int indent = 0) const override {
        printIndent(indent);
        std::cout << "Identifier: " << name << std::endl;
    }


    std::string name;
};

class LiteralNode : public ASTNode {
public:
    LiteralNode(std::string_view value)
        : ASTNode(NodeType::Literal), value(value) {}

    void print(int indent = 0) const override {
        printIndent(indent);
        std::cout << "Literal: " << value << std::endl;
    }


    std::string value;
};

class StringLiteralNode : public ASTNode {
public:
    StringLiteralNode(std::string_view value)
        : ASTNode(NodeType::StringLiteral), value(value) {}

    void print(int indent = 0) const override {
        printIndent(indent);
        std::cout << "StringLiteral: " << value << std::endl;
    }


    std::string value;
};

class IfNode : public ASTNode {
public:
    IfNode(ASTNodePtr condition, ASTNodePtr body, ASTNodePtr else_)
        : ASTNode(NodeType::If), condition(std::move(condition)), body(std::move(body)), else_(std::move(else_)) {}

    void print(int indent = 0) const override {
        printIndent(indent);
        std::cout << "If" << std::endl;
        printIndent(indent + 1);
        condition->print(indent + 1);
        body->print(indent + 1);
        printIndent(indent + 1);
        if (else_) {
            std::cout << "Else" << std::endl;
            printIndent(indent + 1);
            else_->print(indent + 1);
        }
    }


    ASTNodePtr condition;
    ASTNodePtr body;
    ASTNodePtr else_;
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


    ASTNodePtr body;
};

class ContinueNode : public ASTNode {
public:
    ContinueNode() : ASTNode(NodeType::Continue) {}
    void print(int indent = 0) const override {
        printIndent(indent);
        std::cout << "Continue" << std::endl;
    }
};

class BreakNode : public ASTNode {
public:
    BreakNode() : ASTNode(NodeType::Break) {}
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


    std::vector<ASTNodePtr> statements;
};

class TypedefNode : public ASTNode {
public:
    TypedefNode(std::string_view name, std::variant<ASTNodePtr, std::string> type)
        : ASTNode(NodeType::Typedef), name(name), type(std::move(type)) {}

    void print(int indent = 0) const override {
        printIndent(indent);
        std::cout << "Typedef: " << name << std::endl;
        if (std::holds_alternative<ASTNodePtr>(type)) {
            printIndent(indent + 1);
            std::cout << "Type: ";
            std::get<ASTNodePtr>(type)->print(0);
        } else {
            printIndent(indent + 1);
            std::cout << "Type: " << std::get<std::string>(type) << std::endl;
        }
    }


    std::string name;
    std::variant<ASTNodePtr, std::string> type;
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


    std::vector<ASTNodePtr> members;
};

class GlobalVarDeclNode : public ASTNode {
public:
    GlobalVarDeclNode(std::string_view type, std::string_view name, bool initByAddr = false)
        : ASTNode(NodeType::GlobalVarDecl), type(type), name(name), initByAddr(initByAddr) {}

    void print(int indent = 0) const override {
        printIndent(indent);
        std::cout << "GlobalVarDecl: " << type << ": " << name << (initByAddr ? " (Address initialised)" : "") << std::endl;
    }


    std::string type;
    std::string name;
    bool initByAddr;
};

class GlobalVarDeclAssignNode : public ASTNode {
public:
    GlobalVarDeclAssignNode(std::string_view type, std::string_view name, ASTNodePtr expression, bool initByAddr = false)
        : ASTNode(NodeType::GlobalVarDeclAssign), type(type), name(name), expression(std::move(expression)), initByAddr(initByAddr) {}

    void print(int indent = 0) const override {
        printIndent(indent);
        std::cout << "GlobalVarDeclAssign: " << type << ": " << name << (initByAddr ? " (Address initialised)" : "") << std::endl;
        expression->print(indent + 1);
    }


    std::string type;
    std::string name;
    ASTNodePtr expression;
    bool initByAddr;
};

class IncrementNode : public ASTNode {
public:
    IncrementNode(std::string_view variable)
        : ASTNode(NodeType::Increment), variable(variable) {}

    void print(int indent = 0) const override {
        printIndent(indent);
        std::cout << "Increment: " << variable << std::endl;
    }


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


    std::vector<ASTNodePtr> prototypes;
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


    ASTNodePtr ifNode;
};

class FunctionCallNode : public ASTNode {
public:
    FunctionCallNode(std::string_view name, std::vector<ASTNodePtr> arguments)
        : ASTNode(NodeType::FunctionCall), name(name), arguments(std::move(arguments)) {}

    void print(int indent = 0) const override {
        printIndent(indent);
        std::cout << "FunctionCall: " << name << std::endl;
        printIndent(indent + 1);
        if (arguments.empty()) {
            return;
        }
        std::cout << "arguments:" << std::endl;
        for (const auto& arg : arguments) {
            arg->print(indent + 2);
        }
    }


    std::string name;
    std::vector<ASTNodePtr> arguments;
};

class MemoryAddressNode : public ASTNode {
public:
    MemoryAddressNode(std::string_view name) : ASTNode(NodeType::MemoryAddress), name(name) {}

    void print(int indent = 0) const override {
        printIndent(indent);
        std::cout << "MemoryAddress: " << name << std::endl;
    }


    std::string name;
};

class IndexNode : public ASTNode {
public:
    IndexNode(std::string_view name, ASTNodePtr index) : ASTNode(NodeType::Index), name(name), index(std::move(index)) {}

    void print(int indent = 0) const override {
        printIndent(indent);
        std::cout << "Index: " << name << std::endl;
        index->print(indent + 1);
    }


    std::string name;
    ASTNodePtr index;
};

class StructMemberAccessNode : public ASTNode {
public:
    StructMemberAccessNode(ASTNodePtr base, const std::string& memberName)
        : ASTNode(NodeType::StructMemberAccess), base(std::move(base)), memberName(memberName) {}

    void print(int indent = 0) const override {
        printIndent(indent);
        std::cout << "StructMemberAccess: " << std::endl;
        base->print(indent + 1);
        printIndent(indent + 2);
        std::cout << "Accessing member: " << memberName << std::endl;
    }


    ASTNodePtr base;
    std::string memberName;
};

class StructMemberAssignNode : public ASTNode {
public:
    StructMemberAssignNode(ASTNodePtr memberAccess, ASTNodePtr value)
        : ASTNode(NodeType::StructMemberAssign), memberAccess(std::move(memberAccess)), value(std::move(value)) {}

    void print(int indent = 0) const override {
        printIndent(indent);
        std::cout << "StructMemberAssign: " << std::endl;
        memberAccess->print(indent + 1);
        value->print(indent + 1);
    }


    ASTNodePtr memberAccess;
    ASTNodePtr value;
};

} // namespace EntS

#endif // AST_HPP
