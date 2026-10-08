import QtQuick
import QtQuick.Controls
import LiuSu

Grid {
    id: picker
    property string selectedId: "four"
    signal picked(string presetId)
    columns: 2; spacing: 8
    Repeater {
        model: app.layoutPresets()
        delegate: Button {
            id: choice
            required property var modelData
            width: (picker.width-8)/2; height: 78
            padding: 0
            objectName: "picker-" + modelData.id
            Accessible.name: modelData.name + qsTr("模板")
            onClicked: picker.picked(modelData.id)
            background: Rectangle { radius: 3; color: choice.hovered ? "#e9eee3" : picker.selectedId===choice.modelData.id ? "#e8ece2" : "#f7f8f3"; border.width: 1; border.color: picker.selectedId===choice.modelData.id ? "#9eab97" : "#d7dccf" }
            contentItem: Item {
                TemplateMini { width: 49; height: 33; anchors.horizontalCenter: parent.horizontalCenter; y: 10; presetId: choice.modelData.id }
                Text { anchors.horizontalCenter: parent.horizontalCenter; y: 51; text: choice.modelData.name; color: AppTheme.ink; font.family: AppTheme.fontFamily; font.pixelSize: 11 }
            }
        }
    }
}
