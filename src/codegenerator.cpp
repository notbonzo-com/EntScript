#include "codegenerator.hpp"
#include "ast.hpp"
#include <sstream>

extern void printFatal(const char* str);
extern void printError(const char* str);

namespace EntS {

CodeGenerator::CodeGenerator(const std::unordered_map<std::string, std::string>& typedefs, const std::unordered_map<std::string, std::vector<std::string>>& structs)
    : typedefs(typedefs), localVarOffset(0), labelCounter(0), currentArgOffset(0), structDefinitions(structs), totalLocalVarOffset(0) {
    // Initialize System V ABI argument registers
    argumentRegisters = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};
}

void CodeGenerator::generateCode(const ASTNodePtr& root) {
    visitProgramNode(dynamic_cast<const ProgramNode*>(root.get()));
}

std::string CodeGenerator::getGeneratedCode() const {
    std::stringstream ss;
    for (const auto& line : generatedCode) {
        ss << line << "\n";
    }
    return ss.str();
}

int CodeGenerator::resolveTypeSize(const std::string& type) const {
    std::string resolvedType = resolveTypeName(type);
    if (resolvedType == "int8" || resolvedType == "uint8" || resolvedType == "char") return 1;
    if (resolvedType == "int16" || resolvedType == "uint16") return 2;
    if (resolvedType == "int32" || resolvedType == "uint32" || resolvedType == "float") return 4;
    if (resolvedType == "int64" || resolvedType == "uint64" || resolvedType == "double") return 8;
    auto it = structDefinitions.find(resolvedType);
    if (it != structDefinitions.end()) {
        int size = 0;
        for (const auto& member : it->second) {
            size += resolveTypeSize(member);
        }
        return size;
    }
    printFatal("Unknown type size");
    __builtin_unreachable();
}

void CodeGenerator::enterFunction(const FunctionNode* function) {
    currentFunctionName = function->name;
    localVarOffset = 0;
    totalLocalVarOffset = 0;
    currentArgOffset = 16; // Arguments passed on the stack start at 16(%rbp)
    localVarStack.push_back({});
    emitFunctionPrologue(function);

    int numParams = function->params.size();
    for (int i = 0; i < numParams; ++i) {
        const auto& paramNode = dynamic_cast<const ParameterNode*>(function->params[i].get());
        const std::string& paramName = paramNode->name;

        if (i < argumentRegisters.size()) {
            emit("mov [rbp-" + std::to_string(8 * (i + 1)) + "], " + argumentRegisters[i]);
            localVarStack.back()[paramName] = std::make_pair(-8 * (i + 1), paramNode->type);
        } else {
            localVarStack.back()[paramName] = std::make_pair(currentArgOffset, paramNode->type);
            currentArgOffset += 8;
        }
    }
}

void CodeGenerator::exitFunction() {
    emitFunctionEpilogue();
    localVarStack.pop_back();
    currentFunctionName.clear();
}

int CodeGenerator::getLocalVariableOffset(const std::string& name) const {
    for (auto it = localVarStack.rbegin(); it != localVarStack.rend(); ++it) {
        auto varIt = it->find(name);
        if (varIt != it->end()) {
            return varIt->second.first;
        }
    }
    printError("Variable not defined");
    __builtin_unreachable();
}

void CodeGenerator::enterScope() {
    localVarStack.push_back({});
}

void CodeGenerator::exitScope() {
    localVarStack.pop_back();
}

void CodeGenerator::addLocalVariable(const std::string& name, const std::string& type) {
    int size = resolveTypeSize(type);
    localVarOffset -= size;
    totalLocalVarOffset += size;
    localVarStack.back()[name] = std::make_pair(localVarOffset, type);
}

std::string CodeGenerator::getVariableType(const std::string& name) const {
    for (auto it = localVarStack.rbegin(); it != localVarStack.rend(); ++it) {
        auto varIt = it->find(name);
        if (varIt != it->end()) {
            return varIt->second.second;
        }
    }
    printError("Variable type not found");
    __builtin_unreachable();
}

void CodeGenerator::visitProgramNode(const ProgramNode* node) {
    for (const auto& function : node->functions) {
        visitFunctionNode(dynamic_cast<const FunctionNode*>(function.get()));
    }
}

void CodeGenerator::visitFunctionNode(const FunctionNode* node) {
    enterFunction(node);
    visitBlockNode(dynamic_cast<const BlockNode*>(node->body.get()));
    exitFunction();
}

void CodeGenerator::visitVarDeclNode(const VarDeclNode* node) {
    addLocalVariable(node->name, node->type);
}

