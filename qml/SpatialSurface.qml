import QtQuick
import LiuSu

// 命中区域留在外层按钮；只变换展示实体，指针移动不会追着板面改变焦点。
// 阴影独立留在台面，静止时没有计时器、纹理更新或持续动画。
Item {
    id: surface
    default property alias contents: faceContent.data
    property bool active: false
    property bool pressed: false
    property real pointerX: 0
    property real pointerY: 0
    property real tilt: 22
    property real lift: 36
    property real restingY: 0
    property real restingZ: 0
    property color faceColor: "#f7f8f3"
    property color edgeColor: "#c6cec0"
    property real angleX: active ? -pointerY*tilt*0.65 : -2
    property real angleY: active ? pointerX*tilt : restingY
    property real angleZ: active ? 0 : restingZ
    property real altitude: pressed ? 3 : active ? lift : 0
    Behavior on angleX { SpringAnimation { spring: 3.4; damping: 0.32; epsilon: 0.05 } }
    Behavior on angleY { SpringAnimation { spring: 3.4; damping: 0.32; epsilon: 0.05 } }
    Behavior on angleZ { SpringAnimation { spring: 3.4; damping: 0.32; epsilon: 0.05 } }
    Behavior on altitude { SpringAnimation { spring: 3.8; damping: 0.34; epsilon: 0.05 } }
    GradientShadow {
        x: -surface.angleY*0.6; y: 8+surface.altitude*0.65
        width: surface.width; height: surface.height
        strength: 0.24; spread: 18; offsetY: 16; softness: 1
        opacity: 0.8-surface.altitude*0.008
        scale: 1+surface.altitude*0.0008
    }
    Item {
        id: face
        anchors.fill: parent
        transform: [
            Rotation { origin.x: face.width/2; origin.y: face.height/2; axis.x: 1; axis.y: 0; axis.z: 0; angle: surface.angleX },
            Rotation { origin.x: face.width/2; origin.y: face.height/2; axis.x: 0; axis.y: 1; axis.z: 0; angle: surface.angleY },
            Rotation { origin.x: face.width/2; origin.y: face.height/2; angle: surface.angleZ },
            Translate { y: -surface.altitude }
        ]
        Rectangle { x: 3-surface.angleY*0.16; y: 7; width: parent.width; height: parent.height; radius: 5; color: "#aebaaa"; border.color: "#899883" }
        Rectangle { y: 3; width: parent.width; height: parent.height; radius: 5; color: surface.edgeColor }
        Rectangle { anchors.fill: parent; radius: 5; color: surface.faceColor; border.color: surface.active ? "#889b7e" : "#ccd5c6" }
        Item { id: faceContent; anchors.fill: parent }
        Rectangle {
            anchors.fill: parent; radius: 5; opacity: 0.13
            gradient: Gradient {
                orientation: Gradient.Horizontal
                GradientStop { position: 0; color: "#ffffff" }
                GradientStop { position: Math.max(0.2,Math.min(0.8,0.5+surface.angleY/60)); color: "#00ffffff" }
                GradientStop { position: 1; color: "#47613b" }
            }
        }
        Rectangle { x: 1; y: 1; width: parent.width-2; height: 1; color: "#ffffff"; opacity: 0.8 }
    }
}
