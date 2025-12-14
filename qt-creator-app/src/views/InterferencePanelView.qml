import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: interferencePanel
    width: 320
    height: 600
    radius: 10
    color: "#ffffffee"
    border.color: "#16a085"
    border.width: 2

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 15
        spacing: 12

        // Titre
        Text {
            text: "📡 Graphe d'Interférences V2V"
            font.pixelSize: 18
            font.bold: true
            color: "#16a085"
            Layout.alignment: Qt.AlignHCenter
        }

        Rectangle {
            Layout.fillWidth: true
            height: 2
            color: "#1abc9c"
        }

        // Statistiques en temps réel
        GroupBox {
            title: "📊 Statistiques Réseau"
            Layout.fillWidth: true

            ColumnLayout {
                anchors.fill: parent
                spacing: 6

                Text {
                    text: "Arêtes directionnelles: " +
                          (simulationController.interferenceGraph ?
                           simulationController.interferenceGraph.directedEdgeCount : 0)
                    font.pixelSize: 13
                    color: "#2c3e50"
                }

                Text {
                    text: "Connexions potentielles: " +
                          (simulationController.interferenceGraph ?
                           simulationController.interferenceGraph.potentialConnectionCount : 0)
                    font.pixelSize: 13
                    color: "#3498db"
                }

                Text {
                    text: "Véhicules isolés: " +
                          (simulationController.interferenceGraph ?
                           simulationController.interferenceGraph.isolatedVehicleCount : 0)
                    font.pixelSize: 13
                    color: "#e74c3c"
                }

                Rectangle {
                    Layout.fillWidth: true
                    height: 1
                    color: "#bdc3c7"
                }

                Text {
                    id: avgDegreeText
                    text: "Degré sortant moyen: --"
                    font.pixelSize: 12
                    color: "#7f8c8d"
                }

                Text {
                    id: avgDistanceText
                    text: "Distance moyenne: --"
                    font.pixelSize: 12
                    color: "#7f8c8d"
                }
            }
        }

        // Paramètres de transmission
        GroupBox {
            title: "📶 Portée de transmission"
            Layout.fillWidth: true

            ColumnLayout {
                anchors.fill: parent
                spacing: 8

                Text {
                    text: "Plage: 100m - 500m"
                    font.pixelSize: 11
                    color: "#7f8c8d"
                }

                Text {
                    text: "Rayons colorés aléatoirement par véhicule"
                    font.pixelSize: 10
                    color: "#95a5a6"
                    wrapMode: Text.WordWrap
                    Layout.fillWidth: true
                }
            }
        }

        // Liste des arêtes récentes
        GroupBox {
            title: "🔗 Arêtes directionnelles"
            Layout.fillWidth: true
            Layout.fillHeight: true

            ScrollView {
                anchors.fill: parent
                clip: true

                ListView {
                    id: connectionsList
                    model: ListModel { id: connectionsModel }
                    spacing: 4

                    delegate: Rectangle {
                        width: connectionsList.width - 10
                        height: 50
                        radius: 4
                        color: "#ecf0f1"
                        border.color: "#bdc3c7"
                        border.width: 1

                        ColumnLayout {
                            anchors.fill: parent
                            anchors.margins: 6
                            spacing: 2

                            Text {
                                text: "🚗 " + model.fromVehicle + " → 🚗 " + model.toVehicle
                                font.pixelSize: 11
                                font.bold: true
                                color: "#2c3e50"
                            }

                            RowLayout {
                                spacing: 8

                                Text {
                                    text: "📏 " + model.distance
                                    font.pixelSize: 10
                                    color: "#7f8c8d"
                                }

                                Text {
                                    text: "📶 " + model.signalStrength
                                    font.pixelSize: 10
                                    color: model.signalStrength.startsWith("8") ||
                                          model.signalStrength.startsWith("9") ?
                                          "#27ae60" : "#e67e22"
                                }
                            }
                        }
                    }
                }
            }
        }

        // Légende
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 80
            color: "#f8f9fa"
            radius: 5
            border.color: "#dee2e6"
            border.width: 1

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 8
                spacing: 4

                Text {
                    text: "ℹ️ Informations"
                    font.bold: true
                    font.pixelSize: 11
                    color: "#2c3e50"
                }

                Text {
                    text: "• Arêtes: A→B si B dans rayon de A"
                    font.pixelSize: 9
                    color: "#6c757d"
                }

                Text {
                    text: "• Cercles colorés = rayons de transmission"
                    font.pixelSize: 9
                    color: "#6c757d"
                }

                Text {
                    text: "• Signal fort: >80%, Signal faible: <80%"
                    font.pixelSize: 9
                    color: "#6c757d"
                }
            }
        }
    }

    // Timer pour mettre à jour les statistiques détaillées
    Timer {
        interval: 500
        running: simulationController.isRunning
        repeat: true
        onTriggered: {
            if (simulationController.interferenceGraph) {
                var stats = simulationController.interferenceGraph.getStatistics()
                avgDegreeText.text = "Degré sortant moyen: " + stats.averageOutDegree
                avgDistanceText.text = "Distance moyenne: " + stats.averageDistance
            }
        }
    }

    // Timer pour afficher les arêtes
    Timer {
        interval: 1000
        running: simulationController.isRunning
        repeat: true
        onTriggered: {
            if (simulationController.interferenceGraph) {
                var edges = simulationController.interferenceGraph.getDirectedEdgesForVisualization()

                connectionsModel.clear()

                // Afficher les 10 premières arêtes
                var maxDisplay = Math.min(10, edges.length)
                for (var i = 0; i < maxDisplay; i++) {
                    connectionsModel.append(edges[i])
                }
            }
        }
    }

    // Connexions aux signaux
    Connections {
        target: simulationController.interferenceGraph

        function onNewDirectedEdgeEstablished(fromVehicleId, toVehicleId, distance) {
            console.log("🔗 Nouvelle arête:", fromVehicleId, "→", toVehicleId,
                       "(" + Math.round(distance) + "m)")
        }

        function onDirectedEdgeLost(fromVehicleId, toVehicleId) {
            console.log("❌ Arête perdue:", fromVehicleId, "→", toVehicleId)
        }

        function onNewPotentialConnection(vehicleId1, vehicleId2) {
            console.log("💡 Connexion potentielle:", vehicleId1, "↔", vehicleId2)
        }
    }
}
