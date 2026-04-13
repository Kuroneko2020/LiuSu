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
        color: root.down ? "#d7dde480" : (root.hovered ? "#e5ebf180" : "#edf1f580")
        border.color: root.hovered ? "#acb6c280" : "#c3ccd780"
        border.width: 1
    }

    ToolTip.visible: root.hovered
    ToolTip.text: root.tooltipText
}
