#include "StringUtils.h"
#include <algorithm>
#include <cctype>
#include <sstream>
#include <vector>

namespace StringUtils {

std::string Substring(const std::string &text, ssize_t begin, ssize_t finish) noexcept {
    if (finish == 0) finish = text.length();
    if (begin < 0) begin += text.length();
    if (finish < 0) finish += text.length();
    if (begin > finish) return "";
    return text.substr(begin, finish - begin);
}

std::string TitleCase(const std::string &text) noexcept {
    if (text.empty()) return text;
    std::string modified = text;
    if (std::isalpha(modified[0]))
        modified[0] = std::toupper(modified[0]);
    std::transform(modified.begin() + 1, modified.end(), modified.begin() + 1, ::tolower);
    return modified;
}

std::string ToUpper(const std::string &text) noexcept {
    std::string modified = text;
    std::transform(modified.begin(), modified.end(), modified.begin(), ::toupper);
    return modified;
}

std::string ToLower(const std::string &text) noexcept {
    std::string modified = text;
    std::transform(modified.begin(), modified.end(), modified.begin(), ::tolower);
    return modified;
}

std::string TrimLeft(const std::string &text) noexcept {
    size_t start = text.find_first_not_of(" \t\n\r");
    return (start == std::string::npos) ? "" : text.substr(start);
}

std::string TrimRight(const std::string &text) noexcept {
    size_t end = text.find_last_not_of(" \t\n\r");
    return (end == std::string::npos) ? "" : text.substr(0, end + 1);
}

std::string Trim(const std::string &text) noexcept {
    return TrimLeft(TrimRight(text));
}

std::string AlignCenter(const std::string &text, int width, char filler) noexcept {
    int padding = width - text.length();
    if (padding <= 0) return text;
    int left_padding = padding / 2;
    int right_padding = padding - left_padding;
    return std::string(left_padding, filler) + text + std::string(right_padding, filler);
}

std::string AlignLeft(const std::string &text, int width, char filler) noexcept {
    return text + std::string(std::max(0, width - static_cast<int>(text.size())), filler);
}

std::string AlignRight(const std::string &text, int width, char filler) noexcept {
    return std::string(std::max(0, width - static_cast<int>(text.size())), filler) + text;
}

std::string Substitute(const std::string &text, const std::string &old_value, const std::string &new_value) noexcept {
    if (old_value.empty()) return text;
    std::string modified = text;
    size_t position = 0;
    while ((position = modified.find(old_value, position)) != std::string::npos) {
        modified.replace(position, old_value.length(), new_value);
        position += new_value.length();
    }
    return modified;
}

std::vector<std::string> Tokenize(const std::string &text, const std::string &delimiter) noexcept {
    std::vector<std::string> result;
    if (text.empty()) return result;

    if (delimiter.empty()) {
        std::istringstream stream(text);
        std::string token;
        while (stream >> token) {
            result.push_back(token);
        }
        return result;
    }

    size_t start = 0;
    size_t end = text.find(delimiter);
    while (end != std::string::npos) {
        result.push_back(text.substr(start, end - start));
        start = end + delimiter.length();
        end = text.find(delimiter, start);
    }
    result.push_back(text.substr(start));
    return result;
}

std::string Concatenate(const std::string &separator, const std::vector<std::string> &tokens) noexcept {
    if (tokens.empty()) return "";
    std::string result = tokens[0];
    for (size_t index = 1; index < tokens.size(); ++index) {
        result += separator + tokens[index];
    }
    return result;
}

std::string ExpandTabulation(const std::string &text, int tabsize) noexcept {
    std::string result;
    int column = 0;
    if (tabsize == 0) {
        for (char character : text) {
            if (character != '\t') {
                result += character;
            }
        }
        return result;
    }
    for (char character : text) {
        if (character == '\t') {
            int spaces = tabsize - (column % tabsize);
            column += spaces;
            result.append(spaces, ' ');
        } else {
            result += character;
            column++;
        }
    }
    return result;
}

int ComputeEditDistance(const std::string &first, const std::string &second, bool ignore_case) noexcept {
    std::string left = ignore_case ? ToLower(first) : first;
    std::string right = ignore_case ? ToLower(second) : second;

    std::vector<std::vector<int>> table(left.length() + 1, std::vector<int>(right.length() + 1));

    for (size_t i = 0; i <= left.length(); i++) table[i][0] = i;
    for (size_t j = 0; j <= right.length(); j++) table[0][j] = j;

    for (size_t i = 1; i <= left.length(); i++) {
        for (size_t j = 1; j <= right.length(); j++) {
            table[i][j] = std::min({
                table[i - 1][j] + 1,
                table[i][j - 1] + 1,
                table[i - 1][j - 1] + (left[i - 1] != right[j - 1])
            });
        }
    }

    return table[left.length()][right.length()];
}

} // namespace StringUtils
