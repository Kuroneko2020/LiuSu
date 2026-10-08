import QtQuick
import LiuSu

Item {
    id: header
    property bool editing: false
    signal chooseTemplate()
    signal openProject()
    signal saveProject()
    signal newProject()
    signal exportRequested()
    implicitHeight: 58
    Row {
        anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter
        spacing: 11
        Rectangle {
            width: 31; height: 31; color: AppTheme.ink; radius: 1
            LineIcon { anchors.centerIn: parent; width: 23; height: 23; kind: "mark"; color: "#f4f5ef" }
            Rectangle { anchors.right: parent.right; anchors.bottom: parent.bottom; width: 6; height: 6; color: AppTheme.signal }
        }
        Text { text: qsTr("留素"); color: AppTheme.ink; font.family: AppTheme.fontFamily; font.pixelSize: 21; font.weight: Font.DemiBold; font.letterSpacing: 2; anchors.verticalCenter: parent.verticalCenter }
        SectionLabel { text: "LIUSU"; anchors.verticalCenter: parent.verticalCenter }
        Rectangle { width: 1; height: 20; color: AppTheme.line; anchors.verticalCenter: parent.verticalCenter }
        StudioButton { objectName: "templateNavigation"; text: header.editing ? qsTr("模板首页") : qsTr("照片拼版"); iconName: header.editing ? "back" : ""; quiet: true; onClicked: header.chooseTemplate() }
    }
    Row {
        anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter
        spacing: 8
        StudioButton { text: qsTr("新项目"); quiet: true; visible: header.editing; onClicked: header.newProject() }
        StudioButton { text: qsTr("打开项目"); iconName: "open"; quiet: true; onClicked: header.openProject() }
        StudioButton { text: qsTr("保存"); iconName: "save"; quiet: true; visible: header.editing; onClicked: header.saveProject() }
        StudioButton { text: qsTr("导出照片"); iconName: "download"; primary: true; visible: header.editing; onClicked: header.exportRequested() }
    }
    Rectangle { anchors.bottom: parent.bottom; width: parent.width; height: 1; color: AppTheme.line }
}
