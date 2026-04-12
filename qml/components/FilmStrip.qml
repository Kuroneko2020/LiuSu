import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: filmStrip
    color: "#f1f1f1"
    radius: 8
    border.color: "#d2d2d2"
    readonly property int pageCount: appController.project.visiblePageCount
    readonly property int slotsRevision: appController.project.slotsRevision

    RowLayout {
        anchors.fill: parent
        anchors.margins: 8
        spacing: 8

        ListView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            orientation: ListView.Horizontal
            spacing: 8
            model: filmStrip.pageCount

            delegate: Rectangle {
                required property int index
                property int _rev: filmStrip.slotsRevision
                width: 90
                height: 100
                radius: 6
                property int pageIndex: {
                    _rev
                    return appController.project.visiblePageIndexAt(index)
                }
                border.width: appController.project.currentPageIndex === pageIndex ? 2 : 1
                border.color: appController.project.currentPageIndex === pageIndex ? "#4f7cff" : "#bbbbbb"
                color: "#ffffff"
                clip: true

                Column {
                    anchors.fill: parent
                    anchors.margins: 4
                    spacing: 2

                    Rectangle {
                        width: parent.width
                        height: 66
                        color: "#ececec"
                        clip: true

                        Image {
                            height: parent.height
                            width: Math.min(parent.width, height * appController.pageAspectRatio)
                            anchors.centerIn: parent
                            source: appController.pageThumbnailSource(pageIndex)
                            fillMode: Image.PreserveAspectFit
                        }
                    }

                    Label {
                        text: "第" + (pageIndex + 1) + "页"
                        font.pixelSize: 11
                    }
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: appController.project.switchToPage(pageIndex)
                }
            }
        }

        ColumnLayout {
            Layout.preferredWidth: 130
            Button {
                text: "新建空白页"
                Layout.fillWidth: true
                onClicked: appController.createBlankPage(appController.project.currentTemplateChoice())
            }
            Button {
                text: "导出全队列"
                Layout.fillWidth: true
                onClicked: appController.exportQueue()
            }
        }
    }
}
