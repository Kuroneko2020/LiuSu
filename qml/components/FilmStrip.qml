import QtQuick
import QtQuick.Controls

Rectangle {
    id: filmStrip
    color: "#f1f1f1"
    radius: 8
    border.color: "#d2d2d2"
    readonly property int pageCount: appController.project.pageCount

    ListView {
        id: pageList
        anchors.fill: parent
        anchors.margins: 8
        orientation: ListView.Horizontal
        spacing: 8
        clip: true
        boundsBehavior: Flickable.StopAtBounds
        model: filmStrip.pageCount

        delegate: Rectangle {
            required property int index
            readonly property int pageIndex: index
            width: 90
            height: 100
            radius: 6
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
                        source: {
                            appController.pageThumbnailRevision
                            return appController.pageThumbnailSource(pageIndex)
                        }
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
}
