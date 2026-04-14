import QtQuick
import QtQuick.Controls

Rectangle {
    id: filmStrip
    color: "#f5f7fa"
    radius: 8
    border.color: "#d7dde6"
    readonly property int pageCount: appController.project.pageCount

    ListView {
        id: pageList
        anchors.fill: parent
        anchors.margins: 6
        orientation: ListView.Horizontal
        spacing: 8
        clip: true
        boundsBehavior: Flickable.StopAtBounds
        interactive: true
        model: filmStrip.pageCount
        highlightMoveDuration: 120

        function ensureCurrentVisible() {
            positionViewAtIndex(appController.project.currentPageIndex, ListView.Contain)
        }

        Connections {
            target: appController.project
            function onCurrentPageChanged() {
                pageList.ensureCurrentVisible()
            }
        }

        delegate: Rectangle {
            required property int index
            readonly property int pageIndex: index
            readonly property bool active: appController.project.currentPageIndex === pageIndex
            width: Math.round(height * appController.pageAspectRatio)
            height: 62
            radius: 6
            border.width: 1
            border.color: active ? "#9ca9b8" : "#c9d1db"
            color: active ? "#f7f9fc" : "#ffffff"
            antialiasing: true

            Image {
                anchors.fill: parent
                anchors.margins: 2
                source: {
                    appController.thumbnailListRevision
                    appController.pageThumbnailRevisionAt(pageIndex)
                    return appController.pageThumbnailSource(pageIndex)
                }
                fillMode: Image.PreserveAspectFit
                cache: true
            }

            MouseArea {
                id: hoverArea
                anchors.fill: parent
                hoverEnabled: true
                preventStealing: false
                onClicked: appController.project.switchToPage(pageIndex)
            }

            ToolTip.visible: hoverArea.containsMouse
            ToolTip.text: "第" + (pageIndex + 1) + "页"
        }

        ScrollBar.horizontal: ScrollBar {
            policy: ScrollBar.AsNeeded
            opacity: 0.65
            contentItem: Rectangle {
                implicitHeight: 4
                radius: 2
                color: "#a8b3c2"
            }
            background: Rectangle {
                implicitHeight: 4
                radius: 2
                color: "#dbe2ea"
            }
        }
    }

    WheelHandler {
        target: pageList
        orientation: Qt.Horizontal
        onWheel: (event) => {
            pageList.contentX = Math.max(0, Math.min(pageList.contentWidth - pageList.width, pageList.contentX - event.angleDelta.y))
            event.accepted = true
        }
    }

}
