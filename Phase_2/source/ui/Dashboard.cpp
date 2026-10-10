#include "Dashboard.h"
#include "../../third_party/imgui/imgui.h"
#include "../data/DataLoader.h"
#include "../utils/FileUtils.h"
#include <algorithm>
#include <cstdio>
#include <cstring>
#include <string>
#include <iomanip>
#include <sstream>
#include <cmath>

namespace GeoLocalMiner {

Dashboard::Dashboard()
    : statusMessage("System Ready. Loaded Demo Placer Model."),
      statusIsError(false),
      sliderWG(0.35f), sliderWR(0.30f), sliderWS(0.20f), sliderWT(0.15f),
      selectedStrategyIdx(0),
      sliderHighThreshold(0.70f), sliderLowThreshold(0.40f),
      topNSelection(2), // Default Top 20
      needsRecalculation(true) {
    std::strncpy(csvPathBuffer, "src/data/demo_dataset.csv", sizeof(csvPathBuffer) - 1);
    csvPathBuffer[sizeof(csvPathBuffer) - 1] = '\0';
}

void Dashboard::initialize(Grid& grid, ProspectivityEngine& prospectivityEngine,
                          PriorityEngine& priorityEngine, ValidationEngine& validationEngine) {
    MapView dummyView;
    recomputeAll(grid, prospectivityEngine, priorityEngine, validationEngine, dummyView);
}

void Dashboard::recomputeAll(Grid& grid, ProspectivityEngine& prospectivityEngine,
                            PriorityEngine& priorityEngine, ValidationEngine& validationEngine,
                            MapView& mapView) {
    if (grid.getTotalCells() == 0) return;

    // 1. Configure and validate weights
    std::string weightErr;
    prospectivityEngine.forceSetWeights(sliderWG, sliderWR, sliderWS, sliderWT);
    prospectivityEngine.setStrategyByIndex(selectedStrategyIdx);

    // 2. Set classification thresholds
    priorityEngine.forceSetThresholds(sliderHighThreshold, sliderLowThreshold);

    // 3. Evaluate Grid Prospectivity Scores
    prospectivityEngine.evaluateGrid(grid);

    // 4. Classify Zones into High/Medium/Low
    priorityEngine.classifyGrid(grid);

    // 5. Detect Graph Anomaly Clusters
    grid.detectClusters(sliderHighThreshold);

    // 6. Rank Top N Zones using Priority Queue
    size_t topN = 20;
    if (topNSelection == 0) topN = 5;
    else if (topNSelection == 1) topN = 10;
    else if (topNSelection == 2) topN = 20;
    else if (topNSelection == 3) topN = 50;
    else if (topNSelection == 4) topN = 100;

    if (mapView.isRoiEnabled()) {
        int r0, r1, c0, c1;
        mapView.getRoiBounds(r0, r1, c0, c1);
        rankedTargets = priorityEngine.rankSubregionTopN(grid, r0, r1, c0, c1, topN);
        statusMessage = "Evaluated & Ranked Subregion ROI [" + std::to_string(r0) + ".." + std::to_string(r1) +
                        ", " + std::to_string(c0) + ".." + std::to_string(c1) + "] (Top " + std::to_string(rankedTargets.size()) + ")";
    } else {
        rankedTargets = priorityEngine.rankTopN(grid, topN);
        statusMessage = "Calculated grid prospectivity & ranked Top " + std::to_string(rankedTargets.size()) + " zones.";
    }

    // 7. Validate against known occurrences
    valMetrics = validationEngine.validate(grid);
    gridStats = grid.computeStatistics();

    statusIsError = false;
    needsRecalculation = false;
}

void Dashboard::renderHeader() {
    // Header Bar
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.08f, 0.11f, 0.16f, 1.0f));
    ImGui::BeginChild("HeaderPanel", ImVec2(-1, 62), true, ImGuiWindowFlags_NoScrollbar);

    ImGui::TextColored(ImVec4(0.35f, 0.75f, 1.0f, 1.0f), "GeoLocal-Miner");
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(0.70f, 0.75f, 0.82f, 1.0f), "| Multi-Scale Spatial Mineral Prospectivity & Target Prioritization Framework");
    ImGui::SameLine(ImGui::GetWindowWidth() - 320);
    ImGui::TextColored(ImVec4(0.40f, 0.85f, 0.50f, 1.0f), "C++ Zero-Dependency Engine");

    // Important Decision Support Disclaimer
    ImGui::TextColored(ImVec4(1.0f, 0.80f, 0.25f, 1.0f), 
        "[DISCLAIMER] Decision-Support Prototype. A high prospectivity score indicates priority for further field investigation and does not prove mineralization.");

    ImGui::EndChild();
    ImGui::PopStyleColor();
}

