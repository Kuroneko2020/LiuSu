import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: slotRoot
    radius: 8
    color: "transparent"
    border.width: selected ? 2 : 1
    clip: false

    property int slotIndex: -1
    property bool hasImage: false
    property bool selected: false
    property string imageSource: ""
    property bool fillCropMode: false
    property int rotationDegrees: 0
    property bool mirrored: false
    property real cropOffsetX: 0
    property real cropOffsetY: 0
    property real dragDeltaX: 0
    property real dragDeltaY: 0
    property bool swapTargetHighlighted: false

    signal addClicked()
    signal slotClicked()
    signal rotateClicked()
    signal mirrorClicked()
    signal toggleFillMode()
    signal swapRequested(int fromIndex, int toIndex)
    signal contentDragFinished(real dx, real dy)
    signal swapDragStarted(int fromIndex, point scenePos)
    signal swapDragMoved(point scenePos)
    signal swapDragFinished(point scenePos)

    border.color: swapTargetHighlighted ? "#ff8c00" : (selected ? "#4f7cff" : "#c5c5c5")

    Rectangle {
        id: contentViewport
        anchors.fill: parent
        radius: 6
        color: hasImage ? "#151515" : "#eef1f4"
        clip: true

        Image {
            id: photo
            readonly property real liveOffsetX: fillCropMode ? Math.max(-1.0, Math.min(1.0, cropOffsetX + dragDeltaX)) : 0
            readonly property real liveOffsetY: fillCropMode ? Math.max(-1.0, Math.min(1.0, cropOffsetY + dragDeltaY)) : 0
            readonly property real overflowX: Math.max(0, paintedWidth - width)
            readonly property real overflowY: Math.max(0, paintedHeight - height)
            width: parent.width
            height: parent.height
            source: imageSource
            visible: hasImage && source !== ""
            fillMode: fillCropMode ? Image.PreserveAspectCrop : Image.PreserveAspectFit
            horizontalAlignment: Image.AlignHCenter
            verticalAlignment: Image.AlignVCenter
            smooth: true
            cache: true
            rotation: rotationDegrees
            mirror: mirrored
            x: fillCropMode && overflowX > 0 ? (liveOffsetX * overflowX * 0.5) : 0
            y: fillCropMode && overflowY > 0 ? (liveOffsetY * overflowY * 0.5) : 0

            MouseArea {
                anchors.fill: parent
                enabled: hasImage && selected && fillCropMode
                property real lastX
                property real lastY
                property real totalDx
                property real totalDy
                onPressed: {
                    lastX = mouse.x
                    lastY = mouse.y
                    totalDx = 0
                    totalDy = 0
                    slotRoot.dragDeltaX = 0
                    slotRoot.dragDeltaY = 0
                }
                onPositionChanged: {
                    const dx = (mouse.x - lastX) / width
                    const dy = (mouse.y - lastY) / height
                    lastX = mouse.x
                    lastY = mouse.y
                    if (photo.overflowX > 0) {
                        totalDx = Math.max(-1.0 - cropOffsetX, Math.min(1.0 - cropOffsetX, totalDx + dx * 2.0))
                    } else {
                        totalDx = 0
                    }
                    if (photo.overflowY > 0) {
                        totalDy = Math.max(-1.0 - cropOffsetY, Math.min(1.0 - cropOffsetY, totalDy + dy * 2.0))
                    } else {
                        totalDy = 0
                    }
                    slotRoot.dragDeltaX = totalDx
                    slotRoot.dragDeltaY = totalDy
                }
                onReleased: {
                    slotRoot.contentDragFinished(slotRoot.dragDeltaX, slotRoot.dragDeltaY)
                    slotRoot.dragDeltaX = 0
                    slotRoot.dragDeltaY = 0
                }
                onCanceled: {
                    slotRoot.dragDeltaX = 0
                    slotRoot.dragDeltaY = 0
                }
            }
        }
    }

    TapHandler {
        acceptedButtons: Qt.LeftButton
        onTapped: slotRoot.slotClicked()
    }

    Label {
        anchors.centerIn: parent
        visible: !hasImage
        text: "+"
        font.pixelSize: 36
        color: "#4b4b4b"
    }

    Rectangle {
        anchors.fill: parent
        visible: hasImage && photo.status === Image.Error
        color: "#2a2a2a"
        Label {
            anchors.centerIn: parent
            text: "图片加载失败"
            color: "#ffb3b3"
        }
    }

    Button {
        visible: !hasImage
        anchors.centerIn: parent
        text: "导入"
        onClicked: slotRoot.addClicked()
    }

    Rectangle {
        id: swapHandle
        visible: hasImage
        width: 18
        height: 18
        radius: 9
        color: "#333"
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.margins: 6
        Text { anchors.centerIn: parent; text: "⇅"; color: "white"; font.pixelSize: 10 }

        MouseArea {
            anchors.fill: parent
            cursorShape: Qt.OpenHandCursor
            drag.target: null
            onPressed: (mouse) => {
                cursorShape = Qt.ClosedHandCursor
                const p = swapHandle.mapToItem(null, mouse.x, mouse.y)
                slotRoot.swapDragStarted(slotRoot.slotIndex, Qt.point(p.x, p.y))
            }
            onPositionChanged: (mouse) => {
                const p = swapHandle.mapToItem(null, mouse.x, mouse.y)
                slotRoot.swapDragMoved(Qt.point(p.x, p.y))
            }
            onReleased: (mouse) => {
                cursorShape = Qt.OpenHandCursor
                const p = swapHandle.mapToItem(null, mouse.x, mouse.y)
                slotRoot.swapDragFinished(Qt.point(p.x, p.y))
            }
        }
    }

    RowLayout {
        visible: selected && hasImage
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.margins: 4
        spacing: 4

        Button { Layout.fillWidth: true; text: "右转90°"; onClicked: slotRoot.rotateClicked() }
        Button { Layout.fillWidth: true; text: "镜像"; onClicked: slotRoot.mirrorClicked() }
        Button {
            Layout.fillWidth: true
            text: fillCropMode ? "切换为完整放入" : "切换为铺满裁切"
            onClicked: slotRoot.toggleFillMode()
        }
    }

    Rectangle {
        anchors.fill: parent
        color: "transparent"
        border.width: selected && fillCropMode ? 2 : 0
        border.color: "#57b3ff"
        radius: slotRoot.radius
    }
}
