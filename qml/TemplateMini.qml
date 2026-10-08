import QtQuick
import LiuSu

// 槽位坐标只取领域预设；缩略图缩放不会反写页面几何。
Rectangle {
    id: mini
    objectName: "templateMini"
    property string presetId: "four"
    property bool photographs: false
    property var slotRects: app.presetSlots(presetId)
    color: "#ffffff"
    border.width: 1; border.color: "#e0e2dc"
    Repeater {
        model: mini.slotRects
        delegate: Rectangle {
            objectName: "templateMiniSlot"
            required property var modelData
            required property int index
            x: modelData.x * mini.width; y: modelData.y * mini.height
            width: modelData.width * mini.width; height: modelData.height * mini.height
            color: "#d4d9d2"
            border.width: mini.photographs ? 0 : 1
            border.color: "#ffffff"
            clip: true
            Image {
                anchors.fill: parent
                visible: mini.photographs
                source: mini.photographs ? "qrc:/photos/coast.png" : ""
                sourceSize.width: Math.ceil(parent.width * 2)
                fillMode: Image.PreserveAspectCrop
                horizontalAlignment: index % 2 ? Image.AlignRight : Image.AlignLeft
                verticalAlignment: index % 3 ? Image.AlignBottom : Image.AlignTop
            }
        }
    }
}
