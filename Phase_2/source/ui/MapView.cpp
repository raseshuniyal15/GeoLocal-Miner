
#include "MapView.h"
#include "../../third_party/imgui/imgui.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>

namespace GeoLocalMiner {

namespace {
template <typename T>
T clampValue(T value, T minimum, T maximum)
{
    return std::min(maximum, std::max(minimum, value));
}
}

MapView::MapView()
    : currentLayer(MapLayerMode::FINAL_PROSPECTIVITY),
      selectedRow(-1),
      selectedCol(-1),
      showOccurrences(true),
      showNeighborLinks(true),
      showGridLines(true),
      showClusterOutlines(true),
      roiEnabled(false),
      roiRowStart(4),
      roiRowEnd(10),
      roiColStart(1),
      roiColEnd(7),
      zoomFactor(1.0f),
      panOffsetX(0.0f),
      panOffsetY(0.0f)
{
}

uint32_t MapView::getHeatmapColor(double value, float alpha) const
{
    value = clampValue(value, 0.0, 1.0);
    alpha = clampValue(alpha, 0.0f, 1.0f);

    float r, g, b;
    float t;

    if (value < 0.25)
    {
        t = static_cast<float>(value / 0.25);
        r = 0.05f + 0.10f * t;
        g = 0.15f + 0.55f * t;
        b = 0.55f + 0.35f * t;
    }
    else if (value < 0.50)
    {
        t = static_cast<float>((value - 0.25) / 0.25);
        r = 0.15f + 0.25f * t;
        g = 0.70f + 0.20f * t;
        b = 0.90f - 0.50f * t;
    }
    else if (value < 0.75)
    {
        t = static_cast<float>((value - 0.50) / 0.25);
        r = 0.40f + 0.55f * t;
        g = 0.90f + 0.05f * t;
        b = 0.40f - 0.35f * t;
    }
    else
    {
        t = static_cast<float>((value - 0.75) / 0.25);
        r = 0.95f + 0.05f * t;
        g = 0.95f - 0.75f * t;
        b = 0.05f + 0.05f * t;
    }

    return IM_COL32(
        static_cast<int>(r * 255.0f),
        static_cast<int>(g * 255.0f),
        static_cast<int>(b * 255.0f),
        static_cast<int>(alpha * 255.0f)
    );
}

uint32_t MapView::getPriorityColor(
    PriorityLevel priority,
    float alpha
) const
{
    int a = static_cast<int>(
        clampValue(alpha, 0.0f, 1.0f) * 255.0f
    );

    switch (priority)
    {
        case PriorityLevel::HIGH:
            return IM_COL32(220, 40, 40, a);

        case PriorityLevel::MEDIUM:
            return IM_COL32(235, 170, 20, a);

        case PriorityLevel::LOW:
            return IM_COL32(40, 120, 190, a);

        default:
            return IM_COL32(100, 100, 100, a);
    }
}

uint32_t MapView::getClusterColor(
    int clusterId,
    float alpha
) const
{
    static const uint32_t colors[] = {
        IM_COL32(230, 40, 80, 255),
        IM_COL32(40, 190, 220, 255),
        IM_COL32(240, 180, 20, 255),
        IM_COL32(160, 60, 230, 255),
        IM_COL32(40, 220, 120, 255),
        IM_COL32(240, 120, 40, 255),
        IM_COL32(80, 140, 250, 255),
        IM_COL32(210, 70, 180, 255)
    };

    int a = static_cast<int>(
        clampValue(alpha, 0.0f, 1.0f) * 255.0f
    );

    if (clusterId <= 0)
        return IM_COL32(45, 55, 70, a);

    uint32_t color =
        colors[static_cast<std::size_t>(clusterId - 1) % 8];

    return (color & 0x00FFFFFF) |
           (static_cast<uint32_t>(a) << 24);
}

void MapView::render(Grid& grid, bool& cellSelectionChanged)
{
    cellSelectionChanged = false;

    const std::size_t rows = grid.getRows();
    const std::size_t cols = grid.getCols();

    if (rows == 0 || cols == 0)
    {
        ImGui::TextColored(
            ImVec4(0.8f, 0.4f, 0.4f, 1.0f),
            "No grid loaded. Please load a dataset."
        );
        return;
    }

    ImGui::BeginChild("MapView", ImVec2(0, 0), true);

    ImGui::Text("Map View");
    ImGui::Separator();

    const char* layers[] = {
        "Final Prospectivity",
        "Priority Category",
        "Geology (G)",
        "Radiometric (R)",
        "Structure (S)",
        "Terrain (T)",
        "Anomaly Clusters"
    };

    int layer = static_cast<int>(currentLayer);

    if (ImGui::Combo("Layer", &layer, layers, IM_ARRAYSIZE(layers)))
    {
        if (layer >= 0 && layer < IM_ARRAYSIZE(layers))
            currentLayer = static_cast<MapLayerMode>(layer);
    }

    ImGui::SameLine();

    if (ImGui::Button("Reset View"))
    {
        zoomFactor = 1.0f;
        panOffsetX = 0.0f;
        panOffsetY = 0.0f;
    }

    ImGui::Checkbox("Occurrences", &showOccurrences);
    ImGui::SameLine();
    ImGui::Checkbox("Neighbor Links", &showNeighborLinks);
    ImGui::SameLine();
    ImGui::Checkbox("Grid Lines", &showGridLines);
    ImGui::Checkbox("Cluster Outlines", &showClusterOutlines);

    ImGui::Separator();

    ImGui::Text("Zoom: %.2fx", zoomFactor);
    ImGui::SameLine();

    if (ImGui::Button("-"))
        zoomFactor = std::max(0.2f, zoomFactor - 0.1f);

    ImGui::SameLine();

    if (ImGui::Button("+"))
        zoomFactor = std::min(5.0f, zoomFactor + 0.1f);

    ImGui::Separator();

    ImVec2 canvasPos = ImGui::GetCursorScreenPos();
    ImVec2 canvasSize = ImGui::GetContentRegionAvail();

    canvasSize.x = std::max(canvasSize.x, 100.0f);
    canvasSize.y = std::max(canvasSize.y, 100.0f);

    ImGui::InvisibleButton("MapCanvas", canvasSize);

    const bool hovered = ImGui::IsItemHovered();
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImGuiIO& io = ImGui::GetIO();
    const ImVec2 mouse = io.MousePos;

    ImVec2 canvasEnd(
        canvasPos.x + canvasSize.x,
        canvasPos.y + canvasSize.y
    );

    drawList->AddRectFilled(
        canvasPos, canvasEnd, IM_COL32(18, 24, 32, 255)
    );

    if (hovered && ImGui::IsMouseDragging(ImGuiMouseButton_Left))
    {
        panOffsetX += io.MouseDelta.x;
        panOffsetY += io.MouseDelta.y;
    }

    if (hovered && io.MouseWheel != 0.0f)
    {
        const float oldZoom = zoomFactor;

        zoomFactor = clampValue(
            zoomFactor + io.MouseWheel * 0.1f,
            0.2f,
            5.0f
        );

        const float ratio = zoomFactor / oldZoom;
        const float mouseX = mouse.x - canvasPos.x;
        const float mouseY = mouse.y - canvasPos.y;

        panOffsetX = (panOffsetX + mouseX) * ratio - mouseX;
        panOffsetY = (panOffsetY + mouseY) * ratio - mouseY;
    }

    const float cellW = canvasSize.x / static_cast<float>(cols);
    const float cellH = canvasSize.y / static_cast<float>(rows);
    const float cellSize = std::min(cellW, cellH) * zoomFactor;

    const float mapW = cellSize * static_cast<float>(cols);
    const float mapH = cellSize * static_cast<float>(rows);

    const float startX =
        canvasPos.x + (canvasSize.x - mapW) * 0.5f + panOffsetX;

    const float startY =
        canvasPos.y + (canvasSize.y - mapH) * 0.5f + panOffsetY;

    if (hovered &&
        ImGui::IsMouseClicked(ImGuiMouseButton_Left) &&
        cellSize > 0.0f)
    {
        const float localX = mouse.x - startX;
        const float localY = mouse.y - startY;

        if (localX >= 0.0f && localY >= 0.0f &&
            localX < mapW && localY < mapH)
        {
            const int col = static_cast<int>(localX / cellSize);
            const int row = static_cast<int>(localY / cellSize);

            if (row >= 0 && col >= 0 &&
                row < static_cast<int>(rows) &&
                col < static_cast<int>(cols))
            {
                selectedRow = row;
                selectedCol = col;
                cellSelectionChanged = true;
            }
        }
    }

    drawList->PushClipRect(canvasPos, canvasEnd, true);

    const int firstCol = clampValue(
        static_cast<int>(std::floor((canvasPos.x - startX) / cellSize)),
        0,
        static_cast<int>(cols)
    );

    const int lastCol = clampValue(
        static_cast<int>(std::ceil((canvasEnd.x - startX) / cellSize)),
        0,
        static_cast<int>(cols)
    );

    const int firstRow = clampValue(
        static_cast<int>(std::floor((canvasPos.y - startY) / cellSize)),
        0,
        static_cast<int>(rows)
    );

    const int lastRow = clampValue(
        static_cast<int>(std::ceil((canvasEnd.y - startY) / cellSize)),
        0,
        static_cast<int>(rows)
    );

    for (int row = firstRow; row < lastRow; ++row)
    {
        for (int col = firstCol; col < lastCol; ++col)
        {
            Zone& zone = grid.getZone(
                static_cast<std::size_t>(row),
                static_cast<std::size_t>(col)
            );

            const float x = startX + col * cellSize;
            const float y = startY + row * cellSize;

            ImVec2 p1(x, y);
            ImVec2 p2(x + cellSize, y + cellSize);

            uint32_t color = IM_COL32(80, 90, 100, 255);

            switch (currentLayer)
            {
                case MapLayerMode::FINAL_PROSPECTIVITY:
                    color = getHeatmapColor(zone.getProspectivityScore());
                    break;

                case MapLayerMode::PRIORITY_CATEGORY:
                    color = getPriorityColor(zone.getPriority());
                    break;

                case MapLayerMode::GEOLOGY_G:
                    color = getHeatmapColor(zone.getG());
                    break;

                case MapLayerMode::RADIOMETRIC_R:
                    color = getHeatmapColor(zone.getR());
                    break;

                case MapLayerMode::STRUCTURAL_S:
                    color = getHeatmapColor(zone.getS());
                    break;

                case MapLayerMode::TERRAIN_T:
                    color = getHeatmapColor(zone.getT());
                    break;

                case MapLayerMode::ANOMALY_CLUSTERS:
                    color = getClusterColor(zone.getClusterId());
                    break;
            }

            drawList->AddRectFilled(p1, p2, color);

            if (showGridLines)
                drawList->AddRect(p1, p2, IM_COL32(45, 50, 60, 255));

            if (showClusterOutlines && zone.getClusterId() > 0)
            {
                drawList->AddRect(
                    p1, p2, getClusterColor(zone.getClusterId()),
                    0.0f, 0, 2.0f
                );
            }

            if (zone.isBookmarked())
            {
                drawList->AddCircleFilled(
                    ImVec2(x + cellSize * 0.8f, y + cellSize * 0.2f),
                    std::max(2.0f, cellSize * 0.08f),
                    IM_COL32(255, 220, 40, 255)
                );
            }

            if (showOccurrences && zone.hasKnownOccurrence())
            {
                drawList->AddCircleFilled(
                    ImVec2(x + cellSize * 0.5f, y + cellSize * 0.5f),
                    std::max(2.0f, cellSize * 0.15f),
                    IM_COL32(255, 255, 255, 255)
                );
            }

            if (roiEnabled &&
                row >= roiRowStart && row <= roiRowEnd &&
                col >= roiColStart && col <= roiColEnd)
            {
                drawList->AddRect(
                    p1, p2, IM_COL32(255, 255, 0, 255),
                    0.0f, 0, 2.0f
                );
            }

            if (row == selectedRow && col == selectedCol)
            {
                drawList->AddRect(
                    p1, p2, IM_COL32(255, 255, 255, 255),
                    0.0f, 0, 3.0f
                );
            }
        }
    }

    if (showNeighborLinks)
    {
        for (int row = firstRow; row < lastRow; ++row)
        {
            for (int col = firstCol; col < lastCol; ++col)
            {
                const float x = startX + (col + 0.5f) * cellSize;
                const float y = startY + (row + 0.5f) * cellSize;

                if (col + 1 < static_cast<int>(cols))
                {
                    drawList->AddLine(
                        ImVec2(x, y),
                        ImVec2(x + cellSize, y),
                        IM_COL32(180, 180, 180, 80)
                    );
                }

                if (row + 1 < static_cast<int>(rows))
                {
                    drawList->AddLine(
                        ImVec2(x, y),
                        ImVec2(x, y + cellSize),
                        IM_COL32(180, 180, 180, 80)
                    );
                }
            }
        }
    }

    drawList->PopClipRect();
    ImGui::EndChild();
}

void MapView::renderLegend(float width, float height)
{
    ImGui::BeginChild("MapLegend", ImVec2(width, height), true);

    ImGui::Text("Legend");
    ImGui::Separator();

    if (currentLayer == MapLayerMode::PRIORITY_CATEGORY)
    {
        ImGui::ColorButton(
            "High",
            ImVec4(220 / 255.0f, 40 / 255.0f, 40 / 255.0f, 1.0f)
        );
        ImGui::SameLine();
        ImGui::Text("High Priority");

        ImGui::ColorButton(
            "Medium",
            ImVec4(235 / 255.0f, 170 / 255.0f, 20 / 255.0f, 1.0f)
        );
        ImGui::SameLine();
        ImGui::Text("Medium Priority");

        ImGui::ColorButton(
            "Low",
            ImVec4(40 / 255.0f, 120 / 255.0f, 190 / 255.0f, 1.0f)
        );
        ImGui::SameLine();
        ImGui::Text("Low Priority");
    }
    else if (currentLayer == MapLayerMode::ANOMALY_CLUSTERS)
    {
        for (int i = 1; i <= 8; ++i)
        {
            ImGui::PushID(i);

            ImGui::ColorButton(
                "ClusterColor",
                ImGui::ColorConvertU32ToFloat4(getClusterColor(i))
            );
            ImGui::SameLine();
            ImGui::Text("Cluster %d", i);

            ImGui::PopID();
        }
    }
    else
    {
        ImGui::TextColored(
            ImVec4(0.05f, 0.25f, 0.75f, 1.0f), "Low"
        );
        ImGui::SameLine();
        ImGui::Text(" -> ");
        ImGui::SameLine();
        ImGui::TextColored(
            ImVec4(0.95f, 0.15f, 0.05f, 1.0f), "High"
        );
    }

    ImGui::EndChild();
}

} // namespace GeoLocalMiner
