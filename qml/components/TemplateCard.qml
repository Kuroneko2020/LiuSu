import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: card
    radius: 12
    color: "#f4f4f7"
    border.color: expanded ? "#4f7cff" : "#cccccc"
    border.width: expanded ? 2 : 1

    property string templateName: ""
    property int slotCount: 2
    property bool expanded: false

    signal toggleExpand()
    signal manualLayout()
    signal autoLayout()

    implicitHeight: expanded ? 280 : 220

    Behavior on implicitHeight {
        NumberAnimation { duration: 150; easing.type: Easing.InOutQuad }
    }

    MouseArea {
        anchors.fill: parent
        onClicked: card.toggleExpand()
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 14
        spacing: 12

        Label {
            text: card.templateName
            font.bold: true
            font.pixelSize: 18
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: "white"
            border.color: "#dadada"
            radius: 8

            Rectangle {
                anchors.centerIn: parent
                width: parent.width * 0.9
                height: width * (2 / 3)
                color: "#f9f9f9"
                border.color: "#bbb"

                Grid {
                    anchors.fill: parent
                    anchors.margins: 8
                    columns: card.slotCount <= 2 ? 1 : (card.slotCount === 4 ? 2 : 3)
                    rows: card.slotCount <= 2 ? 2 : 2
                    spacing: 4

                    Repeater {
                        model: card.slotCount
                        Rectangle {
                            width: (parent.width - (parent.columns - 1) * parent.spacing) / parent.columns
                            height: (parent.height - (parent.rows - 1) * parent.spacing) / parent.rows
                            color: "#ededed"
                            border.color: "#d0d0d0"
                        }
                    }
                }
            }
        }

        RowLayout {
            visible: card.expanded
            Layout.fillWidth: true
            spacing: 8

            Button {
                Layout.fillWidth: true
                text: "手动布局"
                onClicked: card.manualLayout()
            }
            Button {
                Layout.fillWidth: true
                text: "自动布局"
                onClicked: card.autoLayout()
            }
        }
    }
}
