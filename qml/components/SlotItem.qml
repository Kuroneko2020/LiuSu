import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: slotRoot
    radius: 8
    color: hasImage ? "#111" : "#f0f0f0"
    border.color: selected ? "#4f7cff" : "#c5c5c5"
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

    signal addClicked()
    signal slotClicked()
    signal rotateClicked()
    signal mirrorClicked()
    signal toggleFillMode()
    signal swapRequested(int fromIndex, int toIndex)
    signal contentDragged(real dx, real dy)

    DropArea {
        anchors.fill: parent
        onDropped: (drop) => {
            if (drop.source && drop.source.sourceSlot >= 0 && drop.source.sourceSlot !== slotRoot.slotIndex) {
                slotRoot.swapRequested(drop.source.sourceSlot, slotRoot.slotIndex)
            }
        }
    }

    Image {
        id: photo
        anchors.fill: parent
        anchors.margins: 2
        source: imagePath
        visible: hasImage && source !== ""
        fillMode: fillCropMode ? Image.PreserveAspectCrop : Image.PreserveAspectFit
        smooth: true
        cache: true

        transform: [
            Translate {
                x: fillCropMode ? cropOffsetX * photo.width * 0.08 : 0
                y: fillCropMode ? cropOffsetY * photo.height * 0.08 : 0
            },
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

        Drag.active: dragArea.drag.active
        Drag.hotSpot.x: width / 2
        Drag.hotSpot.y: height / 2

        MouseArea {
            id: dragArea
            anchors.fill: parent
            drag.target: parent
            onPressed: {
                swapHandle.sourceSlot = slotRoot.slotIndex
            }
            onReleased: {
                parent.x = 0
                parent.y = 0
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
}
