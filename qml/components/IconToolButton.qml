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
        color: root.down ? "#d8dee680" : (root.hovered ? "#e7edf380" : "#eef2f680")
        border.color: root.hovered ? "#b1bcc880" : "#c7d0da80"
        border.width: 1
    }

    ToolTip.visible: root.hovered
    ToolTip.text: root.tooltipText
}
