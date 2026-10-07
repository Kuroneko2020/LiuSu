import QtQuick
import QtQuick.Controls

// 应用主窗口：渲染管线可视化预览。
// 只显示渲染结果，不承载任何页面几何；正式界面按 J 稿在 05 子方案实现。
ApplicationWindow {
    id: root

    width: 1080
    height: 720
    visible: true
    title: qsTr("留素 · 渲染预览")
    color: "#edeae4"

    Image {
        anchors.centerIn: parent
        source: preview.previewUrl
        fillMode: Image.PreserveAspectFit
        // 观感上给"相纸"一点厚度
        Rectangle {
            z: -1
            anchors.fill: parent
            anchors.topMargin: 6
            color: "#d9d4c8"
        }
    }

    Label {
        anchors { top: parent.top; topMargin: 18; horizontalCenter: parent.horizontalCenter }
        text: qsTr("PG-001 · 四宫格 · 148 × 100 MM · 预览 144 PPI")
        font.pixelSize: 12
        color: "#6f6a60"
    }
}
