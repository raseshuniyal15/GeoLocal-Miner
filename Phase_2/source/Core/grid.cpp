#include "Grid.h"
#include <cmath>
#include <limits>
#include <algorithm>

namespace GeoLocalMiner {

Grid::Grid()
    : numRows(0), numCols(0), originX(0.0), originY(0.0), cellSize(1000.0),
      layerG(IndicatorType::GEOLOGICAL, "Geology (G)", "Lithological & host rock suitability", "Index"),
      layerR(IndicatorType::RADIOMETRIC, "Radiometrics (R)", "Gamma ray / Thorium channel anomaly", "cps"),
      layerS(IndicatorType::STRUCTURAL, "Structure (S)", "Fault & fracture lineament density", "km/km2"),
      layerT(IndicatorType::TERRAIN, "Terrain (T)", "Geomorphological placer trap favorability", "deg/slope"),
      datasetName("None") {}

Grid::Grid(size_t rows, size_t cols, double origX, double origY, double cellResolution)
    : numRows(rows), numCols(cols), originX(origX), originY(origY), cellSize(cellResolution),
      layerG(IndicatorType::GEOLOGICAL, "Geology (G)", "Lithological & host rock suitability", "Index"),
      layerR(IndicatorType::RADIOMETRIC, "Radiometrics (R)", "Gamma ray / Thorium channel anomaly", "cps"),
      layerS(IndicatorType::STRUCTURAL, "Structure (S)", "Fault & fracture lineament density", "km/km2"),
      layerT(IndicatorType::TERRAIN, "Terrain (T)", "Geomorphological placer trap favorability", "deg/slope"),
      datasetName("Custom Grid") {
    initialize(rows, cols, origX, origY, cellResolution);
}

void Grid::initialize(size_t rows, size_t cols, double origX, double origY, double cellResolution) {
    numRows = rows;
    numCols = cols;
    originX = origX;
    originY = origY;
    cellSize = cellResolution;

    zones.resize(rows, cols);
    layerG.resize(rows, cols);
    layerR.resize(rows, cols);
    layerS.resize(rows, cols);
    layerT.resize(rows, cols);

    int id = 0;
    for (size_t r = 0; r < rows; ++r) {
        for (size_t c = 0; c < cols; ++c) {
            double x = originX + (static_cast<double>(c) + 0.5) * cellSize;
            double y = originY + (static_cast<double>(r) + 0.5) * cellSize;
            zones(r, c) = Zone(id++, static_cast<int>(r), static_cast<int>(c), x, y);
        }
    }

    rebuildSpatialGraph(true);
}

void Grid::clear() {
    numRows = 0;
    numCols = 0;
    zones.clear();
    layerG.resize(0, 0);
    layerR.resize(0, 0);
    layerS.resize(0, 0);
    layerT.resize(0, 0);
    spatialGraph.clear();
    datasetName = "None";
}

Zone& Grid::getZone(size_t r, size_t c) {
    return zones.at(r, c);
}

const Zone& Grid::getZone(size_t r, size_t c) const {
    return zones.at(r, c);
}

Zone* Grid::getZoneById(int id) {
    if (id < 0 || id >= static_cast<int>(zones.size())) return nullptr;
    return &zones.atIndex(static_cast<size_t>(id));
}

const Zone* Grid::getZoneById(int id) const {
    if (id < 0 || id >= static_cast<int>(zones.size())) return nullptr;
    return &zones.atIndex(static_cast<size_t>(id));
}

bool Grid::getZoneIndicesFromCoords(double x, double y, size_t& outR, size_t& outC) const {
    if (cellSize <= 0.0 || numRows == 0 || numCols == 0) return false;

    double relX = x - originX;
    double relY = y - originY;

    if (relX < 0.0 || relY < 0.0) return false;

    int c = static_cast<int>(std::floor(relX / cellSize));
    int r = static_cast<int>(std::floor(relY / cellSize));

    if (r >= 0 && r < static_cast<int>(numRows) && c >= 0 && c < static_cast<int>(numCols)) {
        outR = static_cast<size_t>(r);
        outC = static_cast<size_t>(c);
        return true;
    }
    return false;
}

void Grid::rebuildSpatialGraph(bool includeDiagonals) {
    spatialGraph.buildGridTopology(numRows, numCols, includeDiagonals);
}

std::vector<Zone*> Grid::getNeighbors(size_t r, size_t c) {
    std::vector<Zone*> result;
    if (!inBounds(r, c)) return result;

    int u = static_cast<int>(r * numCols + c);
    std::vector<int> neighborIds = spatialGraph.getNeighborIds(u);

    result.reserve(neighborIds.size());
    for (int nid : neighborIds) {
        if (nid >= 0 && nid < static_cast<int>(zones.size())) {
            result.push_back(&zones.atIndex(static_cast<size_t>(nid)));
        }
    }
    return result;
}

std::vector<const Zone*> Grid::getNeighbors(size_t r, size_t c) const {
    std::vector<const Zone*> result;
    if (!inBounds(r, c)) return result;

    int u = static_cast<int>(r * numCols + c);
    std::vector<int> neighborIds = spatialGraph.getNeighborIds(u);

    result.reserve(neighborIds.size());
    for (int nid : neighborIds) {
        if (nid >= 0 && nid < static_cast<int>(zones.size())) {
            result.push_back(&zones.atIndex(static_cast<size_t>(nid)));
        }
    }
    return result;
}

void Grid::detectClusters(double highThreshold) {
    std::vector<bool> activeMask(zones.size(), false);
    for (size_t i = 0; i < zones.size(); ++i) {
        if (zones.atIndex(i).getProspectivityScore() >= highThreshold) {
            activeMask[i] = true;
        }
        zones.atIndex(i).setClusterId(-1);
    }

    auto clusters = spatialGraph.findConnectedClusters(activeMask);
    for (size_t cid = 0; cid < clusters.size(); ++cid) {
        for (int zoneId : clusters[cid]) {
            if (zoneId >= 0 && zoneId < static_cast<int>(zones.size())) {
                zones.atIndex(static_cast<size_t>(zoneId)).setClusterId(static_cast<int>(cid + 1));
            }
        }
    }
}

void Grid::syncLayersToZones() {
    layerG.updateStatisticsAndNormalize();
    layerR.updateStatisticsAndNormalize();
    layerS.updateStatisticsAndNormalize();
    layerT.updateStatisticsAndNormalize();

    for (size_t r = 0; r < numRows; ++r) {
        for (size_t c = 0; c < numCols; ++c) {
            Zone& z = zones(r, c);
            z.setRawIndicators(layerG.getRawValue(r, c),
                               layerR.getRawValue(r, c),
                               layerS.getRawValue(r, c),
                               layerT.getRawValue(r, c));
            z.setNormalizedIndicators(layerG.getNormalizedValue(r, c),
                                     layerR.getNormalizedValue(r, c),
                                     layerS.getNormalizedValue(r, c),
                                     layerT.getNormalizedValue(r, c));
        }
    }
}

void Grid::syncZonesToLayers() {
    for (size_t r = 0; r < numRows; ++r) {
        for (size_t c = 0; c < numCols; ++c) {
            const Zone& z = zones(r, c);
            layerG.setRawValue(r, c, z.getRawG());
            layerR.setRawValue(r, c, z.getRawR());
            layerS.setRawValue(r, c, z.getRawS());
            layerT.setRawValue(r, c, z.getRawT());

            layerG.setNormalizedValue(r, c, z.getG());
            layerR.setNormalizedValue(r, c, z.getR());
            layerS.setNormalizedValue(r, c, z.getS());
            layerT.setNormalizedValue(r, c, z.getT());
        }
    }
}

GridStatistics Grid::computeStatistics() const {
    GridStatistics stats;
    stats.totalCells = numRows * numCols;
    if (stats.totalCells == 0) return stats;

    double minS = std::numeric_limits<double>::infinity();
    double maxS = -std::numeric_limits<double>::infinity();
    double sum = 0.0;

    for (size_t r = 0; r < numRows; ++r) {
        for (size_t c = 0; c < numCols; ++c) {
            const Zone& z = zones(r, c);
            double s = z.getProspectivityScore();
            if (s < minS) minS = s;
            if (s > maxS) maxS = s;
            sum += s;

            if (z.getPriority() == PriorityLevel::HIGH) {
                stats.highPriorityCount++;
            } else if (z.getPriority() == PriorityLevel::MEDIUM) {
                stats.mediumPriorityCount++;
            } else {
                stats.lowPriorityCount++;
            }

            if (z.hasKnownOccurrence()) {
                stats.knownOccurrencesCount++;
            }
        }
    }

    stats.minScore = minS;
    stats.maxScore = maxS;
    stats.meanScore = sum / static_cast<double>(stats.totalCells);
    return stats;
}

} // namespace GeoLocalMiner
