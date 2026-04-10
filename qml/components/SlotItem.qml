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
    property string imageLabel: ""
    property string imagePath: ""
    property bool fillCropMode: false
    property int rotationDegrees: 0
    property bool mirrored: false

    signal addClicked()
    signal slotClicked()
    signal rotateClicked()
    signal mirrorClicked()
    signal toggleFillMode()

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
            onPressed: console.log("slot swap handle pressed", slotIndex)
        }
    }

    Rectangle {
        visible: hasImage
        width: parent.width * 0.7
        height: parent.height * 0.2
        color: "transparent"
        border.color: "#7f7f7f"
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 6
        Text { anchors.centerIn: parent; text: "内容拖动入口"; font.pixelSize: 10; color: "#eee" }

        MouseArea {
            anchors.fill: parent
            onPressed: console.log("content drag area pressed", slotIndex)
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
