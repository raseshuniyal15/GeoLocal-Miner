#include "DataLoader.h"
#include "../utils/FileUtils.h"
#include "../utils/Normalization.h"

#include <fstream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <cmath>

namespace GeoLocalMiner {

struct ColumnIndices
{
    int row = -1;
    int col = -1;
    int x = -1;
    int y = -1;

    int rawG = -1;
    int rawR = -1;
    int rawS = -1;
    int rawT = -1;

    int normG = -1;
    int normR = -1;
    int normS = -1;
    int normT = -1;

    int occurrence = -1;
    int name = -1;

    bool explicitNorm = false;
};


static ColumnIndices detectColumns(
    const std::vector<std::string>& headers
)
{
    ColumnIndices col;

    for (size_t i = 0; i < headers.size(); ++i)
    {
        std::string h = headers[i];

        for (char& ch : h)
            ch = static_cast<char>(std::tolower(ch));

        h = FileUtils::trim(h);

        if (h == "row" || h == "row_idx" || h == "grid_row")
            col.row = static_cast<int>(i);

        else if (
            h == "col" ||
            h == "column" ||
            h == "col_idx" ||
            h == "grid_col"
        )
            col.col = static_cast<int>(i);

        else if (
            h == "x" ||
            h == "lon" ||
            h == "longitude" ||
            h == "easting"
        )
            col.x = static_cast<int>(i);

        else if (
            h == "y" ||
            h == "lat" ||
            h == "latitude" ||
            h == "northing"
        )
            col.y = static_cast<int>(i);

        else if (h == "rawg")
            col.rawG = static_cast<int>(i);

        else if (h == "rawr")
            col.rawR = static_cast<int>(i);

        else if (h == "raws")
            col.rawS = static_cast<int>(i);

        else if (h == "rawt")
            col.rawT = static_cast<int>(i);

        else if (h == "normg")
        {
            col.normG = static_cast<int>(i);
            col.explicitNorm = true;
        }

        else if (h == "normr")
        {
            col.normR = static_cast<int>(i);
            col.explicitNorm = true;
        }

        else if (h == "norms")
        {
            col.normS = static_cast<int>(i);
            col.explicitNorm = true;
        }

        else if (h == "normt")
        {
            col.normT = static_cast<int>(i);
            col.explicitNorm = true;
        }

        else if (
            h == "g" ||
            h == "geology" ||
            h == "geological" ||
            h == "geol"
        )
            col.normG = static_cast<int>(i);

        else if (
            h == "r" ||
            h == "radiometric" ||
            h == "radiometrics" ||
            h == "rad"
        )
            col.normR = static_cast<int>(i);

        else if (
            h == "s" ||
            h == "structure" ||
            h == "structural" ||
            h == "struct"
        )
            col.normS = static_cast<int>(i);

        else if (
            h == "t" ||
            h == "terrain" ||
            h == "topography" ||
            h == "topo"
        )
            col.normT = static_cast<int>(i);

        else if (
            h == "known_occurrence" ||
            h == "occurrence" ||
            h == "known" ||
            h == "deposit" ||
            h == "occ"
        )
            col.occurrence = static_cast<int>(i);

        else if (
            h == "name" ||
            h == "deposit_name" ||
            h == "site_name" ||
            h == "occurrence_name"
        )
            col.name = static_cast<int>(i);
    }

    if (col.rawG == -1) col.rawG = col.normG;
    if (col.rawR == -1) col.rawR = col.normR;
    if (col.rawS == -1) col.rawS = col.normS;
    if (col.rawT == -1) col.rawT = col.normT;

    if (col.normG == -1) col.normG = col.rawG;
    if (col.normR == -1) col.normR = col.rawR;
    if (col.normS == -1) col.normS = col.rawS;
    if (col.normT == -1) col.normT = col.rawT;

    return col;
}


LoadResult DataLoader::loadFromCsv(
    const std::string& path,
    Grid& grid
)
{
    LoadResult result;

    if (!FileUtils::fileExists(path))
    {
        result.error = "File not found: " + path;
        return result;
    }

    std::ifstream file(path);

    if (!file.is_open())
    {
        result.error = "Failed to open file: " + path;
        return result;
    }

    std::string line;

    if (!std::getline(file, line))
    {
        result.error = "CSV file is empty: " + path;
        return result;
    }

    std::vector<std::string> headers =
        FileUtils::splitCsvLine(line);

    ColumnIndices col = detectColumns(headers);

    if (col.row == -1 && headers.size() >= 8)
    {
        col.row = 0;
        col.col = 1;
        col.x = 2;
        col.y = 3;

        col.rawG = 4;
        col.rawR = 5;
        col.rawS = 6;
        col.rawT = 7;

        col.normG = 4;
        col.normR = 5;
        col.normS = 6;
        col.normT = 7;

        if (headers.size() > 8)
            col.occurrence = 8;

        if (headers.size() > 9)
            col.name = 9;
    }


    struct RowData
    {
        int r = 0;
        int c = 0;

        double x = 0.0;
        double y = 0.0;

        double g = 0.0;
        double rad = 0.0;
        double s = 0.0;
        double t = 0.0;

        double normG = 0.0;
        double normR = 0.0;
        double normS = 0.0;
        double normT = 0.0;

        bool occurrence = false;
        std::string name;
    };


    std::vector<RowData> data;

    int maxRow = -1;
    int maxCol = -1;

    size_t lineNo = 1;

    while (std::getline(file, line))
    {
        ++lineNo;

        std::string clean = FileUtils::trim(line);

        if (clean.empty() || clean[0] == '#')
            continue;

        std::vector<std::string> fields =
            FileUtils::splitCsvLine(clean);

        if (fields.size() < 8)
        {
            ++result.warnings;

            if (result.warningList.size() < 10)
            {
                result.warningList.push_back(
                    "Line " + std::to_string(lineNo) +
                    ": insufficient columns, skipped."
                );
            }

            continue;
        }


        RowData dataRow;
        bool valid = true;

        if (col.row >= 0 &&
            col.row < static_cast<int>(fields.size()))
        {
            valid &= FileUtils::parseInt(
                fields[col.row],
                dataRow.r
            );
        }
        else
        {
            dataRow.r = static_cast<int>(data.size());
        }


        if (col.col >= 0 &&
            col.col < static_cast<int>(fields.size()))
        {
            valid &= FileUtils::parseInt(
                fields[col.col],
                dataRow.c
            );
        }
        else
        {
            dataRow.c = 0;
        }


        if (col.x >= 0 &&
            col.x < static_cast<int>(fields.size()))
        {
            valid &= FileUtils::parseDouble(
                fields[col.x],
                dataRow.x
            );
        }
        else
        {
            dataRow.x = dataRow.c * 1000.0;
        }


        if (col.y >= 0 &&
            col.y < static_cast<int>(fields.size()))
        {
            valid &= FileUtils::parseDouble(
                fields[col.y],
                dataRow.y
            );
        }
        else
        {
            dataRow.y = dataRow.r * 1000.0;
        }


        if (col.rawG >= 0 &&
            col.rawG < static_cast<int>(fields.size()))
        {
            valid &= FileUtils::parseDouble(
                fields[col.rawG],
                dataRow.g
            );
        }
        else
        {
            dataRow.g = 50.0;
        }


        if (col.rawR >= 0 &&
            col.rawR < static_cast<int>(fields.size()))
        {
            valid &= FileUtils::parseDouble(
                fields[col.rawR],
                dataRow.rad
            );
        }
        else
        {
            dataRow.rad = 100.0;
        }


        if (col.rawS >= 0 &&
            col.rawS < static_cast<int>(fields.size()))
        {
            valid &= FileUtils::parseDouble(
                fields[col.rawS],
                dataRow.s
            );
        }
        else
        {
            dataRow.s = 5.0;
        }


        if (col.rawT >= 0 &&
            col.rawT < static_cast<int>(fields.size()))
        {
            valid &= FileUtils::parseDouble(
                fields[col.rawT],
                dataRow.t
            );
        }
        else
        {
            dataRow.t = 20.0;
        }


        if (col.normG >= 0 &&
            col.normG < static_cast<int>(fields.size()))
        {
            valid &= FileUtils::parseDouble(
                fields[col.normG],
                dataRow.normG
            );
        }
        else
        {
            dataRow.normG = dataRow.g;
        }


        if (col.normR >= 0 &&
            col.normR < static_cast<int>(fields.size()))
        {
            valid &= FileUtils::parseDouble(
                fields[col.normR],
                dataRow.normR
            );
        }
        else
        {
            dataRow.normR = dataRow.rad;
        }


        if (col.normS >= 0 &&
            col.normS < static_cast<int>(fields.size()))
        {
            valid &= FileUtils::parseDouble(
                fields[col.normS],
                dataRow.normS
            );
        }
        else
        {
            dataRow.normS = dataRow.s;
        }


        if (col.normT >= 0 &&
            col.normT < static_cast<int>(fields.size()))
        {
            valid &= FileUtils::parseDouble(
                fields[col.normT],
                dataRow.normT
            );
        }
        else
        {
            dataRow.normT = dataRow.t;
        }


        if (col.occurrence >= 0 &&
            col.occurrence < static_cast<int>(fields.size()))
        {
            FileUtils::parseBool(
                fields[col.occurrence],
                dataRow.occurrence
            );
        }


        if (col.name >= 0 &&
            col.name < static_cast<int>(fields.size()))
        {
            dataRow.name = fields[col.name];
        }


        if (!valid)
        {
            ++result.warnings;

            if (result.warningList.size() < 10)
            {
                result.warningList.push_back(
                    "Line " + std::to_string(lineNo) +
                    ": numeric conversion error, skipped."
                );
            }

            continue;
        }


        if (dataRow.r > maxRow)
            maxRow = dataRow.r;

        if (dataRow.c > maxCol)
            maxCol = dataRow.c;

        data.push_back(dataRow);
    }


    if (data.empty())
    {
        result.error = "No valid data rows found in CSV.";
        return result;
    }


    size_t gridRows =
        static_cast<size_t>(maxRow + 1);

    size_t gridCols =
        static_cast<size_t>(maxCol + 1);

    if (gridRows == 0 ||
        gridCols == 0 ||
        gridRows * gridCols > 500000)
    {
        result.error =
            "Invalid grid dimensions: " +
            std::to_string(gridRows) +
            "x" +
            std::to_string(gridCols);

        return result;
    }


    double originX = data[0].x;
    double originY = data[0].y;
    double cellSize = 1000.0;

    if (gridCols > 1 && data.size() > 1)
    {
        cellSize =
            std::abs(data[1].x - data[0].x);

        if (cellSize < 1e-4)
            cellSize = 1000.0;
    }


    grid.initialize(
        gridRows,
        gridCols,
        originX,
        originY,
        cellSize
    );

    grid.setDatasetName(
        FileUtils::getFilename(path)
    );


    for (const RowData& item : data)
    {
        if (!grid.inBounds(
                static_cast<size_t>(item.r),
                static_cast<size_t>(item.c)))
        {
            continue;
        }

        size_t r = static_cast<size_t>(item.r);
        size_t c = static_cast<size_t>(item.c);

        Zone& zone = grid.getZone(r, c);

        zone.setCoordinates(item.x, item.y);

        zone.setRawIndicators(
            item.g,
            item.rad,
            item.s,
            item.t
        );

        zone.setNormalizedIndicators(
            item.normG,
            item.normR,
            item.normS,
            item.normT
        );

        zone.setKnownOccurrence(
            item.occurrence,
            item.name
        );


        grid.getLayerG().setRawValue(r, c, item.g);
        grid.getLayerR().setRawValue(r, c, item.rad);
        grid.getLayerS().setRawValue(r, c, item.s);
        grid.getLayerT().setRawValue(r, c, item.t);

        grid.getLayerG().setNormalizedValue(r, c, item.normG);
        grid.getLayerR().setNormalizedValue(r, c, item.normR);
        grid.getLayerS().setNormalizedValue(r, c, item.normS);
        grid.getLayerT().setNormalizedValue(r, c, item.normT);
    }


    if (!col.explicitNorm)
        grid.syncLayersToZones();


    result.success = true;
    result.rows = data.size();

    return result;
}


bool DataLoader::exportRankedTargetsCsv(
    const std::string& path,
    const std::vector<Zone>& zones,
    std::string& error
)
{
    std::ofstream file(path);

    if (!file.is_open())
    {
        error = "Could not open file for writing: " + path;
        return false;
    }

    file << "rank,id,row,column,x,y,G,R,S,T,"
            "score,priority,known_occurrence,occurrence_name\n";


    for (size_t i = 0; i < zones.size(); ++i)
    {
        const Zone& zone = zones[i];

        file << i + 1 << ","
             << zone.getId() << ","
             << zone.getRow() << ","
             << zone.getCol() << ","
             << std::fixed << std::setprecision(2)
             << zone.getX() << ","
             << zone.getY() << ","
             << std::setprecision(3)
             << zone.getG() << ","
             << zone.getR() << ","
             << zone.getS() << ","
             << zone.getT() << ","
             << std::setprecision(4)
             << zone.getProspectivityScore() << ","
             << zone.getPriorityString() << ","
             << (zone.hasKnownOccurrence() ? 1 : 0)
             << ",\""
             << zone.getOccurrenceName()
             << "\"\n";
    }

    return true;
}


bool DataLoader::exportGridCsv(
    const std::string& path,
    const Grid& grid,
    std::string& error
)
{
    std::ofstream file(path);

    if (!file.is_open())
    {
        error = "Could not open file for writing: " + path;
        return false;
    }


    file << "id,row,column,x,y,"
            "rawG,rawR,rawS,rawT,"
            "normG,normR,normS,normT,"
            "score,priority,known_occurrence,"
            "occurrence_name,cluster_id\n";


    for (size_t r = 0; r < grid.getRows(); ++r)
    {
        for (size_t c = 0; c < grid.getCols(); ++c)
        {
            const Zone& zone =
                grid.getZone(r, c);

            file << zone.getId() << ","
                 << zone.getRow() << ","
                 << zone.getCol() << ","
                 << std::fixed << std::setprecision(2)
                 << zone.getX() << ","
                 << zone.getY() << ","
                 << std::setprecision(3)
                 << zone.getRawG() << ","
                 << zone.getRawR() << ","
                 << zone.getRawS() << ","
                 << zone.getRawT() << ","
                 << zone.getG() << ","
                 << zone.getR() << ","
                 << zone.getS() << ","
                 << zone.getT() << ","
                 << std::setprecision(4)
                 << zone.getProspectivityScore() << ","
                 << zone.getPriorityString() << ","
                 << (zone.hasKnownOccurrence() ? 1 : 0)
                 << ",\""
                 << zone.getOccurrenceName()
                 << "\","
                 << zone.getClusterId()
                 << "\n";
        }
    }

    return true;
}


void DataLoader::generateSyntheticPlacerDataset(
    Grid& grid,
    size_t rows,
    size_t cols
)
{
    grid.initialize(
        rows,
        cols,
        10000.0,
        20000.0,
        1000.0
    );

    grid.setDatasetName(
        "Demo / Synthetic Placer Deposit Model"
    );


    for (size_t r = 0; r < rows; ++r)
    {
        for (size_t c = 0; c < cols; ++c)
        {
            double dr1 = static_cast<double>(r) - 6.0;
            double dc1 = static_cast<double>(c) - 3.0;
            double d1 = std::sqrt(
                dr1 * dr1 + dc1 * dc1
            );

            double dr2 = static_cast<double>(r) - 15.0;
            double dc2 = static_cast<double>(c) - 4.0;
            double d2 = std::sqrt(
                dr2 * dr2 + dc2 * dc2
            );

            double dr3 = static_cast<double>(r) - 12.0;
            double dc3 = static_cast<double>(c) - 16.0;
            double d3 = std::sqrt(
                dr3 * dr3 + dc3 * dc3
            );


            double noise =
                0.05 *
                std::sin(
                    r * 12.9898 +
                    c * 78.233
                );


            double terrainBase =
                0.90 -
                0.035 * static_cast<double>(c);

            double terrainHotspot =
                0.25 *
                std::exp(
                    -0.5 * (d1 * d1) / 4.0
                );

            double terrain =
                Normalization::clamp(
                    terrainBase +
                    terrainHotspot +
                    noise,
                    0.10,
                    0.98
                );


            double rad1 =
                0.85 *
                std::exp(
                    -0.5 * (d1 * d1) / 3.5
                );

            double rad2 =
                0.70 *
                std::exp(
                    -0.5 * (d2 * d2) / 4.0
                );

            double rad3 =
                0.65 *
                std::exp(
                    -0.5 * (d3 * d3) / 5.0
                );

            double radBase =
                0.15 +
                0.05 * std::sin(
                    static_cast<double>(r)
                );

            double radiometric =
                Normalization::clamp(
                    radBase +
                    rad1 +
                    rad2 +
                    rad3 +
                    noise,
                    0.05,
                    0.98
                );


            double geoBase =
                0.25 +
                0.032 * static_cast<double>(c);

            double geo1 =
                0.45 *
                std::exp(
                    -0.5 * (d1 * d1) / 6.0
                );

            double geo3 =
                0.50 *
                std::exp(
                    -0.5 * (d3 * d3) / 4.5
                );

            double geology =
                Normalization::clamp(
                    geoBase +
                    geo1 +
                    geo3 +
                    noise,
                    0.12,
                    0.99
                );


            double trend =
                std::exp(
                    -0.5 *
                    std::pow(
                        (
                            static_cast<double>(r) -
                            0.7 * static_cast<double>(c) -
                            8.0
                        ) / 2.8,
                        2
                    )
                );

            double structHotspot =
                0.60 *
                std::exp(
                    -0.5 * (d2 * d2) / 3.0
                );

            double structure =
                Normalization::clamp(
                    0.20 +
                    0.50 * trend +
                    structHotspot +
                    noise,
                    0.08,
                    0.95
                );


            Zone& zone = grid.getZone(r, c);

            zone.setRawIndicators(
                geology * 100.0,
                radiometric * 250.0,
                structure * 10.0,
                terrain * 45.0
            );

            zone.setNormalizedIndicators(
                geology,
                radiometric,
                structure,
                terrain
            );


            grid.getLayerG().setRawValue(
                r, c, geology * 100.0
            );

            grid.getLayerR().setRawValue(
                r, c, radiometric * 250.0
            );

            grid.getLayerS().setRawValue(
                r, c, structure * 10.0
            );

            grid.getLayerT().setRawValue(
                r, c, terrain * 45.0
            );
        }
    }


    struct KnownOccurrence
    {
        size_t row;
        size_t col;
        const char* name;
    };


    static const KnownOccurrence known[] =
    {
        {6, 3, "Chavara Coastal Monazite Placer Deposit #1"},
        {5, 4, "Monazite Strandline Terrace #2"},
        {7, 3, "Paleo-Beach Sand Heavy Mineral Anomaly #3"},
        {15, 4, "Periyar Basin Delta Inland Placer #4"},
        {14, 5, "River Terraces Thorium Concentration #5"},
        {12, 16, "Inland Charnockite-Pegmatite Thorite Site #6"},
        {11, 15, "Granitic Pluton Contact Occurrence #7"},
        {2, 12, "Isolated Inland Thorium Background Anomaly #8"}
    };


    for (const auto& item : known)
    {
        if (grid.inBounds(item.row, item.col))
        {
            grid.getZone(
                item.row,
                item.col
            ).setKnownOccurrence(
                true,
                item.name
            );
        }
    }

    grid.syncLayersToZones();
}


bool DataLoader::saveSyntheticCsv(
    const std::string& path,
    size_t rows,
    size_t cols
)
{
    Grid grid;

    generateSyntheticPlacerDataset(
        grid,
        rows,
        cols
    );

    std::string error;

    return exportGridCsv(
        path,
        grid,
        error
    );
}

}