import QtQuick
import LiuSu

Item {
    id: canvas
    signal importRequested()
    signal addPagesRequested()
    signal slotOpenRequested(int slotIndex)
    signal filesDropped(var urls)
    readonly property real pageRatio: app.pageWidthMm / app.pageHeightMm
    Row {
        x: 2; y: 3; spacing: 10
        Rectangle { width: 5; height: 5; color: AppTheme.signal; anchors.verticalCenter: parent.verticalCenter }
        SectionLabel { text: app.pageLabel + " / " + app.currentLayoutName; color: AppTheme.ink; anchors.verticalCenter: parent.verticalCenter }
    }
    Row {
        anchors.right: parent.right; y: -8; spacing: 6
        StudioButton { objectName: "importPhotosButton"; text: qsTr("导入照片"); iconName: "import"; onClicked: canvas.importRequested() }
        StudioButton { objectName: "addPagesButton"; text: qsTr("添加页面"); iconName: "add"; quiet: true; onClicked: canvas.addPagesRequested() }
    }
    Rectangle { y: 45; width: parent.width; height: 1; color: AppTheme.line }
    Item {
        id: paper
        objectName: "pagePaper"
        width: Math.max(100, Math.min(canvas.width - 88, (canvas.height - 157) * canvas.pageRatio, 920))
        height: width / canvas.pageRatio
        x: (canvas.width-width)/2
        y: 66 + (canvas.height - 150-height)/2
        property real arrival: 1
        // 编辑面始终正视、位置固定；换页仅轻微更新图像明度，不变换命中区域。
        function arrive() { landing.restart() }
        Connections { target: app; function onCurrentPageChanged() { paper.arrive() } }
        Connections { target: canvas; function onVisibleChanged() { if(canvas.visible) paper.arrive() } }
        NumberAnimation { id: landing; target: paper; property: "arrival"; from: 0; to: 1; duration: 140; easing.type: Easing.OutCubic }
        GradientShadow { anchors.fill: parent; strength: 0.09; spread: 8; offsetY: 8; cornerRadius: 0 }
        Rectangle { x: 1; y: parent.height; width: parent.width-2; height: 2; color: "#c6cbbf" }
        Image {
            anchors.fill: parent
            source: app.previewUrl
            sourceSize.width: Math.ceil(paper.width * Screen.devicePixelRatio)
            sourceSize.height: Math.ceil(paper.height * Screen.devicePixelRatio)
            fillMode: Image.Stretch
            cache: false
            opacity: 0.96+paper.arrival*0.04
        }
        readonly property var selectedRect: app.selectedSlotRect
        Rectangle {
            visible: app.selectedSlot >= 0
            x: (paper.selectedRect.x || 0) * paper.width
            y: (paper.selectedRect.y || 0) * paper.height
            width: (paper.selectedRect.width || 0) * paper.width
            height: (paper.selectedRect.height || 0) * paper.height
            color: "transparent"; border.width: 2; border.color: AppTheme.signal
        }
        MouseArea {
            id: photoMouse
            anchors.fill: parent
            cursorShape: pressed && app.selectedSlotState.hasImage ? Qt.ClosedHandCursor : Qt.PointingHandCursor
            property point startPoint
            property real cropX: 0
            property real cropY: 0
            property real pendingX: 0
            property real pendingY: 0
            property bool dragged: false
            onPressed: (mouse) => {
                app.selectSlotAt(mouse.x/paper.width, mouse.y/paper.height)
                // 台面坐标始终稳定；开始取景时结束轻微换页明度反馈。
                startPoint = photoMouse.mapToItem(canvas,mouse.x,mouse.y)
                landing.stop()
                paper.arrival = 1
                cropX = app.selectedSlotState.cropX || 0
                cropY = app.selectedSlotState.cropY || 0
                dragged = false
            }
            onPositionChanged: (mouse) => {
                if (!pressed || !app.selectedSlotState.hasImage || !app.selectedSlotState.fill) return
                const point = photoMouse.mapToItem(canvas,mouse.x,mouse.y)
                if (!dragged && Math.abs(point.x-startPoint.x)+Math.abs(point.y-startPoint.y) < 4) return
                dragged = true
                pendingX = cropX - (point.x-startPoint.x)/Math.max(1,paper.selectedRect.width*paper.width)*2
                pendingY = cropY - (point.y-startPoint.y)/Math.max(1,paper.selectedRect.height*paper.height)*2
                if (!cropTimer.running) cropTimer.start()
            }
            onReleased: {
                if (dragged) { cropTimer.stop(); app.setCropOffset(app.selectedSlot,pendingX,pendingY) }
            }
            onCanceled: cropTimer.stop()
            onClicked: if (!dragged && app.selectedSlot >= 0 && !app.slotHasImage(app.selectedSlot)) canvas.slotOpenRequested(app.selectedSlot)
            onDoubleClicked: if (app.selectedSlot >= 0 && app.slotHasImage(app.selectedSlot)) canvas.slotOpenRequested(app.selectedSlot)
            Timer {
                id: cropTimer; interval: 32
                // 合并鼠标移动；松手强制落下最终值，不让高频输入重绘所有页面。
                onTriggered: app.setCropOffset(app.selectedSlot, photoMouse.pendingX, photoMouse.pendingY)
            }
        }
        DropArea {
            id: dropArea
            anchors.fill: parent
            onDropped: (drop) => {
                if (!drop.hasUrls) return
                if (drop.urls.length === 1) {
                    app.selectSlotAt(drop.x/paper.width, drop.y/paper.height)
                    if (app.selectedSlot >= 0) app.assignFileToSlot(app.selectedSlot,drop.urls[0])
                } else canvas.filesDropped(drop.urls)
                drop.acceptProposedAction()
            }
        }
        Rectangle { anchors.fill: parent; visible: dropArea.containsDrag; color: "#22ce993f"; border.width: 2; border.color: AppTheme.signal }
        SectionLabel { anchors.left: parent.left; anchors.bottom: parent.top; anchors.bottomMargin: 12; text: app.pageWidthMm + " × " + app.pageHeightMm + " MM"; font.pixelSize: 9 }
    }
    Text {
        anchors.horizontalCenter: parent.horizontalCenter; anchors.bottom: parent.bottom; anchors.bottomMargin: 12
        text: app.filledSlotCount === 0 ? qsTr("点击空格添加照片，或把照片拖到这里") : qsTr("选中照片调整 · 拖动照片取景 · 双击替换")
        color: AppTheme.ink2; font.family: AppTheme.fontFamily; font.pixelSize: 11
    }
    MouseArea { anchors.fill: parent; z: -1; onClicked: app.clearSelection() }
}