void Dashboard::renderControls(Grid& grid, ProspectivityEngine& prospectivityEngine,
                              PriorityEngine& priorityEngine, ValidationEngine& validationEngine,
                              MapView& mapView) {
    ImGui::TextColored(ImVec4(0.3f, 0.8f, 1.0f, 1.0f), "EXPLORATION CONTROLS");
    ImGui::Separator();

    // Dataset Management Section
    if (ImGui::CollapsingHeader("1. Dataset Ingestion", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Text("Active: %s (%zux%zu, %zu cells)", 
                    grid.getDatasetName().c_str(), grid.getRows(), grid.getCols(), grid.getTotalCells());

        if (ImGui::Button("Load Demo Placer Model (20x20)")) {
            DataLoader::generateSyntheticPlacerDataset(grid, 20, 20);
            recomputeAll(grid, prospectivityEngine, priorityEngine, validationEngine, mapView);
            statusMessage = "Loaded Synthetic Placer Dataset (20x20, 400 cells).";
            statusIsError = false;
        }

        ImGui::Spacing();
        ImGui::Text("Load Custom CSV:");
        ImGui::InputText("##CsvPath", csvPathBuffer, sizeof(csvPathBuffer));
        ImGui::SameLine();
        if (ImGui::Button("Import CSV")) {
            LoadResult res = DataLoader::loadFromCsv(csvPathBuffer, grid);
            if (res.success) {
                recomputeAll(grid, prospectivityEngine, priorityEngine, validationEngine, mapView);
                statusMessage = "Imported CSV successfully (" + std::to_string(res.rows) + " rows).";
                statusIsError = false;
            } else {
                statusMessage = "Import failed: " + res.error;
                statusIsError = true;
            }
        }
    }

    ImGui::Spacing();

    // Scoring Model Parameters Section
    if (ImGui::CollapsingHeader("2. Scoring Model & Weights", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::TextDisabled("Configurable prototype weights (sum must equal 1.0):");

        bool changed = false;
        changed |= ImGui::SliderFloat("wG: Geology", &sliderWG, 0.0f, 1.0f, "%.2f");
        changed |= ImGui::SliderFloat("wR: Radiometrics", &sliderWR, 0.0f, 1.0f, "%.2f");
        changed |= ImGui::SliderFloat("wS: Structure", &sliderWS, 0.0f, 1.0f, "%.2f");
        changed |= ImGui::SliderFloat("wT: Terrain", &sliderWT, 0.0f, 1.0f, "%.2f");

        float sum = sliderWG + sliderWR + sliderWS + sliderWT;
        bool validSum = (std::fabs(sum - 1.0f) < 0.01f);

        ImGui::Text("Weight Sum: ");
        ImGui::SameLine();
        if (validSum) {
            ImGui::TextColored(ImVec4(0.3f, 0.9f, 0.3f, 1.0f), "%.2f (Valid)", sum);
        } else {
            ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "%.2f (Invalid: must sum to 1.0)", sum);
        }

        ImGui::SameLine(180);
        if (ImGui::Button("Normalize to 1.0")) {
            if (sum > 1e-4f) {
                sliderWG /= sum;
                sliderWR /= sum;
                sliderWS /= sum;
                sliderWT /= sum;
            } else {
                sliderWG = sliderWR = sliderWS = sliderWT = 0.25f;
            }
            changed = true;
        }

        ImGui::Spacing();
        ImGui::Text("Scoring Strategy (OOP Polymorphism):");
        const char* strategies[] = {
            "Standard Linear Weighted Sum",
            "Geological-Gated (Mineral Systems)",
            "Multiplicative Non-Compensatory"
        };
        if (ImGui::Combo("##StrategyCombo", &selectedStrategyIdx, strategies, 3)) {
            changed = true;
        }

        if (changed) {
            needsRecalculation = true;
        }
    }

    ImGui::Spacing();

    // Priority Classification Thresholds Section
    if (ImGui::CollapsingHeader("3. Priority Thresholds", ImGuiTreeNodeFlags_DefaultOpen)) {
        bool changed = false;
        changed |= ImGui::SliderFloat("High Threshold", &sliderHighThreshold, 0.50f, 0.95f, ">= %.2f");
        changed |= ImGui::SliderFloat("Low Threshold", &sliderLowThreshold, 0.10f, 0.60f, "< %.2f");

        if (sliderLowThreshold >= sliderHighThreshold) {
            sliderLowThreshold = sliderHighThreshold - 0.05f;
            changed = true;
        }

        ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.0f), "Classes:");
        ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "  HIGH:   Score >= %.2f", sliderHighThreshold);
        ImGui::TextColored(ImVec4(1.0f, 0.75f, 0.2f, 1.0f), "  MEDIUM: %.2f <= Score < %.2f", sliderLowThreshold, sliderHighThreshold);
        ImGui::TextColored(ImVec4(0.3f, 0.7f, 1.0f, 1.0f), "  LOW:    Score < %.2f", sliderLowThreshold);

        if (changed) {
            needsRecalculation = true;
        }
    }

    ImGui::Spacing();

    // Multi-Scale ROI Section
    if (ImGui::CollapsingHeader("4. Multi-Scale Subregion (ROI)", ImGuiTreeNodeFlags_DefaultOpen)) {
        bool roiOn = mapView.isRoiEnabled();
        if (ImGui::Checkbox("Enable Local Subregion Focus", &roiOn)) {
            mapView.setRoiEnabled(roiOn);
            needsRecalculation = true;
        }

        if (roiOn) {
            int r0, r1, c0, c1;
            mapView.getRoiBounds(r0, r1, c0, c1);
            int maxR = static_cast<int>(grid.getRows() > 0 ? grid.getRows() - 1 : 0);
            int maxC = static_cast<int>(grid.getCols() > 0 ? grid.getCols() - 1 : 0);

            bool roiChanged = false;
            roiChanged |= ImGui::SliderInt("Row Start", &r0, 0, maxR);
            roiChanged |= ImGui::SliderInt("Row End", &r1, 0, maxR);
            roiChanged |= ImGui::SliderInt("Col Start", &c0, 0, maxC);
            roiChanged |= ImGui::SliderInt("Col End", &c1, 0, maxC);

            if (r1 < r0) r1 = r0;
            if (c1 < c0) c1 = c0;

            if (roiChanged) {
                mapView.setRoiBounds(r0, r1, c0, c1);
                needsRecalculation = true;
            }
        }
    }

    ImGui::Spacing();

    // Top-N Target Ranking Selector
    ImGui::Text("Ranking Target Count (Top N):");
    const char* topNOptions[] = { "Top 5", "Top 10", "Top 20", "Top 50", "Top 100" };
    if (ImGui::Combo("##TopNCombo", &topNSelection, topNOptions, 5)) {
        needsRecalculation = true;
    }

    ImGui::Spacing();

    // Big Recalculate Button
    ImGui::PushStyleColor(ImGuiCol_Button, needsRecalculation ? ImVec4(0.85f, 0.35f, 0.15f, 1.0f) : ImVec4(0.20f, 0.50f, 0.75f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.95f, 0.45f, 0.25f, 1.0f));
    if (ImGui::Button(needsRecalculation ? "RECALCULATE PROSPECTIVITY *" : "RECALCULATE PROSPECTIVITY", ImVec2(-1, 36))) {
        recomputeAll(grid, prospectivityEngine, priorityEngine, validationEngine, mapView);
    }
    ImGui::PopStyleColor(2);

    // Status message line
    if (!statusMessage.empty()) {
        ImGui::Spacing();
        ImVec4 msgColor = statusIsError ? ImVec4(1.0f, 0.3f, 0.3f, 1.0f) : ImVec4(0.4f, 0.85f, 0.4f, 1.0f);
        ImGui::TextColored(msgColor, "%s", statusMessage.c_str());
    }
}

