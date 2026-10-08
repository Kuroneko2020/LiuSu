import QtQuick
import QtQuick.Controls
import LiuSu

Button {
    id: board
    property string code: ""
    property string name: ""
    property string nameEn: ""
    property string subtitle: ""
    property string presetId: ""
    implicitWidth: 260
    implicitHeight: 326
    hoverEnabled: true
    Accessible.name: name + qsTr("模板")
    padding: 0
    background: Item {
        GradientShadow { anchors.fill: parent; strength: board.hovered ? 0.13 : 0.08; spread: 10; offsetY: 12; cornerRadius: 2 }
        Rectangle {
            anchors.fill: parent; color: "#f7f8f3"; radius: 2; border.width: 1
            border.color: board.visualFocus ? AppTheme.signal : board.hovered ? "#a6b2a3" : "#d4d9cf"
        }
        Rectangle { x: 2; y: parent.height; width: parent.width - 4; height: 3; color: "#d0d5ca" }
    }
    transform: Translate {
        y: board.hovered ? -5 : 0
        Behavior on y { NumberAnimation { duration: 220; easing.type: Easing.OutCubic } }
    }
    contentItem: Item {
        Row {
            x: 18; y: 18; spacing: 9
            Text { text: board.code; color: AppTheme.ink; font.family: AppTheme.fontFamily; font.pixelSize: 11; font.weight: Font.DemiBold; font.letterSpacing: 1 }
            Text { text: board.nameEn; color: AppTheme.ink2; font.family: AppTheme.fontFamily; font.pixelSize: 9; font.letterSpacing: 1.2 }
        }
        Rectangle { x: parent.width-24; y: 22; width: 5; height: 5; color: board.hovered ? AppTheme.signal : "#bec6ba" }
        TemplateMini {
            x: (parent.width-width)/2; y: 49
            width: Math.min(parent.width-36,Math.max(40,parent.height-157)*app.pageWidthMm/app.pageHeightMm)
            height: width * app.pageHeightMm / app.pageWidthMm
            presetId: board.presetId; photographs: true
        }
        Text { objectName: "templateCardTitle"; x: 18; y: parent.height - 87; text: board.name; color: AppTheme.ink; font.family: AppTheme.fontFamily; font.pixelSize: 20; font.weight: Font.Medium }
        Text { x: 18; y: parent.height - 57; text: board.subtitle; color: AppTheme.ink2; font.family: AppTheme.fontFamily; font.pixelSize: 11 }
        Rectangle { x: 18; y: parent.height - 31; width: parent.width - 36; height: 1; color: "#dce0d6" }
        Text { x: 18; y: parent.height-23; text: qsTr("选择模板"); color: AppTheme.ink; font.family: AppTheme.fontFamily; font.pixelSize: 10; font.weight: Font.Medium }
        LineIcon { x: parent.width - 34; y: parent.height - 25; width: 16; height: 16; kind: "arrow"; color: board.hovered ? AppTheme.signalDeep : AppTheme.ink2 }
    }
}
