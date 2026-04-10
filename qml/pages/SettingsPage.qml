import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ScrollView {
    ColumnLayout {
        width: parent.width
        spacing: 12
        padding: 20

        Label { text: "设置页"; font.pixelSize: 24; font.bold: true }

        ComboBox {
            Layout.fillWidth: true
            model: ["均衡填充", "人像优先", "证件照优先"]
            currentIndex: model.indexOf(appController.autoLayoutPreset)
            onActivated: appController.autoLayoutPreset = currentText
        }

        TextField {
            Layout.fillWidth: true
            placeholderText: "默认导出路径"
            text: appController.defaultExportPath
            onEditingFinished: appController.defaultExportPath = text
        }

        CheckBox {
            text: "记忆上次导出路径"
            checked: appController.rememberLastPath
            onToggled: appController.rememberLastPath = checked
        }

        ComboBox {
            Layout.fillWidth: true
            model: ["JPG", "PNG"]
            currentIndex: model.indexOf(appController.defaultExportFormat)
            onActivated: appController.defaultExportFormat = currentText
        }

        ComboBox {
            Layout.fillWidth: true
            model: ["300 PPI", "600 PPI", "自定义 PPI"]
            currentIndex: model.indexOf(appController.defaultExportResolution)
            onActivated: appController.defaultExportResolution = currentText
        }

        CheckBox {
            text: "默认裁切标记"
            checked: appController.defaultCropMarks
            onToggled: appController.defaultCropMarks = checked
        }

        ComboBox {
            Layout.fillWidth: true
            model: ["系统", "浅色", "深色"]
            currentIndex: model.indexOf(appController.themePlaceholder)
            onActivated: appController.themePlaceholder = currentText
        }
    }
}
