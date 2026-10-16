#ifndef GEOLOCAL_MINER_DATA_LAYER_H
#define GEOLOCAL_MINER_DATA_LAYER_H

#include <string>
#include <vector>
#include "../data_structures/GridArray.h"

namespace GeoLocalMiner {

enum class IndicatorType {
    GEOLOGICAL = 0,
    RADIOMETRIC = 1,
    STRUCTURAL = 2,
    TERRAIN = 3,
    CUSTOM = 4
};

/**
 * @brief Represents a single thematic geoscience indicator layer over the exploration grid.
 * 
 * Encapsulates raw spatial sensor/field data, computed min/max statistics,
 * and normalized layer grids.
 */
class DataLayer {
private:
    IndicatorType type;
    std::string name;
    std::string description;
    std::string unit;

    GridArray<double> rawValues;
    GridArray<double> normalizedValues;

    double minValue;
    double maxValue;
    double meanValue;
    bool statsUpToDate;

public:
    DataLayer();
    DataLayer(IndicatorType type, const std::string& name, const std::string& description, const std::string& unit = "");

    IndicatorType getType() const { return type; }
    const std::string& getName() const { return name; }
    const std::string& getDescription() const { return description; }
    const std::string& getUnit() const { return unit; }

    void resize(size_t rows, size_t cols);

    void setRawValue(size_t r, size_t c, double val);
    double getRawValue(size_t r, size_t c) const;

    void setNormalizedValue(size_t r, size_t c, double val);
    double getNormalizedValue(size_t r, size_t c) const;

    const GridArray<double>& getRawGrid() const { return rawValues; }
    const GridArray<double>& getNormalizedGrid() const { return normalizedValues; }

    double getMin() const;
    double getMax() const;
    double getMean() const;

    void updateStatisticsAndNormalize();
};

} // namespace GeoLocalMiner

#endif // GEOLOCAL_MINER_DATA_LAYER_H
