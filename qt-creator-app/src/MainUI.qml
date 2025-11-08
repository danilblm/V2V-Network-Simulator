import QtQuick
import QtQuick.Controls
import QtPositioning
import "views"

ApplicationWindow {
    visible: true
    width: 1600
    height: 900
    title: "Simulation V2V avec Graphe d'Interférences – Mulhouse"

    // ✅ Carte principale
    MapView {
        id: mapView
        anchors.fill: parent
        onMapLoaded: {
            console.log("✅ Carte prête")
            mapController.loadOSMData("data/mulhouse.json")
        }
    }

    // ✅ Barre latérale unique à droite (contient tout : carte, simulation, stats)
    Sidebar {
        id: sidebar
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.right: parent.right
    }

    // ✅ Panneau de performance (au centre haut)
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

    // ✅ (Optionnel) panneau du graphe d’interférences si tu veux le garder en bas à droite
    InterferencePanelView {
        id: interferencePanel
        anchors.left: parent.left       // ✅ positionné à gauche
        anchors.bottom: parent.bottom   // ✅ en bas
        anchors.margins: 20             // ✅ petite marge tout autour
    }

}
