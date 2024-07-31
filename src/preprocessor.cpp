#include "preprocessor.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <filesystem>
#include <regex>

namespace fs = std::filesystem;
extern void printFatal(const char* str);

namespace EntS {

Preprocessor::Preprocessor(const std::vector<std::string>& includePaths)
    : includePaths(includePaths) {}

std::optional<std::string> Preprocessor::preprocess(const std::string& filename) {
    std::string content = readFile(filename);
    if (content.empty()) {
        return std::nullopt;
    }

    std::istringstream stream(content);
    std::string line;
    std::ostringstream output;
    std::string currentDir = fs::path(filename).parent_path().string();

    while (std::getline(stream, line)) {
        if (line.empty()) {
            continue;
        }

        if (line.find("#include") == 0) {
            if (!handleInclude(line, currentDir, output)) {
                std::cerr << "Error in line:\n" << line << "\n";
                printFatal("failed to include file");
                return std::nullopt;
            }
        } else if (line.find("#define") == 0) {
            handleDefine(line);
            output << "\n";
        } else if (line.find("#undef") == 0) {
            handleUndef(line);
            output << "\n";
        } else if (line.find("header") == 0) {
            handleHeader(line, output);
        } else {
            output << replaceMacros(line) << "\n";
        }
    }

    return output.str();
}

bool Preprocessor::handleInclude(const std::string& line, const std::string& currentDir, std::ostringstream& output) {
    size_t start = line.find_first_of("\"<") + 1;
    size_t end = line.find_last_of("\">");

    if (start == std::string::npos || end == std::string::npos || start >= end) {
        return false;
    }

    std::string filename = line.substr(start, end - start);
    std::string fullPath = resolveIncludePath(filename, currentDir);
    std::string fileContent = readFile(fullPath);

    if (fileContent.empty()) {
        return false;
    }

    std::istringstream stream(fileContent);
    std::string fileLine;
    std::ostringstream headerContent;
    bool inHeaderBlock = false;

    while (std::getline(stream, fileLine)) {
        if (fileLine.find("header") == 0) {
            inHeaderBlock = true;
            headerContent << fileLine << "\n";
            continue;
        }
        if (inHeaderBlock && fileLine.find("};") == 0) {
            headerContent << fileLine << "\n";
            inHeaderBlock = false;
            break;
        }
        if (inHeaderBlock) {
            headerContent << fileLine << "\n";
        }
    }

    std::istringstream headerStream(headerContent.str());
    std::string headerLine;
    while (std::getline(headerStream, headerLine)) {
        if (headerLine.find("#define") == 0) {
            handleDefine(headerLine);
        } else if (headerLine.find("#undef") == 0) {
            handleUndef(headerLine);
        } else {
            output << replaceMacros(headerLine) << "\n";
        }
    }

    return true;
}

bool Preprocessor::handleDefine(const std::string& line) {
    size_t start = line.find_first_not_of(" \t", 7);
    size_t end = line.find_first_of(" \t", start);
    if (start == std::string::npos || end == std::string::npos) {
        return false;
    }

    std::string name = line.substr(start, end - start);
    std::string value = line.substr(end + 1);
    macros[name] = value;

    return true;
}

bool Preprocessor::handleUndef(const std::string& line) {
    size_t start = line.find_first_not_of(" \t", 6);
    if (start == std::string::npos) {
        return false;
    }

    std::string name = line.substr(start);
    macros.erase(name);

    return true;
}

bool Preprocessor::handleHeader(const std::string& line, std::ostringstream& output) {
    output << line << "\n";
    return true;
}

std::string Preprocessor::resolveIncludePath(const std::string& filename, const std::string& currentDir) {
    if (filename.front() == '"') {
        std::string localPath = currentDir + "/" + filename;
        if (fs::exists(localPath)) {
            return localPath;
        }
    } else if (filename.front() == '<') {
        for (const auto& path : includePaths) {
            std::string fullPath = path + "/" + filename;
            if (fs::exists(fullPath)) {
                return fullPath;
            }
        }
    }
    return filename;
}

std::string Preprocessor::readFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        return "";
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

std::string Preprocessor::replaceMacros(const std::string& line) {
    std::string result = line;
    for (const auto& [key, value] : macros) {
        result = std::regex_replace(result, std::regex("\\b" + key + "\\b"), value);
    }
    return result;
}

} // namespace EntS
