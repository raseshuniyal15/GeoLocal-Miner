# GeoLocal-Miner

A lightweight C++ desktop decision-support system for spatial mineral prospectivity scoring and exploration target prioritization.

Developed for Problem-Based Learning (PBL) Phase-I  
**Team ID:** T260 (Avengers) | **Institution:** Graphic Era Deemed to be University, Dehradun

---

## Important Notice

GeoLocal-Miner is an educational prototype and decision-support tool:
* A high prospectivity score or "High" priority classification **does not confirm or guarantee the presence of minerals**.
* High scores simply indicate that a spatial cell exhibits favorable indicators and should receive **higher priority for subsequent field geological mapping, geochemical sampling, or geophysical surveys**.
* The system is deliberately rule-based and transparent rather than using black-box machine learning models.

---

## Overview

In mineral exploration, conducting field surveys across broad geological terrains is time-consuming and expensive. Inspired by spatial discretization methods used in meteorological and geophysical modeling, GeoLocal-Miner represents an exploration area as a 2D spatial grid (e.g., $20 \times 20 = 400$ cells).

The system integrates four multi-criteria spatial indicators:
* **G (Geological suitability):** Favorable host lithology and rock types.
* **R (Radiometric suitability):** Gamma-ray / radioelement anomaly intensity.
* **S (Structural suitability):** Proximity to faults, shear zones, or lineament intersections.
* **T (Terrain suitability):** Geomorphological slope and drainage favorability.

Input values are normalized, evaluated using configurable multi-criteria scoring strategies, ranked using a custom priority queue, classified into High/Medium/Low priority tiers, and visualized on an interactive desktop heatmap.

---

## Key Features

* **Interactive Desktop GUI:** Zero-dependency desktop interface built with Dear ImGui and native OpenGL 2.0 (compiled into a single 1.1 MB executable).
* **Dynamic 2D Heatmap:** Color-graded prospectivity map with pan/zoom, layer toggles (Score, Category, G, R, S, T, Anomaly Clusters), cell selection, and hover tooltips.
* **Top-N Priority Ranking:** Real-time extraction of highest-priority exploration targets via a custom binary max-heap.
* **Exploration Target Trail:** Interactive itinerary manager backed by a doubly-linked list for field planning.
* **Spatial Neighborhood Graph:** 4-neighborhood / 8-neighborhood topological connectivity analysis and BFS connected anomaly cluster detection.
* **Ground-Truth Validation:** Validation against known benchmark mineral occurrences (analogous to AMD monazite placer deposits), calculating capture rates and exploration enrichment ratios.
* **CSV Import & Export:** Resilient parsing of spatial survey CSV files and export of ranked exploration targets for GIS software integration.

---

## PBL Implementation Details

### Data Structures

| Data Structure | Source File | Usage in Project |
| :--- | :--- | :--- |
| **2D Dynamic Array** | `src/data_structures/GridArray.h` | Contiguous row-major grid container with bounds checking for the 400-cell exploration grid. |
| **Binary Max-Heap** | `src/data_structures/PriorityQueue.h` | Custom heap implementation with sift-up and sift-down for $O(\log N)$ extraction of top-ranked exploration targets. |
| **Doubly-Linked List** | `src/data_structures/LinkedList.h` | Dynamic bidirectional list with full Rule-of-Five memory safety, powering the Exploration Target Trail. |
| **Spatial Graph** | `src/data_structures/Graph.h` | Adjacency list graph modeling cell spatial connectivity and Breadth-First Search (BFS) cluster identification. |

### Object-Oriented Programming (OOP)

* **Encapsulation:** The `Zone` class protects cell attributes (coordinates, raw/normalized values, score, priority, occurrence flag) with validated accessors.
* **Abstraction:** Domain engines (`ProspectivityEngine`, `PriorityEngine`, `ValidationEngine`) hide underlying data structure algorithms from the UI.
* **Inheritance & Polymorphism:** The abstract base class `ScoringStrategy` defines a polymorphic `computeScore()` interface implemented by three concrete strategies:
  1. `LinearWeightedScoringStrategy` (default weighted sum)
  2. `GeologicalGatedScoringStrategy` (mineral systems approach gating score by host lithology)
  3. `MultiplicativeLimitingScoringStrategy` (geometric product requiring all factors to coincide)
* **Composition:** `Grid` composes `GridArray<Zone>`, `DataLayer` instances, and `SpatialGraph`. The main `Application` composes the dashboard, heatmap, and inspector panels.

