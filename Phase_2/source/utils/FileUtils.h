#ifndef GEOLOCAL_MINER_FILE_UTILS_H
#define GEOLOCAL_MINER_FILE_UTILS_H

#include <string>
#include <vector>
namespace GeoLocalMiner {

class FileUtils {
public:
    // Remove whitespace from both ends of a string.
    static std::string trim(const std::string& str);

    // Split a CSV row while handling quoted fields.
    static std::vector<std::string> splitCsvLine(
        const std::string& line,
        char delimiter = ','
    );

    // Convert text into a valid numeric value.
    static bool parseDouble(
        const std::string& str,
        double& outValue
    );

    static bool parseInt(
        const std::string& str,
        int& outValue
    );

    // Convert common textual representations into a boolean.
    static bool parseBool(
        const std::string& str,
        bool& outValue
    );

    // Check whether a file exists at the given path.
    static bool fileExists(const std::string& path);

    // Return the filename without its directory path.
    static std::string getFilename(const std::string& path);
};

} // namespace GeoLocalMiner

#endif // GEOLOCAL_MINER_FILE_UTILS_H