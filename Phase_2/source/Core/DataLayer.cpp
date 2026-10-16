#include "DataLayer.h"
#include "../utils/Normalization.h"
#include <limits>
#include <cmath>

namespace GeoLocalMiner {

DataLayer::DataLayer()
    : type(IndicatorType::CUSTOM),
      name("Unnamed Layer"),
      description(""),
      unit(""),
      minValue(0.0),
      maxValue(1.0),
      meanValue(0.5),
      statsUpToDate(false)
{
}

DataLayer::DataLayer(
    IndicatorType type,
    const std::string& name,
    const std::string& description,
    const std::string& unit
)
    : type(type),
      name(name),
      description(description),
      unit(unit),
      minValue(0.0),
      maxValue(1.0),
      meanValue(0.5),
      statsUpToDate(false)
{
}


void DataLayer::resize(size_t rows, size_t cols)
{
    rawValues.resize(rows, cols, 0.0);
    normalizedValues.resize(rows, cols, 0.0);

    statsUpToDate = false;
}


void DataLayer::setRawValue(
    size_t r,
    size_t c,
    double value
)
{
    rawValues(r, c) = value;
    statsUpToDate = false;
}


double DataLayer::getRawValue(
    size_t r,
    size_t c
) const
{
    return rawValues(r, c);
}


void DataLayer::setNormalizedValue(
    size_t r,
    size_t c,
    double value
)
{
    normalizedValues(r, c) =
        Normalization::clamp(value, 0.0, 1.0);
}


double DataLayer::getNormalizedValue(
    size_t r,
    size_t c
) const
{
    return normalizedValues(r, c);
}


double DataLayer::getMin() const
{
    return minValue;
}


double DataLayer::getMax() const
{
    return maxValue;
}


double DataLayer::getMean() const
{
    return meanValue;
}


void DataLayer::updateStatisticsAndNormalize()
{
    if (rawValues.empty())
    {
        minValue = 0.0;
        maxValue = 1.0;
        meanValue = 0.5;
        statsUpToDate = true;
        return;
    }

    const auto& raw = rawValues.rawData();

    double minV = std::numeric_limits<double>::infinity();
    double maxV = -std::numeric_limits<double>::infinity();
    double sum = 0.0;

    size_t validCount = 0;

    for (double value : raw)
    {
        if (std::isnan(value))
            continue;

        if (value < minV)
            minV = value;

        if (value > maxV)
            maxV = value;

        sum += value;
        ++validCount;
    }

    if (validCount == 0)
    {
        minValue = 0.0;
        maxValue = 1.0;
        meanValue = 0.5;
    }
    else
    {
        minValue = minV;
        maxValue = maxV;
        meanValue =
            sum / static_cast<double>(validCount);
    }

    size_t rows = rawValues.rows();
    size_t cols = rawValues.cols();

    if (normalizedValues.rows() != rows ||
        normalizedValues.cols() != cols)
    {
        normalizedValues.resize(rows, cols);
    }

    for (size_t r = 0; r < rows; ++r)
    {
        for (size_t c = 0; c < cols; ++c)
        {
            double value = rawValues(r, c);

            normalizedValues(r, c) =
                Normalization::minMaxScale(
                    value,
                    minValue,
                    maxValue,
                    0.5
                );
        }
    }

    statsUpToDate = true;
}

}