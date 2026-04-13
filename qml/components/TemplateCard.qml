import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: card

    property string templateName: ""
    property int slotCount: 2
    property int templateChoice: 2
    property bool selected: false

    signal clicked()

    implicitHeight: 250

    Rectangle {
        id: shadowLayer
        anchors.fill: body
        anchors.margins: selected ? -2 : 0
        radius: body.radius + 1
        color: "#000000"
        opacity: selected ? 0.08 : 0.0
        z: -1
    }

    Rectangle {
        id: body
        anchors.fill: parent
        radius: 10
        color: "#f4f6f8"
        border.color: selected ? "#99a5b3" : "#cdd4dc"
        border.width: 1

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 10
            spacing: 8

            Rectangle {
                Layout.fillWidth: true
                Layout.fillHeight: true
                radius: 8
                color: "#ffffff"
                border.color: "#d8dee5"

                Rectangle {
                    anchors.centerIn: parent
                    width: parent.width * 0.88
                    height: width / appController.pageAspectRatio
                    color: "#f7f8fa"
                    border.color: "#cfd5dd"

                    Repeater {
                        model: appController.templateSlotRects(card.templateChoice)
                        delegate: Rectangle {
                            required property var modelData
                            x: modelData.x * parent.width
                            y: modelData.y * parent.height
                            width: modelData.width * parent.width
                            height: modelData.height * parent.height
                            color: "#ebeff3"
                            border.color: "#cfd6df"
                        }
                    }
                }
            }

            Label {
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignHCenter
                text: card.templateName
                font.pixelSize: 14
                font.bold: false
                color: "#3e4854"
            }
        }

        MouseArea {
            anchors.fill: parent
            onClicked: card.clicked()
        }
    }
}
