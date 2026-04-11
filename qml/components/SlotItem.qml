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
    property string imageSource: ""
    property bool fillCropMode: false
    property int rotationDegrees: 0
    property bool mirrored: false
    property real cropOffsetX: 0
    property real cropOffsetY: 0
    property bool swapTargetHighlighted: false

    signal addClicked()
    signal slotClicked()
    signal rotateClicked()
    signal mirrorClicked()
    signal toggleFillMode()
    signal swapRequested(int fromIndex, int toIndex)
    signal contentDragged(real dx, real dy)
    signal swapDragStarted(int fromIndex, point scenePos)
    signal swapDragMoved(point scenePos)
    signal swapDragFinished(point scenePos)

    border.color: swapTargetHighlighted ? "#ff8c00" : (selected ? "#4f7cff" : "#c5c5c5")

    Image {
        id: photo
        anchors.fill: parent
        anchors.margins: 2
        source: imageSource
        visible: hasImage && source !== ""
        fillMode: Image.PreserveAspectFit
        smooth: true
        cache: true

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
