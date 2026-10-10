#include "FileUtils.h"
#include <sstream>
#include <algorithm>
#include <cctype>
#include <fstream>

namespace GeoLocalMiner {

std::string FileUtils::trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return "";

    size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, last - first + 1);
}

std::vector<std::string> FileUtils::splitCsvLine(const std::string& line, char delimiter) {
    std::vector<std::string> fields;
    std::string current;
    bool inQuotes = false;

    for (size_t i = 0; i < line.length(); ++i) {
        char ch = line[i];

        if (ch == '"') {
            if (inQuotes && i + 1 < line.length() && line[i + 1] == '"') {
                current += '"';
                ++i;
            } else {
                inQuotes = !inQuotes;
            }
        } else if (ch == delimiter && !inQuotes) {
            fields.push_back(trim(current));
            current.clear();
        } else {
            current += ch;
        }
    }

    fields.push_back(trim(current));
    return fields;
}

bool FileUtils::parseDouble(const std::string& str, double& outValue) {
    std::string s = trim(str);
    if (s.empty()) return false;

    try {
        size_t idx = 0;
        outValue = std::stod(s, &idx);
        return idx == s.length();
    } catch (...) {
        return false;
    }
}

bool FileUtils::parseInt(const std::string& str, int& outValue) {
    std::string s = trim(str);
    if (s.empty()) return false;

    try {
        size_t idx = 0;
        outValue = std::stoi(s, &idx);
        return idx == s.length();
    } catch (...) {
        return false;
    }
}

bool FileUtils::parseBool(const std::string& str, bool& outValue) {
    std::string s = trim(str);
    std::string lower;

    for (char c : s) {
        lower += static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }

    if (lower == "1" || lower == "true" || lower == "yes" ||
        lower == "y" || lower == "t") {
        outValue = true;
        return true;
    }

    if (lower == "0" || lower == "false" || lower == "no" ||
        lower == "n" || lower == "f" || lower.empty()) {
        outValue = false;
        return true;
    }

    return false;
}

bool FileUtils::fileExists(const std::string& path) {
    std::ifstream file(path);
    return file.good();
}

std::string FileUtils::getFilename(const std::string& path) {
    size_t pos = path.find_last_of("/\\");
    return pos == std::string::npos ? path : path.substr(pos + 1);
}

}