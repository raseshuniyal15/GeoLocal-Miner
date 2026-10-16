#ifndef PRIORITY_ENGINE_H
#define PRIORITY_ENGINE_H

#include <vector>
#include <string>
#include "Zone.h"
#include "../data_structures/PriorityQueue.h"

namespace GeoLocalMiner {

class Grid;

struct PriorityCounts
{
    size_t high = 0;
    size_t medium = 0;
    size_t low = 0;
    size_t total = 0;
};


class PriorityEngine
{
private:
    double highThreshold;
    double lowThreshold;

public:
    PriorityEngine();

    PriorityEngine(
        double high,
        double low
    );

    double getHighThreshold() const
    {
        return highThreshold;
    }

    double getLowThreshold() const
    {
        return lowThreshold;
    }

    bool setThresholds(
        double high,
        double low,
        std::string& errorMsg
    );

    void forceSetThresholds(
        double high,
        double low
    );

    PriorityLevel classifyScore(double score) const;

    void classifyZone(Zone& zone) const;

    PriorityCounts classifyGrid(Grid& grid) const;

    std::vector<Zone> rankTopN(
        const Grid& grid,
        size_t n
    ) const;

    std::vector<Zone> rankSubregionTopN(
        const Grid& grid,
        size_t rStart,
        size_t rEnd,
        size_t cStart,
        size_t cEnd,
        size_t n
    ) const;
};

}

#endif