void CodeGenerator::visitVarDeclAssignNode(const VarDeclAssignNode* node) {
    addLocalVariable(node->name, node->type);
    visitExpressionNode(dynamic_cast<const ExpressionNode*>(node->expression.get()));
    int offset = getLocalVariableOffset(node->name);
    emit("mov [rbp" + std::to_string(offset) + "], rax");
}

void CodeGenerator::visitGlobalVarDeclNode(const GlobalVarDeclNode* node) {
    std::string resolvedType = resolveTypeName(node->type);
    int size = resolveTypeSize(resolvedType);

    if (node->initByAddr) {
        emit("section .bss");
        emit(node->name + " resb " + std::to_string(size));
    } else {
        emit("section .data");
        switch (size) {
            case 1: emit(node->name + " db 0"); break;
            case 2: emit(node->name + " dw 0"); break;
            case 4: emit(node->name + " dd 0"); break;
            case 8: emit(node->name + " dq 0"); break;
        }
    }
}

// Illegal in ent
// void CodeGenerator::visitGlobalVarDeclAssignNode(const GlobalVarDeclAssignNode* node) {
// }

void CodeGenerator::visitAssignNode(const AssignNode* node) {
    visitExpressionNode(dynamic_cast<const ExpressionNode*>(node->expression.get()));
    int offset = getLocalVariableOffset(node->name);
    if (offset < 0) {
        emit("mov [rbp" + std::to_string(offset) + "], rax");
    } else {
        emit("mov [rbp+" + std::to_string(offset) + "], rax");
    }
}

void CodeGenerator::visitExpressionNode(const ExpressionNode* node) {
    if (node == nullptr) {
        return;
    }

    if (node->left && node->left.value()->getType() == NodeType::Literal) {
        visitLiteralNode(dynamic_cast<const LiteralNode*>(node->left->get()));
        emit("push rax");
    } else if (node->left) {
        visitExpressionNode(dynamic_cast<const ExpressionNode*>(node->left->get()));
        emit("push rax");
    }

    if (node->right && node->right.value()->getType() == NodeType::Literal) {
        visitLiteralNode(dynamic_cast<const LiteralNode*>(node->right->get()));
    } else if (node->right) {
        visitExpressionNode(dynamic_cast<const ExpressionNode*>(node->right->get()));
    }

    if (node->left) {
        emit("pop rbx");
    }

    if (node->op == "+") {
        emit("add rax, rbx");
    } else if (node->op == "-") {
        if (!node->left && node->right) { // Unary negation case
            emit("neg rax");
        } else {
            emit("sub rax, rbx");
        }
    } else if (node->op == "*") {
        emit("imul rax, rbx");
    } else if (node->op == "/") {
        emit("xor rdx, rdx");
        emit("idiv rbx");
    } else if (node->op == "==") {
        emit("cmp rax, rbx");
        emit("sete al");
        emit("movzx rax, al");
    } else if (node->op == "!=") {
        emit("cmp rax, rbx");
        emit("setne al");
        emit("movzx rax, al");
    } else if (node->op == "<") {
        emit("cmp rax, rbx");
        emit("setl al");
        emit("movzx rax, al");
    } else if (node->op == "<=") {
        emit("cmp rax, rbx");
        emit("setle al");
        emit("movzx rax, al");
    } else if (node->op == ">") {
        emit("cmp rax, rbx");
        emit("setg al");
        emit("movzx rax, al");
    } else if (node->op == ">=") {
        emit("cmp rax, rbx");
        emit("setge al");
        emit("movzx rax, al");
    } else if (node->op == "&") {
        emit("and rax, rbx");
    } else if (node->op == "|") {
        emit("or rax, rbx");
    } else if (node->op == "&&") {
        emit("and rax, rbx");
    } else if (node->op == "||") {
        emit("or rax, rbx");
    } else if (node->op == "!") {
        emit("cmp rax, 0");
        emit("sete al");
        emit("movzx rax, al");
    }
}

void CodeGenerator::visitReturnNode(const ReturnNode* node) {
    if (node->expression) {
        visitExpressionNode(dynamic_cast<const ExpressionNode*>(node->expression.get()));
    }

    if (totalLocalVarOffset > 0) {
        emit("add rsp, " + std::to_string(totalLocalVarOffset));
        totalLocalVarOffset = 0;
    }

    emit("jmp .L_return_" + currentFunctionName);
}

