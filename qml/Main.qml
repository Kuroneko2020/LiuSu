import QtQuick
import LiuSu
import QtQuick.Controls

// 应用主窗口：亚克力展示板体系（J 稿）。
// 导航：主页 ⇄ 编辑；页面切换用克制的交叉过渡（界面设计基准·第七节）。
ApplicationWindow {
    id: window

    // 自适应屏幕可用区域：高 DPI 缩放下逻辑分辨率可能远小于物理分辨率，
    // 固定尺寸会导致窗口超出屏幕（本机 2560×1440 @175% → 逻辑约 1463×775）。
    // 留 24px 边距余量给任务栏与窗口边框。
    readonly property real availableWidth: Math.max(960, Screen.desktopAvailableWidth - 24)
    readonly property real availableHeight: Math.max(600, Screen.desktopAvailableHeight - 24)

    width: Math.min(1440, availableWidth)
    height: Math.min(900, availableHeight)
    minimumWidth: Math.min(1080, availableWidth)
    minimumHeight: Math.min(680, availableHeight)
    visible: true
    title: qsTr("留素")
    color: AppTheme.bgHi

    StackView {
        id: stack
        anchors.fill: parent

        initialItem: HomePage {
            onRequestEditor: stack.push(editorComponent)
        }

        Component {
            id: editorComponent
            EditorPage {
                onRequestHome: stack.pop()
            }
        }

        pushEnter: Transition {
            NumberAnimation { property: "opacity"; from: 0; to: 1; duration: AppTheme.durSlow; easing.type: AppTheme.easingType }
            NumberAnimation { property: "scale"; from: 0.985; to: 1; duration: AppTheme.durSlow; easing.type: AppTheme.easingType }
        }
        pushExit: Transition {
            NumberAnimation { property: "opacity"; from: 1; to: 0; duration: AppTheme.durBase; easing.type: AppTheme.easingType }
        }
        popEnter: Transition {
            NumberAnimation { property: "opacity"; from: 0; to: 1; duration: AppTheme.durSlow; easing.type: AppTheme.easingType }
        }
        popExit: Transition {
            NumberAnimation { property: "opacity"; from: 1; to: 0; duration: AppTheme.durBase; easing.type: AppTheme.easingType }
        }
    }
}
