#include "../src/utils/Normalization.h"
#include <iostream>
#include <cmath>
#include <vector>
#include <cassert>

using namespace GeoLocalMiner;

void testBasicMinMax() {
    std::cout << "[TEST] Normalization: Basic Min-Max Scaling... ";
    double val = Normalization::minMaxScale(50.0, 0.0, 100.0);
    assert(std::abs(val - 0.50) < 1e-6);

    double minVal = Normalization::minMaxScale(0.0, 0.0, 100.0);
    assert(std::abs(minVal - 0.0) < 1e-6);

    double maxVal = Normalization::minMaxScale(100.0, 0.0, 100.0);
    assert(std::abs(maxVal - 1.0) < 1e-6);

    std::cout << "PASSED" << std::endl;
}

void testEdgeCases() {
    std::cout << "[TEST] Normalization: Edge Cases (Uniform field max == min)... ";
    // Uniform field edge case
    double uniform = Normalization::minMaxScale(42.0, 42.0, 42.0, 0.5);
    assert(std::abs(uniform - 0.5) < 1e-6);

    // Negative range
    double neg = Normalization::minMaxScale(-5.0, -10.0, 0.0);
    assert(std::abs(neg - 0.5) < 1e-6);

    // Clamping out-of-bounds
    double over = Normalization::minMaxScale(150.0, 0.0, 100.0);
    assert(std::abs(over - 1.0) < 1e-6);

    double under = Normalization::minMaxScale(-20.0, 0.0, 100.0);
    assert(std::abs(under - 0.0) < 1e-6);

    std::cout << "PASSED" << std::endl;
}

void testBatchNormalization() {
    std::cout << "[TEST] Normalization: Vector Batch Normalization... ";
    std::vector<double> raw = { 10.0, 20.0, 30.0, 40.0, 50.0 };
    auto norm = Normalization::minMaxNormalize(raw);

    assert(norm.size() == 5);
    assert(std::abs(norm[0] - 0.0) < 1e-6);
    assert(std::abs(norm[2] - 0.5) < 1e-6);
    assert(std::abs(norm[4] - 1.0) < 1e-6);

    std::cout << "PASSED" << std::endl;
}

int main() {
    std::cout << "=== RUNNING TEST SUITE: NORMALIZATION ===" << std::endl;
    testBasicMinMax();
    testEdgeCases();
    testBatchNormalization();
    std::cout << ">>> ALL NORMALIZATION TESTS PASSED! <<<\n" << std::endl;
    return 0;
}
