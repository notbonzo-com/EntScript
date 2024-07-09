#ifndef AST_HPP
#define AST_HPP

#include <iostream>
#include <memory>
#include <string>
#include <vector>

namespace EntS {

enum class NodeType {
    Function,
    VarDecl,
    Assign,
    Return,
    Expr,
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
    Header,
    TypedefStruct,
    StructBody,
    Increment,
    Decrement,
    InlineAsm
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

class FunctionNode : public ASTNode {
public:
    FunctionNode(const std::string& name, std::vector<ASTNodePtr> params, std::vector<ASTNodePtr> body, const std::string& returnType)
        : ASTNode(NodeType::Function), name(name), params(std::move(params)), body(std::move(body)), returnType(returnType) {}

    void print(int indent = 0) const override {
        printIndent(indent);
        std::cout << "Function: " << name << " returns " << returnType << "\n";
        std::cout << "Parameters:\n";
        for (const auto& param : params) {
            if (param) param->print(indent + 1);
        }
        std::cout << "Body:\n";
        for (const auto& stmt : body) {
            if (stmt) stmt->print(indent + 1);
        }
    }

private:
    std::string name;
    std::vector<ASTNodePtr> params;
    std::vector<ASTNodePtr> body;
    std::string returnType;
};

class VarDeclNode : public ASTNode {
public:
    VarDeclNode(const std::string& type, const std::string& name, ASTNodePtr value)
        : ASTNode(NodeType::VarDecl), type(type), name(name), value(std::move(value)) {}

    void print(int indent = 0) const override {
        printIndent(indent);
        std::cout << "VarDecl: " << type << " " << name << "\n";
        if (value) value->print(indent + 1);
    }

    std::string getName() const {
        return name;
    }

private:
    std::string type;
    std::string name;
    ASTNodePtr value;
};

class AssignNode : public ASTNode {
public:
    AssignNode(const std::string& name, ASTNodePtr value)
        : ASTNode(NodeType::Assign), name(name), value(std::move(value)), hasIndex(false) {}

    AssignNode(const std::string& name, ASTNodePtr index, ASTNodePtr value)
        : ASTNode(NodeType::Assign), name(name), index(std::move(index)), value(std::move(value)), hasIndex(true) {}

    void print(int indent = 0) const override {
        printIndent(indent);
        std::cout << "Assign: " << name << "\n";
        if (hasIndex && index) {
            printIndent(indent + 1);
            std::cout << "Index:\n";
            index->print(indent + 2);
        }
        printIndent(indent + 1);
        std::cout << "Value:\n";
        if (value) value->print(indent + 2);
    }

private:
    std::string name;
    ASTNodePtr index;
    ASTNodePtr value;
    bool hasIndex;
};

class ReturnNode : public ASTNode {
public:
    explicit ReturnNode(ASTNodePtr value)
        : ASTNode(NodeType::Return), value(std::move(value)) {}

    void print(int indent = 0) const override {
        printIndent(indent);
        std::cout << "Return\n";
        if (value) value->print(indent + 1);
    }

private:
    ASTNodePtr value;
};

class ExprNode : public ASTNode {
public:
    ExprNode(const std::string& op, ASTNodePtr left, ASTNodePtr right)
        : ASTNode(NodeType::Expr), op(op), left(std::move(left)), right(std::move(right)) {}

    void print(int indent = 0) const override {
        printIndent(indent);
        std::cout << "Expr: " << op << "\n";
        if (left) left->print(indent + 1);
        if (right) right->print(indent + 1);
    }

    const ASTNodePtr& getLeft() const { return left; }
    const ASTNodePtr& getRight() const { return right; }

private:
    std::string op;
    ASTNodePtr left;
    ASTNodePtr right;
};

class IfNode : public ASTNode {
public:
    IfNode(ASTNodePtr condition, std::vector<ASTNodePtr> thenBranch, std::vector<ASTNodePtr> elseBranch)
        : ASTNode(NodeType::If), condition(std::move(condition)), thenBranch(std::move(thenBranch)), elseBranch(std::move(elseBranch)) {}

