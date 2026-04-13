import QtQuick
import QtQuick.Controls

Item {
    id: card

    property int templateChoice: 2
    property bool selected: false

    signal clicked()
    signal manualLayout()
    signal autoLayout()

    implicitWidth: 290
    implicitHeight: 190
    scale: selected ? 1.03 : 1.0

    Behavior on scale {
        NumberAnimation { duration: 140; easing.type: Easing.InOutQuad }
    }

    Rectangle {
        id: shadowLayer
        anchors.fill: previewFrame
        anchors.margins: selected ? -4 : 0
        radius: previewFrame.radius + 2
        color: "#000"
        opacity: selected ? 0.10 : 0.0
        z: -1
    }

    Rectangle {
        id: previewFrame
        anchors.fill: parent
        radius: 10
        color: "#f6f8fb"
        border.color: selected ? "#b3bcc8" : "#d2d8e0"
        border.width: 1

        Rectangle {
            id: pagePreview
            anchors.centerIn: parent
            width: parent.width * 0.88
            height: width / appController.pageAspectRatio
            color: "#f1f4f8"
            border.color: "#cfd6df"

            Repeater {
                model: appController.templateSlotRects(card.templateChoice)
                delegate: Rectangle {
                    required property var modelData
                    x: modelData.x * parent.width
                    y: modelData.y * parent.height
                    width: modelData.width * parent.width
                    height: modelData.height * parent.height
                    color: "#e6ebf2"
                    border.color: "#c7cfda"
                }
            }
        }

        Rectangle {
            visible: selected
            z: 2
            anchors.centerIn: pagePreview
            width: pagePreview.width * 0.62
            height: 44
            radius: 8
            color: "#eff3f7cc"
            border.color: "#c4ccd6"

            Row {
                anchors.centerIn: parent
                spacing: 8

                Button {
                    text: "手动布局"
                    flat: true
                    onClicked: card.manualLayout()
                    background: Rectangle {
                        radius: 6
                        color: parent.down ? "#dce3ea" : (parent.hovered ? "#e7edf3" : "#00000000")
                        border.color: "#bfc8d3"
                    }
                }
                Button {
                    text: "自动布局"
                    flat: true
                    onClicked: card.autoLayout()
                    background: Rectangle {
                        radius: 6
                        color: parent.down ? "#dce3ea" : (parent.hovered ? "#e7edf3" : "#00000000")
                        border.color: "#bfc8d3"
                    }
                }
            }
        }

        MouseArea {
            z: 1
            anchors.fill: parent
            onClicked: card.clicked()
        }
    }
}
