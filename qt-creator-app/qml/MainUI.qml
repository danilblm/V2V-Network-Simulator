import QtQuick
import QtQuick.Controls
import QtPositioning

ApplicationWindow {
    visible: true
    width: 1000
    height: 700
    title: "Carte de Mulhouse – API OSM"

    MapView {
        id: mapView
        anchors.fill: parent
        onMapLoaded: console.log("✅ Carte prête")
    }

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
}
