#include "ValidationEngine.h"
#include "Grid.h"
#include <sstream>
#include <iomanip>

namespace GeoLocalMiner {

std::string ValidationMetrics::getSummaryString() const {
    std::ostringstream oss;
    oss << "Prototype Validation Summary (Reference AMD Occurrences):\n"
        << "  Total Known Occurrences : " << totalOccurrences << "\n"
        << "  Captured in HIGH Priority : " << capturedInHigh << " (" << std::fixed << std::setprecision(1) << highCaptureRate << "%)\n"
        << "  Captured in MEDIUM Priority : " << capturedInMedium << "\n"
        << "  Captured in LOW Priority : " << capturedInLow << "\n"
        << "  High-Priority Area Footprint : " << std::fixed << std::setprecision(1) << highAreaPercentage << "%\n"
        << "  Enrichment / Efficiency Ratio : " << std::fixed << std::setprecision(2) << enrichmentFactor << "x\n"
        << "  [Disclaimer: Decision-support metric only. Does not guarantee mineral presence.]";
    return oss.str();
}

ValidationMetrics ValidationEngine::validate(const Grid& grid) const {
    ValidationMetrics metrics;
    size_t totalCells = grid.getRows() * grid.getCols();
    if (totalCells == 0) return metrics;

    size_t totalHighCells = 0;

    for (size_t r = 0; r < grid.getRows(); ++r) {
        for (size_t c = 0; c < grid.getCols(); ++c) {
            const Zone& z = grid.getZone(r, c);
            if (z.getPriority() == PriorityLevel::HIGH) {
                totalHighCells++;
            }

            if (z.hasKnownOccurrence()) {
                metrics.totalOccurrences++;
                metrics.occurrenceZones.push_back(z);

                if (z.getPriority() == PriorityLevel::HIGH) {
                    metrics.capturedInHigh++;
                } else if (z.getPriority() == PriorityLevel::MEDIUM) {
                    metrics.capturedInMedium++;
                } else {
                    metrics.capturedInLow++;
                }
            }
        }
    }

    if (metrics.totalOccurrences > 0) {
        metrics.highCaptureRate = (static_cast<double>(metrics.capturedInHigh) / 
                                   static_cast<double>(metrics.totalOccurrences)) * 100.0;
        metrics.highPlusMedCaptureRate = (static_cast<double>(metrics.capturedInHigh + metrics.capturedInMedium) / 
                                          static_cast<double>(metrics.totalOccurrences)) * 100.0;
    }

    metrics.highAreaPercentage = (static_cast<double>(totalHighCells) / static_cast<double>(totalCells)) * 100.0;

    if (metrics.highAreaPercentage > 1e-4) {
        // Enrichment = (% captured) / (% area occupied by high priority)
        metrics.enrichmentFactor = metrics.highCaptureRate / metrics.highAreaPercentage;
    } else {
        metrics.enrichmentFactor = 0.0;
    }

    return metrics;
}

} // namespace GeoLocalMiner
