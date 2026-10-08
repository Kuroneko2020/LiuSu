import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import LiuSu

ApplicationWindow {
    id: window
    objectName: "studioWindow"
    width: Math.min(1440, Screen.desktopAvailableWidth - 32)
    height: Math.min(920, Screen.desktopAvailableHeight - 40)
    minimumWidth: Math.min(960, Screen.desktopAvailableWidth - 32)
    minimumHeight: Math.min(640, Screen.desktopAvailableHeight - 40)
    visible: true
    title: qsTr("留素 · 照片工作台")
    color: "#eeeee8"
    font.family: AppTheme.fontFamily
    palette.window: "#f6f7f1"
    palette.button: "#f4f5ee"
    palette.buttonText: AppTheme.ink
    palette.text: AppTheme.ink
    palette.highlight: AppTheme.signalDeep
    palette.highlightedText: "#ffffff"
    property bool editing: false
    property bool visitedEditor: false
    property bool replacingProject: false
    readonly property int gutter: width > 1200 ? 38 : 24
    function showEditor() { visitedEditor = true; editing = true }
    function loadProject(fileUrl) {
        if (app.openProject(fileUrl)) { replacingProject = false; showEditor() }
    }
    function chooseTemplate(presetId) {
        if (visitedEditor && !replacingProject) app.changeCurrentLayout(presetId)
        else app.startManual(presetId)
        replacingProject = false
        showEditor()
    }

    Rectangle {
        anchors.fill: parent
        gradient: Gradient {
            GradientStop { position: 0; color: "#f6f6f1" }
            GradientStop { position: 1; color: "#e8e9e0" }
        }
    }
    WorkspaceHeader {
        id: header
        anchors { left: parent.left; right: parent.right; top: parent.top; leftMargin: window.gutter; rightMargin: window.gutter; topMargin: 15 }
        height: 60
        editing: window.editing
        onChooseTemplate: window.editing = false
        onOpenProject: openProjectDialog.open()
        onSaveProject: saveProjectDialog.open()
        onNewProject: {
            if (app.hasContent) { newProjectConfirmation.openingFile = ""; newProjectConfirmation.open() }
            else { window.replacingProject = true; window.editing = false }
        }
        onExportRequested: editor.showExport()
    }
    StackLayout {
        anchors { left: parent.left; right: parent.right; top: header.bottom; bottom: parent.bottom; leftMargin: window.gutter; rightMargin: window.gutter; topMargin: 18; bottomMargin: 18 }
        currentIndex: window.editing ? 1 : 0
        HomePage {
            canResume: window.visitedEditor
            onTemplateChosen: (presetId) => window.chooseTemplate(presetId)
            onResumeRequested: { window.replacingProject = false; window.showEditor() }
        }
        EditorPage { id: editor; objectName: "editorPage" }
    }
    FileDialog {
        id: openProjectDialog; title: qsTr("打开留素项目")
        fileMode: FileDialog.OpenFile; nameFilters: [qsTr("留素项目 (*.liusu)")]
        onAccepted: {
            if (app.hasContent) { newProjectConfirmation.openingFile = selectedFile; newProjectConfirmation.open() }
            else window.loadProject(selectedFile)
        }
    }
    FileDialog {
        id: saveProjectDialog; title: qsTr("保存留素项目")
        fileMode: FileDialog.SaveFile; defaultSuffix: "liusu"; nameFilters: [qsTr("留素项目 (*.liusu)")]
        onAccepted: app.saveProject(selectedFile)
    }
    Dialog {
        id: newProjectConfirmation
        objectName: "replaceProjectConfirmation"
        property url openingFile: ""
        title: openingFile.toString().length ? qsTr("打开另一个项目？") : qsTr("开始新项目？")
        anchors.centerIn: parent; modal: true; width: 390
        footer: DialogButtonBox {
            StudioButton { objectName: "replaceProjectCancel"; text: qsTr("取消"); DialogButtonBox.buttonRole: DialogButtonBox.RejectRole }
            StudioButton { text: qsTr("继续"); primary: true; DialogButtonBox.buttonRole: DialogButtonBox.AcceptRole }
        }
        contentItem: Text { text: qsTr("当前项目会被替换。需要保留时，请先取消并保存项目。"); wrapMode: Text.WordWrap; color: AppTheme.ink; font.family: AppTheme.fontFamily; font.pixelSize: 14 }
        onAccepted: {
            if (openingFile.toString().length) window.loadProject(openingFile)
            else { window.replacingProject = true; window.editing = false }
        }
    }
    Rectangle {
        id: toast
        visible: app.statusMessage.length > 0
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom; anchors.bottomMargin: 22
        width: Math.min(window.width-60, toastLabel.implicitWidth+42); height: toastLabel.implicitHeight+28
        color: "#262d27"; radius: 4; z: 100
        Text { id: toastLabel; anchors.centerIn: parent; width: Math.min(implicitWidth, window.width-102); text: app.statusMessage; wrapMode: Text.WordWrap; color: "#f4f5ef"; font.family: AppTheme.fontFamily; font.pixelSize: 12 }
        Timer { id: toastTimer; interval: 4800; onTriggered: app.clearStatus() }
        Connections { target: app; function onStatusMessageChanged() { toastTimer.restart() } }
    }
    Shortcut { sequences: [StandardKey.Save]; enabled: window.editing; onActivated: saveProjectDialog.open() }
    Shortcut { sequences: [StandardKey.Open]; onActivated: openProjectDialog.open() }
    Component.onCompleted: if (app.demoPreset.length > 0) showEditor()
}
