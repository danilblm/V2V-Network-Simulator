cmake_minimum_required(VERSION 3.16)
project(V2VProject LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)

# --- Trouver les modules Qt nécessaires
find_package(Qt6 REQUIRED COMPONENTS Core Gui Qml Quick Positioning Location)

# --- Cibles de ton exécutable
qt_add_executable(appV2VProject
    src/main.cpp

    src/controllers/MapController.cpp
    src/controllers/MapController.h
    src/controllers/SimulationController.h
    src/controllers/SimulationController.cpp

    src/models/GraphModel.cpp
    src/models/GraphModel.h
    src/models/VehicleModel.cpp
    src/models/VehicleModel.h
    src/models/InterferenceGraphModel.h
    src/models/InterferenceGraphModel.cpp
)

# --- Déclaration du module QML
qt_add_qml_module(appV2VProject
    URI V2VProject
    VERSION 1.0
    QML_FILES
        src/MainUI.qml
        src/views/MapView.qml
        src/views/SimulationPanelView.qml
        src/views/InterferencePanelView.qml

)

# --- Lien avec les bibliothèques Qt
target_link_libraries(appV2VProject
    PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Qml
        Qt6::Quick
        Qt6::Positioning
        Qt6::Location
)

# --- Inclure le dossier src pour les headers
target_include_directories(appV2VProject PRIVATE src src/controllers)
