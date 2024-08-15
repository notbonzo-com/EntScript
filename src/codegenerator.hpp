#ifndef CODEGENERATOR_HPP
#define CODEGENERATOR_HPP

#include <string>
#include <sstream>
#include "ast.hpp"
#include "formats.hpp"

namespace EntS {

class CodeGenerator {
public:
    CodeGenerator(const ASTNodePtr& ast, OutputFormat format, const std::string& prologue, const std::string& epilogue);

    std::string generateCode();

private:
    void generateNode(const ASTNodePtr& node, std::ostringstream& code);
    
    void generateProgramNode(ProgramNode* node, std::ostringstream& code);
    void generateFunctionNode(FunctionNode* node, std::ostringstream& code);
    void generateVarDeclNode(VarDeclNode* node, std::ostringstream& code);
    void generateVarDeclAssignNode(VarDeclAssignNode* node, std::ostringstream& code);
    void generateAssignNode(AssignNode* node, std::ostringstream& code);
    void generateIfNode(IfNode* node, std::ostringstream& code);
    void generateWhileNode(WhileNode* node, std::ostringstream& code);
    void generateReturnNode(ReturnNode* node, std::ostringstream& code);
    void generateExpressionNode(ExpressionNode* node, std::ostringstream& code);
    void generateLiteralNode(LiteralNode* node, std::ostringstream& code);
    void generateIdentifierNode(IdentifierNode* node, std::ostringstream& code);
    void generateCallNode(CallNode* node, std::ostringstream& code);
    
    void applyPrologue(std::ostringstream& code);
    void applyEpilogue(std::ostringstream& code);

    ASTNodePtr ast;
    OutputFormat format;
    std::string prologue;
    std::string epilogue;
};

} // namespace EntS

#endif // CODEGENERATOR_HPP
