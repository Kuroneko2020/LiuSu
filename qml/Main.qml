import QtQuick
import QtQuick.Controls

// 应用主窗口骨架。只承载视觉占位，不承载任何页面几何或业务状态。
ApplicationWindow {
    id: root

    width: 1080
    height: 720
    visible: true
    title: qsTr("留素")
    color: "#ffffff"

    Label {
        anchors.centerIn: parent
        text: qsTr("留素 · 项目骨架就绪")
        font.pixelSize: 22
    }
}
