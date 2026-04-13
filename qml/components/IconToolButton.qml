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
        color: root.down ? "#d9dde2" : (root.hovered ? "#e6eaef" : "#eef1f4")
        border.color: root.hovered ? "#aeb6c0" : "#c6ccd4"
        border.width: 1
    }

    ToolTip.visible: root.hovered
    ToolTip.text: root.tooltipText
}