void Dashboard::renderSummaryCards() {
    ImGui::TextColored(ImVec4(0.3f, 0.8f, 1.0f, 1.0f), "EVALUATION SUMMARY & VALIDATION SCORECARD");
    ImGui::Separator();

    // Row 1: Grid Overview & Priority Distribution
    ImGui::Columns(4, "SummaryCols", false);

    // Card 1: Grid Dimension & Total
    ImGui::TextDisabled("TOTAL CELLS");
    ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "%zu cells", gridStats.totalCells);
    ImGui::TextDisabled("Mean Score: %.3f", gridStats.meanScore);
    ImGui::NextColumn();

    // Card 2: High Priority Count
    ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "HIGH PRIORITY");
    ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "%zu zones (%.1f%%)", 
                       gridStats.highPriorityCount, 
                       gridStats.totalCells > 0 ? (100.0 * gridStats.highPriorityCount / gridStats.totalCells) : 0.0);
    ImGui::TextDisabled("Target Exploration Zones");
    ImGui::NextColumn();

    // Card 3: Medium Priority Count
    ImGui::TextColored(ImVec4(1.0f, 0.75f, 0.2f, 1.0f), "MEDIUM PRIORITY");
    ImGui::TextColored(ImVec4(1.0f, 0.75f, 0.2f, 1.0f), "%zu zones (%.1f%%)",
                       gridStats.mediumPriorityCount,
                       gridStats.totalCells > 0 ? (100.0 * gridStats.mediumPriorityCount / gridStats.totalCells) : 0.0);
    ImGui::TextDisabled("Secondary Targets");
    ImGui::NextColumn();

    // Card 4: Low Priority Count
    ImGui::TextColored(ImVec4(0.4f, 0.7f, 1.0f, 1.0f), "LOW PRIORITY");
    ImGui::TextColored(ImVec4(0.4f, 0.7f, 1.0f, 1.0f), "%zu zones (%.1f%%)",
                       gridStats.lowPriorityCount,
                       gridStats.totalCells > 0 ? (100.0 * gridStats.lowPriorityCount / gridStats.totalCells) : 0.0);
    ImGui::TextDisabled("Pruned Background Area");
    ImGui::NextColumn();

    ImGui::Columns(1);
    ImGui::Spacing();

    // Row 2: Prototype Validation Scorecard (AMD Occurrence Testing)
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.12f, 0.15f, 0.20f, 0.8f));
    ImGui::BeginChild("ValScorecard", ImVec2(-1, 58), true);
    ImGui::Columns(4, "ValCols", false);

    ImGui::TextColored(ImVec4(1.0f, 0.85f, 0.2f, 1.0f), "KNOWN OCCURRENCES");
    ImGui::Text("%zu Total Documented", valMetrics.total);
    ImGui::NextColumn();

    ImGui::TextColored(ImVec4(0.4f, 0.9f, 0.4f, 1.0f), "HIGH CAPTURE RATE");
    ImGui::Text("%zu / %zu (%.1f%%)", valMetrics.high, valMetrics.total, valMetrics.highRate);
    ImGui::NextColumn();

    ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "HIGH+MED CAPTURE");
    ImGui::Text("%zu / %zu (%.1f%%)", 
                valMetrics.high + valMetrics.medium, 
                valMetrics.total, 
                valMetrics.highMedRate);
    ImGui::NextColumn();

    ImGui::TextColored(ImVec4(1.0f, 0.6f, 0.2f, 1.0f), "ENRICHMENT RATIO");
    ImGui::Text("%.2fx efficiency", valMetrics.enrich);
    ImGui::NextColumn();

    ImGui::Columns(1);
    ImGui::EndChild();
    ImGui::PopStyleColor();
}

