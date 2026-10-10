import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import LiuSu

Item {
    id: home
    property bool canResume: false
    signal templateChosen(string presetId)
    signal resumeRequested()
    property string search: ""
    readonly property var filteredTemplates: app.templateCatalog.filter(t => !search.length || (t.name+" "+t.caption+" "+t.category).toLowerCase().indexOf(search.toLowerCase())>=0)

    Column {
        x: 18; y: 22; spacing: 12
        SectionLabel { text: "TEMPLATE LIBRARY / 模板库"; color: AppTheme.signalDeep }
        Text {
            text: qsTr("选择拼版模板")
            color: AppTheme.ink
            font.family: AppTheme.fontFamily; font.pixelSize: 28; font.weight: Font.Medium
        }
        Text { text: qsTr("选一个模板，把喜欢的照片排进相纸。每一页，都可以不一样。"); color: AppTheme.ink2; font.family: AppTheme.fontFamily; font.pixelSize: 13 }
    }
    Column {
        anchors.right: parent.right; anchors.rightMargin: 18
        y: 32; spacing: 8
        Text { text: qsTr("6 英寸相纸"); color: AppTheme.ink; font.family: AppTheme.fontFamily; font.pixelSize: 18; anchors.right: parent.right }
        SectionLabel { text: app.pageWidthMm + " × " + app.pageHeightMm + " MM"; anchors.right: parent.right; font.letterSpacing: 0.8 }
        StudioButton { visible: home.canResume; text: qsTr("继续编辑"); iconName: "arrow"; quiet: true; anchors.right: parent.right; onClicked: home.resumeRequested() }
    }
    Row {
        id: libraryTools
        x: 18; y: 135; spacing: 12
        TextField { objectName: "templateSearch"; width: 240; height: 34; placeholderText: qsTr("搜索模板、分类…"); onTextChanged: home.search=text; Accessible.name: qsTr("搜索模板") }
        StudioButton { height: 34; text: qsTr("导入模板目录"); iconName: "add"; quiet: true; onClicked: templateFile.open() }
        SectionLabel { anchors.verticalCenter: parent.verticalCenter; text: home.filteredTemplates.length+qsTr(" 个模板"); color: AppTheme.ink2 }
    }
    ListView {
        id: boardRow
        objectName: "templateLibrary"
        x: 0; y: libraryTools.y+48; width: home.width; height: home.height-y-62
        orientation: ListView.Horizontal; spacing: 26; clip: true
        leftMargin: 28; rightMargin: 28
        model: home.filteredTemplates
        ScrollBar.horizontal: ScrollBar { policy: ScrollBar.AsNeeded }
        delegate: Item {
            required property var modelData
            required property int index
            width: home.width < 1100 ? 218 : 280; height: boardRow.height
            LayoutBoard {
                required property var modelData
                modelData: parent.modelData
                objectName: "homeTemplate-" + modelData.id
                width: parent.width; y: 18; height: Math.min(375,boardRow.height-46)
                code: (parent.index+1).toString().padStart(2,"0")
                name: modelData.name
                nameEn: modelData.caption
                subtitle: modelData.slotCount + qsTr(" 张照片 / 页")
                presetId: modelData.id
                onClicked: home.templateChosen(presetId)
            }
        }
    }
    Text { visible: home.filteredTemplates.length===0; anchors.centerIn: boardRow; text: qsTr("没有匹配的模板"); color: AppTheme.ink2; font.family: AppTheme.fontFamily; font.pixelSize: 16 }
    FileDialog { id: templateFile; title: qsTr("导入模板目录"); fileMode: FileDialog.OpenFile; nameFilters: [qsTr("模板目录 (*.json)")]; onAccepted: app.importTemplateCatalog(selectedFile) }
    Rectangle { x: 18; anchors.bottom: footer.top; anchors.bottomMargin: 20; width: parent.width - 36; height: 1; color: AppTheme.line }
    Row {
        id: footer
        x: 18; anchors.bottom: parent.bottom; anchors.bottomMargin: 18; spacing: 28
        Repeater {
            model: [qsTr("01  选择模板"), qsTr("02  导入与排版"), qsTr("03  导出留存")]
            delegate: Text { required property string modelData; text: modelData; color: AppTheme.ink2; font.family: AppTheme.fontFamily; font.pixelSize: 11 }
        }
    }
    SectionLabel { anchors.right: parent.right; anchors.rightMargin: 18; anchors.verticalCenter: footer.verticalCenter; text: "A LITTLE SPACE FOR YOUR MEMORIES."; font.pixelSize: 8 }
}
