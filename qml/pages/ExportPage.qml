import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs

ScrollView {
    id: root

    function ppiPresetIndex(text) {
        if (text === "300 PPI") return 0
        if (text === "600 PPI") return 1
        return 2
    }

    FolderDialog {
        id: exportFolderDialog
        title: "选择导出目录"
        onAccepted: appController.setExportPathFromDialog(selectedFolder)
    }

    Item {
        width: root.availableWidth
        implicitHeight: content.implicitHeight + 32

        ColumnLayout {
            id: content
            anchors.horizontalCenter: parent.horizontalCenter
            width: Math.min(parent.width - 32, 860)
            anchors.top: parent.top
            anchors.topMargin: 16
            spacing: 12

            Label { text: "导出"; font.pixelSize: 24; font.bold: true }
            Label { text: "当前导出范围：" + appController.exportScope; color: "#5f6b79" }

            Frame {
                Layout.fillWidth: true
                ColumnLayout {
                    anchors.fill: parent
                    spacing: 10

                    RowLayout {
                        Layout.fillWidth: true
                        Label { text: "导出路径"; Layout.preferredWidth: 120 }
                        TextField {
                            Layout.fillWidth: true
                            text: appController.exportPath
                            placeholderText: "导出路径"
                            onEditingFinished: appController.exportPath = text
                            background: Rectangle {
                                radius: 6
                                color: "#f1f4f7"
                                border.color: "#c7d0da"
                            }
                        }
                        Button { text: "选择"; onClicked: exportFolderDialog.open() }
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        Label { text: "导出格式"; Layout.preferredWidth: 120 }
                        ComboBox {
                            Layout.fillWidth: true
                            model: ["JPG", "PNG"]
                            currentIndex: model.indexOf(appController.exportFormat)
                            enabled: !appController.originalQualityExport
                            onActivated: appController.exportFormat = currentText
                        }
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        Label { text: "命名规则"; Layout.preferredWidth: 120 }
                        ComboBox {
                            Layout.fillWidth: true
                            model: ["组合命名", "日期-序号"]
                            currentIndex: model.indexOf(appController.namingRule)
                            onActivated: appController.namingRule = currentText
                        }
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        Label { text: "输出 PPI"; Layout.preferredWidth: 120 }
                        ComboBox {
                            id: exportPpiPreset
                            Layout.preferredWidth: 220
                            enabled: !appController.originalQualityExport
                            model: ["300", "600", "自定义"]
                            currentIndex: root.ppiPresetIndex(appController.resolutionPreset)
                            onActivated: {
                                if (currentIndex === 0) appController.resolutionPreset = "300 PPI"
                                else if (currentIndex === 1) appController.resolutionPreset = "600 PPI"
                                else appController.resolutionPreset = "自定义 PPI"
                            }
                        }
                        TextField {
                            Layout.preferredWidth: 120
                            enabled: !appController.originalQualityExport && exportPpiPreset.currentIndex === 2
                            color: enabled ? "#2f3a47" : "#8f98a3"
                            text: String(appController.customPpi)
                            onEditingFinished: {
                                const v = Number(text)
                                if (!isNaN(v)) appController.customPpi = v
                                text = String(appController.customPpi)
                            }
                            background: Rectangle {
                                radius: 6
                                color: parent.enabled ? "#f1f4f7" : "#e9edf1"
                                border.color: parent.enabled ? "#c7d0da" : "#d2d8df"
                            }
                        }
                    }

                    CheckBox {
                        text: "导出裁切标记"
                        checked: appController.cropMarks
                        onToggled: appController.cropMarks = checked
                    }
                    CheckBox {
                        text: "原图导出"
                        checked: appController.originalQualityExport
                        onToggled: appController.originalQualityExport = checked
                    }
                    Label {
                        visible: appController.originalQualityExport
                        text: "原图导出模式下自动使用 PNG，并忽略 PPI。"
                        color: "#4d6b8f"
                    }

                    Button {
                        text: "开始导出"
                        onClicked: appController.runExport()
                    }
                }
            }

            Rectangle {
                Layout.fillWidth: true
                implicitHeight: 44
                radius: 6
                visible: appController.lastExportMessage.length > 0
                color: appController.lastExportSuccess ? "#e8f2ea" : "#f4e8e8"
                border.color: appController.lastExportSuccess ? "#9ebca4" : "#c9a4a4"
                Label {
                    anchors.centerIn: parent
                    text: appController.lastExportMessage
                    color: appController.lastExportSuccess ? "#2f5a38" : "#6e3a3a"
                }
            }
        }
    }
}
