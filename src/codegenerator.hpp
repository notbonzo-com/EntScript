#ifndef CODE_GENERATOR_HPP
#define CODE_GENERATOR_HPP

#include "ast.hpp"
#include <map>
#include <string>
#include <vector>
#include <unordered_map>

namespace EntS {

class CodeGenerator {
public:
    explicit CodeGenerator(const std::unordered_map<std::string, std::string>& typedefs, const std::unordered_map<std::string, std::vector<std::string>>& structs);
    void generateCode(const ASTNodePtr& root);
    std::string getGeneratedCode() const;

private:
    void enterFunction(const FunctionNode* function);
    void exitFunction();

    void enterScope();
    void exitScope();

    std::string getVariableType(const std::string& name) const;
    void addLocalVariable(const std::string& name);

    int getLocalVariableOffset(const std::string& name) const;

    void visitProgramNode(const ProgramNode* node);
    void visitFunctionNode(const FunctionNode* node);
    void visitVarDeclNode(const VarDeclNode* node);
    void visitVarDeclAssignNode(const VarDeclAssignNode* node);
    void visitAssignNode(const AssignNode* node);
    void visitExpressionNode(const ExpressionNode* node);
    void visitReturnNode(const ReturnNode* node);
    void visitIfNode(const IfNode* node);
    void visitWhileNode(const WhileNode* node);
    void visitBlockNode(const BlockNode* node);
    void visitFunctionCallNode(const FunctionCallNode* node);
    void visitLiteralNode(const LiteralNode* node);
    void visitIdentifierNode(const IdentifierNode* node);
    void visitStructMemberAccessNode(const StructMemberAccessNode* node);
	void visitBreakNode(const BreakNode* node);
	void visitContinueNode(const ContinueNode* node);
    void visitGlobalVarDeclNode(const GlobalVarDeclNode* node);
    void visitStructNode(const StructNode* node);
    void visitTypedefNode(const TypedefNode* node);
    void visitSwitchNode(const SwitchNode* node);

    std::string generateLabel(const std::string& prefix);
    std::string generateUniqueLabel();
    int resolveTypeSize(const std::string& type) const;
    void addLocalVariable(const std::string& name, const std::string& type);

    void emit(const std::string& code);
    void emitFunctionPrologue(const FunctionNode* node);
    void emitFunctionEpilogue();

    std::string resolveTypeName(const std::string& type) const;
    int calculateLocalVariableSize(const BlockNode* block);

    // Variables to keep track of context
    std::vector<std::map<std::string, std::pair<int, std::string>>> localVarStack; // Stack of local variable offsets
    std::string currentFunctionName;
    int localVarOffset; // Current stack offset for local variables
    int labelCounter; // For generating unique labels
    std::vector<std::string> generatedCode; // To store generated assembly code

    // System V ABI specifics
    std::vector<std::string> argumentRegisters; // System V ABI argument registers
    int currentArgOffset; // Offset for arguments passed on the stack

	std::unordered_map<std::string, std::string> typedefs;
    std::unordered_map<std::string, std::vector<std::string>> structDefinitions;

    struct LoopContext {
        std::string startLabel;
        std::string endLabel;
    };

    std::vector<LoopContext> loopContextStack;

    int totalLocalVarOffset;
};

} // namespace EntS

#endif // CODE_GENERATOR_HPP