---

## Mathematical Formulation

### 1. Min-Max Normalization
Raw indicator values are normalized into the range $[0.0, 1.0]$:

$$\text{norm}(x) = \frac{x - x_{\min}}{x_{\max} - x_{\min}} \quad (\text{defaults safely to } 0.5 \text{ if } x_{\max} == x_{\min})$$

### 2. Linear Weighted Scoring (Default)
$$\text{Score} = w_G \cdot G + w_R \cdot R + w_S \cdot S + w_T \cdot T \quad \text{where} \quad \sum w_i = 1.0$$

*Default prototype weights:* $w_G = 0.35$, $w_R = 0.30$, $w_S = 0.20$, $w_T = 0.15$ (fully configurable in the GUI).

### 3. Priority Classification
* **HIGH:** $\text{Score} \ge 0.70$ (Immediate field investigation priority)
* **MEDIUM:** $0.40 \le \text{Score} < 0.70$ (Secondary interest)
* **LOW:** $\text{Score} < 0.40$ (Low priority)

---

## Project Structure

```
d:/pbl/
├── CMakeLists.txt              # CMake build configuration
├── Makefile                    # Standalone MinGW Makefile
├── build.bat                   # Automated build script
├── run.bat                     # Application launcher script
├── test.bat                    # Automated test runner script
├── README.md                   # Project documentation
├── bin/                        # Compiled executable binaries
│   ├── GeoLocalMiner.exe       # Desktop application & CLI
│   └── run_all_tests.exe       # Master test suite binary
├── src/
│   ├── main.cpp                # Application entry point (GUI + CLI)
│   ├── core/                   # Zone, Grid, DataLayer, and Engines
│   ├── data_structures/        # Custom GridArray, PriorityQueue, LinkedList, Graph
│   ├── ui/                     # Application, Dashboard, MapView, ZoneDetails
│   ├── data/                   # DataLoader and demo_dataset.csv
│   └── utils/                  # Normalization and FileUtils
├── tests/                      # 6 dedicated unit test suites
└── third_party/                # Embedded Dear ImGui and ImPlot headers
```

---

## How to Build and Run

### Prerequisites
* Windows 7/8/10/11
* MinGW GCC (C++14 support) or MSVC
* CMake 3.15+ (portable version included in `tools/`) or `mingw32-make`

### Quick Start (Batch Scripts)
```cmd
# 1. Build the project and all tests
build.bat

# 2. Run the test suite
test.bat

# 3. Launch the desktop GUI
run.bat
```

### Manual Build via CMake
```bash
cmake -B build -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
cd build && ctest --output-on-failure
```

### Command-Line Modes
`GeoLocalMiner.exe` also supports headless batch evaluation:
```cmd
# Show help
bin\GeoLocalMiner.exe --help

# Batch evaluate a CSV file and export ranked targets without opening the GUI
bin\GeoLocalMiner.exe --export src\data\demo_dataset.csv ranked_output.csv
```

---

## Demo Dataset & Validation

The project includes a realistic synthetic dataset (`src/data/demo_dataset.csv`):
* **Grid:** $20 \times 20$ (400 spatial cells, $10\text{ km} \times 10\text{ km}$ area).
* **Context:** Coastal heavy-mineral placer deposit analogue with 8 known occurrence benchmarks.
* **Validation Results:**
  * **High Priority Capture Rate:** 25.0% (2 of 8 known deposits).
  * **High + Medium Capture Rate:** 75.0% (6 of 8 known deposits).
  * **High Priority Footprint:** 1.75% of total land area (7 of 400 cells).
  * **Enrichment Ratio:** **14.29x** (known occurrences are 14.3 times more concentrated in high-priority zones than by random drilling).

---

## Unit Tests

The test suite covers all core modules:
* `test_normalization.cpp`: Min-max scaling and uniform field edge cases.
* `test_scoring.cpp`: Mathematical boundaries, weight validation, and polymorphic strategies.
* `test_ranking.cpp`: Threshold classification and max-heap extraction order.
* `test_data_structures.cpp`: 2D `GridArray`, custom `PriorityQueue`, `LinkedList` operations, and `SpatialGraph` BFS clusters.
* `test_validation.cpp`: Occurrence capture rates and enrichment calculations.
* `test_loader.cpp`: CSV ingestion, malformed row recovery, and target export.

Run `test.bat` to execute all tests (all 6 suites pass 100%).
