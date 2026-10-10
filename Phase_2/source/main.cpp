
#include "ui/Application.h"
#include "core/Grid.h"
#include "core/ProspectivityEngine.h"
#include "core/PriorityEngine.h"
#include "core/ValidationEngine.h"
#include "data/DataLoader.h"
#include <iostream>
#include <string>

void printBanner() {
    std::cout << "========================================================================\n"
              << "       GeoLocal-Miner: Multi-Scale Spatial Mineral Prospectivity        \n"
              << "            & Exploration Target Prioritization Decision-Support       \n"
              << "========================================================================\n"
              << " Team ID  : T260 (Avengers) | Graphic Era (Deemed to be University)     \n"
              << " Domain   : Geoscientific Computing & Spatial Mineral Systems           \n"
              << " Version  : 1.0.0 (C++ Zero-Dependency Engine)                           \n"
              << " Notice   : Prototype Decision-Support System. A high prospectivity    \n"
              << "            score indicates priority for further field investigation  \n"
              << "            and does NOT confirm mineral occurrence.                   \n"
              << "========================================================================\n"
              << std::endl;
}

void printUsage(const char* progName) {
    std::cout << "Usage: " << progName << " [OPTIONS]\n\n"
              << "Options:\n"
              << "  (no args)             Launch interactive desktop GUI\n"
              << "  --help, -h            Show this help manual\n"
              << "  --version, -v         Display project and team metadata\n"
              << "  --export [CSV] [OUT]  Evaluate dataset and export ranked targets\n"
              << "  --demo-csv            Generate or refresh the demo dataset\n"
              << std::endl;
}

int main(int argc, char* argv[]) {
    if (argc > 1) {
        std::string arg = argv[1];

        if (arg == "--help" || arg == "-h") {
            printBanner();
            printUsage(argv[0]);
            return 0;
        }

        if (arg == "--version" || arg == "-v") {
            printBanner();
            return 0;
        }

        if (arg == "--demo-csv") {
            printBanner();

            std::cout << "[*] Generating synthetic placer dataset (20x20)..."
                      << std::endl;

            bool success =
                GeoLocalMiner::DataLoader::saveSyntheticCsv(
                    "src/data/demo_dataset.csv", 20, 20
                );

            if (success) {
                std::cout << "[+] Demo dataset saved successfully."
                          << std::endl;
                return 0;
            }

            std::cerr << "[-] Failed to save demo dataset."
                      << std::endl;
            return 1;
        }

        if (arg == "--export") {
            printBanner();

            std::string inputFile = argc > 2
                ? argv[2]
                : "src/data/demo_dataset.csv";

            std::string outputFile = argc > 3
                ? argv[3]
                : "ranked_targets_export.csv";

            std::cout << "[*] Loading dataset: "
                      << inputFile << std::endl;

            GeoLocalMiner::Grid grid;

            GeoLocalMiner::LoadResult result =
                GeoLocalMiner::DataLoader::loadFromCsv(
                    inputFile, grid
                );

            if (!result.success) {
                std::cerr << "[!] Dataset loading failed: "
                          << result.error << std::endl;

                std::cout << "[*] Generating fallback synthetic dataset..."
                          << std::endl;

                GeoLocalMiner::DataLoader::generateSyntheticPlacerDataset(
                    grid, 20, 20
                );
            } else {
                std::cout << "[+] Loaded "
                          << result.rows << " rows."
                          << std::endl;
            }

            GeoLocalMiner::ProspectivityEngine prospectivityEngine;
            GeoLocalMiner::PriorityEngine priorityEngine;
            GeoLocalMiner::ValidationEngine validationEngine;

            prospectivityEngine.evaluateGrid(grid);
            priorityEngine.classifyGrid(grid);
            grid.detectClusters(0.70);

            auto rankedTargets = priorityEngine.rankTopN(grid, 20);
            auto validation = validationEngine.validate(grid);

            std::cout << "\n"
                      << validation.getSummaryString()
                      << "\n" << std::endl;

            std::string errorMessage;

            bool exported =
                GeoLocalMiner::DataLoader::exportRankedTargetsCsv(
                    outputFile, rankedTargets, errorMessage
                );

            if (exported) {
                std::cout << "[+] Exported "
                          << rankedTargets.size()
                          << " ranked targets to '"
                          << outputFile << "'."
                          << std::endl;
                return 0;
            }

            std::cerr << "[-] Export failed: "
                      << errorMessage << std::endl;
            return 1;
        }

        std::cerr << "[-] Unknown option: " << arg << "\n\n";
        printUsage(argv[0]);
        return 1;
    }

    printBanner();

    std::cout << "[*] Initializing GeoLocal-Miner Desktop GUI..."
              << std::endl;

    GeoLocalMiner::Application app;

    if (!app.initialize(
            1440,
            900,
            "GeoLocal-Miner: Multi-Scale Spatial Mineral Prospectivity")) {
        std::cerr << "[-] Failed to initialize the application window "
                     "or rendering context."
                  << std::endl;
        return 1;
    }

    std::cout << "[+] Application started. Running event loop."
              << std::endl;

    app.run();

    std::cout << "[*] Application terminated cleanly."
              << std::endl;

    return 0;
}
