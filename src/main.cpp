#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include "preprocessor.hpp"
#include "lexer.hpp"
#include "tokens.hpp"
#include "formats.hpp"
#include "ast.hpp"
#include "parser.hpp"

constexpr std::string_view ANSI_RESET = "\033[0m";
constexpr std::string_view ANSI_BOLD_RED = "\033[1;31m";
constexpr std::string_view ANSI_BOLD_YELLOW = "\033[1;33m";
constexpr std::string_view ANSI_BOLD_WHITE = "\033[1;37m";

using namespace EntS;

void printFatal(const char* str) { 
    std::cout << ANSI_BOLD_WHITE << "ents: " << ANSI_BOLD_RED << "fatal error: " << ANSI_RESET << str << "\n" << "compilation terminated.\n"; 
}

void printError(const char* str) { 
    std::cout << ANSI_BOLD_WHITE << "ents: " << ANSI_BOLD_RED << "error: " << ANSI_RESET << str << "\n"; 
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
    std::cout << "EntS Compiler version 1.0.0\n";
}

std::string readFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        printFatal(("could not open file: " + filename).c_str());
        exit(1);
    }
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    return content;
}

int main(int argc, char** argv)
{
    if (argc < 2) {
        printFatal("no input files");
        return 1;
    }

    std::vector<std::string> inputFiles;
    std::string outputFile;
    bool generateAssemblyOnly = false;
    OutputFormat outputFormat = OutputFormat::ELF;
	std::vector<std::string> incPath = { "/usr/include/ents" };

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
                return 1;
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
        return 1;
    }

    Preprocessor preprocessor(incPath);

    for (const auto& inputFile : inputFiles) {
        auto preprocessedContent = preprocessor.preprocess(inputFile);
        if (!preprocessedContent) {
            printFatal(("failed to preprocess file: " + inputFile).c_str());
            return 1;
        }

        std::cout << "Preprocessed content:\n" << *preprocessedContent << "\n";

        Lexer lexer(*preprocessedContent);
        auto tokens = lexer.tokenize();

        Parser parser(tokens);
        parser.proc = &preprocessor;

        auto ast = parser.parse();
        if (!ast) {
            printFatal(("failed to parse file: " + inputFile).c_str());
            return 1;
        }

        std::cout << "AST:\n";
        ast->print();
    }

    return 0;
}
