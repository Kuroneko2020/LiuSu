import QtQuick
import QtQuick.Controls

Rectangle {
    id: filmStrip
    color: "#f4f6f8"
    radius: 10
    border.color: "#d5dbe3"
    readonly property int pageCount: appController.project.pageCount

    ListView {
        id: pageList
        anchors.fill: parent
        anchors.margins: 8
        orientation: ListView.Horizontal
        spacing: 10
        clip: true
        boundsBehavior: Flickable.StopAtBounds
        model: filmStrip.pageCount

        delegate: Rectangle {
            required property int index
            readonly property int pageIndex: index
            width: 84
            height: 96
            radius: 8
            border.width: appController.project.currentPageIndex === pageIndex ? 2 : 1
            border.color: appController.project.currentPageIndex === pageIndex ? "#8a97a8" : "#c9d0d9"
            color: "#ffffff"
            clip: true

            Rectangle {
                anchors.fill: parent
                anchors.margins: 4
                radius: 6
                color: "#eef2f6"
                clip: true

                Image {
                    height: parent.height
                    width: Math.min(parent.width, height * appController.pageAspectRatio)
                    anchors.centerIn: parent
                    source: {
                        appController.thumbnailListRevision
                        appController.pageThumbnailRevisionAt(pageIndex)
                        return appController.pageThumbnailSource(pageIndex)
                    }
                    fillMode: Image.PreserveAspectFit
                }
            }

            MouseArea {
                id: hoverArea
                anchors.fill: parent
                hoverEnabled: true
                onClicked: appController.project.switchToPage(pageIndex)
            }

            ToolTip.visible: hoverArea.containsMouse
            ToolTip.text: "第" + (pageIndex + 1) + "页"
        }
    }
}
