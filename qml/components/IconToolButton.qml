import QtQuick
import QtQuick.Controls

ToolButton {
    id: root
    property string tooltipText: ""

    implicitWidth: 30
    implicitHeight: 30
    padding: 6

    icon.width: 16
    icon.height: 16

    background: Rectangle {
        radius: 8
        color: root.down
            ? Qt.rgba(0.847, 0.871, 0.902, 0.50)
            : (root.hovered
                ? Qt.rgba(0.906, 0.929, 0.953, 0.50)
                : Qt.rgba(0.933, 0.949, 0.965, 0.50))
        border.color: root.hovered
            ? Qt.rgba(0.694, 0.737, 0.784, 0.50)
            : Qt.rgba(0.780, 0.816, 0.855, 0.50)
        border.width: 1
    }

    ToolTip.visible: root.hovered
    ToolTip.text: root.tooltipText
}