    void print(int indent = 0) const override {
        printIndent(indent);
        std::cout << "If\n";
        if (condition) condition->print(indent + 1);
        std::cout << "Then Branch:\n";
        for (const auto& stmt : thenBranch) {
            if (stmt) stmt->print(indent + 1);
        }
        std::cout << "Else Branch:\n";
        for (const auto& stmt : elseBranch) {
            if (stmt) stmt->print(indent + 1);
        }
    }

private:
    ASTNodePtr condition;
    std::vector<ASTNodePtr> thenBranch;
    std::vector<ASTNodePtr> elseBranch;
};

class WhileNode : public ASTNode {
public:
    WhileNode(ASTNodePtr condition, ASTNodePtr body)
        : ASTNode(NodeType::While), condition(std::move(condition)), body(std::move(body)) {}

    void print(int indent = 0) const override {
        printIndent(indent);
        std::cout << "While\n";
        if (condition) condition->print(indent + 1);
        if (body) body->print(indent + 1);
    }

private:
    ASTNodePtr condition;
    ASTNodePtr body;
};

class ForNode : public ASTNode {
public:
    ForNode(ASTNodePtr init, ASTNodePtr condition, ASTNodePtr increment, ASTNodePtr body)
        : ASTNode(NodeType::For), init(std::move(init)), condition(std::move(condition)), increment(std::move(increment)), body(std::move(body)) {}

    void print(int indent = 0) const override {
        printIndent(indent);
        std::cout << "For\n";
        if (init) init->print(indent + 1);
        if (condition) condition->print(indent + 1);
        if (increment) increment->print(indent + 1);
        if (body) body->print(indent + 1);
    }

private:
    ASTNodePtr init;
    ASTNodePtr condition;
    ASTNodePtr increment;
    ASTNodePtr body;
};

class SwitchNode : public ASTNode {
public:
    SwitchNode(ASTNodePtr condition, std::vector<ASTNodePtr> cases)
        : ASTNode(NodeType::Switch), condition(std::move(condition)), cases(std::move(cases)) {}

    void print(int indent = 0) const override {
        printIndent(indent);
        std::cout << "Switch\n";
        if (condition) condition->print(indent + 1);
        for (const auto& caseNode : cases) {
            if (caseNode) caseNode->print(indent + 1);
        }
    }

private:
    ASTNodePtr condition;
    std::vector<ASTNodePtr> cases;
};

class CaseNode : public ASTNode {
public:
    CaseNode(ASTNodePtr value, std::vector<ASTNodePtr> body)
        : ASTNode(NodeType::Case), value(std::move(value)), body(std::move(body)) {}

    void print(int indent = 0) const override {
        printIndent(indent);
        std::cout << "Case\n";
        if (value) value->print(indent + 1);
        for (const auto& stmt : body) {
            if (stmt) stmt->print(indent + 1);
        }
    }

private:
    ASTNodePtr value;
    std::vector<ASTNodePtr> body;
};

class DefaultNode : public ASTNode {
public:
    explicit DefaultNode(std::vector<ASTNodePtr> body)
        : ASTNode(NodeType::Default), body(std::move(body)) {}

    void print(int indent = 0) const override {
        printIndent(indent);
        std::cout << "Default\n";
        for (const auto& stmt : body) {
            if (stmt) stmt->print(indent + 1);
        }
    }

private:
    std::vector<ASTNodePtr> body;
};

class BlockNode : public ASTNode {
public:
    explicit BlockNode(std::vector<ASTNodePtr> statements)
        : ASTNode(NodeType::Block), statements(std::move(statements)) {}

    void print(int indent = 0) const override {
        printIndent(indent);
        std::cout << "Block\n";
        for (const auto& stmt : statements) {
            if (stmt) stmt->print(indent + 1);
        }
    }

private:
    std::vector<ASTNodePtr> statements;
};

class TypedefNode : public ASTNode {
public:
    TypedefNode(const std::string& oldType, const std::string& newType)
        : ASTNode(NodeType::Typedef), oldType(oldType), newType(newType) {}

