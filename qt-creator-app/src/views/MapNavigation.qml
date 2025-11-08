import QtQuick
import QtLocation
import QtPositioning
import "views"

// ✅ Composant réutilisable : gestion navigation souris pour une Map
Item {
    id: navigation
    required property Map map  // la carte à contrôler

    MouseArea {
        anchors.fill: parent
        property real lastX: 0
        property real lastY: 0

        cursorShape: Qt.OpenHandCursor

        onPressed: (mouse) => {
            cursorShape = Qt.ClosedHandCursor
            lastX = mouse.x
            lastY = mouse.y
            mouse.accepted = true
        }

        onReleased: (mouse) => {
            cursorShape = Qt.OpenHandCursor
        }

        onPositionChanged: (mouse) => {
            if (mouse.buttons & Qt.LeftButton) {
                var dx = mouse.x - lastX
                var dy = mouse.y - lastY

                // conversion pixel → degrés (approximatif)
                var latPerPixel = 0.00005 * Math.pow(2, 14 - map.zoomLevel)
                var lonPerPixel = 0.00007 * Math.pow(2, 14 - map.zoomLevel)

                map.center.latitude -= dy * latPerPixel
                map.center.longitude -= dx * lonPerPixel

                lastX = mouse.x
                lastY = mouse.y
            }
        }

        onWheel: (wheel) => {
            if (wheel.angleDelta.y > 0)
                map.zoomLevel = Math.min(map.zoomLevel + 0.5, map.maximumZoomLevel)
            else
                map.zoomLevel = Math.max(map.zoomLevel - 0.5, map.minimumZoomLevel)
        }

        onDoubleClicked: (mouse) => {
            map.zoomLevel = Math.min(map.zoomLevel + 1, map.maximumZoomLevel)
        }
    }
}
