import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import LiuSu

Column {
    id: picker
    property string selectedId: "four"
    signal picked(string presetId)
    spacing: 8
    readonly property var selected: app.templateCatalog.find(t => t.id===picker.selectedId)
    StudioButton { objectName: "openTemplateLibrary"; width: parent.width; text: (picker.selected ? picker.selected.name : qsTr("自定义模板"))+qsTr(" · 更换"); iconName: "layout"; onClicked: library.open() }
    Popup {
        id: library
        objectName: "templateLibraryPopup"
        parent: Overlay.overlay
        x: (parent.width-width)/2; y: (parent.height-height)/2
        width: Math.min(660,parent.width-48); height: Math.min(520,parent.height-60)
        modal: true; focus: true; padding: 20
        onOpened: search.forceActiveFocus()
        enter: Transition { NumberAnimation { properties: "opacity,scale"; from: 0.9; to: 1; duration: 260; easing.type: Easing.OutCubic } }
        exit: Transition { NumberAnimation { property: "opacity"; from: 1; to: 0; duration: 150 } }
        background: Rectangle { color: "#f3f5ed"; radius: 6; border.color: "#bac7af" }
        contentItem: Item {
            Column {
                id: searchBar; width: parent.width; spacing: 10
                Row {
                    width: parent.width; spacing: 12
                    Text { text: qsTr("模板库"); font.family: AppTheme.fontFamily; font.pixelSize: 24; color: AppTheme.ink }
                    Text { text: app.templateCatalog.length+qsTr(" 个模板"); anchors.verticalCenter: parent.verticalCenter; color: AppTheme.ink2; font.pixelSize: 11 }
                }
                TextField { id: search; width: parent.width; placeholderText: qsTr("搜索名称、分类…"); Accessible.name: qsTr("搜索模板库") }
            }
            GridView {
                anchors { left: parent.left; right: parent.right; top: searchBar.bottom; bottom: importButton.top; topMargin: 18; bottomMargin: 10 }
                clip: true; cellWidth: width/3; cellHeight: 115
                model: app.templateCatalog.filter(t => !search.text.length || (t.name+" "+t.category+" "+t.caption).toLowerCase().indexOf(search.text.toLowerCase())>=0)
                ScrollBar.vertical: ScrollBar { policy: ScrollBar.AsNeeded }
        delegate: Button {
            id: choice
            required property var modelData
            width: GridView.view.cellWidth-10; height: 101
            padding: 0
            objectName: "picker-" + modelData.id
            Accessible.name: modelData.name + qsTr("模板")
            onClicked: { picker.picked(modelData.id); library.close() }
            background: Rectangle { radius: 3; color: choice.hovered ? "#e9eee3" : picker.selectedId===choice.modelData.id ? "#e8ece2" : "#f7f8f3"; border.width: 1; border.color: picker.selectedId===choice.modelData.id ? "#9eab97" : "#d7dccf" }
            contentItem: Item {
                TemplateMini { width: 72; height: 49; anchors.horizontalCenter: parent.horizontalCenter; y: 10; presetId: choice.modelData.id; photographs: true }
                Text { anchors.horizontalCenter: parent.horizontalCenter; y: 69; width: parent.width-12; horizontalAlignment: Text.AlignHCenter; elide: Text.ElideRight; text: choice.modelData.name; color: AppTheme.ink; font.family: AppTheme.fontFamily; font.pixelSize: 11 }
            }
        }
            }
            StudioButton { id: importButton; anchors.bottom: parent.bottom; text: qsTr("导入模板目录"); iconName: "add"; quiet: true; onClicked: templateFile.open() }
        }
    }
    FileDialog { id: templateFile; title: qsTr("导入模板目录"); fileMode: FileDialog.OpenFile; nameFilters: [qsTr("模板目录 (*.json)")]; onAccepted: app.importTemplateCatalog(selectedFile) }
}
