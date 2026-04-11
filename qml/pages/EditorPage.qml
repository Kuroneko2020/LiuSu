import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "../components"

Item {
    id: editor

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 12

        RowLayout {
            Layout.fillWidth: true
            Label { text: "编辑页"; font.pixelSize: 24; font.bold: true }
            Item { Layout.fillWidth: true }
            Button { text: "批量导入"; onClicked: appController.batchImport() }
            Button { text: "导出当前页"; onClicked: appController.exportCurrentPage() }
            Button { text: "删除当前页"; onClicked: appController.project.deleteCurrentPage() }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            radius: 10
            color: "#f7f7f7"
            border.color: "#d9d9d9"

            TemplateCanvas {
                anchors.centerIn: parent
                width: Math.min(parent.width * 0.88, 780)
                height: width * (2 / 3)
            }
        }

        FilmStrip {
            Layout.fillWidth: true
            Layout.preferredHeight: 120
        }
    }
}
