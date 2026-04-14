import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Dialog {
    id: root
    parent: Overlay.overlay
    modal: true
    title: "自定义颜色"
    width: 520
    height: 380
    x: parent ? (parent.width - width) / 2 : 0
    y: parent ? (parent.height - height) / 2 : 0
    standardButtons: Dialog.NoButton

    property color selectedColor: "#ffffff"
    property real hue: 0
    property real sat: 0
    property real val: 1

    signal confirmed(color colorValue)

    function syncFromColor(c) {
        selectedColor = c
        hue = Math.max(0, c.hsvHue)
        sat = Math.max(0, c.hsvSaturation)
        val = Math.max(0, c.hsvValue)
        hexField.text = c.toString()
    }

    onOpened: syncFromColor(selectedColor)

    footer: RowLayout {
        spacing: 10
        Rectangle {
            Layout.preferredWidth: 56
            Layout.preferredHeight: 34
            radius: 6
            color: root.selectedColor
            border.color: "#c7d0da"
        }
        Label { text: "HEX" }
        TextField {
            id: hexField
            Layout.preferredWidth: 170
            onEditingFinished: {
                const c = Qt.color(text)
                if (c.a > 0 || text.toLowerCase() === "#000000") {
                    root.syncFromColor(c)
                } else {
                    text = root.selectedColor.toString()
                }
            }
        }
        Item { Layout.fillWidth: true }
        Button {
            text: "取消"
            implicitWidth: 96
            implicitHeight: 36
            onClicked: root.close()
        }
        Button {
            text: "确定"
            implicitWidth: 96
            implicitHeight: 36
            highlighted: true
            onClicked: {
                root.confirmed(root.selectedColor)
                root.close()
            }
        }
    }

    contentItem: ColumnLayout {
        spacing: 10

        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 12

            Item {
                id: svPanel
                Layout.fillWidth: true
                Layout.fillHeight: true
                implicitHeight: 220
                implicitWidth: 320

                Rectangle {
                    anchors.fill: parent
                    radius: 8
                    color: Qt.hsva(root.hue, 1, 1, 1)
                    border.color: "#c7d0da"
                }
                Rectangle {
                    anchors.fill: parent
                    radius: 8
                    gradient: Gradient {
                        GradientStop { position: 0.0; color: "#ffffff" }
                        GradientStop { position: 1.0; color: "#00ffffff" }
                    }
                }
                Rectangle {
                    anchors.fill: parent
                    radius: 8
                    gradient: Gradient {
                        orientation: Gradient.Vertical
                        GradientStop { position: 0.0; color: "#00000000" }
                        GradientStop { position: 1.0; color: "#ff000000" }
                    }
                }

                Rectangle {
                    x: root.sat * (parent.width - width)
                    y: (1 - root.val) * (parent.height - height)
                    width: 14
                    height: 14
                    radius: 7
                    border.width: 2
                    border.color: "#ffffff"
                    color: "transparent"
                }

                MouseArea {
                    anchors.fill: parent
                    onPressed: (mouse) => update(mouse.x, mouse.y)
                    onPositionChanged: (mouse) => update(mouse.x, mouse.y)
                    function update(px, py) {
                        root.sat = Math.max(0, Math.min(1, px / width))
                        root.val = Math.max(0, Math.min(1, 1 - py / height))
                        root.selectedColor = Qt.hsva(root.hue, root.sat, root.val, 1)
                        hexField.text = root.selectedColor.toString()
                    }
                }
            }

            Item {
                Layout.preferredWidth: 28
                Layout.fillHeight: true
                Rectangle {
                    id: hueBar
                    anchors.fill: parent
                    radius: 6
                    border.color: "#c7d0da"
                    gradient: Gradient {
                        orientation: Gradient.Vertical
                        GradientStop { position: 0.0; color: "#ff0000" }
                        GradientStop { position: 0.17; color: "#ffff00" }
                        GradientStop { position: 0.33; color: "#00ff00" }
                        GradientStop { position: 0.50; color: "#00ffff" }
                        GradientStop { position: 0.67; color: "#0000ff" }
                        GradientStop { position: 0.83; color: "#ff00ff" }
                        GradientStop { position: 1.0; color: "#ff0000" }
                    }
                }
                Rectangle {
                    x: 2
                    y: root.hue * (parent.height - height)
                    width: parent.width - 4
                    height: 6
                    radius: 3
                    color: "#ffffff"
                    border.color: "#4f5d6c"
                }
                MouseArea {
                    anchors.fill: parent
                    onPressed: (mouse) => update(mouse.y)
                    onPositionChanged: (mouse) => update(mouse.y)
                    function update(py) {
                        root.hue = Math.max(0, Math.min(1, py / height))
                        root.selectedColor = Qt.hsva(root.hue, root.sat, root.val, 1)
                        hexField.text = root.selectedColor.toString()
                    }
                }
            }
        }

    }
}
