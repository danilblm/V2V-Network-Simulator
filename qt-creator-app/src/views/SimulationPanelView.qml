import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: simulationPanel
    width: 280
    height: 500
    radius: 10
    color: "#ffffffee"
    border.color: "#2c3e50"
    border.width: 2

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 15
        spacing: 12

        // Titre
        Text {
            text: "🚗 Simulation V2V"
            font.pixelSize: 18
            font.bold: true
            color: "#2c3e50"
            Layout.alignment: Qt.AlignHCenter
        }

        Rectangle {
            Layout.fillWidth: true
            height: 2
            color: "#95a5a6"
        }

        // Contrôles de base
        GroupBox {
            title: "Contrôles"
            Layout.fillWidth: true

            ColumnLayout {
                anchors.fill: parent
                spacing: 8

                Button {
                    text: simulationController.isRunning ? "⏸️ Pause" : "▶️ Démarrer"
                    Layout.fillWidth: true
                    onClicked: {
                        if (simulationController.isRunning) {
                            simulationController.pauseSimulation()
                        } else {
                            simulationController.startSimulation()
                        }
                    }
                }

                Button {
                    text: "🔄 Réinitialiser"
                    Layout.fillWidth: true
                    onClicked: simulationController.resetSimulation()
                }
            }
        }

        // Gestion des véhicules
        GroupBox {
            title: "Véhicules (" + simulationController.vehicleCount + ")"
            Layout.fillWidth: true

            ColumnLayout {
                anchors.fill: parent
                spacing: 8

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 5

                    Button {
                        text: "+ 100"
                        Layout.fillWidth: true
                        onClicked: simulationController.spawnVehicles(100)
                    }

                    Button {
                        text: "+ 500"
                        Layout.fillWidth: true
                        onClicked: simulationController.spawnVehicles(500)
                    }
                }

                Button {
                    text: "+ 2000 véhicules"
                    Layout.fillWidth: true
                    highlighted: true
                    onClicked: simulationController.spawnVehicles(2000)
                }
            }
        }

        // Échelle de temps
        GroupBox {
            title: "Vitesse (x" + simulationController.timeScale.toFixed(1) + ")"
            Layout.fillWidth: true

            ColumnLayout {
                anchors.fill: parent
                spacing: 8

                Slider {
                    id: speedSlider
                    Layout.fillWidth: true
                    from: 0.1
                    to: 10.0
                    value: 1.0
                    stepSize: 0.1

                    onValueChanged: {
                        simulationController.setTimeScale(value)
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 5

                    Button {
                        text: "0.5x"
                        Layout.fillWidth: true
                        onClicked: speedSlider.value = 0.5
                    }

                    Button {
                        text: "1x"
                        Layout.fillWidth: true
                        onClicked: speedSlider.value = 1.0
                    }

                    Button {
                        text: "2x"
                        Layout.fillWidth: true
                        onClicked: speedSlider.value = 2.0
                    }

                    Button {
                        text: "5x"
                        Layout.fillWidth: true
                        onClicked: speedSlider.value = 5.0
                    }
                }
            }
        }

        // Statistiques
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 120
            color: "#ecf0f1"
            radius: 5

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 8
                spacing: 4

                Text {
                    text: "📊 Statistiques"
                    font.bold: true
                    color: "#2c3e50"
                }

                Text {
                    text: "État: " + (simulationController.isRunning ? "🟢 En cours" : "🔴 Arrêté")
                    font.pixelSize: 12
                    color: "#34495e"
                }

                Rectangle {
                    Layout.fillWidth: true
                    height: 1
                    color: "#bdc3c7"
                }

                Text {
                    text: "🛣️ Réseau routier"
                    font.bold: true
                    font.pixelSize: 11
                    color: "#2c3e50"
                }

                Text {
                    id: roadStatsText
                    text: "Routes carrossables uniquement"
                    font.pixelSize: 10
                    color: "#7f8c8d"
                    wrapMode: Text.WordWrap
                    Layout.fillWidth: true
                }
            }
        }

        Item {
            Layout.fillHeight: true
        }
    }

    // Mise à jour des stats au chargement
    Connections {
        target: mapController
        function onRoadReady() {
            try {
                let stats = simulationController.getRoadTypeStats()
                let types = []
                for (let type in stats) {
                    types.push(type + " (" + stats[type] + ")")
                }
                if (types.length > 0) {
                    roadStatsText.text = types.slice(0, 3).join(", ")
                }
            } catch (e) {
                console.log("Erreur stats:", e)
            }
        }
    }
}
