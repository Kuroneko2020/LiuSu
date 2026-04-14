import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Frame {
    id: bar
    property int currentPage: 0
    signal navigate(int pageIndex)
    default property alias rightContent: rightRow.data

    Layout.fillWidth: true
    background: Rectangle {
        radius: 8
        color: "#ffffff"
        border.color: "#d6deea"
    }

    RowLayout {
        anchors.fill: parent
        spacing: 8

        Repeater {
            model: [
                { label: "首页", idx: 0, requireTask: false },
                { label: "编辑", idx: 1, requireTask: true },
                { label: "导出", idx: 2, requireTask: true },
                { label: "设置", idx: 3, requireTask: false }
            ]
            delegate: Button {
                required property var modelData
                text: modelData.label
                enabled: !modelData.requireTask || appController.project.pageCount > 0
                flat: true
                background: Rectangle {
                    radius: 6
                    color: bar.currentPage === modelData.idx ? "#e8eefc" : "transparent"
                    border.color: bar.currentPage === modelData.idx ? "#9bb5f3" : "transparent"
                }
                onClicked: bar.navigate(modelData.idx)
            }
        }

        Item { Layout.fillWidth: true }
        RowLayout { id: rightRow; spacing: 6 }
    }
}
