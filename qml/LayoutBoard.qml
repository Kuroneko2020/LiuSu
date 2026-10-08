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
    property real restingY: 0
    property real restingZ: 0
    implicitWidth: 260
    implicitHeight: 326
    hoverEnabled: true
    Accessible.name: name + qsTr("模板")
    ToolTip.visible: hovered && name.length>12
    ToolTip.text: name
    ToolTip.delay: 650
    padding: 0
    HoverHandler { id: pointer }
    background: Item {}
    contentItem: SpatialSurface {
        objectName: "cardSurface"
        active: board.hovered || board.visualFocus
        pressed: board.down
        pointerX: board.hovered ? Math.max(-1,Math.min(1,pointer.point.position.x/board.width*2-1)) : 0
        pointerY: board.hovered ? Math.max(-1,Math.min(1,pointer.point.position.y/board.height*2-1)) : 0
        restingY: board.restingY
        restingZ: board.restingZ
        Row {
            x: 18; y: 18; spacing: 9
            Text { text: board.code; color: AppTheme.ink; font.family: AppTheme.fontFamily; font.pixelSize: 11; font.weight: Font.DemiBold; font.letterSpacing: 1 }
            Text { text: board.nameEn; color: AppTheme.ink2; font.family: AppTheme.fontFamily; font.pixelSize: 9; font.letterSpacing: 1.2 }
        }
        Rectangle { x: parent.width-24; y: 22; width: 5; height: 5; color: board.hovered ? AppTheme.signal : "#bec6ba" }
        TemplateMini {
            x: (parent.width-width)/2; y: 51
            width: Math.min(parent.width-38,Math.max(30,parent.height-157)*app.pageWidthMm/app.pageHeightMm)
            height: width * app.pageHeightMm / app.pageWidthMm
            presetId: board.presetId; photographs: true
        }
        Text { objectName: "templateCardTitle"; x: 18; y: parent.height - 87; width: parent.width-36; elide: Text.ElideRight; text: board.name; color: AppTheme.ink; font.family: AppTheme.fontFamily; font.pixelSize: 20; font.weight: Font.Medium }
        Text { x: 18; y: parent.height - 57; text: board.subtitle; color: AppTheme.ink2; font.family: AppTheme.fontFamily; font.pixelSize: 11 }
        Rectangle { x: 18; y: parent.height - 31; width: parent.width - 36; height: 1; color: "#dce0d6" }
        Text { x: 18; y: parent.height-23; text: qsTr("选择模板"); color: AppTheme.ink; font.family: AppTheme.fontFamily; font.pixelSize: 10; font.weight: Font.Medium }
        LineIcon { x: parent.width - 34; y: parent.height - 25; width: 16; height: 16; kind: "arrow"; color: board.hovered ? AppTheme.signalDeep : AppTheme.ink2 }
    }
}
