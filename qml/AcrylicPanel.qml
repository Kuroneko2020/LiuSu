import QtQuick
import LiuSu

// 亚克力面板：干净的半透明面板（对照 RhineLabUI——UI 层是扁平的，厚度在内容不在控件）。
// 只做：微透底 + 细边框 + 极轻投影。不搞假玻璃高光/扫光/螺丝。
Item {
    id: root

    property real cornerRadius: 1
    property bool showScrews: false
    property bool showSheen: false
    property bool hoverLift: false
    property real hoverLiftAmount: 2
    property bool showShadow: true
    property real glassOpacity: 1.0
    readonly property bool hovered: hoverArea.containsMouse

    default property alias contentData: contentHolder.data

    y: hoverLift && hovered ? -hoverLiftAmount : 0
    Behavior on y {
        NumberAnimation { duration: AppTheme.durBase; easing.type: AppTheme.easingType }
    }

    // 极轻柔影（3 层，总透明度 < 0.12）
    Item {
        anchors.fill: parent
        anchors.margins: -6
        z: -5
        visible: root.showShadow
        opacity: root.hoverLift && root.hovered ? 0.7 : 1.0
        Behavior on opacity { NumberAnimation { duration: AppTheme.durBase } }
        y: root.hoverLift && root.hovered ? root.hoverLiftAmount : 0
        Repeater {
            model: [
                { yo: 2, ex: 0, a: 0.060 },
                { yo: 5, ex: 3, a: 0.040 },
                { yo: 9, ex: 7, a: 0.022 }
            ]
            delegate: Rectangle {
                required property var modelData
                x: -modelData.ex + 6
                y: modelData.yo + 6
                width: root.width + modelData.ex * 2
                height: root.height + modelData.ex * 2
                radius: root.cornerRadius + modelData.ex
                color: Qt.rgba(28/255, 25/255, 18/255, modelData.a)
            }
        }
    }

    // 面板本体：干净的微透底 + 1px 细边
    Rectangle {
        anchors.fill: parent
        radius: root.cornerRadius
        color: Qt.rgba(250/255, 249/255, 246/255, 0.82 * root.glassOpacity)
        border.width: 1
        border.color: Qt.rgba(28/255, 25/255, 18/255, 0.10)
    }

    // 内容
    Item {
        id: contentHolder
        anchors.fill: parent
        z: 1
    }

    MouseArea {
        id: hoverArea
        anchors.fill: parent
        hoverEnabled: root.hoverLift
        acceptedButtons: Qt.NoButton
    }
}
