import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import "../components"

Item {
    id: home

    property int selectedTemplate: 1
    property int autoLayoutChoice: -1

    FileDialog {
        id: autoLayoutDialog
        title: "选择自动布局图片"
        fileMode: FileDialog.OpenFiles
        nameFilters: ["图片文件 (*.jpg *.jpeg *.png *.webp *.bmp *.tif *.tiff *.heic *.heif)"]
        onAccepted: {
            if (home.autoLayoutChoice > 0) {
                appController.startAutoLayoutWithFiles(home.autoLayoutChoice, selectedFiles)
            }
        }
    }

    Dialog {
        id: confirmNewTaskDialog
        title: "开始新任务"
        modal: true
        standardButtons: Dialog.Ok | Dialog.Cancel
        onAccepted: appController.confirmStartNewSession(true)
        onRejected: appController.confirmStartNewSession(false)
        Label { text: "开始新任务将清空当前排版，是否继续？"; wrapMode: Text.WordWrap }
    }

    Connections {
        target: appController
        function onRequestConfirmNewSession() {
            confirmNewTaskDialog.open()
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 24
        spacing: 18

        Label {
            text: "选择模板"
            font.pixelSize: 28
            font.bold: true
        }
        Label {
            text: "默认 3:2 模板比例"
            color: "#65707d"
            font.pixelSize: 13
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 14

            Repeater {
                model: [
                    { name: "两张拼图", choice: 2, slots: 2 },
                    { name: "四张拼图", choice: 4, slots: 4 },
                    { name: "九张拼图", choice: 9, slots: 9 }
                ]

                delegate: TemplateCard {
                    required property var modelData
                    required property int index
                    Layout.fillWidth: true
                    Layout.preferredWidth: 280
                    templateName: modelData.name
                    templateChoice: modelData.choice
                    slotCount: modelData.slots
                    selected: home.selectedTemplate === index
                    onClicked: home.selectedTemplate = index
                }
            }
        }

        Item { Layout.fillHeight: true }

        Rectangle {
            Layout.fillWidth: true
            radius: 10
            color: "#f3f5f8"
            border.color: "#d2d8e0"
            border.width: 1
            implicitHeight: 64

            RowLayout {
                anchors.fill: parent
                anchors.margins: 12
                spacing: 10

                Label {
                    text: "已选模板：" + ["两张拼图", "四张拼图", "九张拼图"][home.selectedTemplate]
                    color: "#4b5563"
                }
                Item { Layout.fillWidth: true }
                Button {
                    text: "手动布局"
                    onClicked: appController.startManualLayout([2, 4, 9][home.selectedTemplate])
                }
                Button {
                    text: "自动布局"
                    onClicked: {
                        home.autoLayoutChoice = [2, 4, 9][home.selectedTemplate]
                        autoLayoutDialog.open()
                    }
                }
            }
        }
    }
}
