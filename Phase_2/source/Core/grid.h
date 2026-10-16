#ifndef GEOLOCAL_MINER_GRID_H
#define GEOLOCAL_MINER_GRID_H

#include <vector>
#include <string>
#include <memory>
#include "Zone.h"
#include "DataLayer.h"
#include "../data_structures/GridArray.h"
#include "../data_structures/Graph.h"

namespace GeoLocalMiner {

struct GridStatistics {
    double minScore;
    double maxScore;
    double meanScore;
    size_t highPriorityCount;
    size_t mediumPriorityCount;
    size_t lowPriorityCount;
    size_t totalCells;
    size_t knownOccurrencesCount;

    GridStatistics()
        : minScore(0.0), maxScore(0.0), meanScore(0.0),
          highPriorityCount(0), mediumPriorityCount(0), lowPriorityCount(0),
          totalCells(0), knownOccurrencesCount(0) {}
};

/**
 * @brief Manages the 2D spatial grid, indicator layers, and spatial graph connectivity.
 *
 * Demonstrates OOP Composition: A Grid HAS-A GridArray<Zone>, HAS-A DataLayers,
 * and HAS-A SpatialGraph.
 */
class Grid {
private:
    size_t numRows;
    size_t numCols;
    double originX;
    double originY;
    double cellSize; // Spatial resolution in meters or coordinate units

    GridArray<Zone> zones;
    DataLayer layerG;
    DataLayer layerR;
    DataLayer layerS;
    DataLayer layerT;

    SpatialGraph spatialGraph;
    std::string datasetName;

public:
    Grid();
    Grid(size_t rows, size_t cols, double origX = 0.0, double origY = 0.0, double cellResolution = 1000.0);

    void initialize(size_t rows, size_t cols, double origX = 0.0, double origY = 0.0, double cellResolution = 1000.0);
    void clear();

    size_t getRows() const { return numRows; }
    size_t getCols() const { return numCols; }
    size_t getTotalCells() const { return numRows * numCols; }
    double getOriginX() const { return originX; }
    double getOriginY() const { return originY; }
    double getCellSize() const { return cellSize; }
    double getWidth() const { return numCols * cellSize; }
    double getHeight() const { return numRows * cellSize; }

    const std::string& getDatasetName() const { return datasetName; }
    void setDatasetName(const std::string& name) { datasetName = name; }

    bool inBounds(size_t r, size_t c) const {
        return (r < numRows && c < numCols);
    }

    Zone& getZone(size_t r, size_t c);
    const Zone& getZone(size_t r, size_t c) const;

    Zone* getZoneById(int id);
    const Zone* getZoneById(int id) const;

    bool getZoneIndicesFromCoords(double x, double y, size_t& outR, size_t& outC) const;

    // Layer accessors
    DataLayer& getLayerG() { return layerG; }
    const DataLayer& getLayerG() const { return layerG; }

    DataLayer& getLayerR() { return layerR; }
    const DataLayer& getLayerR() const { return layerR; }

    DataLayer& getLayerS() { return layerS; }
    const DataLayer& getLayerS() const { return layerS; }

    DataLayer& getLayerT() { return layerT; }
    const DataLayer& getLayerT() const { return layerT; }

    // Spatial Graph accessors
    const SpatialGraph& getSpatialGraph() const { return spatialGraph; }
    SpatialGraph& getSpatialGraph() { return spatialGraph; }

    std::vector<Zone*> getNeighbors(size_t r, size_t c);
    std::vector<const Zone*> getNeighbors(size_t r, size_t c) const;

    void rebuildSpatialGraph(bool includeDiagonals = true);
    void detectClusters(double highThreshold);

    void syncLayersToZones();
    void syncZonesToLayers();

    GridStatistics computeStatistics() const;
};

} // namespace GeoLocalMiner

#endif // GEOLOCAL_MINER_GRID_H
