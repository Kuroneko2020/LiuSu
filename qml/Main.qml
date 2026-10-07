import QtQuick
import QtQuick.Controls

// 应用主窗口：亚克力展示板体系（J 稿）。
// 导航：主页 ⇄ 编辑；页面切换用克制的交叉过渡（界面设计基准·第七节）。
ApplicationWindow {
    id: window

    width: 1280
    height: 800
    minimumWidth: 1080
    minimumHeight: 700
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
