#include "ProspectivityEngine.h"
#include "Grid.h"
#include <cmath>

namespace GeoLocalMiner {

ProspectivityEngine::ProspectivityEngine()
    : weightG(0.35), weightR(0.30), weightS(0.20), weightT(0.15),
      strategy(std::make_unique<LinearWeightedScoringStrategy>()) {}

ProspectivityEngine::ProspectivityEngine(std::unique_ptr<ScoringStrategy> initialStrategy)
    : weightG(0.35), weightR(0.30), weightS(0.20), weightT(0.15),
      strategy(std::move(initialStrategy)) {
    if (!strategy) {
        strategy = std::make_unique<LinearWeightedScoringStrategy>();
    }
}

bool ProspectivityEngine::setWeights(double g, double r, double s, double t, std::string& errorMsg) {
    if (!strategy) {
        errorMsg = "Scoring strategy is uninitialized.";
        return false;
    }
    if (!strategy->validateWeights(g, r, s, t, errorMsg)) {
        return false;
    }
    weightG = g;
    weightR = r;
    weightS = s;
    weightT = t;
    return true;
}

void ProspectivityEngine::forceSetWeights(double g, double r, double s, double t) {
    weightG = g;
    weightR = r;
    weightS = s;
    weightT = t;
}

void ProspectivityEngine::normalizeWeights() {
    double sum = weightG + weightR + weightS + weightT;
    if (sum > 1e-6) {
        weightG /= sum;
        weightR /= sum;
        weightS /= sum;
        weightT /= sum;
    } else {
        // Fallback to equal weighting
        weightG = 0.25;
        weightR = 0.25;
        weightS = 0.25;
        weightT = 0.25;
    }
}

void ProspectivityEngine::setStrategy(std::unique_ptr<ScoringStrategy> newStrategy) {
    if (newStrategy) {
        strategy = std::move(newStrategy);
    }
}

void ProspectivityEngine::setStrategyByIndex(int index) {
    switch (index) {
        case 0:
            strategy = std::make_unique<LinearWeightedScoringStrategy>();
            break;
        case 1:
            strategy = std::make_unique<GeologicalGatedScoringStrategy>();
            break;
        case 2:
            strategy = std::make_unique<MultiplicativeLimitingScoringStrategy>();
            break;
        default:
            strategy = std::make_unique<LinearWeightedScoringStrategy>();
            break;
    }
}

double ProspectivityEngine::calculateZoneScore(const Zone& zone) const {
    if (!strategy) return 0.0;
    return strategy->computeScore(zone.getG(), zone.getR(), zone.getS(), zone.getT(),
                                  weightG, weightR, weightS, weightT);
}

void ProspectivityEngine::evaluateZone(Zone& zone) const {
    double score = calculateZoneScore(zone);
    zone.setProspectivityScore(score);
}

void ProspectivityEngine::evaluateGrid(Grid& grid) const {
    for (size_t r = 0; r < grid.getRows(); ++r) {
        for (size_t c = 0; c < grid.getCols(); ++c) {
            evaluateZone(grid.getZone(r, c));
        }
    }
}

} // namespace GeoLocalMiner
