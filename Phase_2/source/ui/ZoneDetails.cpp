#include "ZoneDetails.h"
#include "../../third_party/imgui/imgui.h"
#include <string>

namespace GeoLocalMiner {

ZoneDetails::ZoneDetails() : trailMsg("") {}

void ZoneDetails::addTargetToTrail(const Zone& zone) {
    for (size_t i = 0; i < targetTrail.size(); ++i) {
        if (targetTrail[i].getId() == zone.getId()) {
            trailMsg = "Zone already in trail.";
            return;
        }
    }

    targetTrail.push_back(zone);
    trailMsg = "Zone added to trail.";
}

void ZoneDetails::removeTargetFromTrail(size_t idx) {
    if (idx >= targetTrail.size()) return;

    int id = targetTrail[idx].getId();
    targetTrail.removeAt(idx);
    trailMsg = "Removed Zone #" + std::to_string(id) + ".";
}

void ZoneDetails::clearTargetTrail() {
    targetTrail.clear();
    trailMsg = "Trail cleared.";
}

void ZoneDetails::render(Grid& grid, int& selectedRow, int& selectedCol) {
    ImGui::TextColored(ImVec4(0.3f, 0.8f, 1, 1), "ZONE INSPECTOR & TARGET TRAIL");
    ImGui::Separator();

    if (selectedRow < 0 || selectedCol < 0 ||
        !grid.inBounds(selectedRow, selectedCol)) {
        ImGui::TextDisabled("Select a grid cell to inspect its details.");

        ImGui::Text("Target Trail (%zu)", targetTrail.size());
        if (!trailMsg.empty())
            ImGui::Text("%s", trailMsg.c_str());

        if (ImGui::Button("Clear Trail"))
            clearTargetTrail();

        return;
    }

    Zone& z = grid.getZone(
        static_cast<size_t>(selectedRow),
        static_cast<size_t>(selectedCol)
    );

    ImGui::Text("Zone #%d", z.getId());
    ImGui::SameLine(150);
    ImGui::Text("Grid: [%d, %d]", z.getRow(), z.getCol());
    ImGui::Text("Coordinates: (%.1f, %.1f) m", z.getX(), z.getY());

    ImVec4 color;
    switch (z.getPriority()) {
        case PriorityLevel::HIGH:
            color = ImVec4(1, 0.25f, 0.25f, 1);
            break;
        case PriorityLevel::MEDIUM:
            color = ImVec4(1, 0.75f, 0.2f, 1);
            break;
        default:
            color = ImVec4(0.35f, 0.75f, 1, 1);
    }

    ImGui::Text("Priority: ");
    ImGui::SameLine();
    ImGui::TextColored(color, "%s", z.getPriorityString().c_str());

    ImGui::Text("Prospectivity: %.4f", z.getProspectivityScore());
    ImGui::ProgressBar(
        static_cast<float>(z.getProspectivityScore()),
        ImVec2(-1, 16)
    );

    if (z.hasKnownOccurrence()) {
        ImGui::TextColored(
            ImVec4(1, 0.85f, 0.2f, 1),
            "Known occurrence: %s",
            z.getOccurrenceName().c_str()
        );
    } else {
        ImGui::TextDisabled("No known occurrence recorded.");
    }

    if (z.getClusterId() > 0)
        ImGui::Text("Cluster: #%d", z.getClusterId());

    ImGui::Separator();
    ImGui::Text("SPATIAL INDICATORS");

    auto showIndicator = [](const char* name, double val,
                            double raw, const char* unit) {
        ImGui::Text("%s: %.3f (Raw: %.1f %s)",
                    name, val, raw, unit);
        ImGui::ProgressBar(static_cast<float>(val), ImVec2(-1, 12));
    };

    showIndicator("Geology", z.getG(), z.getRawG(), "Index");
    showIndicator("Radiometrics", z.getR(), z.getRawR(), "cps");
    showIndicator("Structure", z.getS(), z.getRawS(), "km/km2");
    showIndicator("Terrain", z.getT(), z.getRawT(), "deg/slope");

    ImGui::Separator();
    ImGui::Text("GRAPH NEIGHBORS");

    auto neighbors = grid.getNeighbors(
        static_cast<size_t>(selectedRow),
        static_cast<size_t>(selectedCol)
    );

    ImGui::BeginChild("Neighbors", ImVec2(-1, 100), true);

    for (const Zone* nz : neighbors) {
        if (!nz) continue;

        std::string label = "Zone #" + std::to_string(nz->getId()) +
            " [" + std::to_string(nz->getRow()) + "," +
            std::to_string(nz->getCol()) + "]";

        if (ImGui::Selectable(label.c_str())) {
            selectedRow = nz->getRow();
            selectedCol = nz->getCol();
        }
    }

    ImGui::EndChild();

    ImGui::Separator();
    ImGui::Text("TARGET TRAIL (%zu)", targetTrail.size());

    if (ImGui::Button("Bookmark Zone"))
        addTargetToTrail(z);

    ImGui::SameLine();

    if (ImGui::Button("Clear Trail"))
        clearTargetTrail();

    if (!trailMsg.empty())
        ImGui::Text("%s", trailMsg.c_str());

    ImGui::BeginChild("Trail", ImVec2(-1, 110), true);

    size_t removeIdx = targetTrail.size();

    for (size_t i = 0; i < targetTrail.size(); ++i) {
        const Zone& item = targetTrail[i];
        ImGui::PushID(static_cast<int>(i));

        std::string label = "Zone #" + std::to_string(item.getId());

        if (ImGui::Selectable(label.c_str())) {
            selectedRow = item.getRow();
            selectedCol = item.getCol();
        }

        ImGui::SameLine();

        if (ImGui::SmallButton("X"))
            removeIdx = i;

        ImGui::PopID();
    }

    if (removeIdx < targetTrail.size())
        removeTargetFromTrail(removeIdx);

    ImGui::EndChild();
}

} // namespace GeoLocalMiner
