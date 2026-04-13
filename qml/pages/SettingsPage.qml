import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs

ScrollView {
    id: root

    FolderDialog {
        id: cacheDirDialog
        title: "选择缓存目录"
        onAccepted: appController.setCacheDirectoryFromDialog(selectedFolder)
    }

    FolderDialog {
        id: exportDirDialog
        title: "选择默认导出目录"
        onAccepted: appController.setDefaultExportPathFromDialog(selectedFolder)
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

            Label { text: "设置"; font.pixelSize: 24; font.bold: true }

            Frame {
                Layout.fillWidth: true
                ColumnLayout {
                    anchors.fill: parent
                    spacing: 10
                    Label { text: "自动拼版默认值"; font.bold: true }

                    RowLayout {
                        Layout.fillWidth: true
                        Label { text: "默认 PPI"; Layout.preferredWidth: 140 }
                        SpinBox {
                            Layout.preferredWidth: 180
                            from: 72
                            to: 1200
                            value: appController.autoDefaultPpi
                            onValueModified: appController.autoDefaultPpi = value
                        }
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        Label { text: "默认填充策略"; Layout.preferredWidth: 140 }
                        ComboBox {
                            Layout.fillWidth: true
                            model: ["放大填充", "原图完整放入"]
                            currentIndex: model.indexOf(appController.autoFillStrategy)
                            onActivated: appController.autoFillStrategy = currentText
                        }
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        Label { text: "方向不符时"; Layout.preferredWidth: 140 }
                        ComboBox {
                            Layout.fillWidth: true
                            model: ["保持原方向", "自动右转 90°"]
                            currentIndex: model.indexOf(appController.autoOrientationPolicy)
                            onActivated: appController.autoOrientationPolicy = currentText
                        }
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        Label { text: "默认导出路径"; Layout.preferredWidth: 140 }
                        TextField {
                            Layout.fillWidth: true
                            text: appController.defaultExportPath
                            onEditingFinished: appController.defaultExportPath = text
                        }
                        Button {
                            text: "选择"
                            onClicked: exportDirDialog.open()
                        }
                    }

                    CheckBox {
                        text: "默认使用上次导出路径"
                        checked: appController.rememberLastPath
                        onToggled: appController.rememberLastPath = checked
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        Label { text: "导出格式"; Layout.preferredWidth: 140 }
                        ComboBox {
                            Layout.fillWidth: true
                            model: ["JPG", "PNG"]
                            currentIndex: model.indexOf(appController.defaultExportFormat)
                            onActivated: appController.defaultExportFormat = currentText
                        }
                    }
                }
            }

            Frame {
                Layout.fillWidth: true
                ColumnLayout {
                    anchors.fill: parent
                    spacing: 10
                    Label { text: "缓存"; font.bold: true }

                    RowLayout {
                        Layout.fillWidth: true
                        Label { text: "缓存目录"; Layout.preferredWidth: 140 }
                        TextField {
                            Layout.fillWidth: true
                            text: appController.cacheDirectory
                            onEditingFinished: appController.cacheDirectory = text
                        }
                        Button {
                            text: "选择"
                            onClicked: cacheDirDialog.open()
                        }
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        Label { text: "预览质量"; Layout.preferredWidth: 140 }
                        ComboBox {
                            id: qualityCombo
                            Layout.fillWidth: true
                            model: [
                                { label: "轻量（1600）", value: 1600 },
                                { label: "标准（2048）", value: 2048 },
                                { label: "精细（2560）", value: 2560 }
                            ]
                            textRole: "label"
                            currentIndex: {
                                for (let i = 0; i < model.length; ++i) {
                                    if (model[i].value === appController.previewMaxEdge) return i
                                }
                                return 0
                            }
                            onActivated: appController.previewMaxEdge = model[index].value
                        }
                    }

                    Button {
                        text: "清理缓存"
                        onClicked: appController.clearPreviewCache()
                    }
                }
            }

            Frame {
                Layout.fillWidth: true
                ColumnLayout {
                    anchors.fill: parent
                    spacing: 10
                    Label { text: "其他"; font.bold: true }
                    RowLayout {
                        Layout.fillWidth: true
                        Label { text: "主题"; Layout.preferredWidth: 140 }
                        ComboBox {
                            Layout.fillWidth: true
                            model: ["系统", "浅色", "深色"]
                            currentIndex: model.indexOf(appController.themePlaceholder)
                            onActivated: appController.themePlaceholder = currentText
                        }
                    }
                }
            }
        }
    }
}
