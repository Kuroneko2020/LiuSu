import QtQuick
import LiuSu

Item {
    id: home
    property bool canResume: false
    signal templateChosen(string presetId)
    signal resumeRequested()

    Column {
        x: 18; y: 22; spacing: 12
        SectionLabel { text: "PHOTOGRAPHS, WELL KEPT."; color: AppTheme.signalDeep }
        Text {
            text: qsTr("照片成页，片刻留素。")
            color: AppTheme.ink
            font.family: AppTheme.fontFamily; font.pixelSize: home.width < 1100 ? 36 : 44; font.weight: Font.Medium; font.letterSpacing: -1
        }
        Text { text: qsTr("选一个模板，把喜欢的照片排进相纸。每一页，都可以不一样。"); color: AppTheme.ink2; font.family: AppTheme.fontFamily; font.pixelSize: 13 }
    }
    Column {
        anchors.right: parent.right; anchors.rightMargin: 18
        y: 32; spacing: 8
        Text { text: "06"; color: AppTheme.ink; font.family: AppTheme.fontFamily; font.pixelSize: 42; font.weight: Font.Light; anchors.right: parent.right }
        SectionLabel { text: qsTr("英寸相纸 / 148 × 100 MM"); anchors.right: parent.right; font.letterSpacing: 0.8 }
        StudioButton { visible: home.canResume; text: qsTr("继续编辑"); iconName: "arrow"; quiet: true; anchors.right: parent.right; onClicked: home.resumeRequested() }
    }
    Row {
        id: boardRow
        x: 18; y: home.height < 560 ? 148 : Math.max(176, home.height * 0.26); spacing: 22
        Repeater {
            model: app.layoutPresets()
            delegate: LayoutBoard {
                required property var modelData
                required property int index
                objectName: "homeTemplate-" + modelData.id
                width: (home.width - 36 - 66) / 4
                height: Math.min(360, home.height - boardRow.y - 90)
                code: "0" + (index+1)
                name: modelData.name
                nameEn: ["SINGLE", "DIPTYCH", "CONTACT", "COLLECTION"][index]
                subtitle: modelData.slotCount + qsTr(" 张照片 / 页")
                presetId: modelData.id
                onClicked: home.templateChosen(presetId)
            }
        }
    }
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
