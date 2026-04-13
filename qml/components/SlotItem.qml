import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: slotRoot
    radius: 0
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

    border.color: swapTargetHighlighted ? "#ff8c00" : (selected ? "#7f7f7f" : "#c5c5c5")

    Rectangle {
        id: contentViewport
        anchors.fill: parent
        radius: 0
        color: hasImage ? "#f4f4f4" : "#eef1f4"
        clip: true

        Item {
            id: contentLayer
            readonly property real sourceW: photo.sourceSize.width > 0 ? photo.sourceSize.width : contentViewport.width
            readonly property real sourceH: photo.sourceSize.height > 0 ? photo.sourceSize.height : contentViewport.height
            readonly property bool quarterTurn: Math.abs(rotationDegrees % 180) === 90
            readonly property real imageW: quarterTurn ? sourceH : sourceW
            readonly property real imageH: quarterTurn ? sourceW : sourceH
            readonly property real imageAspect: imageH > 0 ? imageW / imageH : 1
            readonly property real viewportAspect: contentViewport.height > 0 ? contentViewport.width / contentViewport.height : 1
            readonly property real fittedWidth: imageAspect >= viewportAspect ? contentViewport.width : contentViewport.height * imageAspect
            readonly property real fittedHeight: imageAspect >= viewportAspect ? contentViewport.width / imageAspect : contentViewport.height
            readonly property real croppedWidth: imageAspect >= viewportAspect ? contentViewport.height * imageAspect : contentViewport.width
            readonly property real croppedHeight: imageAspect >= viewportAspect ? contentViewport.height : contentViewport.width / imageAspect
            readonly property real layerWidth: fillCropMode ? croppedWidth : fittedWidth
            readonly property real layerHeight: fillCropMode ? croppedHeight : fittedHeight
            readonly property real overflowX: Math.max(0, layerWidth - contentViewport.width)
            readonly property real overflowY: Math.max(0, layerHeight - contentViewport.height)
            readonly property real liveOffsetX: fillCropMode && overflowX > 0 ? Math.max(-1.0, Math.min(1.0, cropOffsetX + dragDeltaX)) : 0
            readonly property real liveOffsetY: fillCropMode && overflowY > 0 ? Math.max(-1.0, Math.min(1.0, cropOffsetY + dragDeltaY)) : 0

            width: Math.max(1, layerWidth)
            height: Math.max(1, layerHeight)
            x: (contentViewport.width - width) * 0.5 + (fillCropMode ? liveOffsetX * overflowX * 0.5 : 0)
            y: (contentViewport.height - height) * 0.5 + (fillCropMode ? liveOffsetY * overflowY * 0.5 : 0)

            Image {
                id: photo
                anchors.fill: parent
                source: imageSource
                visible: hasImage && source !== ""
                fillMode: Image.Stretch
                smooth: true
                cache: true
                rotation: rotationDegrees
                mirror: mirrored
                transformOrigin: Item.Center
            }

            MouseArea {
                anchors.fill: contentViewport
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
                    const dx = (mouse.x - lastX) / Math.max(1, contentViewport.width)
                    const dy = (mouse.y - lastY) / Math.max(1, contentViewport.height)
                    lastX = mouse.x
                    lastY = mouse.y
                    totalDx = contentLayer.overflowX > 0
                        ? Math.max(-1.0 - cropOffsetX, Math.min(1.0 - cropOffsetX, totalDx + dx * 2.0))
                        : 0
                    totalDy = contentLayer.overflowY > 0
                        ? Math.max(-1.0 - cropOffsetY, Math.min(1.0 - cropOffsetY, totalDy + dy * 2.0))
                        : 0
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
        radius: 0
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

}
