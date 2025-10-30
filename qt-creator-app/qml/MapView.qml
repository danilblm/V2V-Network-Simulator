import QtQuick
import QtQuick.Controls
import QtLocation
import QtPositioning

Item {
    id: mapContainer
    width: parent ? parent.width : 800
    height: parent ? parent.height : 600

    property real minLat: 47.70
    property real maxLat: 47.80
    property real minLon: 7.28
    property real maxLon: 7.40
    signal mapLoaded()

    Map {
        id: map
        anchors.fill: parent
        plugin: Plugin { name: "osm" }

        center: QtPositioning.coordinate(47.7508, 7.3359)
        zoomLevel: 14
        minimumZoomLevel: 12
        maximumZoomLevel: 18

        // 🔒 Limiter les déplacements dans la zone de Mulhouse
        onCenterChanged: {
            if (center.latitude < minLat) center.latitude = minLat
            if (center.latitude > maxLat) center.latitude = maxLat
            if (center.longitude < minLon) center.longitude = minLon
            if (center.longitude > maxLon) center.longitude = maxLon
        }

        // ✅ Déclenchement quand la carte est chargée
        Component.onCompleted: {
            console.log("🗺️ Carte initialisée")
            mapContainer.mapLoaded()
        }
    }

    // 🔄 Réception des routes depuis le contrôleur C++
    Connections {
        target: mapController
        onRoadReady: function(roads) {
            for (let i = 0; i < roads.length; ++i) {
                let path = roads[i]
                let polyline = Qt.createQmlObject(`
                    import QtLocation 6.5;
                    MapPolyline {
                        line.width: 2;
                        line.color: "blue";
                        path: path;
                    }
                `, map)
                map.addMapItem(polyline)
            }
        }
    }
    function zoomIn()  { map.zoomLevel = Math.min(map.zoomLevel + 1, map.maximumZoomLevel) }
    function zoomOut() { map.zoomLevel = Math.max(map.zoomLevel - 1, map.minimumZoomLevel) }
    function centerMulhouse (){map.center= QtPositioning.coordinate(47.7508, 7.3359) ;map.zoomLevel= 14 }

}
