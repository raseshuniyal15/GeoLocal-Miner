#ifndef GEOLOCAL_MINER_MAP_VIEW_H
#define GEOLOCAL_MINER_MAP_VIEW_H

#include "../core/Grid.h"
#include <cstdint>

namespace GeoLocalMiner {

enum class MapLayerMode {
    FINAL_PROSPECTIVITY = 0,
    PRIORITY_CATEGORY,
    GEOLOGY_G,
    RADIOMETRIC_R,
    STRUCTURAL_S,
    TERRAIN_T,
    ANOMALY_CLUSTERS
};

class MapView {
private:
    MapLayerMode currentLayer;

    int selectedRow, selectedCol;
    bool showOccurrences;
    bool showNeighborLinks;
    bool showGridLines;
    bool showClusterOutlines;

    // Region of interest
    bool roiEnabled;
    int roiRowStart, roiRowEnd;
    int roiColStart, roiColEnd;

    float zoomFactor;
    float panOffsetX, panOffsetY;

    uint32_t getHeatmapColor(double value, float alpha = 1.0f) const;
    uint32_t getPriorityColor(PriorityLevel p, float alpha = 1.0f) const;
    uint32_t getClusterColor(int clusterId, float alpha = 1.0f) const;

public:
    MapView();

    MapLayerMode getLayerMode() const { return currentLayer; }
    void setLayerMode(MapLayerMode mode) { currentLayer = mode; }

    int getSelectedRow() const { return selectedRow; }
    int getSelectedCol() const { return selectedCol; }

    void setSelectedCell(int r, int c) {
        selectedRow = r;
        selectedCol = c;
    }

    void clearSelection() {
        selectedRow = selectedCol = -1;
    }

    bool hasSelection() const {
        return selectedRow >= 0 && selectedCol >= 0;
    }

    bool isRoiEnabled() const { return roiEnabled; }
    void setRoiEnabled(bool enabled) { roiEnabled = enabled; }

    void getRoiBounds(int& r0, int& r1, int& c0, int& c1) const {
        r0 = roiRowStart;
        r1 = roiRowEnd;
        c0 = roiColStart;
        c1 = roiColEnd;
    }

    void setRoiBounds(int r0, int r1, int c0, int c1) {
        roiRowStart = r0;
        roiRowEnd = r1;
        roiColStart = c0;
        roiColEnd = c1;
    }

    void setShowOccurrences(bool show) { showOccurrences = show; }
    void setShowNeighborLinks(bool show) { showNeighborLinks = show; }
    void setShowGridLines(bool show) { showGridLines = show; }

    void render(Grid& grid, bool& cellSelectionChanged);
    void renderLegend(float width, float height);
};

} // namespace GeoLocalMiner

#endif