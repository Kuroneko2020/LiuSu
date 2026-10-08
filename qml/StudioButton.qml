import QtQuick
import QtQuick.Controls
import LiuSu

Button {
    id: control
    property string iconName: ""
    property bool primary: false
    property bool quiet: false
    property bool destructive: false
    property string hint: ""
    implicitWidth: caption.implicitWidth + (iconName.length ? 25 : 0) + 28
    implicitHeight: 38
    hoverEnabled: true
    font.family: AppTheme.fontFamily
    Accessible.name: text.length ? text : hint
    ToolTip.visible: hovered && hint.length > 0
    ToolTip.text: hint
    ToolTip.delay: 650
    opacity: enabled ? 1 : 0.38
    transform: Translate { y: control.down ? 2 : control.hovered ? -2 : 0; Behavior on y { NumberAnimation { duration: 140; easing.type: Easing.OutCubic } } }
    background: Rectangle {
        radius: 3
        color: control.primary ? (control.down ? "#080a09" : control.hovered ? "#343a34" : AppTheme.ink)
              : control.down ? "#e1dfd7" : control.hovered ? "#eae8e0" : control.quiet ? "transparent" : "#fafaf7"
        border.width: control.visualFocus || !control.quiet && !control.primary ? 1 : 0
        border.color: control.visualFocus ? AppTheme.signal : "#d8d9d1"
        Behavior on color { ColorAnimation { duration: 120 } }
    }
    contentItem: Item {
        Row {
            anchors.centerIn: parent
            spacing: 8
            LineIcon {
                visible: control.iconName.length > 0
                kind: control.iconName
                width: 17; height: 17
                color: control.primary ? "#f8f8f3" : control.destructive ? AppTheme.danger : AppTheme.ink
                anchors.verticalCenter: parent.verticalCenter
            }
            Text {
                id: caption
                text: control.text
                color: control.primary ? "#f8f8f3" : control.destructive ? AppTheme.danger : AppTheme.ink
                font.family: AppTheme.fontFamily
                font.pixelSize: 13
                font.weight: Font.Medium
                anchors.verticalCenter: parent.verticalCenter
            }
        }
    }
}
