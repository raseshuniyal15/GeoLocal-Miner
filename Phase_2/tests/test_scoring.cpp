#include "../src/core/ProspectivityEngine.h"
#include "../src/core/ScoringStrategy.h"
#include "../src/core/Zone.h"
#include <iostream>
#include <cmath>
#include <cassert>

using namespace GeoLocalMiner;

void testScoreCalculationBoundary() {
    std::cout << "[TEST] Scoring: Boundary Verification (All 1s and All 0s)... ";
    ProspectivityEngine engine;
    engine.forceSetWeights(0.35, 0.30, 0.20, 0.15);

    // Test case: All 1s -> Score must be 1.0
    Zone maxZone(1, 0, 0, 0.0, 0.0);
    maxZone.setNormalizedIndicators(1.0, 1.0, 1.0, 1.0);
    engine.evaluateZone(maxZone);
    assert(std::abs(maxZone.getProspectivityScore() - 1.0) < 1e-6);

    // Test case: All 0s -> Score must be 0.0
    Zone minZone(2, 0, 0, 0.0, 0.0);
    minZone.setNormalizedIndicators(0.0, 0.0, 0.0, 0.0);
    engine.evaluateZone(minZone);
    assert(std::abs(minZone.getProspectivityScore() - 0.0) < 1e-6);

    std::cout << "PASSED" << std::endl;
}

void testKnownIntermediateValues() {
    std::cout << "[TEST] Scoring: Known Intermediate Mathematical Formula... ";
    ProspectivityEngine engine;
    std::string err;
    bool ok = engine.setWeights(0.35, 0.30, 0.20, 0.15, err);
    assert(ok);

    // G=0.8, R=0.6, S=0.4, T=0.2
    // Score = 0.35*0.8 + 0.30*0.6 + 0.20*0.4 + 0.15*0.2
    //       = 0.28 + 0.18 + 0.08 + 0.03 = 0.570
    Zone z(10, 2, 3, 2000.0, 3000.0);
    z.setNormalizedIndicators(0.8, 0.6, 0.4, 0.2);
    engine.evaluateZone(z);

    assert(std::abs(z.getProspectivityScore() - 0.570) < 1e-5);
    std::cout << "PASSED (Computed: " << z.getProspectivityScore() << ")" << std::endl;
}

void testWeightValidation() {
    std::cout << "[TEST] Scoring: Weight Validation Rules... ";
    ProspectivityEngine engine;
    std::string err;

    // Negative weights rejected
    bool res1 = engine.setWeights(-0.1, 0.4, 0.4, 0.3, err);
    assert(!res1);

    // Weights not summing to 1.0 rejected
    bool res2 = engine.setWeights(0.5, 0.5, 0.5, 0.5, err);
    assert(!res2);

    // Valid weights accepted
    bool res3 = engine.setWeights(0.25, 0.25, 0.25, 0.25, err);
    assert(res3);

    // Auto-normalize
    engine.forceSetWeights(2.0, 2.0, 2.0, 2.0);
    engine.normalizeWeights();
    assert(std::abs(engine.getWeightG() - 0.25) < 1e-5);
    assert(std::abs(engine.getWeightSum() - 1.0) < 1e-5);

    std::cout << "PASSED" << std::endl;
}

void testPolymorphicStrategies() {
    std::cout << "[TEST] Scoring: OOP Polymorphism with Alternative Strategies... ";
    ProspectivityEngine engine;
    engine.forceSetWeights(0.35, 0.30, 0.20, 0.15);

    Zone z(5, 1, 1, 100.0, 100.0);
    z.setNormalizedIndicators(0.1, 0.1, 0.9, 0.9); // Low source/rad, high structure/terrain

    // Linear strategy
    engine.setStrategyByIndex(0);
    engine.evaluateZone(z);
    double linearScore = z.getProspectivityScore(); // 0.35*0.1 + 0.30*0.1 + 0.20*0.9 + 0.15*0.9 = 0.035 + 0.03 + 0.18 + 0.135 = 0.380
    assert(std::abs(linearScore - 0.380) < 1e-4);

    // Geological-gated strategy: should penalize because gate = max(0.1, 0.1) = 0.1 -> gatingFactor = 0.3 + 0.7*0.1 = 0.37
    engine.setStrategyByIndex(1);
    engine.evaluateZone(z);
    double gatedScore = z.getProspectivityScore();
    assert(gatedScore < linearScore);
    assert(std::abs(gatedScore - (0.380 * 0.37)) < 1e-3);

    std::cout << "PASSED (Linear=" << linearScore << ", Gated=" << gatedScore << ")" << std::endl;
}

int main() {
    std::cout << "=== RUNNING TEST SUITE: SCORING ENGINE ===" << std::endl;
    testScoreCalculationBoundary();
    testKnownIntermediateValues();
    testWeightValidation();
    testPolymorphicStrategies();
    std::cout << ">>> ALL SCORING TESTS PASSED! <<<\n" << std::endl;
    return 0;
}
