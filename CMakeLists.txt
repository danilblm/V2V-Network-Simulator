cmake_minimum_required(VERSION 3.16)
project(V2VProject LANGUAGES CXX)
set(CMAKE_AUTOMOC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Qml Quick Positioning Location)

qt_add_executable(appV2VProject
    src/main.cpp
    src/controllers/MapController.cpp
    src/controllers/MapController.h
)

qt_add_qml_module(appV2VProject
    URI V2VProject
    VERSION 1.0
    QML_FILES
        qml/MainUI.qml
        qml/MapView.qml
    RESOURCES
        data/mulhouse.json
)

target_link_libraries(appV2VProject
    PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Qml
        Qt6::Quick
        Qt6::Positioning
        Qt6::Location
)
