import QtQuick

// 亚克力工具按钮（g3 次级材质）：比 AcrylicPanel 轻，不带螺丝。
// 悬停浮起 1–2px、按压回缩，动效参数见界面设计基准·第七节。
Item {
    id: btn

    property string text: ""
    property string glyph: ""          // 简单符号占位（图标系统在后续完善）
    property bool primary: false       // 主按钮：近黑底 + 米白字
    property bool danger: false
    property bool active: false
    property bool interactive: true

    signal clicked()

    implicitWidth: contentRow.width + 28
    implicitHeight: 36
    opacity: interactive ? 1.0 : 0.45

    readonly property bool hovered: hoverArea.containsMouse && interactive
    readonly property bool pressed: hoverArea.pressed && interactive

    y: (hovered && !pressed) ? -1.5 : 0
    Behavior on y {
        NumberAnimation { duration: AppTheme.durFast; easing.type: AppTheme.easingType }
    }

    // 底盘
    Rectangle {
        id: bg
        anchors.fill: parent
        radius: 4
        // 主按钮：纯黑块；次级：奶白半透明
        gradient: btn.primary ? null : gradientSecondary
        color: btn.primary ? (btn.pressed ? "#000000" : (btn.hovered ? Qt.darker(AppTheme.ink, 1.15) : AppTheme.ink))
                           : "transparent"
        border.width: btn.primary ? 0 : 1
        border.color: Qt.rgba(1, 1, 1, 0.65)
        clip: true

        Gradient {
            id: gradientSecondary
            GradientStop { position: 0.0; color: Qt.rgba(1, 1, 1, 0.70) }
            GradientStop { position: 1.0; color: Qt.rgba(1, 1, 1, 0.48) }
        }

        // 顶缘高光（玻璃厚度感）
        Rectangle {
            anchors { left: parent.left; right: parent.right; top: parent.top }
            height: 1
            color: Qt.rgba(1, 1, 1, 0.9)
            visible: !btn.primary
        }
    }
    // 悬停/主按钮：投影
    Rectangle {
        anchors { fill: parent; topMargin: 4 }
        radius: 4
        color: Qt.rgba(28 / 255, 25 / 255, 18 / 255, btn.hovered || btn.primary ? 0.18 : 0)
        z: -1
        Behavior on color { ColorAnimation { duration: AppTheme.durFast } }
    }
    // 激活（当前工具）标记：琥珀小横条
    Rectangle {
        visible: btn.active
        anchors { horizontalCenter: parent.horizontalCenter; bottom: parent.bottom; bottomMargin: 3 }
        width: 14
        height: 2
        color: AppTheme.signal
    }

    Row {
        id: contentRow
        anchors.centerIn: parent
        spacing: 6
        Text {
            visible: btn.glyph.length > 0
            text: btn.glyph
            color: btn.primary ? "#f4f1ea" : (btn.danger && btn.hovered ? AppTheme.danger : AppTheme.ink2)
            font.pixelSize: 13
            anchors.verticalCenter: parent.verticalCenter
        }
        Text {
            visible: btn.text.length > 0
            text: btn.text
            color: btn.primary ? "#f4f1ea" : (btn.danger && btn.hovered ? AppTheme.danger : AppTheme.ink2)
            font.family: AppTheme.fontFamily
            font.pixelSize: 13
            font.bold: true
            anchors.verticalCenter: parent.verticalCenter
        }
    }

    MouseArea {
        id: hoverArea
        anchors.fill: parent
        hoverEnabled: true
        cursorShape: btn.interactive ? Qt.PointingHandCursor : Qt.ArrowCursor
        onClicked: if (btn.interactive) btn.clicked()
    }
}
