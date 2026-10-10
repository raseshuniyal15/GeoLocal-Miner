#ifndef SCORING_STRATEGY_H
#define SCORING_STRATEGY_H
#include <string>
/*VARIABLE NAMING
G	Normalized geological value
R	Normalized radiometric value
S	Normalized seismic value
T	Normalized terrain value
wG	Weight given to geological factor
wR	Weight given to radiometric factor
wS	Weight given to seismic factor
wT	Weight given to terrain factor
err	Stores weight validation error message*/
/* FUNCTION NAME AND THEIR USE
getName()	Returns the name of the scoring method
getFormulaDescription()	Returns the formula used by the method
computeScore()	Calculates the prospectivity score
validateWeights()	Checks whether the four weights are valid
*/
/* CLASSES
ScoringStrategy	Base class for all scoring methods
LinearWeightedScoringStrategy Uses the normal weighted-sum formula
GeologicalGatedScoringStrategy Adjusts the score using G/R as a geological gate
MultiplicativeLimitingScoringStrategy Multiplies the indicators so weak factors limit the result
*/
namespace GeoLocalMiner {

class ScoringStrategy
{
public:
    virtual ~ScoringStrategy() = default;

    virtual std::string getName() const = 0;
    virtual std::string getFormulaDescription() const = 0;

    virtual double computeScore(
        double G, double R, double S, double T,
        double wG, double wR, double wS, double wT) const = 0;

    virtual bool validateWeights(
        double wG, double wR, double wS, double wT,
        std::string& err) const;
};


class LinearWeightedScoringStrategy : public ScoringStrategy
{
public:
    std::string getName() const override
    {
        return "Linear Weighted Sum";
    }

    std::string getFormulaDescription() const override
    {
        return "Score = wG*G + wR*R + wS*S + wT*T";
    }

    double computeScore(
        double G, double R, double S, double T,
        double wG, double wR, double wS, double wT) const override;
};


class GeologicalGatedScoringStrategy : public ScoringStrategy
{
public:
    std::string getName() const override
    {
        return "Geological Gated";
    }

    std::string getFormulaDescription() const override
    {
        return "Score = weighted score * (0.30 + 0.70 * max(G,R))";
    }

    double computeScore(
        double G, double R, double S, double T,
        double wG, double wR, double wS, double wT) const override;
};


class MultiplicativeLimitingScoringStrategy : public ScoringStrategy
{
public:
    std::string getName() const override
    {
        return "Multiplicative";
    }

    std::string getFormulaDescription() const override
    {
        return "Score = G^wG * R^wR * S^wS * T^wT";
    }

    double computeScore(
        double G, double R, double S, double T,
        double wG, double wR, double wS, double wT) const override;
};

}

#endif