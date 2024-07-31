#ifndef PREPROCESSOR_HPP
#define PREPROCESSOR_HPP

#include <string>
#include <vector>
#include <unordered_map>
#include <optional>

namespace EntS {

class Preprocessor {
public:
    Preprocessor(const std::vector<std::string>& includePaths);
    std::optional<std::string> preprocess(const std::string& filename);

private:
    bool handleInclude(const std::string& line, const std::string& currentDir, std::ostringstream& output);
    bool handleDefine(const std::string& line);
    bool handleUndef(const std::string& line);
    bool handleHeader(const std::string& line, std::ostringstream& output);
    std::string resolveIncludePath(const std::string& filename, const std::string& currentDir);
    std::string readFile(const std::string& filename);
    std::string replaceMacros(const std::string& line);

    std::vector<std::string> includePaths;
    std::unordered_map<std::string, std::string> macros;
};

} // namespace EntS

#endif // PREPROCESSOR_HPP