    void print(int indent = 0) const override {
        printIndent(indent);
        std::cout << "Typedef: " << oldType << " as " << newType << "\n";
    }

private:
    std::string oldType;
    std::string newType;
};

class TypedefStructNode : public ASTNode {
public:
    TypedefStructNode(ASTNodePtr structNode, const std::string& name)
        : ASTNode(NodeType::TypedefStruct), structNode(std::move(structNode)), name(name) {}
    
    void print(int indent = 0) const override {
        printIndent(indent);
        std::cout << "TypedefStruct: " << name << "\n";
        if (structNode) structNode->print(indent + 1);
    }
    
private:
    ASTNodePtr structNode;
    std::string name;
};

class StructBodyNode : public ASTNode {
public:
    explicit StructBodyNode(std::vector<ASTNodePtr> fields)
        : ASTNode(NodeType::StructBody), fields(std::move(fields)) {}
    
    void print(int indent = 0) const override {
        printIndent(indent);
        std::cout << "StructBody:\n";
        for (const auto& field : fields) {
            if (field) field->print(indent + 1);
        }
    }

private:
    std::vector<ASTNodePtr> fields;
};

class StructNode : public ASTNode {
public:
    StructNode(const std::string& name, std::vector<ASTNodePtr> fields)
        : ASTNode(NodeType::Struct), name(name), fields(std::move(fields)) {}

    void print(int indent = 0) const override {
        printIndent(indent);
        std::cout << "Struct: " << name << "\n";
        for (const auto& field : fields) {
            if (field) field->print(indent + 1);
        }
    }

private:
    std::string name;
    std::vector<ASTNodePtr> fields;
};

class GlobalVarDeclNode : public ASTNode {
public:
    GlobalVarDeclNode(const std::string& type, const std::string& name)
        : ASTNode(NodeType::GlobalVarDecl), type(type), name(name) {}

    void print(int indent = 0) const override {
        printIndent(indent);
        std::cout << "GlobalVarDecl: " << type << " " << name << "\n";
    }

    std::string getName() const {
        return name;
    }

private:
    std::string type;
    std::string name;
};

class ContinueNode : public ASTNode {
public:
    ContinueNode() : ASTNode(NodeType::Continue) {}

    void print(int indent = 0) const override {
        printIndent(indent);
        std::cout << "Continue\n";
    }
};

class BreakNode : public ASTNode {
public:
    BreakNode() : ASTNode(NodeType::Break) {}

    void print(int indent = 0) const override {
        printIndent(indent);
        std::cout << "Break\n";
    }
};

class HeaderNode : public ASTNode {
public:
    HeaderNode(std::vector<ASTNodePtr> body) : ASTNode(NodeType::Header), body(std::move(body)) {}

    void print(int indent = 0) const override {
        printIndent(indent);
        std::cout << "Header\n";
        for (const auto& stmt : body) {
            if (stmt) stmt->print(indent + 1);
        }
    }

private:
    std::vector<ASTNodePtr> body;
};

class IncrementNode : public ASTNode {
public:
    IncrementNode(const std::string& name) : ASTNode(NodeType::Increment), name(name) {}

    void print(int indent = 0) const override {
        printIndent(indent);
        std::cout << "Increment: " << name << "\n";
    }

private:
    std::string name;
};

class DecrementNode : public ASTNode {
public:
    DecrementNode(const std::string& name) : ASTNode(NodeType::Decrement), name(name) {}

    void print(int indent = 0) const override {
        printIndent(indent);
        std::cout << "Decrement: " << name << "\n";
    }

private:
    std::string name;
};

inline void print_ast(const ASTNode& node, int indent = 0) {
    node.print(indent);
}

class InlineAsmNode : public ASTNode {
public:
    explicit InlineAsmNode(const std::vector<std::string>& lines)
        : ASTNode(NodeType::InlineAsm), lines(lines) {}

    void print(int indent = 0) const override {
        printIndent(indent);
        std::cout << "InlineAsm\n";
        for (const auto& line : lines) {
            printIndent(indent + 1);
            std::cout << line << "\n";
        }
    }

private:
    std::vector<std::string> lines;
};

} // namespace EntS

#endif // AST_HPP
