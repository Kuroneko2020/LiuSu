import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: slotRoot
    radius: 8
    color: hasImage ? "#111" : "#f0f0f0"
    border.width: selected ? 2 : 1
    clip: true

    property int slotIndex: -1
    property bool hasImage: false
    property bool selected: false
    property string imagePath: ""
    property bool fillCropMode: false
    property int rotationDegrees: 0
    property bool mirrored: false
    property real cropOffsetX: 0
    property real cropOffsetY: 0
    property bool dropHover: false

    signal addClicked()
    signal slotClicked()
    signal rotateClicked()
    signal mirrorClicked()
    signal toggleFillMode()
    signal swapRequested(int fromIndex, int toIndex)
    signal contentDragged(real dx, real dy)

    DropArea {
        anchors.fill: parent
        onEntered: slotRoot.dropHover = true
        onExited: slotRoot.dropHover = false
        onDropped: (drop) => {
            slotRoot.dropHover = false
            if (drop.source && drop.source.sourceSlot >= 0 && drop.source.sourceSlot !== slotRoot.slotIndex) {
                slotRoot.swapRequested(drop.source.sourceSlot, slotRoot.slotIndex)
            }
        }
    }
    border.color: dropHover ? "#ff8c00" : (selected ? "#4f7cff" : "#c5c5c5")

    Image {
        id: photo
        anchors.fill: parent
        anchors.margins: 2
        source: imagePath
        visible: hasImage && source !== ""
        fillMode: fillCropMode ? Image.Stretch : Image.PreserveAspectFit
        smooth: true
        cache: true
        readonly property real srcW: Math.max(1, sourceSize.width)
        readonly property real srcH: Math.max(1, sourceSize.height)
        readonly property real targetRatio: Math.max(0.001, width / Math.max(1, height))
        readonly property real srcRatio: srcW / srcH
        readonly property real cropW: srcRatio > targetRatio ? srcH * targetRatio : srcW
        readonly property real cropH: srcRatio > targetRatio ? srcH : (srcW / targetRatio)
        readonly property real rangeX: Math.max(0, srcW - cropW)
        readonly property real rangeY: Math.max(0, srcH - cropH)
        sourceClipRect: fillCropMode
                        ? Qt.rect((rangeX / 2) + (Math.max(-1, Math.min(1, cropOffsetX)) * rangeX * 0.5),
                                  (rangeY / 2) + (Math.max(-1, Math.min(1, cropOffsetY)) * rangeY * 0.5),
                                  cropW, cropH)
                        : Qt.rect(0, 0, srcW, srcH)

        transform: [
            Rotation {
                angle: slotRoot.rotationDegrees
                origin.x: photo.width / 2
                origin.y: photo.height / 2
            },
            Scale {
                xScale: slotRoot.mirrored ? -1 : 1
                yScale: 1
                origin.x: photo.width / 2
                origin.y: photo.height / 2
            }
        ]

        MouseArea {
            anchors.fill: parent
            enabled: hasImage && selected && fillCropMode
            property real lastX
            property real lastY
            onPressed: {
                lastX = mouse.x
                lastY = mouse.y
            }
            onPositionChanged: {
                const dx = (mouse.x - lastX) / width
                const dy = (mouse.y - lastY) / height
                lastX = mouse.x
                lastY = mouse.y
                slotRoot.contentDragged(dx * 2.0, dy * 2.0)
            }
        }
    }

    MouseArea {
        anchors.fill: parent
        onClicked: slotRoot.slotClicked()
    }

    Label {
        anchors.centerIn: parent
        visible: !hasImage
        text: "+"
        font.pixelSize: 36
        color: "#4b4b4b"
    }

    Button {
        visible: !hasImage
        anchors.centerIn: parent
        text: "导入"
        onClicked: slotRoot.addClicked()
    }

    Rectangle {
        id: swapHandle
        visible: hasImage && !(selected && fillCropMode)
        width: 18
        height: 18
        radius: 9
        color: "#333"
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.margins: 6
        Text { anchors.centerIn: parent; text: "⇅"; color: "white"; font.pixelSize: 10 }

        property int sourceSlot: -1

        Drag.active: dragHandler.active
        Drag.source: swapHandle
        Drag.hotSpot.x: width / 2
        Drag.hotSpot.y: height / 2

        DragHandler {
            id: dragHandler
            target: null
            xAxis.enabled: true
            yAxis.enabled: true
            onActiveChanged: {
                if (active) {
                    swapHandle.sourceSlot = slotRoot.slotIndex
                }
            }
        }

        TapHandler {
            acceptedButtons: Qt.LeftButton
            onTapped: {
                swapHandle.sourceSlot = slotRoot.slotIndex
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
            text: fillCropMode ? "铺满裁切" : "完整放入"
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
