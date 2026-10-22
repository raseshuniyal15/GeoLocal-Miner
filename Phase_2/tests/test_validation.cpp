#include "../src/core/ValidationEngine.h"
#include "../src/core/ProspectivityEngine.h"
#include "../src/core/PriorityEngine.h"
#include "../src/core/Grid.h"
#include "../src/data/DataLoader.h"
#include <iostream>
#include <cassert>

using namespace GeoLocalMiner;

void testValidationMetrics() {
    std::cout << "[TEST] Validation: AMD Known Occurrences Capture & Enrichment... ";
    Grid grid;
    DataLoader::generateSyntheticPlacerDataset(grid, 20, 20);

    ProspectivityEngine pe;
    PriorityEngine prie(0.70, 0.40);
    ValidationEngine ve;

    pe.evaluateGrid(grid);
    prie.classifyGrid(grid);

    ValidationMetrics metrics = ve.validate(grid);

    // Verify there are 8 known occurrences in the demo dataset
    assert(metrics.totalOccurrences == 8);
    // Known hotspots should be captured in High or Medium
    assert(metrics.capturedInHigh > 0);
    assert(metrics.highCaptureRate > 0.0);
    assert(metrics.highPlusMedCaptureRate >= metrics.highCaptureRate);
    assert(metrics.enrichmentFactor > 1.0); // Model must be better than random guessing

    std::cout << "PASSED (Total Occs: " << metrics.totalOccurrences 
              << ", High Captured: " << metrics.capturedInHigh
              << " (" << metrics.highCaptureRate << "%), Enrichment: " 
              << metrics.enrichmentFactor << "x)" << std::endl;
}

int main() {
    std::cout << "=== RUNNING TEST SUITE: VALIDATION ENGINE ===" << std::endl;
    testValidationMetrics();
    std::cout << ">>> ALL VALIDATION TESTS PASSED! <<<\n" << std::endl;
    return 0;
}
