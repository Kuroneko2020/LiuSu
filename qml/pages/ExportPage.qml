import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ScrollView {
    ColumnLayout {
        width: parent.width
        spacing: 12
        padding: 20

        Label { text: "导出页"; font.pixelSize: 24; font.bold: true }
        Label { text: "当前导出范围：" + appController.exportScope; color: "#555" }

        RowLayout {
            Layout.fillWidth: true
            TextField {
                Layout.fillWidth: true
                placeholderText: "导出路径"
                text: appController.exportPath
                onEditingFinished: appController.exportPath = text
            }
            Button {
                text: "选择..."
                onClicked: appController.chooseExportPath()
            }
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
            text: "开始导出"
            onClicked: appController.runExport()
        }

        Rectangle {
            Layout.fillWidth: true
            implicitHeight: 44
            radius: 6
            visible: appController.lastExportMessage.length > 0
            color: appController.lastExportSuccess ? "#e6f7e8" : "#fdeaea"
            border.color: appController.lastExportSuccess ? "#8bcf91" : "#e58f8f"
            Label {
                anchors.centerIn: parent
                text: appController.lastExportMessage
                color: appController.lastExportSuccess ? "#23642a" : "#922e2e"
            }
        }
    }
}
