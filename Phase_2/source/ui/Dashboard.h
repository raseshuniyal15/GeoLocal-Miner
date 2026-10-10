
#ifndef GEOLOCAL_MINER_DASHBOARD_H
#define GEOLOCAL_MINER_DASHBOARD_H

#include "../core/Grid.h"
#include "../core/ProspectivityEngine.h"
#include "../core/PriorityEngine.h"
#include "../core/ValidationEngine.h"
#include "MapView.h"
#include "ZoneDetails.h"

#include <string>
#include <vector>

namespace GeoLocalMiner {

class Dashboard {
private:
    char csvPathBuffer[256];
    std::string statusMessage;
    bool statusIsError;

    float sliderWG;
    float sliderWR;
    float sliderWS;
    float sliderWT;

    int selectedStrategyIdx;

    float sliderHighThreshold;
    float sliderLowThreshold;

    // 0 = 5, 1 = 10, 2 = 20, 3 = 50, 4 = 100
    int topNSelection;
    std::vector<Zone> rankedTargets;

    GridStatistics gridStats;
    ValidationMetrics valMetrics;

    bool needsRecalculation;

public:
    Dashboard();

    void initialize(
        Grid& grid,
        ProspectivityEngine& prospectivityEngine,
        PriorityEngine& priorityEngine,
        ValidationEngine& validationEngine
    );

    void recomputeAll(
        Grid& grid,
        ProspectivityEngine& prospectivityEngine,
        PriorityEngine& priorityEngine,
        ValidationEngine& validationEngine,
        MapView& mapView
    );

    void renderHeader();

    void renderControls(
        Grid& grid,
        ProspectivityEngine& prospectivityEngine,
        PriorityEngine& priorityEngine,
        ValidationEngine& validationEngine,
        MapView& mapView
    );

    void renderSummaryCards();

    void renderRankingTable(
        Grid& grid,
        MapView& mapView,
        ZoneDetails& zoneDetails
    );
};

} // namespace GeoLocalMiner

#endif