void CodeGenerator::visitIfNode(const IfNode* node) {
    std::string elseLabel = generateUniqueLabel();
    std::string endLabel = generateUniqueLabel();

    visitExpressionNode(dynamic_cast<const ExpressionNode*>(node->condition.get()));
    emit("cmp rax, 0");
    emit("je " + elseLabel);

    visitBlockNode(dynamic_cast<const BlockNode*>(node->body.get()));
    emit("jmp " + endLabel);

    emit(elseLabel + ":");
    if (node->else_ && node->else_.get()) {
        NodeType elseType = node->else_->getType();
        if (elseType == NodeType::Block) {
            visitBlockNode(dynamic_cast<const BlockNode*>(node->else_.get()));
        } else if (elseType == NodeType::If) {
            visitIfNode(dynamic_cast<const IfNode*>(node->else_.get()));
        }
    }

    emit(endLabel + ":");
}

void CodeGenerator::visitWhileNode(const WhileNode* node) {
    std::string startLabel = generateUniqueLabel();
    std::string endLabel = generateUniqueLabel();

    loopContextStack.push_back({startLabel, endLabel});

    emit(startLabel + ":");
    visitExpressionNode(dynamic_cast<const ExpressionNode*>(node->condition.get()));
    emit("cmp rax, 0");
    emit("je " + endLabel);

    visitBlockNode(dynamic_cast<const BlockNode*>(node->body.get()));
    emit("jmp " + startLabel);

    emit(endLabel + ":");

    loopContextStack.pop_back();
}

void CodeGenerator::visitBlockNode(const BlockNode* node) {
    if (!node) {
        std::cout << getGeneratedCode();
        printFatal("BlockNode cannot be null");
    }
    int localVarSize = calculateLocalVariableSize(node);

    if (localVarSize % 16 != 0) {
        localVarSize += 16 - (localVarSize % 16);
    }

    enterScope();
    if (localVarSize > 0) {
        emit("sub rsp, " + std::to_string(localVarSize));
        totalLocalVarOffset += localVarSize;
    }

    if (node->statements.empty()) {
        return;
    }
    for (const auto& statement : node->statements) {
        switch (statement->getType()) {
            case NodeType::VarDecl:
                visitVarDeclNode(dynamic_cast<const VarDeclNode*>(statement.get()));
                break;
            case NodeType::VarDeclAssign:
                visitVarDeclAssignNode(dynamic_cast<const VarDeclAssignNode*>(statement.get()));
                break;
            case NodeType::Assign:
                visitAssignNode(dynamic_cast<const AssignNode*>(statement.get()));
                break;
            case NodeType::Return:
                visitReturnNode(dynamic_cast<const ReturnNode*>(statement.get()));
                break;
            case NodeType::If:
                visitIfNode(dynamic_cast<const IfNode*>(statement.get()));
                break;
            case NodeType::While:
                visitWhileNode(dynamic_cast<const WhileNode*>(statement.get()));
                break;
            case NodeType::FunctionCall:
                visitFunctionCallNode(dynamic_cast<const FunctionCallNode*>(statement.get()));
                break;
            case NodeType::Switch:
                visitSwitchNode(dynamic_cast<const SwitchNode*>(statement.get())); 
                break;
            default:
                std::cout << std::endl << "Offender: " << toString(statement->getType()) << std::endl;
                printFatal("Unhandled node type in BlockNode");
                break;
        }
    }

    if (localVarSize > 0) {
        emit("add rsp, " + std::to_string(localVarSize));
        totalLocalVarOffset -= localVarSize;
    }
    exitScope();
}

void CodeGenerator::visitFunctionCallNode(const FunctionCallNode* node) {
    for (int i = node->arguments.size() - 1; i >= 0; --i) {
        visitExpressionNode(dynamic_cast<const ExpressionNode*>(node->arguments[i].get()));
        if (i < argumentRegisters.size()) {
            emit("mov " + argumentRegisters[i] + ", rax");
        } else {
            emit("push rax");
        }
    }
    emit("call " + node->name);
    emit("add rsp, " + std::to_string(8 * std::max(0, int(node->arguments.size()) - int(argumentRegisters.size()))));
}

void CodeGenerator::visitLiteralNode(const LiteralNode* node) {
    emit("mov rax, " + node->value);
}

// StringLiteral
// todo, allocate the string into .rodata and mov the addr of it into rax

void CodeGenerator::visitIdentifierNode(const IdentifierNode* node) {
    int offset = getLocalVariableOffset(node->name);
    emit("mov rax, [rbp" + std::to_string(offset) + "]");
}

