#ifndef DATA_LOADER_H
#define DATA_LOADER_H

#include <string>
#include <vector>

#include "../core/Grid.h"
#include "../core/Zone.h"

namespace GeoLocalMiner {

struct LoadResult
{
    bool success;
    std::string error;
    size_t rows;
    size_t warnings;
    std::vector<std::string> warningList;

    LoadResult()
        : success(false),
          error(""),
          rows(0),
          warnings(0)
    {
    }
};


class DataLoader
{
public:

    static LoadResult loadFromCsv(
        const std::string& path,
        Grid& grid
    );

    static bool exportRankedTargetsCsv(
        const std::string& path,
        const std::vector<Zone>& zones,
        std::string& error
    );

    static bool exportGridCsv(
        const std::string& path,
        const Grid& grid,
        std::string& error
    );

    static void generateSyntheticPlacerDataset(
        Grid& grid,
        size_t rows = 20,
        size_t cols = 20
    );

    static bool saveSyntheticCsv(
        const std::string& path,
        size_t rows = 20,
        size_t cols = 20
    );
};

}

#endif