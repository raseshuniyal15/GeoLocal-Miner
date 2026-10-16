#include "PriorityEngine.h"
#include "Grid.h"
#include <algorithm>

namespace GeoLocalMiner {

struct ZoneHeapComparator {
    bool operator()(const Zone& a, const Zone& b) const {
        return a.getProspectivityScore() < b.getProspectivityScore(); // For max-heap
    }
};

PriorityEngine::PriorityEngine()
    : highThreshold(0.70), lowThreshold(0.40) {}

PriorityEngine::PriorityEngine(double highCutoff, double lowCutoff)
    : highThreshold(highCutoff), lowThreshold(lowCutoff) {}

bool PriorityEngine::setThresholds(double high, double low, std::string& errorMsg) {
    if (low < 0.0 || high > 1.0) {
        errorMsg = "Thresholds must be within the range [0.0, 1.0].";
        return false;
    }
    if (low >= high) {
        errorMsg = "Low threshold must be strictly less than High threshold.";
        return false;
    }
    highThreshold = high;
    lowThreshold = low;
    return true;
}

void PriorityEngine::forceSetThresholds(double high, double low) {
    highThreshold = high;
    lowThreshold = low;
}

PriorityLevel PriorityEngine::classifyScore(double score) const {
    if (score >= highThreshold) {
        return PriorityLevel::HIGH;
    } else if (score >= lowThreshold) {
        return PriorityLevel::MEDIUM;
    } else {
        return PriorityLevel::LOW;
    }
}

void PriorityEngine::classifyZone(Zone& zone) const {
    zone.setPriority(classifyScore(zone.getProspectivityScore()));
}

PriorityCounts PriorityEngine::classifyGrid(Grid& grid) const {
    PriorityCounts counts;
    counts.totalCount = grid.getRows() * grid.getCols();

    for (size_t r = 0; r < grid.getRows(); ++r) {
        for (size_t c = 0; c < grid.getCols(); ++c) {
            Zone& z = grid.getZone(r, c);
            classifyZone(z);
            if (z.getPriority() == PriorityLevel::HIGH) {
                counts.highCount++;
            } else if (z.getPriority() == PriorityLevel::MEDIUM) {
                counts.mediumCount++;
            } else {
                counts.lowCount++;
            }
        }
    }
    return counts;
}

std::vector<Zone> PriorityEngine::rankTopN(const Grid& grid, size_t n) const {
    CustomPriorityQueue<Zone, ZoneHeapComparator> pq;

    for (size_t r = 0; r < grid.getRows(); ++r) {
        for (size_t c = 0; c < grid.getCols(); ++c) {
            pq.push(grid.getZone(r, c));
        }
    }

    return pq.extractTopN(n);
}

std::vector<Zone> PriorityEngine::rankSubregionTopN(const Grid& grid, size_t rStart, size_t rEnd,
                                                    size_t cStart, size_t cEnd, size_t n) const {
    CustomPriorityQueue<Zone, ZoneHeapComparator> pq;

    size_t rows = grid.getRows();
    size_t cols = grid.getCols();

    size_t r0 = std::min(rStart, rows > 0 ? rows - 1 : 0);
    size_t r1 = std::min(rEnd, rows > 0 ? rows - 1 : 0);
    size_t c0 = std::min(cStart, cols > 0 ? cols - 1 : 0);
    size_t c1 = std::min(cEnd, cols > 0 ? cols - 1 : 0);

    for (size_t r = r0; r <= r1; ++r) {
        for (size_t c = c0; c <= c1; ++c) {
            pq.push(grid.getZone(r, c));
        }
    }

    return pq.extractTopN(n);
}

} // namespace GeoLocalMiner
