# V2V Network Simulator

> Interactive **Vehicle-to-Vehicle (V2V)** communication and mobility simulator built with **Qt 6 / QML and C++17**. Vehicles drive along a real road network imported from **OpenStreetMap** (Mulhouse, France) while the app computes and visualises their wireless interference graph in real time.

<p align="left">
  <img alt="C++" src="https://img.shields.io/badge/C%2B%2B-17-00599C?logo=cplusplus&logoColor=white" />
  <img alt="Qt" src="https://img.shields.io/badge/Qt-6-41CD52?logo=qt&logoColor=white" />
  <img alt="QML" src="https://img.shields.io/badge/QML-Quick-41CD52?logo=qt&logoColor=white" />
  <img alt="CMake" src="https://img.shields.io/badge/CMake-3.16%2B-064F8C?logo=cmake&logoColor=white" />
  <img alt="OpenStreetMap" src="https://img.shields.io/badge/Data-OpenStreetMap-7EBC6F?logo=openstreetmap&logoColor=white" />
</p>

---

## ✨ Overview

The simulator loads a city road graph, spawns hundreds to thousands of moving vehicles, and continuously evaluates which vehicles can talk to each other over a limited radio range. Each car is a node; an edge appears when one vehicle falls inside another's transmission range. The result is a live, pannable map of the V2V mesh as traffic flows through the city.

It is designed to stay smooth with **thousands of vehicles** thanks to a spatial-grid broad-phase that replaces the naïve O(n²) neighbour search with an O(n) one.

## 🚀 Features

- **Real road network** — imports OpenStreetMap data and keeps only drivable roads (motorway → residential/service), filtering out footways, cycleways, paths, steps, etc.
- **Realistic mobility** — vehicles travel edge-by-edge along the graph, pick the next road without doing U-turns, and adapt their speed to the road type.
- **Live V2V interference graph** — directed communication links (A reaches B within its range), potential connections (overlapping ranges), isolated vehicles, distance and signal-strength per link.
- **Scalable** — spawn **+100 / +500 / +2000** vehicles; a `SpatialGrid` (≈500 m cells) keeps neighbour queries near O(n) so the simulation stays interactive.
- **Interactive control panel** — load/center the map, zoom, start / pause / reset, adjust the time scale from **0.1× to 10×**, and read live statistics.
- **Map-coloured roads** — each road class is drawn with its own colour and width for readability.
- **Runs at ~30 Hz** with delta-time integration so motion is frame-rate independent.

## 🧱 Architecture

A clean **Model–View–Controller** split between native C++ logic and a QML front-end:

```
qt-creator-app/
├── CMakeLists.txt          # Qt6 + QML module setup
├── data/
│   └── mulhouse.json       # OpenStreetMap export (road geometry)
└── src/
    ├── main.cpp            # App bootstrap, exposes controllers to QML
    ├── controllers/
    │   ├── MapController    # Parses OSM JSON → road graph, road styling
    │   └── SimulationController  # Spawning, time stepping, V2V queries
    ├── models/
    │   ├── GraphModel       # Road graph (nodes / edges)
    │   ├── VehicleModel     # Per-vehicle position, speed, routing
    │   ├── InterferenceGraphModel  # Directed edges, signal strength
    │   └── SpatialGridModel # O(n) neighbour broad-phase
    └── views/               # QML UI (MapView, Sidebar, panels, vehicle item)
```

Controllers are registered as QML context properties (`mapController`, `simulationController`) and expose `Q_PROPERTY` / `Q_INVOKABLE` members, so the QML layer stays declarative while all the heavy computation runs in C++.

## 🛠 Tech stack

| Area | Technology |
| --- | --- |
| Language | C++17 |
| UI framework | Qt 6 (Quick / QML) |
| Modules | Qt Core, Gui, Qml, Quick, Positioning, Location |
| Build | CMake 3.16+ |
| Map data | OpenStreetMap (Overpass-style JSON export) |

## 📦 Build & run

### Prerequisites

- **Qt 6** (with the *Quick*, *Positioning* and *Location* modules)
- **CMake 3.16+** and a **C++17** compiler
- Qt Creator is the easiest way to open and run the project

### With Qt Creator

1. Open `qt-creator-app/CMakeLists.txt` as a project.
2. Select a Qt 6 kit and build.
3. Run — the OSM data is loaded automatically from `data/mulhouse.json`.

### From the command line

```bash
cd qt-creator-app
cmake -S . -B build -DCMAKE_PREFIX_PATH=/path/to/Qt/6.x.x/<compiler>
cmake --build build
```

> The app resolves `data/mulhouse.json` relative to the executable directory. If the map doesn't appear, launch the binary from the build folder (or use the **📂 Charger carte** button in the sidebar).

## 🎮 Usage

Use the control panel on the right:

| Control | Action |
| --- | --- |
| 📂 Charger carte / 🎯 Centrer | Load the OSM roads / re-center on Mulhouse |
| 🔍 Zoom + / − | Zoom the map |
| ▶️ Démarrer / ⏸️ Pause / 🔄 Réinitialiser | Run, pause or reset the simulation |
| +100 / +500 / +2000 | Spawn vehicles |
| Speed slider (0.1× – 10×) | Change the simulation time scale |

The stats panel shows the running state, vehicle count, live V2V connection count and the detected road types.

> **Note:** the OSM data shipped here covers the **Mulhouse** area. To simulate another city, replace `data/mulhouse.json` with an equivalent OpenStreetMap export and adjust the bounding box passed to `initializeSpatialGrid(...)` in `main.cpp`.

## 🤝 Contributing

Commit message and branch-naming conventions used in this project are documented in [CONTRIBUTING.md](CONTRIBUTING.md).
