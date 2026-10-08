import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import LiuSu

Item {
    id: editor
    function showExport() { inspector.showExport() }
    function addPages() { addDialog.presetId = app.templateCatalog.some(t => t.id===app.currentLayoutId) ? app.currentLayoutId : app.templateCatalog[0].id; pageCountInput.value = 1; addDialog.open() }
    function replacePhoto(slotIndex) { replaceDialog.slotIndex = slotIndex; replaceDialog.open() }

    PageCanvas {
        id: canvas
        objectName: "pageCanvas"
        anchors { left: parent.left; right: inspector.left; top: parent.top; bottom: queue.top; rightMargin: 28; bottomMargin: 17 }
        onImportRequested: importDialog.open()
        onAddPagesRequested: editor.addPages()
        onSlotOpenRequested: (slotIndex) => editor.replacePhoto(slotIndex)
        onFilesDropped: (urls) => app.importPhotos(urls)
    }
    EditorInspector {
        id: inspector
        objectName: "editorInspector"
        width: editor.width > 1160 ? 276 : 252
        anchors { top: parent.top; right: parent.right; bottom: queue.top; bottomMargin: 17 }
        onReplaceRequested: editor.replacePhoto(app.selectedSlot)
        onExportPageRequested: exportPageDialog.open()
        onExportAllRequested: exportAllDialog.open()
        onBackgroundRequested: backgroundDialog.open()
    }
    PageQueue {
        id: queue
        anchors { left: parent.left; right: parent.right; bottom: parent.bottom }
        height: 122
        onAddPagesRequested: editor.addPages()
        onDeleteRequested: deleteDialog.open()
    }

    FileDialog {
        id: importDialog
        title: qsTr("导入照片 · 从当前页起依次填入空位")
        fileMode: FileDialog.OpenFiles
        nameFilters: [qsTr("照片 (*.jpg *.jpeg *.png *.webp *.bmp)")]
        onAccepted: app.importPhotos(selectedFiles)
    }
    FileDialog {
        id: replaceDialog
        property int slotIndex: -1
        title: qsTr("选择照片"); fileMode: FileDialog.OpenFile
        nameFilters: [qsTr("照片 (*.jpg *.jpeg *.png *.webp *.bmp)")]
        onAccepted: app.assignFileToSlot(slotIndex,selectedFile)
    }
    FileDialog {
        id: exportPageDialog
        title: qsTr("导出当前页"); fileMode: FileDialog.SaveFile
        defaultSuffix: app.exportJpeg ? "jpg" : "png"
        nameFilters: app.exportJpeg ? [qsTr("JPEG (*.jpg)")] : [qsTr("PNG (*.png)")]
        onAccepted: app.exportCurrentPage(selectedFile,app.exportPpi,app.exportJpeg,app.exportQuality)
    }
    FolderDialog {
        id: exportAllDialog; title: qsTr("选择导出目录")
        onAccepted: app.exportAllPages(selectedFolder,app.exportPpi,app.exportJpeg,app.exportQuality)
    }
    ColorDialog {
        id: backgroundDialog; title: qsTr("项目纸面背景")
        selectedColor: app.backgroundHex
        onAccepted: app.setBackground(selectedColor.toString())
    }
    Dialog {
        id: addDialog
        objectName: "addPagesDialog"
        property string presetId: "four"
        anchors.centerIn: parent
        width: 380; modal: true
        title: qsTr("添加空白页面")
        footer: DialogButtonBox {
            StudioButton { text: qsTr("取消"); DialogButtonBox.buttonRole: DialogButtonBox.RejectRole }
            StudioButton { objectName: "addPagesAccept"; text: qsTr("添加页面"); primary: true; DialogButtonBox.buttonRole: DialogButtonBox.AcceptRole }
        }
        contentItem: Column {
            spacing: 16
            Text { text: qsTr("选择初始模板"); color: AppTheme.ink; font.family: AppTheme.fontFamily; font.pixelSize: 13 }
            TemplatePicker { width: parent.width; selectedId: addDialog.presetId; onPicked: (presetId) => addDialog.presetId=presetId }
            Row {
                spacing: 18
                Text { text: qsTr("页面数量"); color: AppTheme.ink; font.family: AppTheme.fontFamily; font.pixelSize: 13; anchors.verticalCenter: parent.verticalCenter }
                SpinBox { id: pageCountInput; objectName: "pageCountInput"; from: 1; to: 100; value: 1; editable: true; Accessible.name: qsTr("添加页数") }
            }
            Text { width: parent.width; text: qsTr("可以一次添加 10 页，之后逐页更换模板。"); wrapMode: Text.WordWrap; color: AppTheme.ink2; font.family: AppTheme.fontFamily; font.pixelSize: 12 }
        }
        onAccepted: app.addPages(presetId,pageCountInput.value)
    }
    Dialog {
        id: deleteDialog
        anchors.centerIn: parent; width: 360; modal: true
        title: qsTr("删除第 %1 页？").arg(app.currentPageIndex+1)
        footer: DialogButtonBox {
            StudioButton { text: qsTr("取消"); DialogButtonBox.buttonRole: DialogButtonBox.RejectRole }
            StudioButton { text: qsTr("删除页面"); destructive: true; DialogButtonBox.buttonRole: DialogButtonBox.AcceptRole }
        }
        contentItem: Text { text: qsTr("这一页及其照片编辑状态将从项目中移除。原始照片文件保留。"); wrapMode: Text.WordWrap; color: AppTheme.ink2; font.family: AppTheme.fontFamily; font.pixelSize: 13 }
        onAccepted: app.deleteCurrentPage()
    }
}
