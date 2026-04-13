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

    Item {
        width: root.availableWidth
        implicitHeight: content.implicitHeight + 32

        ColumnLayout {
            id: content
            anchors.horizontalCenter: parent.horizontalCenter
            width: Math.min(parent.width - 32, 820)
            anchors.top: parent.top
            anchors.topMargin: 16
            spacing: 12

            Label { text: "设置"; font.pixelSize: 24; font.bold: true }

            Frame {
                Layout.fillWidth: true
                ColumnLayout {
                    anchors.fill: parent
                    spacing: 10
                    Label { text: "导出"; font.bold: true }

                    RowLayout { Layout.fillWidth: true; Label { text: "自动布局策略"; Layout.preferredWidth: 120 }
                        ComboBox { Layout.fillWidth: true; model: ["均衡填充", "人像优先", "证件照优先"]; currentIndex: model.indexOf(appController.autoLayoutPreset); onActivated: appController.autoLayoutPreset = currentText }
                    }
                    RowLayout { Layout.fillWidth: true; Label { text: "默认导出路径"; Layout.preferredWidth: 120 }
                        TextField { Layout.fillWidth: true; text: appController.defaultExportPath; onEditingFinished: appController.defaultExportPath = text }
                    }
                    RowLayout { Layout.fillWidth: true; Label { text: "默认格式"; Layout.preferredWidth: 120 }
                        ComboBox { Layout.fillWidth: true; model: ["JPG", "PNG"]; currentIndex: model.indexOf(appController.defaultExportFormat); onActivated: appController.defaultExportFormat = currentText }
                    }
                    RowLayout { Layout.fillWidth: true; Label { text: "默认分辨率"; Layout.preferredWidth: 120 }
                        ComboBox { Layout.fillWidth: true; model: ["300 PPI", "600 PPI", "自定义 PPI"]; currentIndex: model.indexOf(appController.defaultExportResolution); onActivated: appController.defaultExportResolution = currentText }
                    }
                    RowLayout { Layout.fillWidth: true; Label { text: "自定义 PPI"; Layout.preferredWidth: 120 }
                        SpinBox { Layout.fillWidth: true; from: 72; to: 1200; value: appController.defaultCustomPpi; enabled: appController.defaultExportResolution === "自定义 PPI"; onValueModified: appController.defaultCustomPpi = value }
                    }
                    CheckBox { text: "记忆上次导出路径"; checked: appController.rememberLastPath; onToggled: appController.rememberLastPath = checked }
                    CheckBox { text: "默认裁切标记"; checked: appController.defaultCropMarks; onToggled: appController.defaultCropMarks = checked }
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
                        Label { text: "缓存目录"; Layout.preferredWidth: 120 }
                        TextField { Layout.fillWidth: true; text: appController.cacheDirectory; onEditingFinished: appController.cacheDirectory = text }
                        Button { text: "选择"; onClicked: cacheDirDialog.open() }
                    }
                    RowLayout {
                        Layout.fillWidth: true
                        Label { text: "预览最长边"; Layout.preferredWidth: 120 }
                        ComboBox { Layout.fillWidth: true; model: [1600, 2048, 2560]; currentIndex: model.indexOf(appController.previewMaxEdge); onActivated: appController.previewMaxEdge = Number(currentText) }
                    }
                    Label { text: "编辑预览与缩略图缓存会使用该配置。"; color: "#6b7280"; font.pixelSize: 12 }
                    Button { text: "清理缓存"; onClicked: appController.clearPreviewCache() }
                }
            }

            Frame {
                Layout.fillWidth: true
                ColumnLayout {
                    anchors.fill: parent
                    spacing: 10
                    Label { text: "界面"; font.bold: true }
                    RowLayout { Layout.fillWidth: true; Label { text: "主题"; Layout.preferredWidth: 120 }
                        ComboBox { Layout.fillWidth: true; model: ["系统", "浅色", "深色"]; currentIndex: model.indexOf(appController.themePlaceholder); onActivated: appController.themePlaceholder = currentText }
                    }
                }
            }
        }
    }
}