void Dashboard::renderRankingTable(Grid& grid, MapView& mapView, ZoneDetails& zoneDetails) {
    (void)zoneDetails;
    ImGui::TextColored(ImVec4(0.3f, 0.8f, 1.0f, 1.0f), "PRIORITY QUEUE RANKED TARGETS");
    ImGui::SameLine();
    ImGui::TextDisabled("(Extracted via Custom Binary Max-Heap PriorityQueue: %zu candidates)", rankedTargets.size());

    // Export & Action Buttons
    ImGui::SameLine(ImGui::GetWindowWidth() - 480);
    if (ImGui::Button("Export Targets (CSV)")) {
        std::string err;
        if (DataLoader::exportRankedTargetsCsv("ranked_targets_export.csv", rankedTargets, err)) {
            statusMessage = "Exported ranked targets to 'ranked_targets_export.csv'.";
            statusIsError = false;
        } else {
            statusMessage = "Export failed: " + err;
            statusIsError = true;
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("Export Grid (CSV)")) {
        std::string err;
        if (DataLoader::exportGridCsv("grid_evaluation_export.csv", grid, err)) {
            statusMessage = "Exported complete grid matrix to 'grid_evaluation_export.csv'.";
            statusIsError = false;
        } else {
            statusMessage = "Export failed: " + err;
            statusIsError = true;
        }
    }

    ImGui::Separator();

    // Render Table
    static ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | 
                                   ImGuiTableFlags_ScrollY | ImGuiTableFlags_Resizable;

    if (ImGui::BeginTable("RankingTable", 11, flags, ImVec2(-1, 160))) {
        ImGui::TableSetupColumn("Rank", ImGuiTableColumnFlags_WidthFixed, 42.0f);
        ImGui::TableSetupColumn("Zone ID", ImGuiTableColumnFlags_WidthFixed, 60.0f);
        ImGui::TableSetupColumn("Grid [R,C]", ImGuiTableColumnFlags_WidthFixed, 75.0f);
        ImGui::TableSetupColumn("Coordinates (m)", ImGuiTableColumnFlags_WidthFixed, 130.0f);
        ImGui::TableSetupColumn("G", ImGuiTableColumnFlags_WidthFixed, 45.0f);
        ImGui::TableSetupColumn("R", ImGuiTableColumnFlags_WidthFixed, 45.0f);
        ImGui::TableSetupColumn("S", ImGuiTableColumnFlags_WidthFixed, 45.0f);
        ImGui::TableSetupColumn("T", ImGuiTableColumnFlags_WidthFixed, 45.0f);
        ImGui::TableSetupColumn("Score", ImGuiTableColumnFlags_WidthFixed, 65.0f);
        ImGui::TableSetupColumn("Priority", ImGuiTableColumnFlags_WidthFixed, 75.0f);
        ImGui::TableSetupColumn("Known AMD Occurrence", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableHeadersRow();

        for (size_t i = 0; i < rankedTargets.size(); ++i) {
            const Zone& tz = rankedTargets[i];
            ImGui::TableNextRow();

            bool isSelected = (mapView.getSelectedRow() == tz.getRow() && mapView.getSelectedCol() == tz.getCol());
            if (isSelected) {
                ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, IM_COL32(0, 120, 200, 70));
            }

            // Col 0: Rank
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("#%zu", i + 1);

            // Col 1: Zone ID
            ImGui::TableSetColumnIndex(1);
            char rowSelectId[32];
            std::snprintf(rowSelectId, sizeof(rowSelectId), "#%d##row%u", tz.getId(), static_cast<unsigned int>(i));
            if (ImGui::Selectable(rowSelectId, isSelected, ImGuiSelectableFlags_SpanAllColumns)) {
                mapView.setSelectedCell(tz.getRow(), tz.getCol());
            }

            // Col 2: Grid [R, C]
            ImGui::TableSetColumnIndex(2);
            ImGui::Text("[%d, %d]", tz.getRow(), tz.getCol());

            // Col 3: Coordinates
            ImGui::TableSetColumnIndex(3);
            ImGui::Text("(%.0f, %.0f)", tz.getX(), tz.getY());

            // Col 4: G
            ImGui::TableSetColumnIndex(4);
            ImGui::Text("%.2f", tz.getG());

            // Col 5: R
            ImGui::TableSetColumnIndex(5);
            ImGui::Text("%.2f", tz.getR());

            // Col 6: S
            ImGui::TableSetColumnIndex(6);
            ImGui::Text("%.2f", tz.getS());

            // Col 7: T
            ImGui::TableSetColumnIndex(7);
            ImGui::Text("%.2f", tz.getT());

            // Col 8: Score
            ImGui::TableSetColumnIndex(8);
            ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.4f, 1.0f), "%.4f", tz.getProspectivityScore());

            // Col 9: Priority
            ImGui::TableSetColumnIndex(9);
            ImVec4 pColor;
            if (tz.getPriority() == PriorityLevel::HIGH) pColor = ImVec4(1.0f, 0.3f, 0.3f, 1.0f);
            else if (tz.getPriority() == PriorityLevel::MEDIUM) pColor = ImVec4(1.0f, 0.75f, 0.2f, 1.0f);
            else pColor = ImVec4(0.4f, 0.7f, 1.0f, 1.0f);
            ImGui::TextColored(pColor, "%s", tz.getPriorityString().c_str());

            // Col 10: Known Occurrence
            ImGui::TableSetColumnIndex(10);
            if (tz.hasKnownOccurrence()) {
                ImGui::TextColored(ImVec4(1.0f, 0.85f, 0.2f, 1.0f), "* %s", tz.getOccurrenceName().c_str());
            } else {
                ImGui::TextDisabled("-");
            }
        }

        ImGui::EndTable();
    }
}

} // namespace GeoLocalMiner
