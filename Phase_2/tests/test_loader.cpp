#include "../src/data/DataLoader.h"
#include "../src/core/Grid.h"
#include "../src/core/ProspectivityEngine.h"
#include "../src/core/PriorityEngine.h"
#include "../src/utils/FileUtils.h"
#include <iostream>
#include <fstream>
#include <cassert>

using namespace GeoLocalMiner;

void testCsvLoading() {
    std::cout << "[TEST] DataLoader: Loading CSV File... ";
    Grid grid;
    std::string demoPath = "src/data/demo_dataset.csv";
    if (!FileUtils::fileExists(demoPath)) {
        demoPath = "../src/data/demo_dataset.csv";
    }
    LoadResult res = DataLoader::loadFromCsv(demoPath, grid);

    assert(res.success);
    assert(grid.getRows() == 20);
    assert(grid.getCols() == 20);
    assert(grid.getTotalCells() == 400);

    // Verify cell indicators were populated
    const Zone& z0 = grid.getZone(0, 0);
    assert(z0.getId() == 0);
    assert(z0.getX() > 0.0);
    assert(z0.getY() > 0.0);

    std::cout << "PASSED (Loaded " << res.rowsLoaded << " cells)" << std::endl;
}

void testMalformedCsvHandling() {
    std::cout << "[TEST] DataLoader: Graceful Handling of Malformed CSV... ";
    // Write temporary corrupt CSV
    std::string testPath = "test_corrupt.csv";
    {
        std::ofstream out(testPath.c_str());
        out << "row,col,x,y,G,R,S,T,known_occurrence\n";
        out << "0,0,100,100,0.5,0.5,0.5,0.5,0\n";
        out << "invalid,row,with,missing,fields\n"; // Malformed line
        out << "0,1,200,100,0.8,0.7,0.6,0.5,1\n";
        out << "1,0,100,200,bad_num,0.5,0.5,0.5,0\n"; // Bad number line
        out << "1,1,200,200,0.9,0.9,0.9,0.9,0\n";
    }

    Grid grid;
    LoadResult res = DataLoader::loadFromCsv(testPath, grid);
    if (!res.success) {
        std::cerr << "\n[DEBUG ERROR] " << res.errorMessage << std::endl;
    }
    assert(res.success);
    assert(res.warningCount > 0); // Warnings caught rather than crashing
    assert(grid.getRows() == 2);
    assert(grid.getCols() == 2);

    // Clean up
    std::remove(testPath.c_str());
    std::cout << "PASSED (Warnings caught: " << res.warningCount << ")" << std::endl;
}

void testExportCsv() {
    std::cout << "[TEST] DataLoader: Target & Grid CSV Export... ";
    Grid grid;
    DataLoader::generateSyntheticPlacerDataset(grid, 10, 10);

    ProspectivityEngine pe;
    PriorityEngine prie;
    pe.evaluateGrid(grid);
    prie.classifyGrid(grid);

    auto targets = prie.rankTopN(grid, 5);
    std::string err;
    bool ok1 = DataLoader::exportRankedTargetsCsv("test_export_targets.csv", targets, err);
    assert(ok1);
    assert(FileUtils::fileExists("test_export_targets.csv"));

    bool ok2 = DataLoader::exportGridCsv("test_export_grid.csv", grid, err);
    assert(ok2);
    assert(FileUtils::fileExists("test_export_grid.csv"));

    // Clean up
    std::remove("test_export_targets.csv");
    std::remove("test_export_grid.csv");

    std::cout << "PASSED" << std::endl;
}

int main() {
    std::cout << "=== RUNNING TEST SUITE: DATA LOADER & FILE I/O ===" << std::endl;
    testCsvLoading();
    testMalformedCsvHandling();
    testExportCsv();
    std::cout << ">>> ALL DATA LOADER TESTS PASSED! <<<\n" << std::endl;
    return 0;
}
