import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import "../components"

ScrollView {
    id: root
    property int currentPage: 3
    signal navigateRequested(int pageIndex)

    function ppiPresetIndex(value) {
        if (value === 300) return 0
        if (value === 600) return 1
        return 2
    }

    FolderDialog {
        id: cacheDirDialog
        title: "选择缓存目录"
        onAccepted: appController.setCacheDirectoryFromDialog(selectedFolder)
    }
    FolderDialog {
        id: textureDirDialog
        title: "选择纹理目录"
        onAccepted: appController.setTextureDirectoryFromDialog(selectedFolder)
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

            PageTopBar {
                Layout.fillWidth: true
                currentPage: root.currentPage
                onNavigate: (pageIndex) => root.navigateRequested(pageIndex)
            }
            Label { text: "设置"; font.pixelSize: 22; font.bold: true }

            Frame {
                Layout.fillWidth: true
                ColumnLayout {
                    anchors.fill: parent
                    spacing: 10
                    Label { text: "自动拼版默认值"; font.bold: true }

                    RowLayout {
                        Layout.fillWidth: true
                        Label { text: "默认 PPI"; Layout.preferredWidth: 140 }
                        ComboBox {
                            id: defaultPpiPreset
                            Layout.preferredWidth: 220
                            model: ["300", "600", "自定义"]
                            currentIndex: root.ppiPresetIndex(appController.autoDefaultPpi)
                            onActivated: {
                                if (currentIndex === 0) appController.autoDefaultPpi = 300
                                else if (currentIndex === 1) appController.autoDefaultPpi = 600
                            }
                        }
                        TextField {
                            Layout.preferredWidth: 120
                            text: String(appController.autoDefaultPpi)
                            enabled: defaultPpiPreset.currentIndex === 2
                            color: enabled ? "#2f3a47" : "#8f98a3"
                            onEditingFinished: {
                                const v = Number(text)
                                if (!isNaN(v)) appController.autoDefaultPpi = v
                                text = String(appController.autoDefaultPpi)
                            }
                            background: Rectangle {
                                radius: 6
                                color: parent.enabled ? "#f1f4f7" : "#e9edf1"
                                border.color: parent.enabled ? "#c7d0da" : "#d2d8df"
                            }
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
                            background: Rectangle {
                                radius: 6
                                color: "#f1f4f7"
                                border.color: "#c7d0da"
                            }
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
                    CheckBox {
                        text: "原图导出"
                        checked: appController.autoOriginalQualityExport
                        onToggled: appController.autoOriginalQualityExport = checked
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
                            background: Rectangle {
                                radius: 6
                                color: "#f1f4f7"
                                border.color: "#c7d0da"
                            }
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
                    RowLayout {
                        Layout.fillWidth: true
                        Label { text: "缓存保留时间"; Layout.preferredWidth: 140 }
                        ComboBox {
                            Layout.fillWidth: true
                            model: [
                                { label: "1 天", value: 1 },
                                { label: "1 周", value: 7 },
                                { label: "1 个月", value: 30 }
                            ]
                            textRole: "label"
                            currentIndex: {
                                for (let i = 0; i < model.length; ++i) {
                                    if (model[i].value === appController.cacheRetentionDays) return i
                                }
                                return 2
                            }
                            onActivated: appController.cacheRetentionDays = model[index].value
                        }
                    }

                    Button {
                        text: "清理缓存"
                        onClicked: appController.clearPreviewCache()
                    }
                    RowLayout {
                        Layout.fillWidth: true
                        Label { text: "纹理目录"; Layout.preferredWidth: 140 }
                        TextField {
                            Layout.fillWidth: true
                            text: appController.textureDirectory
                            onEditingFinished: appController.textureDirectory = text
                            background: Rectangle {
                                radius: 6
                                color: "#f1f4f7"
                                border.color: "#c7d0da"
                            }
                        }
                        Button { text: "选择文件夹"; onClicked: textureDirDialog.open() }
                        Button { text: "打开"; onClicked: appController.openTextureDirectory() }
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
