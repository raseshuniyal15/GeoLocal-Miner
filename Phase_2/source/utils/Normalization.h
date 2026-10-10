#ifndef GEOLOCAL_MINER_NORMALIZATION_H
#define GEOLOCAL_MINER_NORMALIZATION_H

#include <string>
#include <vector>

namespace GeoLocalMiner {

class Normalization {
public:
    // Scale a value into the [0, 1] range.
    static double minMaxScale(
        double value,
        double minVal,
        double maxVal,
        double defaultValue = 0.5
    );

    // Normalize all values using the same minimum and maximum.
    static std::vector<double> minMaxNormalize(
        const std::vector<double>& rawValues,
        double defaultValue = 0.5
    );

    // Find the minimum and maximum finite values.
    // Returns false when no valid values are available.
    static bool computeMinMax(
        const std::vector<double>& values,
        double& outMin,
        double& outMax
    );

    // Keep a value within the requested range.
    static double clamp(
        double value,
        double lower = 0.0,
        double upper = 1.0
    );

    // Convert a value into a sigmoid-normalized score.
    static double sigmoidNormalize(
        double value,
        double mean,
        double stdDev
    );
};

} // namespace GeoLocalMiner

#endif // GEOLOCAL_MINER_NORMALIZATION_H