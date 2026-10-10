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
    property real hoverLiftAmount: AppTheme.hoverLift
    property bool showShadow: true
    property real glassOpacity: 1.0
    readonly property bool hovered: hoverArea.containsMouse

    default property alias contentData: contentHolder.data

    // 位移走 transform 不走 y：板常被 Row / Column / Grid 排位，绑 y 会把布局
    // 算好的位置覆盖成 0，整块板跳到容器原点。transform 不参与布局排位。
    transform: Translate {
        y: root.hoverLift && root.hovered ? -root.hoverLiftAmount : 0
        Behavior on y {
            NumberAnimation { duration: AppTheme.durBase; easing.type: AppTheme.easingType }
        }
    }

    // 渐变阴影：单层连续衰减，替代原先 3 层矩形叠加。
    // 抬起时影子留在地面（反向补偿）并随之变淡。
    GradientShadow {
        anchors.fill: parent
        z: -5
        visible: root.showShadow
        cornerRadius: root.cornerRadius
        strength: 0.13
        spread: 6
        offsetY: 5
        opacity: root.hoverLift && root.hovered ? 0.7 : 1.0
        Behavior on opacity { NumberAnimation { duration: AppTheme.durBase } }
        // 反向补偿：本体被 transform 抬起时影子要留在地面，故走相反的 Translate
        // 抵消（anchors.fill 会吃掉 y 绑定，只能用 transform）。
        transform: Translate {
            y: root.hoverLift && root.hovered ? root.hoverLiftAmount : 0
            Behavior on y {
                NumberAnimation { duration: AppTheme.durBase; easing.type: AppTheme.easingType }
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
