#ifndef GEOLOCAL_MINER_PROSPECTIVITY_ENGINE_H
#define GEOLOCAL_MINER_PROSPECTIVITY_ENGINE_H

#include <memory>
#include <string>
#include "ScoringStrategy.h"
#include "Zone.h"

namespace GeoLocalMiner {

class Grid; // Forward declaration

/**
 * @brief Computational scoring engine for calculating mineral prospectivity.
 * 
 * Manages indicator weights, validates constraints, and executes the selected
 * polymorphic scoring strategy across spatial zones.
 */
class ProspectivityEngine {
private:
    double weightG; // Weight for Geological suitability
    double weightR; // Weight for Radiometric suitability
    double weightS; // Weight for Structural suitability
    double weightT; // Weight for Terrain suitability

    std::unique_ptr<ScoringStrategy> strategy;

public:
    ProspectivityEngine();
    explicit ProspectivityEngine(std::unique_ptr<ScoringStrategy> initialStrategy);

    // Getters
    double getWeightG() const { return weightG; }
    double getWeightR() const { return weightR; }
    double getWeightS() const { return weightS; }
    double getWeightT() const { return weightT; }
    double getWeightSum() const { return weightG + weightR + weightS + weightT; }

    const ScoringStrategy* getStrategy() const { return strategy.get(); }
    std::string getStrategyName() const { return strategy ? strategy->getName() : "None"; }
    std::string getStrategyFormula() const { return strategy ? strategy->getFormulaDescription() : ""; }

    // Setters & Configuration
    bool setWeights(double g, double r, double s, double t, std::string& errorMsg);
    void forceSetWeights(double g, double r, double s, double t);
    void normalizeWeights();

    void setStrategy(std::unique_ptr<ScoringStrategy> newStrategy);
    void setStrategyByIndex(int index);

    // Evaluation
    double calculateZoneScore(const Zone& zone) const;
    void evaluateZone(Zone& zone) const;
    void evaluateGrid(Grid& grid) const;
};

} // namespace GeoLocalMiner

#endif // GEOLOCAL_MINER_PROSPECTIVITY_ENGINE_H
