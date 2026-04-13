import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import "../components"

Item {
    id: home

    property int selectedTemplate: -1
    property int autoLayoutChoice: -1
    readonly property var templateChoices: [2, 4, 9]

    onVisibleChanged: {
        if (visible) {
            selectedTemplate = -1
        }
    }

    FileDialog {
        id: autoLayoutDialog
        title: "选择自动布局图片"
        fileMode: FileDialog.OpenFiles
        nameFilters: ["图片文件 (*.jpg *.jpeg *.png *.webp *.bmp *.tif *.tiff *.heic *.heif)"]
        onAccepted: {
            if (home.autoLayoutChoice > 0) {
                appController.startAutoLayoutWithFiles(home.autoLayoutChoice, selectedFiles)
            }
            home.selectedTemplate = -1
        }
        onRejected: home.selectedTemplate = -1
    }

    Dialog {
        id: confirmNewTaskDialog
        parent: Overlay.overlay
        title: "开始新任务"
        modal: true
        width: 360
        height: 170
        x: parent ? (parent.width - width) / 2 : 0
        y: parent ? (parent.height - height) / 2 : 0
        standardButtons: Dialog.Ok | Dialog.Cancel
        onAccepted: appController.confirmStartNewSession(true)
        onRejected: appController.confirmStartNewSession(false)
        Label {
            anchors.fill: parent
            anchors.margins: 16
            text: "开始新任务将清空当前排版，是否继续？"
            verticalAlignment: Text.AlignVCenter
            wrapMode: Text.WordWrap
        }
    }

    Connections {
        target: appController
        function onRequestConfirmNewSession() {
            confirmNewTaskDialog.open()
        }
    }

    MouseArea {
        anchors.fill: parent
        z: 0
        onClicked: home.selectedTemplate = -1
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 24
        z: 1

        Item { Layout.fillHeight: true }

        RowLayout {
            Layout.alignment: Qt.AlignHCenter
            spacing: 24

            Repeater {
                model: home.templateChoices

                delegate: TemplateCard {
                    required property int index
                    required property int modelData
                    Layout.preferredWidth: 380
                    Layout.preferredHeight: 250
                    templateChoice: modelData
                    selected: home.selectedTemplate === index
                    onClicked: home.selectedTemplate = (home.selectedTemplate === index ? -1 : index)
                    onManualLayout: {
                        appController.startManualLayout(modelData)
                        home.selectedTemplate = -1
                    }
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
