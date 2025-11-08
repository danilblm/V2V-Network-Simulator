import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: sidebar
    width: 320
    radius: 10
    color: "#ffffffee"
    border.color: "#2c3e50"
    border.width: 2
    anchors.top: parent.top
    anchors.bottom: parent.bottom
    anchors.right: parent.right
    anchors.margins: 20

    ScrollView {
        id: scroll
        anchors.fill: parent
        clip: true
        anchors.margins: 8
        contentWidth: sidebar.width - 24  // ✅ empêche le contenu de dépasser

        ColumnLayout {
            id: layout
            width: scroll.contentWidth
            spacing: 12
            anchors.horizontalCenter: parent.horizontalCenter

            // 🧭 Titre principal
            Text {
                text: "🧭 Panneau de Contrôle"
                font.pixelSize: 18
                font.bold: true
                color: "#2c3e50"
                Layout.alignment: Qt.AlignHCenter
            }

            Rectangle { Layout.fillWidth: true; height: 2; color: "#95a5a6" }

            // 🗺️ MAP NAVIGATION
            GroupBox {
                title: "🗺️ Navigation de la Carte"
                Layout.fillWidth: true
                padding: 8  // ✅ réduit le padding interne pour compacter
                font.bold: true

                ColumnLayout {
                    anchors.fill: parent
                    spacing: 6

                    Button { text: "📂 Charger carte"; Layout.fillWidth: true; onClicked: mapController.loadOSMData("data/mulhouse.json") }
                    Button { text: "🎯 Centrer Mulhouse"; Layout.fillWidth: true; onClicked: mapView.centerMulhouse() }
                    Button { text: "🔍 Zoom +"; Layout.fillWidth: true; onClicked: mapView.zoomIn() }
                    Button { text: "🔎 Zoom −"; Layout.fillWidth: true; onClicked: mapView.zoomOut() }
                }
            }

            // 🚗 SIMULATION
            GroupBox {
                title: "🚗 Simulation V2V"
                Layout.fillWidth: true
                padding: 8

                ColumnLayout {
                    anchors.fill: parent
                    spacing: 6

                    Button {
                        text: simulationController.isRunning ? "⏸️ Pause" : "▶️ Démarrer"
                        Layout.fillWidth: true
                        background: Rectangle {
                            color: simulationController.isRunning ? "#e67e22" : "#27ae60"
                            radius: 5
                        }
                        contentItem: Text {
                            text: parent.text
                            color: "white"
                            font.bold: true
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }
                        onClicked: {
                            if (simulationController.isRunning)
                                simulationController.pauseSimulation()
                            else
                                simulationController.startSimulation()
                        }
                    }

                    Button { text: "🔄 Réinitialiser"; Layout.fillWidth: true; onClicked: simulationController.resetSimulation() }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 5
                        Button { text: "+100"; Layout.fillWidth: true; onClicked: simulationController.spawnVehicles(100) }
                        Button { text: "+500"; Layout.fillWidth: true; onClicked: simulationController.spawnVehicles(500) }
                    }

                    Button { text: "+2000 véhicules"; Layout.fillWidth: true; onClicked: simulationController.spawnVehicles(2000) }
                }
            }

            // ⚙️ VITESSE
            GroupBox {
                title: "⚙️ Vitesse (x" + simulationController.timeScale.toFixed(1) + ")"
                Layout.fillWidth: true
                padding: 8

                ColumnLayout {
                    anchors.fill: parent
                    spacing: 6

                    Slider {
                        id: speedSlider
                        Layout.fillWidth: true
                        from: 0.1
                        to: 10.0
                        value: 1.0
                        stepSize: 0.1
                        onValueChanged: simulationController.setTimeScale(value)
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 5
                        Button { text: "0.5x"; Layout.fillWidth: true; onClicked: speedSlider.value = 0.5 }
                        Button { text: "1x"; Layout.fillWidth: true; onClicked: speedSlider.value = 1.0 }
                        Button { text: "2x"; Layout.fillWidth: true; onClicked: speedSlider.value = 2.0 }
                        Button { text: "5x"; Layout.fillWidth: true; onClicked: speedSlider.value = 5.0 }
                    }
                }
            }

            // 📊 STATISTIQUES
            GroupBox {
                title: "📊 Statistiques"
                Layout.fillWidth: true
                padding: 8

                ColumnLayout {
                    anchors.fill: parent
                    spacing: 4

                    Text {
                        text: "État : " + (simulationController.isRunning ? "🟢 En cours" : "🔴 Arrêté")
                        font.pixelSize: 13
                        color: "#2c3e50"
                    }

                    Rectangle { Layout.fillWidth: true; height: 1; color: "#bdc3c7" }

                    Text {
                        text: "🛣️ Réseau routier"
                        font.bold: true
                        font.pixelSize: 12
                        color: "#2c3e50"
                    }

                    Text {
                        id: roadStatsText
                        text: "Routes carrossables uniquement"
                        font.pixelSize: 11
                        color: "#7f8c8d"
                        wrapMode: Text.WordWrap
                        Layout.fillWidth: true
                    }

                    Connections {
                        target: mapController
                        function onRoadReady() {
                            try {
                                let stats = simulationController.getRoadTypeStats()
                                let types = []
                                for (let type in stats)
                                    types.push(type + " (" + stats[type] + ")")
                                if (types.length > 0)
                                    roadStatsText.text = types.slice(0, 3).join(", ")
                            } catch (e) {
                                console.log("Erreur stats:", e)
                            }
                        }
                    }
                }
            }

            Item { Layout.fillHeight: true }
        }
    }
}