void CodeGenerator::visitStructMemberAccessNode(const StructMemberAccessNode* node) {
    visitIdentifierNode(dynamic_cast<const IdentifierNode*>(node->base.get()));

    std::string structType = resolveTypeName(getVariableType(dynamic_cast<const IdentifierNode*>(node->base.get())->name));
    
    const auto& structDef = structDefinitions.find(structType);
    if (structDef == structDefinitions.end()) {
        printFatal("Struct type not found in definitions");
        __builtin_unreachable();
    }

    int memberOffset = 0;
    bool memberFound = false;
    for (const auto& member : structDef->second) {
        if (member == node->memberName) {
            memberFound = true;
            break;
        }
        memberOffset += resolveTypeSize(member);
    }

    if (!memberFound) {
        printFatal("Struct member not found");
        __builtin_unreachable();
    }

    emit("add rax, " + std::to_string(memberOffset));
}

void CodeGenerator::visitSwitchNode(const SwitchNode* node) {
    std::string endLabel = generateUniqueLabel();
    std::string defaultLabel = generateUniqueLabel();
    std::vector<std::string> caseLabels;

    for (size_t i = 0; i < node->cases.size(); ++i) {
        caseLabels.push_back(generateUniqueLabel());
    }

    visitExpressionNode(dynamic_cast<const ExpressionNode*>(node->condition.get()));
    emit("mov rbx, rax");
    
    for (size_t i = 0; i < node->cases.size(); ++i) {
        const auto& caseNode = dynamic_cast<const CaseNode*>(node->cases[i].get());
        if (caseNode) {
            visitBlockNode(dynamic_cast<const BlockNode*>(caseNode->body.get()));
            emit("cmp rbx, rax");
            emit("je " + caseLabels[i]);
        } else if (dynamic_cast<const DefaultNode*>(node->cases[i].get())) {
            emit("jmp " + defaultLabel);
        }
    }

    emit("jmp " + endLabel);

    for (size_t i = 0; i < node->cases.size(); ++i) {
        const auto& caseNode = dynamic_cast<const CaseNode*>(node->cases[i].get());
        emit(caseLabels[i] + ":");
        if (caseNode) {
            visitBlockNode(dynamic_cast<const BlockNode*>(caseNode->body.get()));
        }
    }

    emit(defaultLabel + ":");
    for (const auto& caseNode : node->cases) {
        if (auto defaultNode = dynamic_cast<const DefaultNode*>(caseNode.get())) {
            visitBlockNode(dynamic_cast<const BlockNode*>(defaultNode->body.get()));
            break;
        }
    }

    emit(endLabel + ":");
}

void CodeGenerator::visitBreakNode(const BreakNode* node) {
    if (!loopContextStack.empty()) {
        emit("jmp " + loopContextStack.back().endLabel);
    } else {
        printFatal("Break statement not within a loop");
    }
}

void CodeGenerator::visitContinueNode(const ContinueNode* node) {
    if (!loopContextStack.empty()) {
        emit("jmp " + loopContextStack.back().startLabel);
    } else {
        printFatal("Continue statement not within a loop");
    }
}

void CodeGenerator::visitTypedefNode(const TypedefNode* node) {
    // we actually dont need to do anything as the parser provides all the necessary information
}

void CodeGenerator::visitStructNode(const StructNode* node) {
    // we actually dont need to do anything as the parser provides all the necessary information
}

std::string CodeGenerator::generateLabel(const std::string& prefix) {
    return prefix + std::to_string(labelCounter++);
}

std::string CodeGenerator::generateUniqueLabel() {
    return "L" + std::to_string(labelCounter++);
}

void CodeGenerator::emit(const std::string& code) {
    generatedCode.push_back(code);
}

int CodeGenerator::calculateLocalVariableSize(const BlockNode* block) {
    int totalSize = 0;

    for (const auto& statement : block->statements) {
        switch (statement->getType()) {
            case NodeType::VarDecl: {
                const auto* varDeclNode = dynamic_cast<const VarDeclNode*>(statement.get());
                totalSize += resolveTypeSize(varDeclNode->type);
                break;
            }
            case NodeType::VarDeclAssign: {
                const auto* varDeclAssignNode = dynamic_cast<const VarDeclAssignNode*>(statement.get());
                totalSize += resolveTypeSize(varDeclAssignNode->type);
                break;
            }
            // each block handles its own local variables
            default:
                break;
        }
    }

    return totalSize;
}

void CodeGenerator::emitFunctionPrologue(const FunctionNode* node) {
    emit("section .text");
    emit(".global " + currentFunctionName);
    emit(currentFunctionName + ":");
    emit("push rbp");
    emit("mov rbp, rsp");
}

void CodeGenerator::emitFunctionEpilogue() {
    emit(".L_return_" + currentFunctionName + ":");
    emit("leave");
    emit("ret");
}

std::string CodeGenerator::resolveTypeName(const std::string& type) const {
    auto it = typedefs.find(type);
    if (it != typedefs.end()) {
        return it->second;
    }
    return type;
}

} // namespace EntS
