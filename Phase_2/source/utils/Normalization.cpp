#include "Normalization.h"

#include <algorithm>
#include <cmath>
#include <limits>

namespace GeoLocalMiner {

double Normalization::clamp(double value, double lower, double upper)
{
    if (!std::isfinite(value)) {
        return lower;
    }

    if (lower > upper) {
        std::swap(lower, upper);
    }

    return std::max(lower, std::min(value, upper));
}

double Normalization::minMaxScale(
    double value,
    double minVal,
    double maxVal,
    double defaultValue)
{
    if (!std::isfinite(value) ||
        !std::isfinite(minVal) ||
        !std::isfinite(maxVal)) {
        return clamp(defaultValue);
    }

    const double range = maxVal - minVal;

    if (std::abs(range) < 1e-9) {
        return clamp(defaultValue);
    }

    const double scaled = (value - minVal) / range;
    return clamp(scaled);
}

bool Normalization::computeMinMax(
    const std::vector<double>& values,
    double& outMin,
    double& outMax)
{
    outMin = 0.0;
    outMax = 0.0;

    double minValue = std::numeric_limits<double>::infinity();
    double maxValue = -std::numeric_limits<double>::infinity();
    bool foundValidValue = false;

    for (double value : values) {
        if (!std::isfinite(value)) {
            continue;
        }

        minValue = std::min(minValue, value);
        maxValue = std::max(maxValue, value);
        foundValidValue = true;
    }

    if (!foundValidValue) {
        return false;
    }

    outMin = minValue;
    outMax = maxValue;
    return true;
}

std::vector<double> Normalization::minMaxNormalize(
    const std::vector<double>& rawValues,
    double defaultValue)
{
    if (rawValues.empty()) {
        return {};
    }

    double minValue;
    double maxValue;

    if (!computeMinMax(rawValues, minValue, maxValue)) {
        return std::vector<double>(
            rawValues.size(),
            clamp(defaultValue)
        );
    }

    std::vector<double> normalized;
    normalized.reserve(rawValues.size());

    for (double value : rawValues) {
        normalized.push_back(
            minMaxScale(value, minValue, maxValue, defaultValue)
        );
    }

    return normalized;
}

double Normalization::sigmoidNormalize(
    double value,
    double mean,
    double stdDev)
{
    if (!std::isfinite(value) ||
        !std::isfinite(mean) ||
        !std::isfinite(stdDev) ||
        std::abs(stdDev) < 1e-9) {
        return 0.5;
    }

    const double z = (value - mean) / stdDev;

    // Prevent extreme values from overflowing exp().
    if (z >= 40.0) return 1.0;
    if (z <= -40.0) return 0.0;

    return 1.0 / (1.0 + std::exp(-z));
}

} // namespace GeoLocalMiner