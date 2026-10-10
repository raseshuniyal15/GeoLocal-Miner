#ifndef ZONE_H
#define ZONE_H

#include <string>

namespace GeoLocalMiner {

enum class PriorityLevel
{
    LOW = 0,
    MEDIUM = 1,
    HIGH = 2
};

class Zone
{
private:
    int zoneNo = -1;
    int gridR = 0, gridC = 0;

    double coordX = 0.0, coordY = 0.0;

    double geoRaw = 0.0, radRaw = 0.0;
    double seisRaw = 0.0, terrRaw = 0.0;

    double geoN = 0.0, radN = 0.0;
    double seisN = 0.0, terrN = 0.0;

    double prosScore = 0.0;
    PriorityLevel prio = PriorityLevel::LOW;

    bool oreKnown = false;
    std::string oreTag;

    bool markFlag = false;
    int clusterNo = -1;

public:
    Zone();
    Zone(int n, int r, int c, double x, double y);

    int getId() const { return zoneNo; }
    int getRow() const { return gridR; }
    int getCol() const { return gridC; }

    double getX() const { return coordX; }
    double getY() const { return coordY; }

    double getRawG() const { return geoRaw; }
    double getRawR() const { return radRaw; }
    double getRawS() const { return seisRaw; }
    double getRawT() const { return terrRaw; }

    double getG() const { return geoN; }
    double getR() const { return radN; }
    double getS() const { return seisN; }
    double getT() const { return terrN; }

    double getProspectivityScore() const { return prosScore; }
    PriorityLevel getPriority() const { return prio; }
    std::string getPriorityString() const;

    bool hasKnownOccurrence() const { return oreKnown; }
    const std::string& getOccurrenceName() const { return oreTag; }

    bool isBookmarked() const { return markFlag; }
    int getClusterId() const { return clusterNo; }

    void setId(int n) { zoneNo = n; }

    void setGridPosition(int r, int c)
    {
        gridR = r;
        gridC = c;
    }

    void setCoordinates(double x, double y)
    {
        coordX = x;
        coordY = y;
    }

    void setRawIndicators(double g, double r, double s, double t);
    void setNormalizedIndicators(double g, double r, double s, double t);

    void setG(double v);
    void setR(double v);
    void setS(double v);
    void setT(double v);

    void setProspectivityScore(double v)
    {
        prosScore = v;
    }

    void setPriority(PriorityLevel p)
    {
        prio = p;
    }

    void setKnownOccurrence(bool v, const std::string& name = "")
    {
        oreKnown = v;
        oreTag = name;
    }

    void setBookmarked(bool v)
    {
        markFlag = v;
    }

    void setClusterId(int n)
    {
        clusterNo = n;
    }

    std::string getFormattedCoordinates() const;
    std::string toString() const;

    bool operator<(const Zone& z) const
    {
        return prosScore < z.prosScore;
    }
};

}

#endif