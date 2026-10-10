#ifndef VALIDATION_ENGINE_H
#define VALIDATION_ENGINE_H

#include <vector>
#include <string>
#include "Zone.h"
/*
total Total number of known/documented mineral occurrences
high Number of known occurrences that fall inside high priority zones
medium Number of known occurrences inside medium priority zones
low	Number of known occurrences inside low priority zones
highRate Percentage of all occurrences captured by high zones
highMedRate	Percentage captured by high+medium zones together
highArea Percentage of the total grid classified as high
enrich Enrichment factor; compares occurrence capture against the area classified as high
occurrenceZones Collection of Zone objects containing known mineral occurrences
*/
namespace GeoLocalMiner {

class Grid;

struct ValidationMetrics
{
    size_t total = 0;
    size_t high = 0;
    size_t medium = 0;
    size_t low = 0;

    double highRate = 0.0;
    double highMedRate = 0.0;
    double highArea = 0.0;
    double enrich = 0.0;

    std::vector<Zone> occurrenceZones;

    std::string getSummaryString() const;
};

class ValidationEngine
{
public:
    ValidationMetrics validate(const Grid& grid) const;
};

}

#endif