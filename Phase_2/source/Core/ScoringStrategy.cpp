#include "ScoringStrategy.h"
#include "../utils/Normalization.h"
#include <cmath>
#include <sstream>
#include <iomanip>

namespace GeoLocalMiner {

bool ScoringStrategy::validateWeights(
    double wG, double wR, double wS, double wT,
    std::string& err) const
{
    if (wG < 0.0 || wR < 0.0 || wS < 0.0 || wT < 0.0)
    {
        err = "Weights cannot be negative.";
        return false;
    }

    double sum = wG + wR + wS + wT;

    if (std::abs(sum - 1.0) > 0.001)
    {
        std::ostringstream out;

        out << "Weights must sum to 1.0 (current sum: "
            << std::fixed << std::setprecision(3)
            << sum << ").";

        err = out.str();
        return false;
    }

    return true;
}


double LinearWeightedScoringStrategy::computeScore(
    double G, double R, double S, double T,
    double wG, double wR, double wS, double wT) const
{
    // Formula: Score = wG*G + wR*R + wS*S + wT*T
    double score =
        (wG * G) +
        (wR * R) +
        (wS * S) +
        (wT * T);

    return Normalization::clamp(score, 0.0, 1.0);
}


double GeologicalGatedScoringStrategy::computeScore(
    double G, double R, double S, double T,
    double wG, double wR, double wS, double wT) const
{
    // Formula: Base Score = wG*G + wR*R + wS*S + wT*T
    double base =
        (wG * G) +
        (wR * R) +
        (wS * S) +
        (wT * T);

    // Formula: Gate = max(G, R)
    double gate = (G > R) ? G : R;

    // Formula: Factor = 0.30 + 0.70 * Gate
    double factor = 0.30 + (0.70 * gate);

    // Formula: Final Score = Base Score * Factor
    double score = base * factor;

    return Normalization::clamp(score, 0.0, 1.0);
}


double MultiplicativeLimitingScoringStrategy::computeScore(
    double G, double R, double S, double T,
    double wG, double wR, double wS, double wT) const
{
    // Small value prevents the score from becoming exactly zero
    const double eps = 0.02;

    double g = G * (1.0 - eps) + eps;
    double r = R * (1.0 - eps) + eps;
    double s = S * (1.0 - eps) + eps;
    double t = T * (1.0 - eps) + eps;

    // Formula: Score = G^wG * R^wR * S^wS * T^wT
    double score =
        std::pow(g, wG) *
        std::pow(r, wR) *
        std::pow(s, wS) *
        std::pow(t, wT);

    // Formula: Remapped = (Score - eps) / (1 - eps)
    double remapped = (score - eps) / (1.0 - eps);

    return Normalization::clamp(remapped, 0.0, 1.0);
}

}