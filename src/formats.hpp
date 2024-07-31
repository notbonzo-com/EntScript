#ifndef FORMATS_HPP
#define FORMATS_HPP

#include <string_view>
#include <array>
#include <optional>
#include <utility>
#include <ranges>
#include <algorithm>

namespace EntS {
    
    enum class OutputFormat {
        ELF,
        OBJ,
        BIN
    };

    constexpr auto format_map = std::to_array<std::pair<std::string_view, OutputFormat>>({
        {"elf", OutputFormat::ELF},
        {"obj", OutputFormat::OBJ},
        {"bin", OutputFormat::BIN}
    });

    namespace outputParsing {
        constexpr std::optional<OutputFormat> getFormat(std::string_view format) {
            if (auto it = std::ranges::find_if(format_map, [&format](const auto& mapping) {
                return format == mapping.first;
            }); it != format_map.end()) {
                return it->second;
            }
            return std::nullopt;
        }

        constexpr std::string_view toString(OutputFormat format) {
            switch (format) {
                case OutputFormat::ELF: return "elf";
                case OutputFormat::OBJ: return "obj";
                case OutputFormat::BIN: return "bin";
                default: return "unknown"; /* Something bad has happened */
            }
        }
        
    } /* namespace outputParsing */
} /* namespace EntS */

#endif /* FORMATS_HPP */
