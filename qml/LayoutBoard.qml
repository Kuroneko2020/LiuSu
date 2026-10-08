import QtQuick
import LiuSu

// 主页布局展示板（J 稿 LY-01..04）：照片页衬在亚克力板后。
// 展示板内含：档案编号行 / 页面预览（真实预设几何） / 名称行；
// 选中时浮出操作按钮（手动排版 / 自动填充照片）。
Item {
    id: board

    property string code: ""
    property string name: ""
    property string nameEn: ""          // 英文副名（SINGLE / DUO / QUAD / GRID-9）
    property string subtitle: ""
    property string presetId: ""
    property var slotRects: []           // [{x,y,w,h}] 归一化，来自 AppController
    property bool selected: false

    signal activated()
    signal manualRequested()
    signal autoRequested()

    implicitWidth: 232
    implicitHeight: 312

    // 悬浮感：选中或悬停时整板抬升
    y: (selected || hoverArea.containsMouse) ? -6 : 0
    Behavior on y {
        NumberAnimation { duration: AppTheme.durBase; easing.type: AppTheme.easingType }
    }

    AcrylicPanel {
        anchors.fill: parent
        hoverLift: false
        // 选中：琥珀信号描边
        Rectangle {
            anchors.fill: parent
            anchors.margins: -2
            radius: 3
            color: "transparent"
            border.width: 2
            border.color: AppTheme.signal
            visible: board.selected
        }
    }

    // 档案编号行
    Text {
        id: codeText
        anchors { left: parent.left; top: parent.top; leftMargin: 16; topMargin: 14 }
        text: board.code
        color: board.selected ? AppTheme.signalDeep : AppTheme.ink
        font.family: AppTheme.fontFamily
        font.pixelSize: 11
        font.bold: true
        font.letterSpacing: 1.8
    }
    Text {
        anchors { left: codeText.right; leftMargin: 8; baseline: codeText.baseline }
        text: board.name + (board.nameEn.length > 0 ? " · " + board.nameEn : "")
        color: AppTheme.ink2
        font.family: AppTheme.fontFamily
        font.pixelSize: 11
        font.letterSpacing: 1.2
    }

    // 页面预览：按真实预设几何绘制槽位；照片衬在玻璃后（奶雾 + 反光）
    Item {
        id: pagePreview
        anchors.horizontalCenter: parent.horizontalCenter
        y: 50
        width: parent.width * 0.76
        height: width / 1.48

        // 相纸落影（预览下方偏移块，模拟纸张浮起）
        Rectangle {
            anchors { fill: parent; topMargin: 5; leftMargin: 2; rightMargin: -2; bottomMargin: -3 }
            z: -1
            color: Qt.rgba(28 / 255, 25 / 255, 18 / 255, 0.13)
        }
        Rectangle {
            anchors.fill: parent
            color: "#ffffff"
            border.width: 1
            border.color: Qt.rgba(28 / 255, 25 / 255, 18 / 255, 0.10)
        }
        Repeater {
            model: board.slotRects
            Rectangle {
                required property var modelData
                required property int index
                x: modelData.x * pagePreview.width
                y: modelData.y * pagePreview.height
                width: modelData.width * pagePreview.width
                height: modelData.height * pagePreview.height
                // 演示色轮换（仅为区分槽位，非真实照片）
                color: ["#cf9455", "#7f9a7d", "#79839a", "#b07d63",
                        "#b3a267", "#8ba391", "#95a0b4", "#ab8874", "#a29c7e"][index % 9]
            }
        }
        // 玻璃后奶雾：照片隔着一层亚克力看的柔和感（J 稿 .haze）
        Rectangle {
            anchors.fill: parent
            gradient: Gradient {
                GradientStop { position: 0.0; color: Qt.rgba(250 / 255, 250 / 255, 248 / 255, 0.26) }
                GradientStop { position: 0.5; color: Qt.rgba(250 / 255, 250 / 255, 248 / 255, 0.10) }
                GradientStop { position: 1.0; color: Qt.rgba(250 / 255, 250 / 255, 248 / 255, 0.22) }
            }
        }
        // 玻璃反光扫带（J 稿 .pane 的斜向高光）
        Rectangle {
            anchors.fill: parent
            gradient: Gradient {
                orientation: Gradient.Horizontal
                GradientStop { position: 0.0; color: Qt.rgba(1, 1, 1, 0) }
                GradientStop { position: 0.50; color: Qt.rgba(1, 1, 1, 0) }
                GradientStop { position: 0.63; color: Qt.rgba(1, 1, 1, 0.26) }
                GradientStop { position: 0.76; color: Qt.rgba(1, 1, 1, 0.04) }
                GradientStop { position: 0.90; color: Qt.rgba(1, 1, 1, 0) }
            }
        }
    }

    // 名称行
    Text {
        anchors { horizontalCenter: parent.horizontalCenter; bottom: subtitleText.top; bottomMargin: 3 }
        text: board.name
        color: AppTheme.ink
        font.family: AppTheme.fontFamily
        font.pixelSize: 14
        font.bold: true
        font.letterSpacing: 1.2
    }
    Text {
        id: subtitleText
        anchors { horizontalCenter: parent.horizontalCenter; bottom: parent.bottom; bottomMargin: 16 }
        text: board.subtitle
        color: AppTheme.ink3
        font.family: AppTheme.fontFamily
        font.pixelSize: 10
        font.letterSpacing: 1.6
    }

    // 选中态行动按钮（浮在板前）
    Row {
        anchors { horizontalCenter: parent.horizontalCenter; top: parent.top; topMargin: 62 }
        spacing: 8
        opacity: board.selected ? 1 : 0
        visible: opacity > 0
        Behavior on opacity {
            NumberAnimation { duration: AppTheme.durBase; easing.type: AppTheme.easingType }
        }

        Rectangle {
            width: 88
            height: 36
            color: manualArea.containsMouse ? AppTheme.ink : Qt.rgba(1, 1, 1, 0.95)
            border.width: 1
            border.color: AppTheme.ink
            Text {
                anchors.centerIn: parent
                text: qsTr("手动排版")
                color: manualArea.containsMouse ? "#f4f1ea" : AppTheme.ink
                font.family: AppTheme.fontFamily
                font.pixelSize: 12
                font.bold: true
            }
            MouseArea {
                id: manualArea
                anchors.fill: parent
                hoverEnabled: true
                cursorShape: Qt.PointingHandCursor
                onClicked: board.manualRequested()
            }
        }
        Rectangle {
            width: 118
            height: 36
            color: autoArea.containsMouse ? Qt.darker(AppTheme.ink, 1.15) : AppTheme.ink
            Row {
                anchors.centerIn: parent
                spacing: 7
                Rectangle { width: 6; height: 6; color: AppTheme.signal; anchors.verticalCenter: parent.verticalCenter }
                Text {
                    text: qsTr("自动填充照片")
                    color: "#f4f1ea"
                    font.family: AppTheme.fontFamily
                    font.pixelSize: 12
                    font.bold: true
                }
            }
            MouseArea {
                id: autoArea
                anchors.fill: parent
                hoverEnabled: true
                cursorShape: Qt.PointingHandCursor
                onClicked: board.autoRequested()
            }
        }
    }

    MouseArea {
        id: hoverArea
        anchors.fill: parent
        hoverEnabled: true
        cursorShape: Qt.PointingHandCursor
        z: -1
        onClicked: board.activated()
    }
}
