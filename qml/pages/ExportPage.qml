import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ScrollView {
    ColumnLayout {
        width: parent.width
        spacing: 12
        padding: 20

        Label { text: "导出页"; font.pixelSize: 24; font.bold: true }

        TextField {
            Layout.fillWidth: true
            placeholderText: "导出路径"
            text: appController.exportPath
            onEditingFinished: appController.exportPath = text
        }

        ComboBox {
            Layout.fillWidth: true
            model: ["JPG", "PNG"]
            currentIndex: model.indexOf(appController.exportFormat)
            onActivated: appController.exportFormat = currentText
        }

        ComboBox {
            Layout.fillWidth: true
            model: ["组合命名", "日期-序号"]
            currentIndex: model.indexOf(appController.namingRule)
            onActivated: appController.namingRule = currentText
        }

        ComboBox {
            Layout.fillWidth: true
            model: ["300 PPI", "600 PPI", "自定义 PPI"]
            currentIndex: model.indexOf(appController.resolutionPreset)
            onActivated: appController.resolutionPreset = currentText
        }

        SpinBox {
            Layout.fillWidth: true
            from: 72
            to: 1200
            value: appController.customPpi
            enabled: appController.resolutionPreset === "自定义 PPI"
            onValueModified: appController.customPpi = value
        }

        CheckBox {
            text: "导出裁切标记"
            checked: appController.cropMarks
            onToggled: appController.cropMarks = checked
        }

        Button {
            text: "导出当前页（占位）"
            onClicked: appController.exportCurrentPage()
        }
    }
}
