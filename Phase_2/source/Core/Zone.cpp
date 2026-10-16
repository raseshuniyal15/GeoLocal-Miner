#include "Zone.h"
#include "../utils/Normalization.h"
#include <sstream>
#include <iomanip>
namespace GeoLocalMiner {
Zone::Zone()
    : zoneNo(-1), gridR(0), gridC(0),
      coordX(0.0), coordY(0.0),
      geoRaw(0.0), radRaw(0.0), seisRaw(0.0), terrRaw(0.0),
      geoN(0.0), radN(0.0), seisN(0.0), terrN(0.0),
      prosScore(0.0), prio(PriorityLevel::LOW),
      oreKnown(false), oreTag(""),
      markFlag(false), clusterNo(-1)
{
}
Zone::Zone(int zoneNo, int gridR, int gridC, double coordX, double coordY)
    : zoneNo(zoneNo), gridR(gridR), gridC(gridC),
      coordX(coordX), coordY(coordY),
      geoRaw(0.0), radRaw(0.0), seisRaw(0.0), terrRaw(0.0),
      geoN(0.0), radN(0.0), seisN(0.0), terrN(0.0),
      prosScore(0.0), prio(PriorityLevel::LOW),
      oreKnown(false), oreTag(""),
      markFlag(false), clusterNo(-1)
{
}
void Zone::setRawIndicators(double g, double r, double s, double t)
{
    geoRaw = g;
    radRaw = r;
    seisRaw = s;
    terrRaw = t;
}
void Zone::setNormalizedIndicators(double g, double r, double s, double t)
{
    geoN = Normalization::clamp(g, 0.0, 1.0);
    radN = Normalization::clamp(r, 0.0, 1.0);
    seisN = Normalization::clamp(s, 0.0, 1.0);
    terrN = Normalization::clamp(t, 0.0, 1.0);
}
void Zone::setG(double v)
{
    geoN = Normalization::clamp(v, 0.0, 1.0);
}
void Zone::setR(double v)
{
    radN = Normalization::clamp(v, 0.0, 1.0);
}
void Zone::setS(double v)
{
    seisN = Normalization::clamp(v, 0.0, 1.0);
}
void Zone::setT(double v)
{
    terrN = Normalization::clamp(v, 0.0, 1.0);
}
void Zone::setProspectivityScore(double v)
{
    prosScore = Normalization::clamp(v, 0.0, 1.0);
}
std::string Zone::getPriorityString() const
{
    switch (prio)
    {
        case PriorityLevel::HIGH:
            return "HIGH";
        case PriorityLevel::MEDIUM:
            return "MEDIUM";
        case PriorityLevel::LOW:
            return "LOW";
        default:
            return "UNKNOWN";
    }
}
std::string Zone::getFormattedCoordinates() const
{
    std::ostringstream out;
    out << std::fixed << std::setprecision(4)
        << "(" << coordX << ", " << coordY << ")";
    return out.str();
}
std::string Zone::toString() const
{
    std::ostringstream out;

    out << "Zone #" << zoneNo
        << " [" << gridR << "," << gridC << "] "
        << "Coord: " << getFormattedCoordinates()
        << " | Score: " << std::fixed << std::setprecision(3)
        << prosScore
        << " (" << getPriorityString() << ") | "
        << "G:" << geoN
        << " R:" << radN
        << " S:" << seisN
        << " T:" << terrN;

    if (oreKnown)
    {
        out << " [KNOWN: "
            << (oreTag.empty() ? "AMD Deposit" : oreTag)
            << "]";
    }

    return out.str();
}
}