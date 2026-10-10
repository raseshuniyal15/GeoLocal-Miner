#ifndef GEOLOCAL_MINER_APPLICATION_H
#define GEOLOCAL_MINER_APPLICATION_H

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <string>

#include "../core/Grid.h"
#include "../core/ProspectivityEngine.h"
#include "../core/PriorityEngine.h"
#include "../core/ValidationEngine.h"

#include "MapView.h"
#include "Dashboard.h"
#include "ZoneDetails.h"

namespace GeoLocalMiner {

class Application {
private:
    // Native window and rendering context
    HWND hWnd;
    HDC hDC;
    HGLRC hRC;

    int windowWidth;
    int windowHeight;
    std::string windowTitle;
    bool isRunning;

    // Core processing modules
    Grid grid;
    ProspectivityEngine prospectivityEngine;
    PriorityEngine priorityEngine;
    ValidationEngine validationEngine;

    // User interface components
    MapView mapView;
    Dashboard dashboard;
    ZoneDetails zoneDetails;

    void setupTheme();
    bool initWindow();
    bool initOpenGL();
    void cleanup();

public:
    Application();
    ~Application();

    bool initialize(
        int width = 1440,
        int height = 900,
        const char* title =
            "GeoLocal-Miner: Spatial Mineral Prospectivity Prioritization"
    );

    void run();

    // Handles native Windows messages.
    static LRESULT WINAPI WndProc(
        HWND hWnd,
        UINT msg,
        WPARAM wParam,
        LPARAM lParam
    );
};

} // namespace GeoLocalMiner

#endif // GEOLOCAL_MINER_APPLICATION_H