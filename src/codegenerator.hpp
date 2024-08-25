#ifndef CODEGENERATOR_HPP
#define CODEGENERATOR_HPP

#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Verifier.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Value.h>
#include <memory>
#include "ast.hpp"
#include <unordered_map>
#include <vector>

namespace EntS {

class CodeGenerator {
public:
    CodeGenerator(const std::string& moduleName, const std::unordered_map<std::string, std::string>& typedefs);

};

} // namespace EntS

#endif // CODEGENERATOR_HPP
