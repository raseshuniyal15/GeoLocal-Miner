#ifndef GEOLOCAL_MINER_ZONE_DETAILS_H
#define GEOLOCAL_MINER_ZONE_DETAILS_H

#include "../core/Grid.h"
#include "../core/Zone.h"
#include "../data_structures/LinkedList.h"
#include <string>

namespace GeoLocalMiner {

class ZoneDetails {
private:
    LinkedList<Zone> targetTrail;
    std::string trailMsg;

public:
    ZoneDetails();

    LinkedList<Zone>& getTargetTrail() {
        return targetTrail;
    }

    const LinkedList<Zone>& getTargetTrail() const {
        return targetTrail;
    }

    void addTargetToTrail(const Zone& zone);
    void removeTargetFromTrail(size_t idx);
    void clearTargetTrail();

    // Draw the details panel for the selected grid cell.
    void render(Grid& grid, int& selectedRow, int& selectedCol);
};

} // namespace GeoLocalMiner

#endif
