import QtQuick
import QtQuick.Controls
import LiuSu

Item {
    id: queue
    signal addPagesRequested()
    signal deleteRequested()
    property int modelRevision: 0
    implicitHeight: 122
    Connections { target: app; function onChanged() { queue.modelRevision++ } }
    Rectangle { width: parent.width; height: 1; color: AppTheme.line }
    Row {
        y: 10; spacing: 9
        SectionLabel { text: "PAGES / 页队列"; color: AppTheme.ink }
        Text { text: app.pageCount + qsTr(" 页"); color: AppTheme.ink2; font.family: AppTheme.fontFamily; font.pixelSize: 10 }
    }
    StudioButton { anchors.right: parent.right; y: 1; height: 28; text: qsTr("删除当前页"); iconName: "trash"; quiet: true; destructive: true; enabled: app.pageCount > 1; onClicked: queue.deleteRequested() }
    ListView {
        id: pageList
        objectName: "pageQueue"
        anchors { top: parent.top; bottom: parent.bottom; left: parent.left; right: parent.right; topMargin: 35 }
        orientation: ListView.Horizontal
        spacing: 12; clip: true
        model: app.pageCount
        currentIndex: app.currentPageIndex
        highlightMoveDuration: 160
        onCurrentIndexChanged: positionViewAtIndex(currentIndex, ListView.Contain)
        ScrollBar.horizontal: ScrollBar { policy: ScrollBar.AsNeeded }
        delegate: Button {
            id: pageCard
            required property int index
            property var info: { queue.modelRevision; return app.pageInfo(index) }
            width: 105; height: 79; padding: 0
            Accessible.name: qsTr("第 %1 页，%2").arg(index+1).arg(info.name)
            onClicked: app.setCurrentPage(index)
            background: Rectangle { color: "transparent"; border.width: app.currentPageIndex === pageCard.index ? 2 : 1; border.color: app.currentPageIndex === pageCard.index ? AppTheme.signal : "#d3d7cd"; radius: 2 }
            contentItem: Item {
                Image {
                    x: 6; y: 5; width: parent.width-12; height: 48
                    source: { queue.modelRevision; return app.pageThumbnailUrl(pageCard.index) }
                    sourceSize.width: 190; sourceSize.height: 128; fillMode: Image.PreserveAspectFit; cache: false
                }
                Text { x: 7; y: 59; text: (pageCard.index+1).toString().padStart(2,"0") + " / " + pageCard.info.name; color: AppTheme.ink; font.family: AppTheme.fontFamily; font.pixelSize: 10 }
            }
        }
        footer: Item {
            width: 75; height: 79
            StudioButton { x: 12; width: 55; height: 55; iconName: "add"; hint: qsTr("添加页面，可一次创建多页"); onClicked: queue.addPagesRequested() }
        }
    }
}
