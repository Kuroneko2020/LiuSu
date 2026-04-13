import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import "../components"

Item {
    id: home

    property int selectedTemplate: -1
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

        Item { Layout.fillHeight: true }

        RowLayout {
            Layout.alignment: Qt.AlignHCenter
            spacing: 22

            Repeater {
                model: [2, 4, 9]

                delegate: TemplateCard {
                    required property int index
                    required property int modelData
                    Layout.preferredWidth: 340
                    Layout.preferredHeight: 220
                    templateChoice: modelData
                    selected: home.selectedTemplate === index
                    onClicked: home.selectedTemplate = index
                    onManualLayout: appController.startManualLayout(modelData)
                    onAutoLayout: {
                        home.autoLayoutChoice = modelData
                        autoLayoutDialog.open()
                    }
                }
            }
        }

        Item { Layout.fillHeight: true }
    }
}
