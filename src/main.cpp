#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <filesystem>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/Bitcode/BitcodeWriter.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/Host.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Target/TargetOptions.h>
#include <llvm/MC/TargetRegistry.h>

#include "preprocessor.hpp"
#include "lexer.hpp"
#include "tokens.hpp"
#include "formats.hpp"
#include "ast.hpp"
#include "parser.hpp"
#include "codegenerator.hpp"

constexpr std::string_view ANSI_RESET = "\033[0m";
constexpr std::string_view ANSI_BOLD_RED = "\033[1;31m";
constexpr std::string_view ANSI_BOLD_YELLOW = "\033[1;33m";
constexpr std::string_view ANSI_BOLD_WHITE = "\033[1;37m";

constexpr std::string_view version = "0.1.0";

constexpr std::string_view libDir = SYSROOT"/lib/ents";
constexpr std::string_view incDir = SYSROOT"/include/ents";

using namespace EntS;

void printFatal(const char* str) { 
    std::cout << ANSI_BOLD_WHITE << "ents: " << ANSI_BOLD_RED << "fatal error: " << ANSI_RESET << str << "\n" << "compilation terminated.\n"; 
    exit(1);
}
void printError(const char* str) { 
    std::cout << ANSI_BOLD_WHITE << "ents: " << ANSI_BOLD_RED << "error: " << ANSI_RESET << str << "\n"; 
    exit(1);
}
void printWarning(const char* str) { 
    std::cout << ANSI_BOLD_WHITE << "ents: " << ANSI_BOLD_YELLOW << "warning: " << ANSI_RESET << str << "\n"; 
}

void printHelp() {
    std::cout << "Usage: ents [options] <input-files>\n"
              << "Options:\n"
              << "  -h, --help            Display this help message\n"
              << "  -v, --version         Display version information\n"
              << "  -o, --output <file>   Specify output file\n"
              << "  -S                    Generate assembly code only\n"
              << "  -f, --format <format> Specify output format (obj, elf; default is elf)\n"
              << "  -I, --include <path>  Adds a specific folder into the include path\n";
}

void printVersion() {
    std::cout << "EntS Compiler version " << version << "\n";
}

std::string readFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        printFatal(("could not open file: " + filename).c_str());
    }
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    return content;
}

int main(int argc, char** argv) {
    if (argc < 2) {
        printFatal("no input files");
    }

    std::vector<std::string> inputFiles;
    std::string outputFile = "a.out";
    bool generateAssemblyOnly = false;
    OutputFormat outputFormat = OutputFormat::ELF;
    std::vector<std::string> incPath = { std::string(incDir) };

    std::vector<std::string> checkDirs = { std::string(libDir) + "/crt0.o", std::string(libDir) + "/intlibe.a" };
    for (const auto& dir : checkDirs) {
        if (!std::filesystem::exists(dir)) {
            printFatal(("library file not found: " + dir).c_str());
        }
    }

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-h" || arg == "--help") {
            printHelp();
            return 0;
        } else if (arg == "-v" || arg == "--version") {
            printVersion();
            return 0;
        } else if ((arg == "-o" || arg == "--output") && i + 1 < argc) {
            outputFile = argv[++i];
        } else if (arg == "-S") {
            generateAssemblyOnly = true;
        } else if ((arg == "-f" || arg == "--format") && i + 1 < argc) {
            std::string formatStr = argv[++i];
            auto formatOpt = outputParsing::getFormat(formatStr);
            if (!formatOpt) {
                printFatal("invalid format specifier");
            }
            outputFormat = *formatOpt;
        } else if ((arg == "-I" || arg == "--include") && i + 1 < argc) {
            incPath.push_back(argv[++i]);
        } else if (arg[0] != '-') {
            inputFiles.push_back(arg);
        } else {
            printWarning(("unknown flag: " + arg).c_str());
        }
    }

    if (inputFiles.empty()) {
        printFatal("no input files");
    }

    LLVMInitializeX86TargetInfo();
    LLVMInitializeX86Target();
    LLVMInitializeX86TargetMC();
    LLVMInitializeX86AsmPrinter();
    LLVMInitializeX86AsmParser();

    std::string targetTriple = llvm::sys::getDefaultTargetTriple();
    std::string error;
    const llvm::Target* target = llvm::TargetRegistry::lookupTarget(targetTriple, error);

    if (!target) {
        printFatal(("Could not find target: " + error).c_str());
    }

    llvm::TargetOptions opt;
    auto RM = llvm::Optional<llvm::Reloc::Model>();
    llvm::TargetMachine* TM = target->createTargetMachine(targetTriple, "generic", "", opt, RM);

    for (const auto& inputFile : inputFiles) {
        Preprocessor preprocessor(incPath);
        auto preprocessedContent = preprocessor.preprocess(inputFile);
        if (!preprocessedContent) {
            printFatal(("failed to preprocess file: " + inputFile).c_str());
        }

        Lexer lexer(*preprocessedContent);
        auto tokens = lexer.tokenize();

        Parser parser(tokens);
        auto ast = parser.parse();

        CodeGenerator codeGen(inputFile, parser.getTypedefs());
        llvm::Module* module = codeGen.generateCode(ast);

        if (!module) {
            printFatal("failed to generate code");
        }

        module->setDataLayout(TM->createDataLayout());
        module->setTargetTriple(targetTriple);

        llvm::legacy::PassManager pass;

        std::error_code EC;
        llvm::raw_fd_ostream dest(outputFile, EC, llvm::sys::fs::OF_None);

        if (EC) {
            printFatal(("could not open output file: " + outputFile).c_str());
        }

        if (generateAssemblyOnly) {
            if (TM->addPassesToEmitFile(pass, dest, nullptr, llvm::CGFT_AssemblyFile)) {
                printFatal("target does not support generation of assembly files");
            }
        } else {
            if (outputFormat == OutputFormat::ELF) {
                llvm::WriteBitcodeToFile(*module, dest);
            } else if (outputFormat == OutputFormat::OBJ) {
                if (TM->addPassesToEmitFile(pass, dest, nullptr, llvm::CGFT_ObjectFile)) {
                    printFatal("target does not support generation of object files");
                }
            }
        }

        pass.run(*module);
    }

    return 0;
}
