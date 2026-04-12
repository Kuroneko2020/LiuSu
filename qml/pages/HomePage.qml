import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import "../components"


Item {
    id: home
    focus: true
    Keys.onEscapePressed: expandedTemplate = -1

    property int expandedTemplate: -1
    property bool suppressNextCollapse: false
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

    MouseArea {
        anchors.fill: parent
        acceptedButtons: Qt.LeftButton
        onClicked: {
            if (!home.suppressNextCollapse) {
                home.expandedTemplate = -1
            }
            home.suppressNextCollapse = false
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 24
        spacing: 20

        Label {
            text: "选择模板"
            font.pixelSize: 28
            font.bold: true
        }
        Label {
            text: "默认 3:2 模板比例，预览已居中并预留裁切白边。"
            color: "#666"
        }

        RowLayout {
            spacing: 18

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
                    Layout.preferredWidth: 300
                    templateName: modelData.name
                    templateChoice: modelData.choice
                    slotCount: modelData.slots
                    expanded: home.expandedTemplate === index

                    onToggleExpand: {
                        home.suppressNextCollapse = true
                        home.expandedTemplate = expanded ? -1 : index
                    }
                    onManualLayout: {
                        home.suppressNextCollapse = true
                        appController.startManualLayout(modelData.choice)
                    }
                    onAutoLayout: {
                        home.suppressNextCollapse = true
                        home.autoLayoutChoice = modelData.choice
                        autoLayoutDialog.open()
                    }
                }
            }
        }

        Item { Layout.fillHeight: true }
    }
}
