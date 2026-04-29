import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ApplicationWindow {
    id: root
    width: 1233
    height: 888
    minimumWidth: 1233
    minimumHeight: 888
    visible: true
    title: "留素"

    property int currentPage: 0 // 0 home, 1 editor, 2 export, 3 settings
    readonly property bool hasTaskContext: appController.project.pageCount > 0
    onActiveChanged: {
        if (!root.active) {
            appController.project.clearSelection()
        }
    }

    Connections {
        target: appController
        function onRequestNavigateToEditor() {
            if (root.hasTaskContext) {
                root.currentPage = 1
            }
        }
        function onRequestNavigateToExport() {
            if (root.hasTaskContext) {
                root.currentPage = 2
            }
        }
    }

    StackLayout {
        anchors.fill: parent
        currentIndex: root.currentPage

        HomePage { currentPage: root.currentPage; onNavigateRequested: (p) => root.currentPage = p }
        EditorPage { currentPage: root.currentPage; onNavigateRequested: (p) => root.currentPage = p }
        ExportPage { currentPage: root.currentPage; onNavigateRequested: (p) => root.currentPage = p }
        SettingsPage { currentPage: root.currentPage; onNavigateRequested: (p) => root.currentPage = p }
    }
}
