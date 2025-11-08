import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ColumnLayout {
    id: root
    property alias title: header.text
    property alias content: contentItem.children
    property bool expanded: false

    Rectangle {
        id: headerRect
        Layout.fillWidth: true
        height: 36
        radius: 6
        color: "#3498db"
        border.color: "#2c3e50"

        Text {
            id: header
            anchors.centerIn: parent
            color: "white"
            font.bold: true
        }

        MouseArea {
            anchors.fill: parent
            cursorShape: Qt.PointingHandCursor
            onClicked: root.expanded = !root.expanded
        }
    }

    Item {
        id: contentItem
        Layout.fillWidth: true
        visible: root.expanded
        opacity: visible ? 1 : 0

        Behavior on opacity { NumberAnimation { duration: 200 } }
    }
}
