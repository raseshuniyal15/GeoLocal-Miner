#include "../src/core/PriorityEngine.h"
#include "../src/core/Grid.h"
#include "../src/core/Zone.h"
#include <iostream>
#include <vector>
#include <cassert>

using namespace GeoLocalMiner;

void testClassification() {
    std::cout << "[TEST] Ranking: Priority Classification Thresholds... ";
    PriorityEngine engine(0.70, 0.40);

    assert(engine.classifyScore(0.95) == PriorityLevel::HIGH);
    assert(engine.classifyScore(0.70) == PriorityLevel::HIGH); // Boundary case
    assert(engine.classifyScore(0.699) == PriorityLevel::MEDIUM);
    assert(engine.classifyScore(0.40) == PriorityLevel::MEDIUM); // Boundary case
    assert(engine.classifyScore(0.399) == PriorityLevel::LOW);
    assert(engine.classifyScore(0.0) == PriorityLevel::LOW);

    // Dynamic threshold update
    std::string err;
    bool ok = engine.setThresholds(0.80, 0.50, err);
    assert(ok);
    assert(engine.classifyScore(0.75) == PriorityLevel::MEDIUM); // Was high under 0.70, now med under 0.80
    assert(engine.classifyScore(0.85) == PriorityLevel::HIGH);

    std::cout << "PASSED" << std::endl;
}

void testPriorityQueueHeapOrdering() {
    std::cout << "[TEST] Ranking: Priority Queue Heap Order (Top-N Extraction)... ";
    Grid grid(3, 3, 0.0, 0.0, 1000.0);

    // Assign specific scores across 9 cells
    double scores[3][3] = {
        { 0.15, 0.85, 0.42 },
        { 0.92, 0.33, 0.76 },
        { 0.60, 0.99, 0.25 }
    };

    for (size_t r = 0; r < 3; ++r) {
        for (size_t c = 0; c < 3; ++c) {
            grid.getZone(r, c).setProspectivityScore(scores[r][c]);
        }
    }

    PriorityEngine engine(0.70, 0.40);
    auto top4 = engine.rankTopN(grid, 4);

    assert(top4.size() == 4);
    // Highest must be 0.99 (row 2, col 1)
    assert(top4[0].getProspectivityScore() == 0.99);
    // Second must be 0.92 (row 1, col 0)
    assert(top4[1].getProspectivityScore() == 0.92);
    // Third must be 0.85 (row 0, col 1)
    assert(top4[2].getProspectivityScore() == 0.85);
    // Fourth must be 0.76 (row 1, col 2)
    assert(top4[3].getProspectivityScore() == 0.76);

    std::cout << "PASSED" << std::endl;
}

int main() {
    std::cout << "=== RUNNING TEST SUITE: PRIORITY & RANKING ENGINE ===" << std::endl;
    testClassification();
    testPriorityQueueHeapOrdering();
    std::cout << ">>> ALL RANKING TESTS PASSED! <<<\n" << std::endl;
    return 0;
}
