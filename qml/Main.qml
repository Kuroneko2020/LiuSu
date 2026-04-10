import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ApplicationWindow {
    id: root
    width: 1280
    height: 800
    visible: true
    title: "6 英寸相纸照片拼版工具"

    property int currentPage: 0 // 0 home, 1 editor, 2 export, 3 settings

    Connections {
        target: appController
        function onRequestNavigateToEditor() {
            root.currentPage = 1
        }
        function onRequestNavigateToExport() {
            root.currentPage = 2
        }
    }

    header: ToolBar {
        RowLayout {
            anchors.fill: parent
            anchors.margins: 8
            Label {
                text: "Photo Template Editor"
                font.bold: true
            }
            Item { Layout.fillWidth: true }
            Button { text: "首页"; onClicked: root.currentPage = 0 }
            Button { text: "编辑"; onClicked: root.currentPage = 1 }
            Button { text: "导出"; onClicked: root.currentPage = 2 }
            Button { text: "设置"; onClicked: root.currentPage = 3 }
        }
    }

    StackLayout {
        anchors.fill: parent
        currentIndex: root.currentPage

        HomePage { }
        EditorPage { }
        ExportPage { }
        SettingsPage { }
    }
}
