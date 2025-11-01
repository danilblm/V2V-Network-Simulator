import QtQuick
import QtQuick.Controls
import QtPositioning

import "views"

ApplicationWindow {
    visible: true
    width: 1600
    height: 900
    title: "Simulation V2V avec Graphe d'Interférences – Mulhouse"

    MapView {
        id: mapView
        anchors.fill: parent
        onMapLoaded: {
            console.log("✅ Carte prête")
            // Charger automatiquement la carte au démarrage
            mapController.loadOSMData("data/mulhouse.json")
        }
    }

    // Panneau de contrôle carte (haut droite)
    Rectangle {
        anchors.right: parent.right
        anchors.top: parent.top
        width: 240
        height: 180
        radius: 10
        color: "#ffffffdd"
        border.color: "#2c3e50"
        border.width: 1
        anchors.margins: 20

        Column {
            anchors.centerIn: parent
            spacing: 10

            Button {
                text: "Charger carte"
                onClicked: mapController.loadOSMData("data/mulhouse.json")
            }

            Button {
                text: "Centrer Mulhouse"
                onClicked: mapView.centerMulhouse()
            }

            Button {
                text: "Zoom +"
                onClicked: mapView.zoomIn()
            }

            Button {
                text: "Zoom −"
                onClicked: mapView.zoomOut()
            }
        }
    }

    // Panneau de simulation (gauche haut)
    SimulationPanelView {
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.margins: 20
    }

    // ✅ NOUVEAU: Panneau du graphe d'interférences (droite bas)
    InterferencePanelView {
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.margins: 20
    }

    // Indicateur de performance (haut centre)
    Rectangle {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top
        anchors.margins: 20
        width: 200
        height: 60
        radius: 8
        color: "#2c3e50dd"
        border.color: "#1abc9c"
        border.width: 2

        Column {
            anchors.centerIn: parent
            spacing: 4

            Text {
                text: "🚗 " + simulationController.vehicleCount + " véhicules"
                font.pixelSize: 14
                font.bold: true
                color: "#ecf0f1"
                anchors.horizontalCenter: parent.horizontalCenter
            }

            Text {
                text: "📡 " + (simulationController.interferenceGraph ?
                      simulationController.interferenceGraph.connectionCount : 0) +
                      " connexions V2V"
                font.pixelSize: 12
                color: "#1abc9c"
                anchors.horizontalCenter: parent.horizontalCenter
            }
        }
    }
}
