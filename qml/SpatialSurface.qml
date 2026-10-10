import QtQuick
import LiuSu

// 材质反馈不变换内容或祖先容器；照片比较与取景需要视觉和命中坐标同时稳定。
// 反光只在内容下方，不给照片蒙雾、染色。静止时没有持续动画或纹理更新。
Item {
    id: surface
    default property alias contents: faceContent.data
    property bool active: false
    property bool pressed: false
    property color faceColor: "#f7f8f3"
    property color edgeColor: "#c6cec0"
    property real light: pressed ? 0.25 : active ? 1 : 0
    Behavior on light { NumberAnimation { duration: 140; easing.type: Easing.OutCubic } }
    GradientShadow {
        anchors.fill: parent
        strength: 0.12; spread: 7; offsetY: 7; cornerRadius: 6
        opacity: 0.8+surface.light*0.12
    }
    // 薄下缘与内侧高光在静止时也能说明厚度，不叠多层挤出实体。
    Rectangle {
        x: 0; y: 2; width: parent.width; height: parent.height; radius: 6
        color: "transparent"; border.color: Qt.rgba(surface.edgeColor.r,surface.edgeColor.g,surface.edgeColor.b,0.65)
    }
    Rectangle {
        anchors.fill: parent; radius: 6
        gradient: Gradient {
            GradientStop { position: 0; color: Qt.rgba(surface.faceColor.r,surface.faceColor.g,surface.faceColor.b,0.84) }
            GradientStop { position: 0.46; color: Qt.rgba(surface.faceColor.r,surface.faceColor.g,surface.faceColor.b,0.42) }
            GradientStop { position: 1; color: Qt.rgba(surface.faceColor.r,surface.faceColor.g,surface.faceColor.b,0.68) }
        }
        border.color: surface.pressed ? "#9aa48f" : Qt.rgba(surface.edgeColor.r,surface.edgeColor.g,surface.edgeColor.b,0.7+surface.light*0.3)
    }
    Rectangle {
        anchors.fill: parent; anchors.margins: 1; radius: 5
        gradient: Gradient {
            orientation: Gradient.Horizontal
            GradientStop { position: 0; color: Qt.rgba(1,1,1,0.22+surface.light*0.09) }
            GradientStop { position: 0.3; color: "#00ffffff" }
            GradientStop { position: 0.8; color: "#00ffffff" }
            GradientStop { position: 1; color: "#14ffffff" }
        }
        border.color: Qt.rgba(1,1,1,0.48+surface.light*0.18)
    }
    Item { id: faceContent; anchors.fill: parent }
}
