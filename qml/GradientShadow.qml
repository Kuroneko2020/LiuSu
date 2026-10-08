import QtQuick
import QtQuick.Effects

// 单层渐变阴影（全应用唯一的投影表达）。
//
// 为什么不用叠层矩形：旧写法把 3–4 层固定透明度的圆角矩形按不同偏移/外扩堆在
// 一起近似柔和过渡，每层边缘都是硬边，整体呈可辨的离散台阶。本组件改成一条
// 连续衰减的阴影体——实心圆角矩形经垂直渐变柔化，透明度沿下落方向连续过渡，
// 不再有分层断口。
//
// 用法：放在本体下方（z 取负），anchors.fill 本体即可；spread / offsetY 控制
// 影子的外扩与下落。仅是 UI 观感层，几何与导出真相仍在 domain / render。
Item {
    id: root

    // 阴影体圆角：调用方传本体圆角，组件据外扩量自行加圆。
    property real cornerRadius: 2
    // 阴影最深处的不透明度。
    property real strength: 0.16
    // 阴影体相对本体的外扩量。
    property real spread: 6
    // 垂直下落量（光源在上方，影子往下落）。
    property real offsetY: 8
    // 柔化程度 0..1：越大越柔。取 0 时只出渐变体，不做柔边。
    property real softness: 0.8
    // 影子色相（不带透明度，透明度由 strength 控制）。
    property color tint: Qt.rgba(28 / 255, 25 / 255, 18 / 255, 1.0)

    // 阴影体：比本体外扩并下落。渐变自上而下连续衰减，柔化只负责消掉
    // 边缘硬边——两者叠加得到平滑的渐变阴影。
    Rectangle {
        id: shadowShape
        // 只作柔化输入、不进场景：MultiEffect 另出一份效果结果，
        // 不隐藏就会"实心块 + 柔化块"叠印（本组件最易踩的坑）。
        visible: false
        x: -root.spread
        y: root.offsetY - root.spread
        width: root.width + root.spread * 2
        height: root.height + root.spread * 2
        radius: root.cornerRadius + root.spread
        gradient: shadowGradient
    }

    Gradient {
        id: shadowGradient
        GradientStop {
            position: 0.0
            color: Qt.rgba(root.tint.r, root.tint.g, root.tint.b, root.strength)
        }
        GradientStop {
            position: 0.55
            color: Qt.rgba(root.tint.r, root.tint.g, root.tint.b, root.strength * 0.42)
        }
        GradientStop {
            position: 1.0
            color: Qt.rgba(root.tint.r, root.tint.g, root.tint.b, 0)
        }
    }

    // 柔化通道：源是静态形状，纹理只需生成一次，不逐帧重算（性能底线）。
    // softness 取 0 时 blur 为 0，等价于直接出渐变体，退化路径不另建分支。
    MultiEffect {
        anchors.fill: shadowShape
        source: shadowShape
        autoPaddingEnabled: true
        blurEnabled: true
        blur: root.softness
        blurMax: 24
    }
}
