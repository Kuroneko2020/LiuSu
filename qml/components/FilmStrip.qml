import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    color: "#f1f1f1"
    radius: 8
    border.color: "#d2d2d2"

    RowLayout {
        anchors.fill: parent
        anchors.margins: 8
        spacing: 8

        ListView {
            id: pagesView
            Layout.fillWidth: true
            Layout.fillHeight: true
            orientation: ListView.Horizontal
            spacing: 8
            model: appController.project.pageCount

            delegate: Rectangle {
                required property int index
                width: 80
                height: 95
                radius: 6
                border.width: appController.project.currentPageIndex === index ? 2 : 1
                border.color: appController.project.currentPageIndex === index ? "#4f7cff" : "#bbbbbb"
                color: appController.project.isPageValid(index) ? "#ffffff" : "#f8f8f8"

                Column {
                    anchors.centerIn: parent
                    spacing: 6
                    Text { text: "第" + (index + 1) + "页" }
                    Text { text: appController.project.isPageValid(index) ? "有效" : "空白"; font.pixelSize: 11 }
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: appController.project.switchToPage(index)
                }
            }
        }

        ColumnLayout {
            Layout.preferredWidth: 130
            Button {
                text: "新建空白页"
                Layout.fillWidth: true
                onClicked: appController.createBlankPage(2)
            }
            Button {
                text: "导出全队列"
                Layout.fillWidth: true
                onClicked: appController.exportQueue()
            }
        }
    }
}
